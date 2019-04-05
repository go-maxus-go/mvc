#pragma once

#include <memory>

#include "details/observer.h"

#include "controller.h"


namespace mvc {

// General View class
template<class Model>
class View
    : public details::Observer<Model>
{
    using Obs = details::Observer<Model>;
public:
    using CtrlPtr = std::shared_ptr<Controller<Model>>;
    using ViewPtr = std::shared_ptr<View<Model>>;
    using ModelPtrC = std::shared_ptr<const Model>;

    View(CtrlPtr ctrl)
        : m_self(ViewPtr(this, [](auto){}))
        , m_ctrl(std::move(ctrl))
    {
        m_ctrl->attach(m_self);
    }

    auto createRequest() { return m_ctrl->createRequest(); }
    auto removeRequest(ModelPtrC model) { return m_ctrl->removeRequest(std::move(model)); }
    auto updateRequest(ModelPtrC model) { return m_ctrl->updateRequest(std::move(model)); }

    decltype(auto) models() const { return m_ctrl->models(); }

protected:
    using Obs::created;
    using Obs::updated;
    using Obs::removed;

private:
    ViewPtr m_self;
    const CtrlPtr m_ctrl;
};

} // namespace mvc
