#include "Models/Fwd.h"

#include "Ctrls/PageLoader.h"

#include "Views/ContentView.h"
#include "Views/ProgressView.h"


int main()
{
    // Components creation (better to use DI)
    auto controller = std::make_shared<Ctrls::PageLoader>();

    auto progressView = std::make_shared<Views::ProgressView>(controller);
    auto contentView = std::make_shared<Views::ContentView>(controller);

    // Start of work
    controller->createRequest()->url = "www.google.com";
}
