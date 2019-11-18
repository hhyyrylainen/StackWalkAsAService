// ------------------------------------ //
#include "StackWalkOperation.h"

#include <Wt/WLogger.h>

#include <boost/filesystem.hpp>

using namespace sws;
// ------------------------------------ //
StackWalkOperation::StackWalkOperation(const std::string& file) : FilePath(file)
{
    if(!boost::filesystem::exists(file))
        throw std::runtime_error("file provided to StackWalkOperation does not exist");
}

StackWalkOperation::~StackWalkOperation()
{
    std::unique_lock<std::mutex> lock(Mutex);
    EnsureFileIsDeleted();
}
// ------------------------------------ //
void StackWalkOperation::OnStackWalkFinished(bool success, const std::string& result)
{
    {
        std::unique_lock<std::mutex> lock(Mutex);

        Finished = true;
        Success = success;
        Result = result;

        EnsureFileIsDeleted();
    }

    if(OnFinishedCallback)
        OnFinishedCallback(*this);
}

// ------------------------------------ //
void StackWalkOperation::EnsureFileIsDeleted()
{
    if(!FileDeleted) {
        try {
            boost::filesystem::remove(FilePath);
        } catch(const boost::filesystem::filesystem_error& e) {
            Wt::log("error") << "Failed to delete crash dump file (" << FilePath
                             << "): " << e.what();
        }

        FileDeleted = true;
    }
}
