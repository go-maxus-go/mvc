#pragma once

#include <iostream>

#include <mvc/view.h>

#include "Models/Page.h"

#include "Ctrls/Fwd.h"
#include "Ctrls/PageLoader.h"


namespace Views {

class ProgressView : public mvc::View<Models::Page>
{
    using BaseView = mvc::View<Models::Page>;
public:
    using BaseView::BaseView;

protected:
    void updated(const ModelPtrC & model, const ModelPtrC & from) override
    {
        if (model->loadingProgress != from->loadingProgress)
            std::cout << "Loading progress: " << model->loadingProgress << "%" << std::endl;
    }
};

} // namespace Views
