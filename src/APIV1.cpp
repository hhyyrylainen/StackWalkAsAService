// ------------------------------------ //
#include "APIV1.h"

#include "StackWalkOperation.h"
#include "StackWalkRunner.h"

#include <Wt/Http/Request.h>
#include <Wt/Http/Response.h>
#include <Wt/WLogger.h>

#include <thread>

using namespace sws;
// ------------------------------------ //
APIV1::APIV1(const std::shared_ptr<StackWalkRunner>& runner) : Runner(runner) {}
// ------------------------------------ //
void APIV1::handleRequest(const Wt::Http::Request& request, Wt::Http::Response& response)
{
    response.setMimeType("text/plain");

    std::string result;

    if(request.uploadedFiles().size() != 1) {

        response.setStatus(400);
        result = "Expected one form data uploaded file. Got: " +
                 std::to_string(request.uploadedFiles().size());
    } else {

        auto& file = request.uploadedFiles().begin()->second;

        Wt::log("info") << "API stackwalk on file: " << file.spoolFileName();

        try {
            auto op = std::make_shared<StackWalkOperation>(file.spoolFileName());

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

            if(!op->HasSucceeded()){
                response.setStatus(500);                
            }
            
        } catch(const std::exception& e) {
            response.setStatus(500);
            result = "Internal server error: " + std::string(e.what());
        }
    }

    response.setContentLength(result.length());
    response.out() << result;
}
