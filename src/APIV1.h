#pragma once

#include <Wt/WResource.h>

namespace sws {

class StackWalkRunner;

//! Provides a basic blocking API to do stackwalking
class APIV1 : public Wt::WResource {
public:
    APIV1(std::shared_ptr<StackWalkRunner> runner);

    void handleRequest(
        const Wt::Http::Request& request, Wt::Http::Response& response) override;

private:
    std::shared_ptr<StackWalkRunner> Runner;
};

} // namespace sws
