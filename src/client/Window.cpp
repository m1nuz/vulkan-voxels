#include "Window.hpp"
#include "Journal.hpp"
#include "Tags.hpp"

namespace Application {

static auto center_window(GLFWwindow* window, GLFWmonitor* monitor) {
    if (!monitor)
        return;

    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    if (!mode)
        return;

    int monitor_x = 0, monitor_y = 0;
    glfwGetMonitorPos(monitor, &monitor_x, &monitor_y);

    int width = 0, height = 0;
    glfwGetWindowSize(window, &width, &height);

    glfwSetWindowPos(window, monitor_x + (mode->width - width) / 2, monitor_y + (mode->height - height) / 2);
}

[[nodiscard]] auto process_window_events(std::shared_ptr<Window> w) -> bool {
    if (glfwWindowShouldClose(w->window)) {
        return false;
    }

    glfwPollEvents();
    return true;
}

[[nodiscard]] auto create_window(const CreateWindowInfo& info) -> std::shared_ptr<Window> {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    auto window = glfwCreateWindow(info.width, info.height, std::data(info.title), nullptr, nullptr);
    if (!window) {
        const char* description = nullptr;
        const auto err = glfwGetError(&description);
        Journal::critical(Tags::Window, "Error: {} {}", err, description);
        return nullptr;
    }

    if (info.centered) {
        center_window(window, glfwGetPrimaryMonitor());
    }

    glfwSetCursorPos(window, info.width / 2.f, info.height / 2.f);
    glfwShowWindow(window);

    Window w;
    w.title = info.title;
    w.width = info.width;
    w.height = info.height;
    w.window = window;

    return std::make_shared<Window>(w);
}

auto destroy_window(std::shared_ptr<Window> w) -> void {
    glfwDestroyWindow(w->window);
}

} // namespace Application