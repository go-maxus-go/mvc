#pragma once

#include <memory>


namespace Models {

struct Page;
using PagePtr  = std::shared_ptr<Page>;
using PagePtrC = std::shared_ptr<const Page>;

} // namespace Models
