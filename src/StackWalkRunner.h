#pragma once

#include <boost/asio.hpp>

#include <atomic>
#include <condition_variable>
#include <deque>
#include <string_view>
#include <thread>

namespace sws {

enum class StackWalkType;

class StackWalkOperation;

class StackWalkRunner {
public:
    //! \param executablePath The path to the current executable (argv[0])
    //! \exception std::runtime_error if cannot work properly (missing stackwalk executable)
    StackWalkRunner(const std::string_view& executablePath, int threadCount = 1);
    ~StackWalkRunner();

    StackWalkRunner(const StackWalkRunner& other) = delete;
    StackWalkRunner(StackWalkRunner&& other) = delete;
    StackWalkRunner& operator=(StackWalkRunner&& other) = delete;
    StackWalkRunner& operator=(const StackWalkRunner& other) = delete;

    void QueueOperation(const std::shared_ptr<StackWalkOperation>& operation);

    //! \todo This doesn't currently abort the running process if this is currently running
    void CancelOperation(StackWalkOperation const* operation);

    //! \brief Immediately runs stackwalk with the '-h' flag and returns output
    std::string RunStackWalkHelp(StackWalkType type) const;

    void Start();
    void Stop();

    const std::string& ExecutableFromWalkType(StackWalkType type) const;

    const auto& GetSymbolsFolder() const
    {
        return SymbolsFolder;
    }

private:
    void RunStackWalkThread();


private:
    std::string StackWalkExecutable;
    std::string MinGWStackWalkExecutable;
    std::string SymbolsFolder;

    std::atomic<bool> Run{false};

    std::mutex Mutex;
    std::condition_variable BackgroundThreadNotify;

    std::deque<std::shared_ptr<StackWalkOperation>> Queue;

    std::thread BackgroundThread;
    boost::asio::io_service ChildProcessIoService;
};

} // namespace sws
