#pragma once

#include <mvc/controller.h>

#include "Models/Page.h"


namespace Ctrls {

class PageLoader : public mvc::Controller<Models::Page>
{
protected:
    void aboutToCreate(const ModelPtr & model) override
    {
        if (model->status == ::Models::Page::Status::Created)
            model->status = ::Models::Page::Status::Loading;

        if (model->status == ::Models::Page::Status::Loading)
            updateRequest(model);
    }

    void aboutToUpdate(const ModelPtrC & model, const ModelPtr & to) override
    {
        if (to->status == ::Models::Page::Status::Loading) {
            if (to->loadingProgress >= 100) {
                to->status = ::Models::Page::Status::Loaded;
                to->content = "Ready to search ...";
                return;
            }
            updateRequest(model)->loadingProgress += 20;
        }
    }

private:
    ModelPtr m_profile;
};

} // namespace Ctrls
