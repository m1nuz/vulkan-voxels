#include "Application.hpp"
#include "Journal.hpp"
#include "Tags.hpp"
#include "Window.hpp"

#include <string>

namespace Application {

static auto process_events(Application& app) {
    while (!app._events.empty()) {
        // dispatch event
        app._events.pop();
    }
}

auto run(Configuration& conf, Application& app) -> int {
    Journal::message(Tags::App, "Start");

    if (glfwInit() != GLFW_TRUE) {
        Journal::critical(Tags::App, "Initialization failed!");
        return EXIT_FAILURE;
    }

    app._window = create_window({ .title = conf.title, .width = conf.window_width, .height = conf.window_height });

    app._running = true;
    while (app._running) {
        process_events(app);
        app._running = process_window_events(app._window);
    }

    destroy_window(app._window);

    glfwTerminate();

    Journal::message(Tags::App, "Shutdown");
    return EXIT_SUCCESS;
}

} // namespace Application