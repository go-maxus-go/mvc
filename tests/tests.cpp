#include <vector>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <mvc/view.h>
#include <mvc/controller.h>


struct TestModel
{
    int value;
};

struct TestController : mvc::Controller<TestModel>
{
    int aboutToCreateCounter = 0;
    int aboutToRemoveCounter = 0;
    int aboutToUpdateCounter = 0;

    int lastModelValue = 0;
    int lastToValue = 0;

    int forcedValue = 0;
protected:
    // mvc::Controller callbacks
    void aboutToCreate(const ModelPtr & model) override
    {
        ++aboutToCreateCounter;
        lastModelValue = model->value;
    }

    void aboutToRemove(const ModelPtrC & model) override
    {
        ++aboutToRemoveCounter;
        lastModelValue = model->value;
    }

    void aboutToUpdate(const ModelPtrC & model, const ModelPtr & to) override
    {
        ++aboutToUpdateCounter;
        lastModelValue = model->value;
        lastToValue = to->value;
        if (forcedValue)
            to->value = forcedValue;
    }
};

using TestControllerPtr = std::shared_ptr<TestController>;

struct TestView: mvc::View<TestModel>
{
    using BaseView = mvc::View<TestModel>;
    using BaseView::BaseView;

    std::vector<ModelPtrC> models;

    using ModelLog = std::tuple<ModelPtrC /*model*/, int /*from*/, int /*to*/>;
    std::vector<ModelLog> log;

protected:
    void created(const ModelPtrC & model) override
    {
        models.push_back(model);
    }

    void removed(const ModelPtrC & model) override
    {
        auto it = std::remove(models.begin(), models.end(), model);
        models.erase(it, models.end());
    }

    void updated(const ModelPtrC & model, const ModelPtrC &from) override
    {
        log.emplace_back(std::make_tuple(model, from->value, model->value));
    }
};


TEST_CASE("Controller can create, remove and modify model objects", "[mvc]")
{
    auto createModel = [](auto && ctrl, int value) {
        auto creator = ctrl->createRequest();
        creator->value = value;
        return creator.toPtr();
    };
    auto ctrl = std::make_shared<TestController>();

    const auto model1 = createModel(ctrl, 69);
    REQUIRE(ctrl->models().size() == 1);
    REQUIRE((*ctrl->models().find(model1))->value == 69);
    REQUIRE(model1->value == 69);

    const auto model2 = createModel(ctrl, 42);
    REQUIRE(ctrl->models().size() == 2);
    REQUIRE((*ctrl->models().find(model1))->value == 69);
    REQUIRE((*ctrl->models().find(model2))->value == 42);
    REQUIRE(model1->value == 69);
    REQUIRE(model2->value == 42);

    ctrl->updateRequest(model1)->value = 11;
    REQUIRE(ctrl->models().size() == 2);
    REQUIRE((*ctrl->models().find(model1))->value == 11);
    REQUIRE((*ctrl->models().find(model2))->value == 42);
    REQUIRE(model1->value == 11);
    REQUIRE(model2->value == 42);

    ctrl->updateRequest(model2)->value = 22;
    REQUIRE(ctrl->models().size() == 2);
    REQUIRE((*ctrl->models().find(model1))->value == 11);
    REQUIRE((*ctrl->models().find(model2))->value == 22);
    REQUIRE(model1->value == 11);
    REQUIRE(model2->value == 22);

    ctrl->removeRequest(model1);
    REQUIRE(ctrl->models().size() == 1);
    REQUIRE((*ctrl->models().find(model2))->value == 22);
    REQUIRE(model2->value == 22);

    ctrl->removeRequest(model2);
    REQUIRE(ctrl->models().size() == 0);
}

TEST_CASE("Controller receives 'about' notifications", "[mvc]")
{
    auto ctrl = std::make_shared<TestController>();
    ctrl->createRequest()->value = 69;

    REQUIRE(ctrl->models().size() == 1);
    REQUIRE(ctrl->aboutToCreateCounter == 1);
    REQUIRE(ctrl->aboutToUpdateCounter == 0);
    REQUIRE(ctrl->aboutToRemoveCounter == 0);
    REQUIRE(ctrl->lastModelValue == 69);
    REQUIRE(ctrl->lastToValue == 0);

    auto model = *ctrl->models().begin();
    ctrl->updateRequest(model)->value = 42;
    REQUIRE(ctrl->aboutToCreateCounter == 1);
    REQUIRE(ctrl->aboutToUpdateCounter == 1);
    REQUIRE(ctrl->aboutToRemoveCounter == 0);
    REQUIRE(ctrl->lastModelValue == 69);
    REQUIRE(ctrl->lastToValue == 42);

    ctrl->forcedValue = 22;
    ctrl->updateRequest(model)->value = 11;
    REQUIRE(model->value == 22);
    REQUIRE(ctrl->aboutToCreateCounter == 1);
    REQUIRE(ctrl->aboutToUpdateCounter == 2);
    REQUIRE(ctrl->aboutToRemoveCounter == 0);
    REQUIRE(ctrl->lastModelValue == 42);
    REQUIRE(ctrl->lastToValue == 11);

    ctrl->removeRequest(model);
    REQUIRE(ctrl->aboutToCreateCounter == 1);
    REQUIRE(ctrl->aboutToUpdateCounter == 2);
    REQUIRE(ctrl->aboutToRemoveCounter == 1);
}

TEST_CASE("View receive create, remove and update notifications", "[mvc]")
{
    auto ctrl = std::make_shared<TestController>();
    auto view = std::make_shared<TestView>(ctrl);

    ctrl->createRequest()->value = 69;
    REQUIRE(view->models.size() == 1);
    REQUIRE(view->models[0]->value == 69);

    ctrl->createRequest()->value = 42;
    REQUIRE(view->models.size() == 2);
    REQUIRE(view->models[0]->value == 69);
    REQUIRE(view->models[1]->value == 42);

    ctrl->updateRequest(view->models[0])->value = 11;
    REQUIRE(view->models.size() == 2);
    REQUIRE(view->models[0]->value == 11);
    REQUIRE(view->models[1]->value == 42);

    ctrl->updateRequest(view->models[1])->value = 22;
    REQUIRE(view->models.size() == 2);
    REQUIRE(view->models[0]->value == 11);
    REQUIRE(view->models[1]->value == 22);

    ctrl->removeRequest(view->models[0]);
    REQUIRE(view->models.size() == 1);
    REQUIRE(view->models[0]->value == 22);

    ctrl->removeRequest(view->models[0]);
    REQUIRE(view->models.size() == 0);
}

TEST_CASE("Views can create, remove and update notifications", "[mvc]")
{
    auto ctrl = std::make_shared<TestController>();
    auto v1 = std::make_shared<TestView>(ctrl);
    auto v2 = std::make_shared<TestView>(ctrl);

    v1->createRequest()->value = 69;
    REQUIRE(v1->models.size() == 1);
    REQUIRE(v1->models[0]->value == 69);
    REQUIRE(v2->models.size() == 1);
    REQUIRE(v2->models[0]->value == 69);

    v2->createRequest()->value = 42;
    REQUIRE(v1->models.size() == 2);
    REQUIRE(v1->models[0]->value == 69);
    REQUIRE(v1->models[1]->value == 42);
    REQUIRE(v2->models.size() == 2);
    REQUIRE(v2->models[0]->value == 69);
    REQUIRE(v2->models[1]->value == 42);

    v1->updateRequest(v1->models[0])->value = 11;
    REQUIRE(v1->models.size() == 2);
    REQUIRE(v1->models[0]->value == 11);
    REQUIRE(v1->models[1]->value == 42);
    REQUIRE(v2->models.size() == 2);
    REQUIRE(v2->models[0]->value == 11);
    REQUIRE(v2->models[1]->value == 42);

    v2->updateRequest(v2->models[1])->value = 22;
    REQUIRE(v1->models.size() == 2);
    REQUIRE(v1->models[0]->value == 11);
    REQUIRE(v1->models[1]->value == 22);
    REQUIRE(v2->models.size() == 2);
    REQUIRE(v2->models[0]->value == 11);
    REQUIRE(v2->models[1]->value == 22);

    v1->removeRequest(v1->models[1]);
    REQUIRE(v1->models.size() == 1);
    REQUIRE(v1->models[0]->value == 11);
    REQUIRE(v2->models.size() == 1);
    REQUIRE(v2->models[0]->value == 11);

    v2->removeRequest(v2->models[0]);
    REQUIRE(v1->models.size() == 0);
    REQUIRE(v2->models.size() == 0);
}

TEST_CASE("Create, remove and update operations put in a queue", "[mvc]")
{
    struct CurrTestView: mvc::View<TestModel>
    {
        using BaseView = mvc::View<TestModel>;
        using BaseView::BaseView;
        std::vector<std::string> log;
        int createdValue = 0;
        int removedValue = 0;
        int updatedValue = 0;
        int updatedFromValue = 0;
     protected:
        void created(const ModelPtrC & model) override
        {
            createdValue = model->value;
            updateRequest(model)->value = 69;
            log.push_back("created");
        }
        void removed(const ModelPtrC & model) override
        {
            removedValue = model->value;
            log.push_back("removed");
        }
        void updated(const ModelPtrC & model, const ModelPtrC & from) override
        {
            updatedValue = model->value;
            updatedFromValue = from->value;
            removeRequest(model);
            log.push_back("updated");
        }
    };

    auto ctrl = std::make_shared<TestController>();
    auto view = std::make_shared<CurrTestView>(ctrl);

    view->createRequest()->value = 42;
    REQUIRE(view->createdValue == 42);
    REQUIRE(view->removedValue == 69);
    REQUIRE(view->updatedValue == 69);
    REQUIRE(view->updatedFromValue == 42);
    REQUIRE(view->log[0] == "created");
    REQUIRE(view->log[1] == "updated");
    REQUIRE(view->log[2] == "removed");
}

TEST_CASE("Views can attached and detached from controller", "[mvc]")
{
    auto ctrl = std::make_shared<TestController>();
    auto v1 = std::make_shared<TestView>(ctrl);
    auto v2 = std::make_shared<TestView>(ctrl);

    ctrl->detach(v2);
    v1->createRequest()->value = 42;
    REQUIRE(v2->models.size() == 0);

    ctrl->attach(v2);
    v1->updateRequest(v1->models[0])->value = 69;
    REQUIRE(v2->models.size() == 0);
    REQUIRE(v2->log.size() == 1);
    REQUIRE(std::get<0>(v2->log[0]) == v1->models[0]);
    REQUIRE(std::get<1>(v2->log[0]) == 42);
    REQUIRE(std::get<2>(v2->log[0]) == 69);

    v2.reset();
    v1->removeRequest(v1->models[0]);
    REQUIRE(v1->models.size() == 0);
}
