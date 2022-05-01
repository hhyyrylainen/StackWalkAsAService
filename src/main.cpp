#include "APIV1.h"
#include "Application.h"
#include "StackWalkRunner.h"

#include <Wt/WServer.h>

#include <iostream>

int main(int argc, char** argv)
{
    try {
        Wt::WServer server(argv[0]);

        const auto runner = std::make_shared<sws::StackWalkRunner>(argc > 0 ? argv[0] : "");

        server.setServerConfiguration(argc, argv, WTHTTP_CONFIGURATION);

        sws::APIV1 API(runner);

        server.addResource(&API, "/api/v1");

        server.addEntryPoint(
            Wt::EntryPointType::Application, [runner](const Wt::WEnvironment& env) {
                return std::make_unique<sws::Application>(env, runner);
            });

        if(server.start()) {
            int signal = Wt::WServer::waitForShutdown();

            std::cerr << "Shutdown (signal = " << signal << ")" << std::endl;

            // This cancels all running operations
            runner->Stop();

            server.stop();

            if(signal == SIGHUP)
                Wt::WServer::restart(argc, argv, environ);
        }
    } catch(const std::exception& e) {
        std::cout << "Fatal application error: " << e.what() << "\n";
        return 1;
    }
}
