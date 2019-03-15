# Model-View-Controller C++ implementation

## The main idea
Model - is pure data represented by naked structures. Controller - owns model and can modify it. View - knows Model and able to create requests for model updates.

## The simplest example
Entire code is here. It is described below.
```cpp
#include <mvc/view.h>
#include <mvc/controller.h>
#include <iostream> // required only for this example

struct MyModel
{
    int value;
};

class MyController : public mvc::Controller<MyModel> {};

class MyView : public mvc::View<MyModel>
{
    std::string m_name;
    using BaseView = mvc::View<MyModel>;
public:
    using BaseView::BaseView; // base class contructor using

    void setName(const std::string & name) // give a name for the view
    {
        m_name = name;
    }

protected: // Callback implementations
    void created(const ModelPtrC & model) override
    {
        std::cout << m_name << ": " << "created model, value = "
            << model->value << std::endl;
    }

    void removed(const ModelPtrC & model) override
    {
        std::cout << m_name << ": " << "removed model, value = "
            << model->value << std::endl;
    }

    void updated(const ModelPtrC & model, const ModelPtrC & from) override
    {
        std::cout << m_name << ": " << "updated model, value from = "
            << from->value << ", to = " << model->value << std::endl;
    }
};

int main()
{
    // Create controller
    auto ctrl = std::make_shared<MyController>();

    // Create views
    auto view1 = std::make_shared<MyView>(ctrl);
    auto view2 = std::make_shared<MyView>(ctrl);
    view1->setName("view #1");
    view2->setName("view #2");

    // The interesting part will be described a bit later
    auto model = [&view1] {
        auto creator = view1->createRequest();
        creator->value = 42;
        return creator.toPtr();
    }();

    // Update model
    view2->updateRequest(model)->value = 123;

    // All models must be removed, for the sake of consistency
    view1->removeRequest(model);
}
```
### Header files to include
Entire pattern is represented by two header files "mvc/view.h" and "mvc/controller.h".

```cpp
#include <mvc/view.h>
#include <mvc/controller.h>
#include <iostream> // required only for this example
```
### Model
Model is represented by struct data class
```cpp
struct MyModel
{
    int value;
};
```
### Controller
Controller is a derived class from the base template controller class
```cpp
class MyController : public mvc::Controller<MyModel> {};
```
### View
View is a derived class from the base template view class. Some method are called when model objects changes somehow.
```cpp
class MyView : public mvc::View<MyModel>
{
    std::string m_name;
    using BaseView = mvc::View<MyModel>;
public:
    using BaseView::BaseView; // base class contructor using

    void setName(const std::string & name) // give a name for the view
    {
        m_name = name;
    }

protected: // Callback implementations
    void created(const ModelPtrC & model) override
    {
        std::cout << m_name << ": " << "created model, value = "
            << model->value << std::endl;
    }

    void removed(const ModelPtrC & model) override
    {
        std::cout << m_name << ": " << "removed model, value = "
            << model->value << std::endl;
    }

    void updated(const ModelPtrC & model, const ModelPtrC & from) override
    {
        std::cout << m_name << ": " << "updated model, value from = "
            << from->value << ", to = " << model->value << std::endl;
    }
};
```
### How to use
At least one controller must be created. View requires a pointer to the controller.
```cpp
int main()
{
    // Create controller
    auto ctrl = std::make_shared<MyController>();

    // Create views
    auto view1 = std::make_shared<MyView>(ctrl);
    auto view2 = std::make_shared<MyView>(ctrl);
    view1->setName("view #1");
    view2->setName("view #2");

    // The interesting part will be described a bit later
    auto model = [&view1] {
        auto creator = view1->createRequest();
        creator->value = 42;
        return creator.toPtr();
    }();

    // Update model
    view2->updateRequest(model)->value = 123;

    // All models must be removed, for the sake of consistency
    view1->removeRequest(model);
}
```
### The interesting part
All requests(create, remove, update) are represented by special classes. These classes can create a copy of a model object. These class should be user as pointer to a model object. When request object is created you can work with it in the same way as you work with a pointer to a model object. In destructor they call controller and apply changes to a model object.  
In the example lambda was used to get a pointer to the model object but destroy the request class.

## More complex example
You can find more complex example in the "example folder". You can build it using
```
cd example && cmake . && make && ./example
```
