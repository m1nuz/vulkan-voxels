#include <algorithm>
#include <array>
#include <chrono>
#include <functional>
#include <iterator>
#include <optional>
#include <variant>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

#include <journal.h>

#include "syswm.h"
#include "vulkan.h"

volatile int log_level = DEFAULT_LOG_LEVEL;

namespace app {

    auto run( std::function<void( const event_t & )> on_event ) {
        LOG_INFO( APP_TAG, "%1", "Startup" );

        auto display_instance = syswm::create_instance( );
        if ( !display_instance ) {
            LOG_ERROR( APP_TAG, "%1", "Couldn't create application instance" );
            return EXIT_FAILURE;
        }

        auto vk_instance = vulkan::init( display_instance.value( ).connection, display_instance.value( ).window );
        if ( !vk_instance ) {
            LOG_ERROR( APP_TAG, "%1", "Couldn't init vulkan" );
            return EXIT_FAILURE;
        }

        on_event( syswm::on_init{} );

        constexpr auto timestep = 0.005;
        auto current_time = std::chrono::high_resolution_clock::now( );
        auto last_time = current_time;
        auto dt_accumulator = 0.0;
        auto timesteps = 0ull;

        LOG_INFO( APP_TAG, "%1", "Running..." );

        // Main message loop
        while ( display_instance.value().running ) {
            syswm::poll_events( display_instance.value() );

            last_time = current_time;
            current_time = std::chrono::high_resolution_clock::now( );
            const auto diff_time = std::chrono::duration<double, std::milli>( current_time - last_time ).count( );
            dt_accumulator += std::clamp( diff_time, 0.0, 0.2 );

            while ( dt_accumulator >= timestep ) {
                dt_accumulator -= timestep;

                on_event( syswm::on_update{} );

                timesteps++;
            }

            const auto interpolation = dt_accumulator / timestep;

            on_event( syswm::on_present{static_cast<float>( interpolation ), timesteps} );

            vulkan::submit_and_present( vk_instance.value( ) );
        }

        on_event( syswm::on_done{} );

        vulkan::cleanup( vk_instance.value( ) );

        syswm::destroy_instance( display_instance.value( ) );

        LOG_INFO( APP_TAG, "%1", "Exit" );

        return EXIT_SUCCESS;
    }

} // namespace app

extern int main( int argc, char *argv[] ) {
    (void)argc, (void)argv;

    return app::run( []( const auto &ev ) {
        std::visit(
            []( auto &&arg ) {
                using T = std::decay_t<decltype( arg )>;

                if constexpr ( std::is_same_v<T, app::syswm::on_init> ) {
                    LOG_INFO( app::APP_TAG, "%1", "Init" );
                } else if constexpr ( std::is_same_v<T, app::syswm::on_update> ) {
                    // LOG_INFO( APP_TAG, "%1", "Update" );
                } else if constexpr ( std::is_same_v<T, app::syswm::on_present> ) {
                    // LOG_INFO( APP_TAG, "App present %1 %2", arg.interpolation, arg.timesteps );
                } else if constexpr ( std::is_same_v<T, app::syswm::on_done> ) {
                    LOG_INFO( app::APP_TAG, "%1", "Done" );
                }
            },
            ev );
    } );
}
