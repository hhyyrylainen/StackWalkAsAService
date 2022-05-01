// ------------------------------------ //
#include "APIV1.h"

#include "StackWalkOperation.h"
#include "StackWalkRunner.h"

#include <Wt/Http/Request.h>
#include <Wt/Http/Response.h>
#include <Wt/WLogger.h>

#include <thread>
#include <utility>

using namespace sws;
// ------------------------------------ //
APIV1::APIV1(std::shared_ptr<StackWalkRunner> runner) : Runner(std::move(runner)) {}
// ------------------------------------ //
void APIV1::handleRequest(const Wt::Http::Request& request, Wt::Http::Response& response)
{
    response.setMimeType("text/plain");

    std::string result;

    auto* requestedWalker = request.getParameter("stackwalkType");

    if(!requestedWalker)
        requestedWalker = request.getParameter("walkType");

    if(request.uploadedFiles().size() != 1) {

        response.setStatus(400);
        result = "Expected one form data uploaded file. Got: " +
                 std::to_string(request.uploadedFiles().size());
    } else {

        StackWalkType walkType = StackWalkType::Normal;

        if(requestedWalker) {

            if(*requestedWalker == "normal") {
                walkType = StackWalkType::Normal;
            } else if(*requestedWalker == "mingw" || *requestedWalker == "MinGW") {
                walkType = StackWalkType::MinGW;
            } else {
                response.setStatus(400);
                result = "Unknown stackwalk type";
                response.setContentLength(result.length());
                response.out() << result;
                return;
            }

            Wt::log("info") << "Requested walk type is: " << *requestedWalker;
        }

        auto& file = request.uploadedFiles().begin()->second;

        Wt::log("info") << "API stackwalk on file: " << file.spoolFileName();

        try {
            auto op = std::make_shared<StackWalkOperation>(file.spoolFileName(), walkType);

            // We will delete the file ourselves
            file.stealSpoolFile();

            // Queue the operation
            Runner->QueueOperation(op);

            // We can't actually return here, instead we must kind of busy wait for the
            // operation to finish
            while(!op->HasFinished()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }

            result = op->GetResult();

            if(!op->HasSucceeded()) {
                response.setStatus(500);
            } else {
                response.setStatus(200);
            }

        } catch(const std::exception& e) {
            response.setStatus(500);
            result = "Internal server error: " + std::string(e.what());
        }
    }

    response.setContentLength(result.length());
    response.out() << result;
}
