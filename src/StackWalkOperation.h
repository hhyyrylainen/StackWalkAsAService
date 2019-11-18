#pragma once

#include <atomic>
#include <functional>
#include <mutex>

namespace sws {

class StackWalkRunner;

//! \brief
class StackWalkOperation {
    friend StackWalkRunner;

public:
    //! \exceptions std::runtime_error if file doesn't exist
    StackWalkOperation(const std::string& file);
    ~StackWalkOperation();

    StackWalkOperation(const StackWalkOperation& other) = delete;
    StackWalkOperation(StackWalkOperation&& other) = delete;
    StackWalkOperation& operator=(StackWalkOperation&& other) = delete;
    StackWalkOperation& operator=(const StackWalkOperation& other) = delete;

    //! \note Changing the callbacks after queueing this operation is not safe
    void SetOnStartedCallback(std::function<void(StackWalkOperation& operation)> callback)
    {
        OnStartedCallback = callback;
    }

    //! \copydoc SetOnStartedCallback
    void SetOnFinishedCallback(std::function<void(StackWalkOperation& operation)> callback)
    {
        OnFinishedCallback = callback;
    }

    bool HasStartedProcessing() const
    {
        return Started;
    }

    bool HasFinished() const
    {
        return Finished;
    }

    bool HasSucceeded() const
    {
        return Success;
    }

    const std::string GetResult() const
    {
        std::unique_lock<std::mutex> lock(Mutex);
        if(!Finished)
            return "pending";

        return Result;
    }

    const std::string& GetFilePath() const
    {
        return FilePath;
    }

protected:
    void OnStackWalkFinished(bool success, const std::string& result);

    void MarkStarted()
    {
        Started = true;

        if(OnStartedCallback)
            OnStartedCallback(*this);
    }

private:
    //! \note Mutex must be locked before this
    void EnsureFileIsDeleted();

private:
    //! For thread safe result updating and file deleting
    mutable std::mutex Mutex;

    std::atomic<bool> Started{false};
    std::atomic<bool> Finished{false};
    std::atomic<bool> Success{false};

    std::string Result;

    const std::string FilePath;
    bool FileDeleted = false;

    // Callbacks
    std::function<void(StackWalkOperation& operation)> OnStartedCallback;
    std::function<void(StackWalkOperation& operation)> OnFinishedCallback;
};

} // namespace sws
