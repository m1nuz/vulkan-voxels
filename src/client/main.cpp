#include "Application.hpp"

extern int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    Application::Configuration conf;
    Application::Application app;
    return Application::run(conf, app);
}