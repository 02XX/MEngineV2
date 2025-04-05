#include "Application.hpp"
#include "Context.hpp"
#include "boost/di.hpp"
#include <memory>
#include <vector>
namespace di = boost::di;
using namespace MEngine;
int main()
{
    Application app;
    app.Run();
    return 0;
}