#pragma once

#include <cassert>

#include <deque>
#include <vector>
#include <memory>
#include <algorithm>
#include <functional>
#include <unordered_set>

#include "details/observer.h"


namespace mvc {

//! General controller class
template<class Model>
class Controller
{
    using CtrlPtr = std::shared_ptr<Controller<Model>>;
public:
    Controller() : m_self(CtrlPtr(this, [](auto){}))
    {}
    virtual ~Controller()
    {
        assert(m_models.empty() && "All model objects must be removed");
    }

    // Aliases
    using ModelPtr = std::shared_ptr<Model>;
    using ModelPtrC = std::shared_ptr<const Model>;
    using ViewPtr = std::shared_ptr<details::Observer<Model>>;
    using Models = std::unordered_set<ModelPtrC>;

    // Attach and detach observers (views)
    void attach(ViewPtr view);
    void detach(const ViewPtr & view);

    // Provides copies of a model, accept changes, calls "aboutToUpdate" and "notifyUpdated"
    class ModelCreator;
    class ModelRemover;
    class ModelUpdater;

    // Model change for client code (usually for views)
    ModelCreator createRequest();
    ModelRemover removeRequest(ModelPtrC model);
    ModelUpdater updateRequest(ModelPtrC model);

    const Models & models() const { return m_models; }

protected:
    virtual void aboutToCreate(const ModelPtr  & /*model*/) {}
    virtual void aboutToRemove(const ModelPtrC & /*model*/) {}
    virtual void aboutToUpdate(const ModelPtrC & /*model*/, const ModelPtr & /*to*/) {}

    void processEvent(std::function<void()> fun);

private:
    void create(ModelPtrC model);
    void remove(ModelPtrC model);
    void update(ModelPtrC model, ModelPtr to);

    // Use it to notify views about model status
    void notifyCreated(const ModelPtrC & model);
    void notifyRemoved(const ModelPtrC & model);
    void notifyUpdated(const ModelPtrC & model, const ModelPtrC & to);

    void notify(std::function<void(ViewPtr)> fun);

private:
    CtrlPtr m_self;
    bool m_lock = false;
    std::deque<std::function<void()>> m_events;
    std::vector<std::weak_ptr<details::Observer<Model>>> m_views;
    Models m_models;
};

template <class Model>
void Controller<Model>::attach(ViewPtr view)
{
    assert(m_views.end() == std::find_if(
            m_views.begin(),
            m_views.end(),
            [v = view.get()](auto val) { return val.lock().get() == v; }
        ) && "Current view is already added"
    );
    m_views.push_back(std::move(view));
}

template <class Model>
void Controller<Model>::detach(const ViewPtr & view)
{
    auto it = std::remove_if(
        m_views.begin(),
        m_views.end(),
        [v = view.get()](auto value) { return value.lock().get() == v; }
    );
    assert(it != m_views.end() && "View isn't found");
    m_views.erase(it, m_views.end());
}

template <class Model>
auto Controller<Model>::createRequest() -> ModelCreator
{
    return ModelCreator(m_self);
}

template <class Model>
auto Controller<Model>::updateRequest(ModelPtrC model) -> ModelUpdater
{
    return ModelUpdater(m_self, std::move(model));
}

template <class Model>
auto Controller<Model>::removeRequest(ModelPtrC model) -> ModelRemover
{
    return ModelRemover(m_self, std::move(model));
}

template <class Model>
void Controller<Model>::create(ModelPtrC model)
{
    assert(m_models.find(model) == m_models.end() && "Model object already exists");
    m_models.insert(std::move(model));
}

template <class Model>
void Controller<Model>::remove(ModelPtrC model)
{
    assert(m_models.find(model) != m_models.end() && "Model object doens't exists");
    m_models.erase(model);
}

template <class Model>
void Controller<Model>::update(ModelPtrC model, ModelPtr to)
{
    assert(m_models.find(model) != m_models.end() && "Model object doens't exists");
    // unfortunately here we have to use const_cast
    std::swap(*(std::const_pointer_cast<Model>(model)), *to);
}

template <class Model>
void Controller<Model>::notifyCreated(const ModelPtrC & model)
{
    notify([model](auto &&view){ view->created(model); });
}

template <class Model>
void Controller<Model>::notifyUpdated(const ModelPtrC & model, const ModelPtrC & from)
{
    notify([model, from = std::move(from)](auto && view){ view->updated(model, from); });
}

template <class Model>
void Controller<Model>::notifyRemoved(const ModelPtrC & model)
{
    notify([model](auto && view){ view->removed(model); });
}

template <class Model>
void Controller<Model>::notify(std::function<void(ViewPtr)> fun)
{
    for (size_t i = 0; i < m_views.size();) {
        if (auto v = m_views[i++].lock())
            fun(v);
        else
            detach(v);
    }
}

template <class Model>
void Controller<Model>::processEvent(std::function<void()> event)
{
    m_events.push_back(event);
    if (m_lock)
        return;

    class BoolLock
    {
        bool & m_lock;
    public:
        BoolLock(bool & lock) : m_lock(lock) { m_lock = true; }
        ~BoolLock() { m_lock = false; }
    } lock(m_lock);

    while (!m_events.empty()) {
        m_events.front()(); // execute event
        m_events.pop_front();
    }
}

template <class Model>
class Controller<Model>::ModelCreator
{
    std::shared_ptr<Controller<Model>> m_ctrl;
    ModelPtr m_model;
public:
    ModelCreator(const ModelCreator &) = delete;
    ModelCreator(ModelCreator &&) = default;

    ModelCreator& operator =(const ModelCreator &) = delete;
    ModelCreator& operator =(ModelCreator &&) = default;

    ModelCreator(std::shared_ptr<Controller<Model>> ctrl)
        : m_ctrl(std::move(ctrl))
        , m_model(std::make_shared<Model>())
    {}

    ~ModelCreator()
    {
        m_ctrl->processEvent([ctrl = m_ctrl, model = std::move(m_model)] {
            ctrl->aboutToCreate(model);
            ctrl->create(model);
            ctrl->notifyCreated(model);
        });
    }

    ModelPtr operator->()
    {
        return m_model;
    }

    operator ModelPtrC()
    {
        return m_model;
    }

    ModelPtrC toPtr()
    {
        return m_model;
    }
};

template <class Model>
class Controller<Model>::ModelUpdater
{
    std::shared_ptr<Controller<Model>> m_ctrl;
    ModelPtrC m_model;
    ModelPtr m_to;
public:
    ModelUpdater(const ModelUpdater &) = delete;
    ModelUpdater(ModelUpdater &&) = default;

    ModelUpdater& operator =(const ModelUpdater &) = delete;
    ModelUpdater& operator =(ModelUpdater &&) = default;

    ModelUpdater(
        std::shared_ptr<Controller<Model>> ctrl,
        ModelPtrC model)
        : m_ctrl(std::move(ctrl))
        , m_model(std::move(model))
        , m_to(std::make_shared<Model>(*m_model))
    {}

    ~ModelUpdater()
    {
        m_ctrl->processEvent([ctrl = m_ctrl, model = std::move(m_model), to = std::move(m_to)] {
            ctrl->aboutToUpdate(model, to);
            ctrl->update(model, to); // swap data
            ctrl->notifyUpdated(model, to);
        });
    }

    ModelPtr operator->()
    {
        return m_to;
    }

    operator ModelPtr()
    {
        return m_to;
    }

    ModelPtr toPtr()
    {
        return m_to;
    }
};

template <class Model>
class Controller<Model>::ModelRemover
{
    std::shared_ptr<Controller<Model>> m_ctrl;
    ModelPtrC m_model;
public:
    ModelRemover(const ModelRemover &) = delete;
    ModelRemover(ModelRemover &&) = default;

    ModelRemover& operator =(const ModelRemover &) = delete;
    ModelRemover& operator =(ModelRemover &&) = default;

    ModelRemover(std::shared_ptr<Controller<Model>> ctrl, ModelPtrC model)
        : m_ctrl(std::move(ctrl))
        , m_model(std::move(model))
    {}

    ~ModelRemover()
    {
        m_ctrl->processEvent([ctrl = m_ctrl, model = std::move(m_model)] {
            ctrl->aboutToRemove(model);
            ctrl->remove(model);
            ctrl->notifyRemoved(model);
        });
    }

    ModelPtrC operator->()
    {
        return m_model;
    }

    operator ModelPtrC()
    {
        return m_model;
    }

    ModelPtrC toPtr()
    {
        return m_model;
    }
};

} // namespace mvc
