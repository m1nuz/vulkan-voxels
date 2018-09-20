#include <array>
#include <functional>
#include <iterator>
#include <optional>
#include <variant>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

#include <vulkan/vulkan.h>
#include <xcb/xcb.h>

#include <config.hh>
#include <journal.h>

volatile int log_level = DEFAULT_LOG_LEVEL;

constexpr char APP_TAG[] = "App";
constexpr char VULKAN_TAG[] = "Vulkan";
constexpr char VULKAN_DBG_TAG[] = "Vulkan.Dbg";

#define LOG_DEBUG_CHECKPOINT( tag ) LOG_DEBUG( tag, "%1", __FUNCTION__ )

namespace app {

    namespace common {

        template <typename T> constexpr inline auto make_version( auto major, auto minor, auto patch ) -> T {
            return ( ( ( major ) << 22 ) | ( ( minor ) << 12 ) | ( patch ) );
        }

    } // namespace common

    namespace vulkan {

        namespace debugging {

            const char *validation_layer_names[] = {"VK_LAYER_LUNARG_standard_validation"};

            VkDebugUtilsMessengerEXT debug_utils_messenger_cb;

            static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                                 VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                                 const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                                 void *pUserData ) {

                LOG_ERROR( VULKAN_DBG_TAG, " %1 %2", pCallbackData->messageIdNumber, pCallbackData->pMessage );

                return VK_FALSE;
            }

            auto CreateDebugUtilsMessengerEXT( VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                               const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pCallback ) {
                auto vkCreateDebugUtilsMessenger_fp =
                    (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr( instance, "vkCreateDebugUtilsMessengerEXT" );
                if ( vkCreateDebugUtilsMessenger_fp )
                    return vkCreateDebugUtilsMessenger_fp( instance, pCreateInfo, pAllocator, pCallback );

                return VK_ERROR_EXTENSION_NOT_PRESENT;
            }

            void DestroyDebugUtilsMessengerEXT( VkInstance instance, VkDebugUtilsMessengerEXT callback,
                                                const VkAllocationCallbacks *pAllocator ) {
                auto vkDestroyDebugUtilsMessenger_fp =
                    (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr( instance, "vkDestroyDebugUtilsMessengerEXT" );
                if ( vkDestroyDebugUtilsMessenger_fp ) {
                    vkDestroyDebugUtilsMessenger_fp( instance, callback, pAllocator );
                }
            }

            auto setup( const VkInstance instance ) {
                LOG_DEBUG_CHECKPOINT( VULKAN_DBG_TAG );

                VkDebugUtilsMessengerCreateInfoEXT create_info = {};
                create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
                create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                              VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                              VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
                create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                          VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
                create_info.pfnUserCallback = debugCallback;
                create_info.pUserData = nullptr;

                const auto res = CreateDebugUtilsMessengerEXT( instance, &create_info, nullptr, &debug_utils_messenger_cb );

                return res == VK_SUCCESS;
            }

            auto cleanup( const VkInstance instance ) {
                LOG_DEBUG_CHECKPOINT( VULKAN_DBG_TAG );

                DestroyDebugUtilsMessengerEXT( instance, debug_utils_messenger_cb, nullptr );
            }
        } // namespace debugging

        constexpr const char *error_string( const VkResult code ) {
            switch ( code ) {
#define STR( r )                                                                                                                           \
    case VK_##r:                                                                                                                           \
        return #r
                STR( NOT_READY );
                STR( TIMEOUT );
                STR( EVENT_SET );
                STR( EVENT_RESET );
                STR( INCOMPLETE );
                STR( ERROR_OUT_OF_HOST_MEMORY );
                STR( ERROR_OUT_OF_DEVICE_MEMORY );
                STR( ERROR_INITIALIZATION_FAILED );
                STR( ERROR_DEVICE_LOST );
                STR( ERROR_MEMORY_MAP_FAILED );
                STR( ERROR_LAYER_NOT_PRESENT );
                STR( ERROR_EXTENSION_NOT_PRESENT );
                STR( ERROR_FEATURE_NOT_PRESENT );
                STR( ERROR_INCOMPATIBLE_DRIVER );
                STR( ERROR_TOO_MANY_OBJECTS );
                STR( ERROR_FORMAT_NOT_SUPPORTED );
                STR( ERROR_SURFACE_LOST_KHR );
                STR( ERROR_NATIVE_WINDOW_IN_USE_KHR );
                STR( SUBOPTIMAL_KHR );
                STR( ERROR_OUT_OF_DATE_KHR );
                STR( ERROR_INCOMPATIBLE_DISPLAY_KHR );
                STR( ERROR_VALIDATION_FAILED_EXT );
                STR( ERROR_INVALID_SHADER_NV );
#undef STR
            default:
                return "UNKNOWN_ERROR";
            }
        }

        [[nodiscard]] auto create_instance( const bool validation ) -> std::optional<VkInstance> {
            LOG_DEBUG_CHECKPOINT( VULKAN_TAG );

            VkInstance instance;

            VkApplicationInfo app_info = {};
            app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            app_info.pApplicationName = "Hello Triangle";
            app_info.applicationVersion = common::make_version<uint32_t>( 1, 0, 0 );
            app_info.pEngineName = "No Engine";
            app_info.engineVersion = common::make_version<uint32_t>( 1, 0, 0 );
            app_info.apiVersion = VK_API_VERSION_1_0;

            std::vector<const char *> extensions = {VK_KHR_XCB_SURFACE_EXTENSION_NAME};

            if ( validation )
                extensions.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );

            VkInstanceCreateInfo create_info = {};
            create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            create_info.pApplicationInfo = &app_info;
            create_info.enabledExtensionCount = std::size( extensions );
            create_info.ppEnabledExtensionNames = std::data( extensions );
            create_info.enabledLayerCount = std::size( debugging::validation_layer_names );
            create_info.ppEnabledLayerNames = debugging::validation_layer_names;

            const auto result = vkCreateInstance( &create_info, nullptr, &instance );

            if ( result == VK_SUCCESS )
                return instance;

            LOG_ERROR( VULKAN_TAG, "%1", error_string( result ) );

            return {};
        }

        auto destroy_instance( const VkInstance instance ) {
            LOG_DEBUG_CHECKPOINT( VULKAN_TAG );

            vkDestroyInstance( instance, nullptr );
        }

        namespace detail {

            [[nodiscard]] auto enumerate_physical_devices( const VkInstance instance ) -> std::vector<VkPhysicalDevice> {
                uint32_t gpu_count = 0;
                const auto check_count_result = vkEnumeratePhysicalDevices( instance, &gpu_count, nullptr );

                if ( check_count_result != VK_SUCCESS ) {
                    LOG_ERROR( VULKAN_TAG, "%1", error_string( check_count_result ) );
                    return {};
                }

                std::vector<VkPhysicalDevice> devices;
                devices.resize( gpu_count );

                const auto devices_result = vkEnumeratePhysicalDevices( instance, &gpu_count, std::data( devices ) );

                if ( devices_result != VK_SUCCESS ) {
                    LOG_ERROR( VULKAN_TAG, "%1", error_string( check_count_result ) );
                    return {};
                }

                return devices;
            }

            auto is_device_suitable( VkPhysicalDevice device ) {
                VkPhysicalDeviceProperties device_properties;
                VkPhysicalDeviceFeatures device_features;
                VkPhysicalDeviceMemoryProperties device_memory_properties;

                vkGetPhysicalDeviceProperties( device, &device_properties );
                vkGetPhysicalDeviceFeatures( device, &device_features );
                vkGetPhysicalDeviceMemoryProperties( device, &device_memory_properties );

                const bool is_gpu = device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;

                uint32_t queue_family_count = 0;
                vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

                std::vector<VkQueueFamilyProperties> queue_families;
                queue_families.resize( queue_family_count );
                vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, std::data( queue_families ));

                bool is_present_graphics = false;
                auto queue_family_index = 0u;
                auto idx = 0u;
                for ( const auto& queue_family : queue_families ) {
                    if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                        is_present_graphics = true;
                        queue_family_index = idx;
                    }

                    idx++;
                }

                return is_gpu & is_present_graphics;
            }

            [[nodiscard]] auto pick_physical_device( const VkInstance instance ) {
                LOG_DEBUG_CHECKPOINT( VULKAN_TAG );

                auto physical_devices = enumerate_physical_devices( instance );
                if ( physical_devices.empty( ) )
                    return false;

                auto selected_device_index = 0u;
                for (const auto& device : physical_devices) {
                    if ( is_device_suitable( device ) )
                        break;

                    selected_device_index++;
                }

                if (selected_device_index > physical_devices.size()) {
                    LOG_ERROR( VULKAN_TAG, "%1", "Invalid device index" );
                    return false;
                }

                auto selected_device = physical_devices[selected_device_index];

                return true;
            }

        } // namespace detail

        struct context {
            context() = default;

            VkInstance instance;
            bool debugging = false;
        };

        [[nodiscard]] auto init( const bool debugging_flag = true ) -> std::optional<context> {
            using namespace detail;

            context ctx;

            auto instance = create_instance( debugging_flag );
            if ( !instance )
                return {};

            if ( debugging_flag )
                if ( !debugging::setup( instance.value( ) ) )
                    return {};

            if ( !pick_physical_device( instance.value( ) ) )
                return {};

            return ctx;
        }

        auto cleanup( context& ctx) {
            if ( ctx.debugging )
                vulkan::debugging::cleanup( ctx.instance );

            vulkan::destroy_instance( ctx.instance );
        }

    } // namespace vulkan

    namespace detail {
        struct on_init {
            on_init( ) = default;
        };

        struct on_done {
            on_done( ) = default;
        };

        struct on_update {
            on_update( ) = default;
        };

        using event = std::variant<on_init, on_done, on_update>;
    } // namespace detail

    auto run( std::function<void( const detail::event & )> on_event ) {
        LOG_DEBUG( APP_TAG, "%1", "Start running" );

        auto vk_ctx = vulkan::init();
        if (!vk_ctx)
            return EXIT_FAILURE;

        on_event(detail::on_init{});

        on_event(detail::on_done{});

        vulkan::cleanup(vk_ctx.value());

        return EXIT_SUCCESS;
    }

} // namespace app

extern int main( int argc, char *argv[] ) {

    return app::run( []( const auto &ev ) {
        std::visit(
            []( auto &&arg ) {
                using T = std::decay_t<decltype( arg )>;

                if constexpr ( std::is_same_v<T, app::detail::on_init> ) {
                    LOG_INFO( APP_TAG, "%1", "App init" );
                } else if constexpr ( std::is_same_v<T, app::detail::on_update> ) {
                    LOG_INFO( APP_TAG, "%1", "App update" );
                } else if constexpr ( std::is_same_v<T, app::detail::on_done> ) {
                    LOG_INFO( APP_TAG, "%1", "App done" );
                }
            },
            ev );
    } );
}
