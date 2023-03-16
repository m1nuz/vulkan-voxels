#pragma once

#if defined( VK_USE_PLATFORM_WIN32_KHR )
#include <Windows.h>

#elif defined( VK_USE_PLATFORM_XCB_KHR )
#include <cstdlib>
#include <dlfcn.h>
#include <xcb/xcb.h>
#include <xcb/xcb_atom.h>
#include <xcb/xcb_icccm.h>

#elif defined( VK_USE_PLATFORM_XLIB_KHR )
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <cstdlib>
#include <dlfcn.h>

#endif

#include <journal.h>

#include "vulkan.h"

#include <chrono>
#include <optional>
#include <string>

namespace app {

    struct Application {
        xcb_connection_t* connection = nullptr;
        xcb_window_t window = 0;
        int32_t window_width = 1920;
        int32_t window_height = 1080;

        xcb_intern_atom_reply_t* protocols_reply;
        xcb_intern_atom_reply_t* atom_wm_delete_window;

        bool running = true;
        bool fullscreen = false;

        std::string_view title = "VkApplication";
    };

    constexpr char APP_TAG[] = "App";

    static inline auto intern_atom_helper( xcb_connection_t* conn, bool only_if_exists, std::string_view str ) {
        auto cookie = xcb_intern_atom( conn, only_if_exists, static_cast<uint16_t>( std::size( str ) ), std::data( str ) );
        return xcb_intern_atom_reply( conn, cookie, nullptr );
    }

    auto create_instance( ) noexcept -> std::optional<Application> {
        Application instance;
        int screen_index = 0;
        int32_t width = 0;
        int32_t height = 0;

        // Set up a window using XCB
        instance.connection = xcb_connect( nullptr, &screen_index );

        if ( !instance.connection )
            return { };

        const auto setup = xcb_get_setup( instance.connection );

        auto screen_iterator = xcb_setup_roots_iterator( setup );

        while ( screen_index-- > 0 ) {
            xcb_screen_next( &screen_iterator );
        }

        auto screen = screen_iterator.data;

        instance.window = xcb_generate_id( instance.connection );

        if ( instance.fullscreen ) {
            width = screen->width_in_pixels;
            height = screen->height_in_pixels;
        } else {
            width = instance.window_width;
            height = instance.window_height;
        }

        const uint32_t value_list[] = { screen->black_pixel,
            XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_STRUCTURE_NOTIFY
                | XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE };

        xcb_create_window( instance.connection, XCB_COPY_FROM_PARENT, instance.window, screen->root, 0, 0, width, height, 0,
            XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK, value_list );

        xcb_change_property( instance.connection, XCB_PROP_MODE_REPLACE, instance.window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8,
            std::size( instance.title ), std::data( instance.title ) );

        // Prepare notification for window destruction
        instance.protocols_reply = intern_atom_helper( instance.connection, true, "WM_PROTOCOLS" );
        instance.atom_wm_delete_window = intern_atom_helper( instance.connection, false, "WM_DELETE_WINDOW" );

        xcb_change_property( instance.connection, XCB_PROP_MODE_REPLACE, instance.window, ( *instance.protocols_reply ).atom, 4, 32, 1,
            &( *instance.atom_wm_delete_window ).atom );

        if ( instance.fullscreen ) {
            xcb_intern_atom_reply_t* atom_wm_state = intern_atom_helper( instance.connection, false, "_NET_WM_STATE" );
            xcb_intern_atom_reply_t* atom_wm_fullscreen = intern_atom_helper( instance.connection, false, "_NET_WM_STATE_FULLSCREEN" );
            xcb_change_property( instance.connection, XCB_PROP_MODE_REPLACE, instance.window, atom_wm_state->atom, XCB_ATOM_ATOM, 32, 1,
                &( atom_wm_fullscreen->atom ) );
            free( atom_wm_fullscreen );
            free( atom_wm_state );
        } else {
            xcb_size_hints_t hints;

            xcb_icccm_size_hints_set_min_size( &hints, width, height );
            xcb_icccm_size_hints_set_max_size( &hints, width, height );

            xcb_icccm_set_wm_size_hints( instance.connection, instance.window, XCB_ATOM_WM_NORMAL_HINTS, &hints );
        }

        // Display window
        xcb_map_window( instance.connection, instance.window );

        instance.window_width = width;
        instance.window_height = height;

        return { instance };
    }

    auto poll_events( Application& instance ) {
        xcb_generic_event_t* event;
        while ( ( event = xcb_poll_for_event( instance.connection ) ) ) {
            switch ( event->response_type & 0x7f ) {
            case XCB_CLIENT_MESSAGE: {
                if ( reinterpret_cast<xcb_client_message_event_t*>( event )->data.data32[0] == ( *instance.atom_wm_delete_window ).atom ) {
                    instance.running = false;
                    free( instance.atom_wm_delete_window );
                }
                break;
            }
            case XCB_MOTION_NOTIFY: {
                // const auto motion = reinterpret_cast<const xcb_motion_notify_event_t *>( event );
                break;
            }
            case XCB_BUTTON_PRESS: {
                // const auto press = reinterpret_cast<const xcb_button_press_event_t *>( event );
                break;
            }
            case XCB_BUTTON_RELEASE: {
                // const auto release = reinterpret_cast<const xcb_button_release_event_t *>( event );
                break;
            }
            case XCB_KEY_PRESS: {
                // const auto key_press = reinterpret_cast<const xcb_key_release_event_t *>( event );
                instance.running = false;
                break;
            }
            case XCB_KEY_RELEASE: {
                // const auto key_release = reinterpret_cast<const xcb_key_release_event_t *>( event );
                break;
            }
            case XCB_DESTROY_NOTIFY: {
                instance.running = false;
                break;
            }
            }

            free( event );
        }
    }

    auto destroy_instance( Application& instance ) noexcept -> void {

        xcb_destroy_window( instance.connection, instance.window );
        xcb_disconnect( instance.connection );

        instance.connection = nullptr;
        instance.window = 0;
    }

    auto on_initialize( ) -> void;
    auto on_update( ) -> void;
    auto on_finalize( ) -> void;

    auto run( int argc, char* arg[] ) -> int {
        auto app_instance = create_instance( );
        if ( !app_instance ) {
            LOG_ERROR( APP_TAG, "%1", "Couldn't create application instance" );
            return EXIT_FAILURE;
        }

        auto vk_instance = vulkan::init( app_instance.value( ).connection, app_instance.value( ).window );
        if ( !vk_instance ) {
            LOG_ERROR( APP_TAG, "%1", "Couldn't init vulkan" );
            return EXIT_FAILURE;
        }

        on_initialize( );

        while ( app_instance.value( ).running ) {
            poll_events( app_instance.value( ) );

            on_update( );

            vulkan::submit_and_present( vk_instance.value( ) );
        }

        on_finalize( );

        vulkan::cleanup( vk_instance.value( ) );

        destroy_instance( app_instance.value( ) );
    }

} // namespace app

inline volatile int log_level = DEFAULT_LOG_LEVEL;

int main( int argc, char* argv[] ) {
    return app::run( argc, argv );
}