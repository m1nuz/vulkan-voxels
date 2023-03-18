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

static auto cleanup(Application& app) -> void {
    destroy_window(app._window);

    glfwTerminate();

    Journal::message(Tags::App, "Shutdown");
}

auto run(Configuration& conf, Application& app) -> int {
    Journal::message(Tags::App, "Start");

    if (glfwInit() != GLFW_TRUE) {
        Journal::critical(Tags::App, "Initialization failed!");
        exit(EXIT_FAILURE);
    }

    app._window = create_window({ .title = conf.title, .width = conf.window_width, .height = conf.window_height });

    app._running = true;
    while (app._running) {
        process_events(app);

        Input input;
        app._running = process_window_events(app._window, input);

        Game::update_world(app._world);

        Game::present(app._renderer, app._world);
    }

    cleanup(app);

    return EXIT_SUCCESS;
}

} // namespace Application