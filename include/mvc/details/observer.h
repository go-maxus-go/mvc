#pragma once

#include <memory>


namespace mvc {
namespace details {

template<class Model>
struct Observer
{
    virtual ~Observer() = default;

    using ModelPtrC = std::shared_ptr<const Model>;
    virtual void created(const ModelPtrC & /*model*/) {}
    virtual void removed(const ModelPtrC & /*model*/) {}
    virtual void updated(const ModelPtrC & /*model*/,
                         const ModelPtrC & /*from */) {}
};

} // namespace details
} // namespace mvc
