// ------------------------------------ //
#include "StackWalkRunner.h"

#include "StackWalkOperation.h"

#include <Wt/WLogger.h>

#include <boost/process.hpp>

#include <chrono>
#include <filesystem>
#include <iostream>

namespace bp = boost::process;
using namespace sws;
// ------------------------------------ //
#ifdef _WIN32
#define STACKWALK_EXECUTABLE_NAME "minidump_stackwalk.exe"
#else
#define STACKWALK_EXECUTABLE_NAME "minidump_stackwalk"
#endif //_WIN32

constexpr auto EXPECTED_STACKWALK_HELP_CONTAINS = "a stack trace for the provided";

constexpr auto SYSTEMWIDE_INSTALL1 = "/usr/bin/" STACKWALK_EXECUTABLE_NAME;
constexpr auto SYSTEMWIDE_INSTALL2 = "/usr/local/bin/" STACKWALK_EXECUTABLE_NAME;

// TODO: allow configuring from the command line or config file
constexpr auto STANDARD_SYMBOL_LOCATIONS = {"/Symbols", "/symbols"};


constexpr auto STACKWALK_SUCCESSFUL_OUTPUT_CONTAINS = "Crash reason";
// ------------------------------------ //
StackWalkRunner::StackWalkRunner(
    const std::string_view& executablePath, int threadCount /*= 1*/)
{
    if(threadCount != 1) {
        throw std::runtime_error("other threadcount than 1 is not implemented");
    }

    // Try to find where the stackwalk executable is

    if(const auto exeRelative =
            std::filesystem::path(executablePath).parent_path() / STACKWALK_EXECUTABLE_NAME;
        std::filesystem::exists(exeRelative)) {

        StackWalkExecutable = exeRelative.string();

    } else if(const auto workDirRelative =
                  std::filesystem::current_path() / STACKWALK_EXECUTABLE_NAME;
              std::filesystem::exists(workDirRelative)) {

        StackWalkExecutable = workDirRelative.string();

    } else if(std::filesystem::exists(SYSTEMWIDE_INSTALL1)) {

        StackWalkExecutable = SYSTEMWIDE_INSTALL1;

    } else if(std::filesystem::exists(SYSTEMWIDE_INSTALL2)) {

        StackWalkExecutable = SYSTEMWIDE_INSTALL2;
    }

    // Check that the detected path is valid
    if(StackWalkExecutable.empty() || !std::filesystem::exists(StackWalkExecutable)) {
        throw std::runtime_error("could not find '" STACKWALK_EXECUTABLE_NAME
                                 "' in any of the predefined locations");
    }

    StackWalkExecutable = std::filesystem::canonical(StackWalkExecutable).string();

    // Check that the executable runs and returns sensible output
    if(const auto output = RunStackWalkHelp();
        output.find(EXPECTED_STACKWALK_HELP_CONTAINS) == std::string::npos) {

        std::cout << "Stackwalk output: " << output << "\n";

        throw std::runtime_error("running stackwalk with '-h' flag printed unexpected output");
    }

    // Detect symbols folder
    for(const auto& location : STANDARD_SYMBOL_LOCATIONS) {
        if(std::filesystem::exists(location)) {
            SymbolsFolder = location;
            break;
        }
    }

    if(SymbolsFolder.empty()) {
        SymbolsFolder = std::filesystem::canonical(
            std::filesystem::path(executablePath).parent_path() / "Symbols")
                            .string();
        try {
            std::filesystem::create_directories(SymbolsFolder);
        } catch(const boost::filesystem::filesystem_error& e) {
            Wt::log("error") << "Failed to make sure symbols folder (" << SymbolsFolder
                             << ") exists: " << e.what();
        }
    }

    Wt::log("info") << "Detected stackwalk path: " << StackWalkExecutable
                    << ", symbols path: " << SymbolsFolder;

    Start();
}

StackWalkRunner::~StackWalkRunner()
{
    Stop();

    // TODO: set failure status on any tasks in Queue
}
// ------------------------------------ //
std::string StackWalkRunner::RunStackWalkHelp() const
{
    boost::asio::io_service ios;
    std::future<std::string> output;

    bp::child c(StackWalkExecutable, "-h", bp::std_out > output, ios);

    ios.run();
    // Apparently the -h flag also returns a failure code
    /*int result =*/c.exit_code();

    // if(result != 0) {
    //     std::cout << "Stackwalk exit code: " << result << "\n";
    //     std::cout << "Output: " << output.get() << "\n";
    //     throw std::runtime_error("Stackwalk exited with non-zero exit code");
    // }

    return output.get();
}
// ------------------------------------ //
void StackWalkRunner::Start()
{
    bool expected = false;

    if(!Run.compare_exchange_strong(expected, true, std::memory_order::memory_order_acq_rel,
           std::memory_order::memory_order_relaxed))
        return;

    std::unique_lock<std::mutex> lock(Mutex);

    BackgroundThread = std::thread(std::bind(&StackWalkRunner::RunStackWalkThread, this));
}

void StackWalkRunner::Stop()
{
    ChildProcessIoService.stop();

    {
        std::unique_lock<std::mutex> lock(Mutex);
        Run = false;
        BackgroundThreadNotify.notify_all();
    }

    if(BackgroundThread.joinable())
        BackgroundThread.join();
}
// ------------------------------------ //
void StackWalkRunner::QueueOperation(const std::shared_ptr<StackWalkOperation>& operation)
{
    if(!operation)
        return;

    std::unique_lock<std::mutex> lock(Mutex);

    Queue.push_back(operation);

    BackgroundThreadNotify.notify_one();
}

void StackWalkRunner::CancelOperation(StackWalkOperation const* operation)
{
    std::unique_lock<std::mutex> lock(Mutex);

    for(auto iter = Queue.begin(); iter != Queue.end(); ++iter) {
        if(iter->get() == operation) {

            Queue.erase(iter);
            return;
        }
    }
}
// ------------------------------------ //
void StackWalkRunner::RunStackWalkThread()
{
    std::unique_lock<std::mutex> lock(Mutex);

    while(Run) {

        // Nothing to do
        if(Queue.empty()) {
            BackgroundThreadNotify.wait(lock);
            continue;
        }

        std::shared_ptr<StackWalkOperation> operation = Queue.front();

        Queue.pop_front();

        std::future<std::string> output;
        std::future<std::string> error;

        ChildProcessIoService.reset();

        lock.unlock();

        operation->MarkStarted();

        Wt::log("info") << "Beginning stack walk on file: " << operation->GetFilePath();

        const auto start = std::chrono::high_resolution_clock::now();

        bp::child c(StackWalkExecutable, operation->GetFilePath(), SymbolsFolder,
            bp::std_out > output, bp::std_err > error, ChildProcessIoService);

        ChildProcessIoService.run();
        lock.lock();

        if(!Run) {
            // This was interrupted
            operation->OnStackWalkFinished(false, "interrupted");
            break;
        }

        const auto end = std::chrono::high_resolution_clock::now();
        const std::chrono::duration<float> duration = end - start;

        const int result = c.exit_code();
        auto outputStr = output.get();
        const auto errorStr = error.get();

        // Seems like this always fails even when providing some output
        // the error output also always seems to have non-fatal errors in it
        // if(result != 0) {
        // }

        bool success =
            outputStr.find(STACKWALK_SUCCESSFUL_OUTPUT_CONTAINS) != std::string::npos;

        Wt::log("info") << "Stackwalk finished, success: " << success
                        << " elapsed: " << duration.count() << "s";

        if(!success)
            outputStr += "Error output: " + errorStr;

        operation->OnStackWalkFinished(success, outputStr);
    }
}
