#include <cstring>

#include <config.hh>
#include <journal.h>

#include "syswm.h"

namespace app {

    namespace syswm {

        namespace detail {
            static inline auto intern_atom_helper( xcb_connection_t *conn, bool only_if_exists, const char *str ) {
                auto cookie = xcb_intern_atom( conn, only_if_exists, strlen( str ), str );
                return xcb_intern_atom_reply( conn, cookie, nullptr );
            }

            auto create_instance( ) noexcept -> std::optional<instance_t> {
                LOG_DEBUG_CHECKPOINT( APP_TAG );

                instance_t instance;
                int screen_index;

                // Set up a window using XCB
                instance.connection = xcb_connect( nullptr, &screen_index );

                if ( !instance.connection )
                    return {};

                const auto setup = xcb_get_setup( instance.connection );

                auto screen_iterator = xcb_setup_roots_iterator( setup );

                while ( screen_index-- > 0 ) {
                    xcb_screen_next( &screen_iterator );
                }

                auto screen = screen_iterator.data;

                instance.window = xcb_generate_id( instance.connection );

                const uint32_t value_list[] = {screen->black_pixel, XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_KEY_PRESS |
                                                                        XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_STRUCTURE_NOTIFY |
                                                                        XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_BUTTON_PRESS |
                                                                        XCB_EVENT_MASK_BUTTON_RELEASE};

                xcb_create_window( instance.connection, XCB_COPY_FROM_PARENT, instance.window, screen->root, 0, 0, 500, 500, 0,
                                   XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK, value_list );

                xcb_flush( instance.connection );

                xcb_change_property( instance.connection, XCB_PROP_MODE_REPLACE, instance.window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8,
                                     strlen( WINDOW_TITLE ), WINDOW_TITLE );

                // Prepare notification for window destruction
                instance.protocols_reply = intern_atom_helper( instance.connection, true, "WM_PROTOCOLS" );
                instance.atom_wm_delete_window = intern_atom_helper( instance.connection, false, "WM_DELETE_WINDOW" );

                xcb_change_property( instance.connection, XCB_PROP_MODE_REPLACE, instance.window, ( *instance.protocols_reply ).atom, 4, 32,
                                     1, &( *instance.atom_wm_delete_window ).atom );

                if ( instance.settings.fullscreen ) {
                    xcb_intern_atom_reply_t *atom_wm_state = intern_atom_helper( instance.connection, false, "_NET_WM_STATE" );
                    xcb_intern_atom_reply_t *atom_wm_fullscreen =
                        intern_atom_helper( instance.connection, false, "_NET_WM_STATE_FULLSCREEN" );
                    xcb_change_property( instance.connection, XCB_PROP_MODE_REPLACE, instance.window, atom_wm_state->atom, XCB_ATOM_ATOM,
                                         32, 1, &( atom_wm_fullscreen->atom ) );
                    free( atom_wm_fullscreen );
                    free( atom_wm_state );
                }

                // Display window
                xcb_map_window( instance.connection, instance.window );
                xcb_flush( instance.connection );

                return instance;
            }

            auto destroy_instance( instance_t &instance ) noexcept -> void {
                LOG_DEBUG_CHECKPOINT( APP_TAG );

                xcb_destroy_window( instance.connection, instance.window );
                xcb_disconnect( instance.connection );

                instance.connection = nullptr;
                instance.window = 0;
            }

            auto poll_events( instance_t &instance ) noexcept -> void {
                xcb_generic_event_t *event;
                while ( event = xcb_poll_for_event( instance.connection ) ) {
                    switch ( event->response_type & 0x7f ) {
                    case XCB_CLIENT_MESSAGE: {
                        if ( ( *(xcb_client_message_event_t *)event ).data.data32[0] ==
                             ( *instance.atom_wm_delete_window ).atom ) {
                            instance.running = false;
                            free( instance.atom_wm_delete_window );
                        }
                        break;
                    }
                    case XCB_MOTION_NOTIFY: {
                        const auto motion = reinterpret_cast<const xcb_motion_notify_event_t *>( event );
                        break;
                    }
                    case XCB_BUTTON_PRESS: {
                        const auto press = reinterpret_cast<const xcb_button_press_event_t *>( event );
                        break;
                    }
                    case XCB_BUTTON_RELEASE: {
                        const auto release = reinterpret_cast<const xcb_button_release_event_t *>( event );
                        break;
                    }
                    case XCB_KEY_PRESS: {
                        const auto key_press = reinterpret_cast<const xcb_key_release_event_t *>( event );
                        instance.running = false;
                        break;
                    }
                    case XCB_KEY_RELEASE: {
                        const auto key_release = reinterpret_cast<const xcb_key_release_event_t *>( event );
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

        } // namespace detail

    } // namespace syswm

} // namespace app
