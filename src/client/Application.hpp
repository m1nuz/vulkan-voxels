#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "config.hpp"

#include "Event.hpp"
#include "Renderer.hpp"
#include "World.hpp"

namespace Application {

struct Configuration {
    int32_t window_width = 1920;
    int32_t window_height = 1080;
    std::string_view title = WINDOW_TITLE;
    bool fullscreen = false;
    bool vsync = false;
    bool window_centered = true;
    bool debug_graphics = true;
};

using Threads = std::vector<std::jthread>;

struct Window;

struct Application {
    Game::World _world;
    Game::Renderer _renderer;

    Threads _threads;
    EventQueue _events;
    std::shared_ptr<Window> _window;
    std::atomic_bool _running = false;
};

auto run(Configuration& conf, Application& app) -> int;

} // namespace Application