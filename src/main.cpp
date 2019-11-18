#include "Application.h"
#include "StackWalkRunner.h"

#include <iostream>

int main(int argc, char** argv)
{
    try {
        const auto runner = std::make_shared<sws::StackWalkRunner>(argc > 0 ? argv[0] : "");

        return Wt::WRun(argc, argv, [runner](const Wt::WEnvironment& env) {
            return std::make_unique<sws::Application>(env, runner);
        });
    } catch(const std::runtime_error& e) {
        std::cout << "Fatal application error: " << e.what() << "\n";
        return 1;
    }
}
