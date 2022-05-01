// ------------------------------------ //
#include "Application.h"

#include "ResultWidget.h"
#include "StackWalkOperation.h"
#include "StackWalkRunner.h"

#include <Wt/WBootstrapTheme.h>
#include <Wt/WBreak.h>
#include <Wt/WComboBox.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WEnvironment.h>
#include <Wt/WPushButton.h>

using namespace sws;
// ------------------------------------ //
Application::Application(
    const Wt::WEnvironment& env, std::shared_ptr<StackWalkRunner> runner) :
    Wt::WApplication(env),
    Runner(std::move(runner))
{
    setTitle("StackWalk as a Service");
    setTheme(std::make_shared<Wt::WBootstrapTheme>());
    // useStyleSheet(const WLink &link)

    root()->addWidget(
        std::make_unique<Wt::WText>("<h1>Google Breakpad StackWalk as a web app</h1>"));

    root()->addWidget(std::make_unique<Wt::WBreak>());

    root()->addWidget(std::make_unique<Wt::WText>(
        "<p>This is a very basic web service offering running StackWalk on an uploaded file. "
        "There is also a blocking API provided. This service should not be exposed to the web "
        "as it is very easy to DOS this with the API.</p>"));

    root()->addWidget(std::make_unique<Wt::WBreak>());

    root()->addWidget(std::make_unique<Wt::WText>(
        "<p>All needed symbol files need to be added to the "
        "symbol directory (" +
        Runner->GetSymbolsFolder() +
        ") before this service can be used. The used stackwalk executable is " +
        Runner->ExecutableFromWalkType(StackWalkType::Normal) + ". MinGW stackwalk is: " +
        Runner->ExecutableFromWalkType(StackWalkType::MinGW) + "</p>"));

    root()->addWidget(std::make_unique<Wt::WBreak>());

    // Box showing uploads
    OperatedDumps = root()->addWidget(std::make_unique<Wt::WPanel>());
    OperatedDumps->setTitle("StackWalk Results");
    OperatedDumps->setHidden(true);
    PanelResultContainer =
        OperatedDumps->setCentralWidget(std::make_unique<Wt::WContainerWidget>());

    root()->addWidget(std::make_unique<Wt::WBreak>());

    ErrorText = root()->addWidget(std::make_unique<Wt::WText>(""));

    if(!env.javaScript()) {
        ErrorText->setText("<h2>This site does not work correctly without JavaScript!</h2>");
    }

    root()->addWidget(std::make_unique<Wt::WBreak>());

    root()->addWidget(std::make_unique<Wt::WText>(".dmp file to process: "));

    Upload = root()->addWidget(std::make_unique<Wt::WFileUpload>());
    Upload->setMultiple(true);
    Upload->setFileTextSize(40);

    root()->addWidget(std::make_unique<Wt::WBreak>());

    root()->addWidget(std::make_unique<Wt::WText>("Walk type to perform: "));

    Wt::WComboBox* typeSelection = root()->addWidget(std::make_unique<Wt::WComboBox>());
    typeSelection->addItem("Normal");
    typeSelection->addItem("MinGW");

    root()->addWidget(std::make_unique<Wt::WBreak>());

    Wt::WPushButton* submit =
        root()->addWidget(std::make_unique<Wt::WPushButton>("Upload Selected File"));
    submit->setDisabled(true);

    // Signals

    // Button enable when can upload something
    Upload->changed().connect([this, submit] { submit->setDisabled(!Upload->canUpload()); });

    // If type is changed after upload, allow uploading again
    typeSelection->changed().connect([this, submit] { submit->setDisabled(Upload->empty()); });

    // Successful upload callback
    Upload->uploaded().connect([this, submit, typeSelection]() {
        if(Upload->uploadedFiles().empty()) {
            ErrorText->setText("No files uploaded");
            return;
        }

        submit->setDisabled(true);
        OperatedDumps->setHidden(false);

        // Without a WTimer this needs JS to work
        enableUpdates();

        for(auto& file : Upload->uploadedFiles()) {

            StackWalkType walkType = StackWalkType::Normal;

            switch(typeSelection->currentIndex()) {
            case 0:
                // normal
                break;
            case 1: walkType = StackWalkType::MinGW; break;
            default: Wt::log("error") << "Unknown stackwalk type index selected";
            }

            Wt::log("info") << "Uploaded crashdump file is: " << file.spoolFileName()
                            << " selected walk type: " << static_cast<int>(walkType);

            auto op = std::make_shared<StackWalkOperation>(file.spoolFileName(), walkType);

            // We will delete the file ourselves
            file.stealSpoolFile();

            auto widget = PanelResultContainer->addNew<ResultWidget>(op);

            CreatedOperations.push_back(widget);
            // PanelResultContainer->addChild(std::make_unique<Wt::WBreak>());

            // Queue the operation for running in the background
            Runner->QueueOperation(op);
        }

        Wt::log("info") << "Finished handling upload of " << Upload->uploadedFiles().size()
                        << " files";
    });

    Upload->fileTooLarge().connect([this](int64_t size) {
        ErrorText->setText("File is too large to upload");

        // Attempted size KiB: size / 1024
        // Allowed: WApplication::instance()->maximumRequestSize() / 1024)
    });

    submit->clicked().connect([this] { Upload->upload(); });
}
// ------------------------------------ //
void Application::finalize() {}
// ------------------------------------ //
void Application::UpdateResultWidget(ResultWidget* widget)
{
    for(auto* created : CreatedOperations) {
        if(created == widget) {

            created->UpdateFromOperation();
            triggerUpdate();
            return;
        }
    }

    Wt::log("error") << "Update result widget call received for non-existant widget";
}
