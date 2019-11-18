#pragma once

#include <Wt/WApplication.h>
#include <Wt/WFileUpload.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPanel.h>
#include <Wt/WText.h>
// #include <Wt/WGroupBox.h>

namespace sws {

class StackWalkRunner;
class ResultWidget;

class Application : public Wt::WApplication {
public:
    Application(const Wt::WEnvironment& env, const std::shared_ptr<StackWalkRunner>& runner);


    void UpdateResultWidget(ResultWidget* widget);

private:
    // Wt::WLineEdit* nameEdit_;

    Wt::WText* ErrorText;
    Wt::WFileUpload* Upload;

    Wt::WPanel* OperatedDumps;
    Wt::WContainerWidget* PanelResultContainer;

    std::vector<ResultWidget*> CreatedOperations;

    std::shared_ptr<StackWalkRunner> Runner;
};

} // namespace sws
