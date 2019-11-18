#pragma once

#include <Wt/WContainerWidget.h>
#include <Wt/WText.h>
#include <Wt/WPanel.h>

namespace sws {

class StackWalkOperation;

//! \brief Shows results of a stackwalk in the UI. Also handles deleting the file after
//! stackwalking
class ResultWidget : public Wt::WContainerWidget {
public:
    ResultWidget(
        const std::shared_ptr<StackWalkOperation>& operation);

    void UpdateFromOperation();
    
private:
    Wt::WText* FileName;
    Wt::WText* StatusText;
    Wt::WText* SuccessText;
    
    Wt::WPanel* ResultPanel;
    
    Wt::WText* ResultText;

    std::shared_ptr<StackWalkOperation> Operation;
};

} // namespace sws
