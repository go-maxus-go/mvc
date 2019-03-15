#pragma once

#include <iostream>

#include <mvc/view.h>

#include "Models/Page.h"

#include "Ctrls/Fwd.h"
#include "Ctrls/PageLoader.h"


namespace Views {

class ContentView : public mvc::View<Models::Page>
{
    using BaseView = mvc::View<Models::Page>;
public:
    using BaseView::BaseView;
    ~ContentView() override
    {
        if (m_page != nullptr)
            removeRequest(m_page);
    }

protected:
    void created(const ModelPtrC & model) override
    {
        m_page = model;
        std::cout << "Page opened: " << model->url << std::endl;
    }
    void removed(const ModelPtrC & model) override
    {
        std::cout << "Page closed: " << model->url << std::endl;
    }
    void updated(const ModelPtrC & model, const ModelPtrC &) override
    {
        if (model->status == Models::Page::Status::Loaded)
            std::cout << "Page content: " << model->content << std::endl;
    }

private:
    ModelPtrC m_page;
};

} // namespace Views
