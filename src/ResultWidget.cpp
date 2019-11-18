// ------------------------------------ //
#include "ResultWidget.h"

#include "Application.h"
#include "StackWalkOperation.h"

#include <Wt/Utils.h>
#include <Wt/WAnimation.h>
#include <Wt/WServer.h>

using namespace sws;
// ------------------------------------ //
ResultWidget::ResultWidget(const std::shared_ptr<StackWalkOperation>& operation) :
    Operation(operation)
{
    std::string fileName = operation->GetFilePath();

    const auto slash = fileName.find_last_of('/');

    if(slash != std::string::npos)
        fileName = fileName.substr(slash + 1);

    FileName = addWidget(std::make_unique<Wt::WText>(fileName));
    FileName->setMargin(5, Wt::Side::Right);

    StatusText = addWidget(std::make_unique<Wt::WText>("Queued"));
    StatusText->setMargin(5, Wt::Side::Right);

    SuccessText = addWidget(std::make_unique<Wt::WText>(""));
    SuccessText->setMargin(5, Wt::Side::Right);

    ResultPanel = addWidget(std::make_unique<Wt::WPanel>());
    ResultPanel->setTitle("Stack dump");
    ResultPanel->setCollapsible(true);
    ResultPanel->setCollapsed(true);

    Wt::WAnimation animation(
        Wt::AnimationEffect::SlideInFromTop, Wt::TimingFunction::EaseOut, 300);

    ResultPanel->setAnimation(animation);

    {
        auto tmpText = std::make_unique<Wt::WText>("Result is pending...");
        ResultText = tmpText.get();

        ResultPanel->setCentralWidget(std::move(tmpText));
    }

    const auto session = Wt::WApplication::instance()->sessionId();

    // Register operation callbacks
    const auto updateCallback = [this, session](StackWalkOperation& op) {
        Wt::WServer::instance()->post(session, [=]() {
            static_cast<Application*>(Wt::WApplication::instance())->UpdateResultWidget(this);
        });
    };

    operation->SetOnStartedCallback(updateCallback);
    operation->SetOnFinishedCallback(updateCallback);
}
// ------------------------------------ //
void ResultWidget::UpdateFromOperation()
{
    if(Operation->HasFinished()) {

        StatusText->setText("Finished");
        SuccessText->setText(Operation->HasSucceeded() ? "Succeeded" : "Failed");

        ResultText->setText(
            "<pre>" + Wt::Utils::htmlEncode(Operation->GetResult()) + "</pre>");

    } else if(Operation->HasStartedProcessing()) {
        StatusText->setText("Processing");
    }
}
