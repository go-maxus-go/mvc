#pragma once

#include <string>


namespace Models {

struct Page
{
    std::string url;
    std::string content;
    enum class Status {
        Created, Loading, Loaded
    } status = Status::Created;
    int loadingProgress = 0;
};

} // namespace Models
