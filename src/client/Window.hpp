#pragma once

#include <memory>
#include <string>

#include <GLFW/glfw3.h>

namespace Application {

struct Window {
    std::string title;
    int32_t width = 0;
    int32_t height = 0;
    GLFWwindow* window = nullptr;
};

struct CreateWindowInfo {
    std::string_view title;
    int32_t width = 0;
    int32_t height = 0;
    bool centered = true;
};

auto create_window(const CreateWindowInfo& info) -> std::shared_ptr<Window>;
auto destroy_window(std::shared_ptr<Window> w) -> void;
auto process_window_events(std::shared_ptr<Window> w) -> bool;

} // namespace Application