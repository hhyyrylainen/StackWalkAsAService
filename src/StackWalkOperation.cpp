// ------------------------------------ //
#include "StackWalkOperation.h"

#include <Wt/WLogger.h>

#include <boost/filesystem.hpp>

using namespace sws;
// ------------------------------------ //
StackWalkOperation::StackWalkOperation(const std::string& file, StackWalkType type) :
    WalkType(type), FilePath(file)
{
    if(!boost::filesystem::exists(file))
        throw std::runtime_error("file provided to StackWalkOperation does not exist");
}

StackWalkOperation::~StackWalkOperation()
{
    std::unique_lock<std::recursive_mutex> lock(Mutex);
    EnsureFileIsDeleted(lock);
}
// ------------------------------------ //
void StackWalkOperation::OnStackWalkFinished(bool success, const std::string& result)
{
    std::unique_lock<std::recursive_mutex> lock(Mutex);

    Finished = true;
    Success = success;
    Result = result;

    EnsureFileIsDeleted(lock);

    if(OnFinishedCallback)
        OnFinishedCallback(*this);
}

// ------------------------------------ //
void StackWalkOperation::EnsureFileIsDeleted(std::unique_lock<std::recursive_mutex>& guard)
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
