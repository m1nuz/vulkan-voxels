#pragma once

#if defined( VK_USE_PLATFORM_WIN32_KHR )
#include <Windows.h>

#elif defined( VK_USE_PLATFORM_XCB_KHR )
#include <cstdlib>
#include <dlfcn.h>
#include <xcb/xcb.h>

#elif defined( VK_USE_PLATFORM_XLIB_KHR )
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <cstdlib>
#include <dlfcn.h>

#endif

#include <variant>
#include <optional>

namespace app {

    constexpr char APP_TAG[] = "App";

    typedef struct settings_type {
        settings_type( ) = default;

        bool validation = false;
        bool vsync = false;
        bool fullscreen = false;
        bool overlay = false;
    } settings_t;

    namespace syswm {

        typedef struct instance_type {
            instance_type( ) = default;

            xcb_connection_t *connection = nullptr;
            xcb_window_t window = 0;

            xcb_intern_atom_reply_t *protocols_reply;
            xcb_intern_atom_reply_t *atom_wm_delete_window;

            bool running = true;

            settings_t settings;
        } instance_t;

        struct on_init {
            on_init( ) = default;
        };

        struct on_done {
            on_done( ) = default;
        };

        struct on_update {
            on_update( ) = default;
        };

        struct on_present {
            on_present( ) = default;

            on_present( const float _interpolation, const uint64_t _timesteps ) : interpolation{_interpolation}, timesteps{_timesteps} {
            }

            float interpolation = 0.f;
            uint64_t timesteps = 0ull;
        };

        using event = std::variant<on_init, on_done, on_update, on_present>;

        namespace detail {

            auto create_instance( ) noexcept -> std::optional<instance_t>;
            auto destroy_instance( instance_t &instance ) noexcept -> void;
            auto poll_events(instance_t &instance ) noexcept -> void;

        } // namespace detail

        using detail::create_instance;
        using detail::destroy_instance;
        using detail::poll_events;

    } // namespace syswm

    using instance_t = syswm::instance_t;
    using event_t = syswm::event;

} // namespace app
