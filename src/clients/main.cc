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

#include <vulkan/vulkan.h>
#include <xcb/xcb.h>

#include <config.hh>
#include <journal.h>

volatile int log_level = DEFAULT_LOG_LEVEL;

constexpr char APP_TAG[] = "App";
constexpr char VULKAN_TAG[] = "Vulkan";
constexpr char VULKAN_DBG_TAG[] = "Vulkan.Dbg";

#define LOG_DEBUG_CHECKPOINT( tag ) LOG_DEBUG( tag, "%1", __FUNCTION__ )

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
                                                             void *pUserData ) noexcept {

            LOG_ERROR( VULKAN_DBG_TAG, " %1 %2", pCallbackData->messageIdNumber, pCallbackData->pMessage );

            return VK_FALSE;
        }

        auto CreateDebugUtilsMessengerEXT( VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                           const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pCallback ) noexcept {
            auto vkCreateDebugUtilsMessenger_fp =
                (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr( instance, "vkCreateDebugUtilsMessengerEXT" );
            if ( vkCreateDebugUtilsMessenger_fp )
                return vkCreateDebugUtilsMessenger_fp( instance, pCreateInfo, pAllocator, pCallback );

            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }

        void DestroyDebugUtilsMessengerEXT( VkInstance instance, VkDebugUtilsMessengerEXT callback,
                                            const VkAllocationCallbacks *pAllocator ) noexcept {
            auto vkDestroyDebugUtilsMessenger_fp =
                (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr( instance, "vkDestroyDebugUtilsMessengerEXT" );
            if ( vkDestroyDebugUtilsMessenger_fp ) {
                vkDestroyDebugUtilsMessenger_fp( instance, callback, pAllocator );
            }
        }

        auto setup( const VkInstance instance ) noexcept {
            LOG_DEBUG_CHECKPOINT( VULKAN_DBG_TAG );

            VkDebugUtilsMessengerCreateInfoEXT create_info = {};
            create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                          VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                      VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            create_info.pfnUserCallback = debugCallback;
            create_info.pUserData = nullptr;

            const auto res = CreateDebugUtilsMessengerEXT( instance, &create_info, nullptr, &debug_utils_messenger_cb );

            return res == VK_SUCCESS;
        }

        auto cleanup( const VkInstance instance ) noexcept {
            LOG_DEBUG_CHECKPOINT( VULKAN_DBG_TAG );

            DestroyDebugUtilsMessengerEXT( instance, debug_utils_messenger_cb, nullptr );
        }
    } // namespace debugging

    constexpr const char *error_string( const VkResult code ) noexcept {
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

    [[nodiscard]] auto create_instance( const bool validation ) noexcept -> std::optional<VkInstance> {
        LOG_DEBUG_CHECKPOINT( VULKAN_TAG );

        VkInstance instance;

        VkApplicationInfo app_info = {};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = "Hello Triangle";
        app_info.applicationVersion = common::make_version<uint32_t>( 1, 0, 0 );
        app_info.pEngineName = "No Engine";
        app_info.engineVersion = common::make_version<uint32_t>( 1, 0, 0 );
        app_info.apiVersion = VK_API_VERSION_1_0;

        std::vector<const char *> extensions = {VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_XCB_SURFACE_EXTENSION_NAME};

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

        if ( result != VK_SUCCESS ) {
            LOG_ERROR( VULKAN_TAG, "%1", error_string( result ) );
            return {};
        }

        uint32_t supported_extension_count = 0;
        if ( vkEnumerateInstanceExtensionProperties( nullptr, &supported_extension_count, nullptr ) != VK_SUCCESS ) {
            return {};
        }

        std::vector<VkExtensionProperties> supported_extensions;
        supported_extensions.resize( supported_extension_count );
        if ( vkEnumerateInstanceExtensionProperties( nullptr, &supported_extension_count, std::data( supported_extensions ) ) !=
             VK_SUCCESS ) {
            return {};
        }

        return instance;
    }

    auto destroy_instance( const VkInstance instance ) noexcept {
        LOG_DEBUG_CHECKPOINT( VULKAN_TAG );

        vkDestroyInstance( instance, nullptr );
    }

    namespace detail {

        [[nodiscard]] auto check_extension_availability( std::string_view extension_name,
                                                         const std::vector<VkExtensionProperties> &available_extensions ) noexcept {
            for ( const auto &ext : available_extensions ) {
                if ( strcmp( ext.extensionName, extension_name.data( ) ) == 0 ) {
                    return true;
                }
            }

            return false;
        }

        [[nodiscard]] auto check_available_device_extensions( VkPhysicalDevice physical_device ) noexcept {
            LOG_DEBUG_CHECKPOINT( VULKAN_TAG );

            uint32_t extensions_count = 0;
            if ( ( vkEnumerateDeviceExtensionProperties( physical_device, nullptr, &extensions_count, nullptr ) != VK_SUCCESS ) ||
                 ( extensions_count == 0 ) )
                return false;

            std::vector<VkExtensionProperties> available_extensions;
            available_extensions.resize( extensions_count );

            if ( vkEnumerateDeviceExtensionProperties( physical_device, nullptr, &extensions_count, &available_extensions[0] ) !=
                 VK_SUCCESS )
                return false;

            std::vector<const char *> device_extensions_needed = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

            for ( auto ext : device_extensions_needed )
                if ( !check_extension_availability( ext, available_extensions ) )
                    return false;

            return true;
        }

        [[nodiscard]] auto enumerate_physical_devices( const VkInstance instance ) -> std::vector<VkPhysicalDevice> {
            LOG_DEBUG_CHECKPOINT( VULKAN_TAG );

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

        auto is_device_suitable( VkPhysicalDevice physical_device, VkSurfaceKHR presentation_surface,
                                 uint32_t &selected_graphics_queue_family_index, uint32_t &selected_present_queue_family_index ) {
            LOG_DEBUG_CHECKPOINT( VULKAN_TAG );

            if ( !check_available_device_extensions( physical_device ) ) {
                LOG_ERROR( VULKAN_TAG, "%1", "Not all needed extensions available" );
                return false;
            }

            VkPhysicalDeviceProperties device_properties;
            VkPhysicalDeviceFeatures device_features;
            VkPhysicalDeviceMemoryProperties device_memory_properties;

            vkGetPhysicalDeviceProperties( physical_device, &device_properties );
            vkGetPhysicalDeviceFeatures( physical_device, &device_features );
            vkGetPhysicalDeviceMemoryProperties( physical_device, &device_memory_properties );

            const auto major_version = VK_VERSION_MAJOR( device_properties.apiVersion );

            if ( ( major_version < 1 ) || ( device_properties.limits.maxImageDimension2D < 4096 ) ) {
                LOG_ERROR( VULKAN_TAG, "Physical device %1 doesn't support required parameters!", physical_device );
                return false;
            }

            // Discrete gpu only!
            //            if (const bool is_gpu = device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU; !is_gpu) {
            //                return true;
            //            }

            uint32_t queue_families_count = 0;
            vkGetPhysicalDeviceQueueFamilyProperties( physical_device, &queue_families_count, nullptr );

            if ( queue_families_count == 0 ) {
                LOG_ERROR( VULKAN_TAG, "Physical device %1 doesn't have any queue families! ", physical_device );
                return false;
            }

            std::vector<VkQueueFamilyProperties> queue_family_properties;
            queue_family_properties.resize( queue_families_count );
            std::vector<VkBool32> queue_present_support;
            queue_present_support.resize( queue_families_count );

            auto graphics_queue_family_index = UINT32_MAX;
            auto present_queue_family_index = UINT32_MAX;

            vkGetPhysicalDeviceQueueFamilyProperties( physical_device, &queue_families_count, std::data( queue_family_properties ) );

            auto idx = 0;
            for ( const auto &property : queue_family_properties ) {
                vkGetPhysicalDeviceSurfaceSupportKHR( physical_device, idx, presentation_surface, &queue_present_support[idx] );

                // Select first queue that supports graphics
                if ( property.queueCount > 0 && property.queueFlags & VK_QUEUE_GRAPHICS_BIT ) {
                    if ( graphics_queue_family_index == UINT32_MAX ) {
                        graphics_queue_family_index = idx;
                    }
                }

                // If there is queue that supports both graphics and present - prefer it
                if ( queue_present_support[idx] ) {
                    selected_graphics_queue_family_index = selected_present_queue_family_index = idx;
                    return true;
                }

                idx++;
            }

            // We don't have queue that supports both graphics and present so we have to use separate queues
            for ( uint32_t i = 0; i < queue_families_count; ++i ) {
                if ( queue_present_support[i] ) {
                    present_queue_family_index = i;
                    break;
                }
            }

            if ( ( graphics_queue_family_index == UINT32_MAX ) || ( present_queue_family_index == UINT32_MAX ) ) {
                LOG_ERROR( VULKAN_TAG, "Could not find queue families with required properties on physical device %1! ", physical_device );
                return false;
            }

            selected_graphics_queue_family_index = graphics_queue_family_index;
            selected_present_queue_family_index = present_queue_family_index;
            return true;
        }

        [[nodiscard]] auto pick_physical_device( const VkInstance instance, VkSurfaceKHR presentation_surface,
                                                 uint32_t &selected_graphics_queue_family_index,
                                                 uint32_t &selected_present_queue_family_index ) -> std::optional<VkPhysicalDevice> {
            LOG_DEBUG_CHECKPOINT( VULKAN_TAG );

            auto physical_devices = enumerate_physical_devices( instance );
            if ( physical_devices.empty( ) )
                return {};

            auto selected_device_index = 0;
            for ( const auto &device : physical_devices ) {
                if ( is_device_suitable( device, presentation_surface, selected_graphics_queue_family_index,
                                         selected_present_queue_family_index ) )
                    break;

                selected_device_index++;
            }

            if ( selected_device_index >= physical_devices.size( ) ) {
                LOG_ERROR( VULKAN_TAG, "%1", "Invalid device index" );
                return {};
            }

            const auto physical_device = physical_devices[selected_device_index];

            return physical_device;
        }

        [[nodiscard]] auto create_logical_device( VkPhysicalDevice physical_device, const uint32_t selected_graphics_queue_family_index,
                                                  const uint32_t selected_present_queue_family_index ) -> std::optional<VkDevice> {
            LOG_DEBUG_CHECKPOINT( VULKAN_TAG );

            std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
            const float queue_priorities[] = {1.0f};

            if ( selected_graphics_queue_family_index != UINT32_MAX ) {
                VkDeviceQueueCreateInfo queue_create_info = {};
                queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queue_create_info.queueFamilyIndex = selected_graphics_queue_family_index;
                queue_create_info.queueCount = std::size( queue_priorities );
                queue_create_info.pQueuePriorities = std::data( queue_priorities );

                queue_create_infos.push_back( queue_create_info );
            }

            if ( selected_graphics_queue_family_index != selected_present_queue_family_index ) {
                VkDeviceQueueCreateInfo queue_create_info = {};
                queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queue_create_info.queueFamilyIndex = selected_present_queue_family_index;
                queue_create_info.queueCount = std::size( queue_priorities );
                queue_create_info.pQueuePriorities = std::data( queue_priorities );

                queue_create_infos.push_back( queue_create_info );
            }

            std::vector<const char *> extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

            VkDeviceCreateInfo device_create_info = {};
            device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            device_create_info.pQueueCreateInfos = std::data( queue_create_infos );
            device_create_info.queueCreateInfoCount = std::size( queue_create_infos );
            device_create_info.ppEnabledExtensionNames = std::data( extensions );
            device_create_info.enabledExtensionCount = std::size( extensions );

            VkDevice device;

            if ( vkCreateDevice( physical_device, &device_create_info, nullptr, &device ) != VK_SUCCESS ) {
                LOG_ERROR( VULKAN_TAG, "%1", "Could not create vulkan device!" );
                return {};
            }

            return device;
        }

        auto destroy_logical_device( VkDevice device ) {
            LOG_DEBUG_CHECKPOINT( VULKAN_TAG );

            if ( const auto res = vkDeviceWaitIdle( device ); res != VK_SUCCESS )
                LOG_ERROR( VULKAN_TAG, "Couldn't wait device %1", error_string( res ) );

            vkDestroyDevice( device, nullptr );
        }

        auto create_swap_chain( VkPhysicalDevice physical_device, VkDevice device, VkSurfaceKHR presentation_surface ) {
            LOG_DEBUG_CHECKPOINT( VULKAN_TAG );

            if ( device != VK_NULL_HANDLE ) {
                vkDeviceWaitIdle( device );
            }

            VkSurfaceCapabilitiesKHR surface_capabilities;
            if ( const auto res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR( physical_device, presentation_surface, &surface_capabilities );
                 res != VK_SUCCESS ) {
                LOG_ERROR( VULKAN_TAG, "(%1) Could not check presentation surface capabilities!", error_string( res ) );

                return false;
            }

            uint32_t formats_count;
            if ( const auto res = vkGetPhysicalDeviceSurfaceFormatsKHR( physical_device, presentation_surface, &formats_count, nullptr );
                 ( res != VK_SUCCESS ) || ( formats_count == 0 ) ) {

                LOG_ERROR( VULKAN_TAG, "(%1) Error occurred during presentation surface formats enumeration!", error_string( res ) );

                return false;
            }

            return true;
        }

    } // namespace detail

    struct queue_parameters {
        VkQueue handle = VK_NULL_HANDLE;
        uint32_t family_index = 0;
    };

    struct context {
        context( ) = default;

        VkInstance instance = VK_NULL_HANDLE;
        VkPhysicalDevice physical_device = VK_NULL_HANDLE;
        VkDevice device = VK_NULL_HANDLE;
        queue_parameters graphics_queue = {};
        queue_parameters present_queue = {};
        VkSurfaceKHR presentation_surface;
        bool debugging = false;
    };

    [[nodiscard]] auto init( xcb_connection_t *connection, xcb_window_t window, const bool debugging_flag = true )
        -> std::optional<context> {
        LOG_DEBUG_CHECKPOINT( VULKAN_TAG );

        using namespace detail;

        context ctx;

        auto instance = create_instance( debugging_flag );
        if ( !instance )
            return {};

        if ( debugging_flag )
            if ( !debugging::setup( instance.value( ) ) )
                return {};

        ctx.instance = instance.value( );
        ctx.debugging = debugging_flag;

        VkXcbSurfaceCreateInfoKHR surface_create_info = {};
        surface_create_info.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
        surface_create_info.connection = connection;
        surface_create_info.window = window;

        if ( vkCreateXcbSurfaceKHR( ctx.instance, &surface_create_info, nullptr, &ctx.presentation_surface ) != VK_SUCCESS ) {
            LOG_ERROR( VULKAN_TAG, "%1", "Could not create presentation surface!" );
            return {};
        }

        auto selected_graphics_queue_family_index = UINT32_MAX;
        auto selected_present_queue_family_index = UINT32_MAX;
        auto selected_physical_device = pick_physical_device( ctx.instance, ctx.presentation_surface, selected_graphics_queue_family_index,
                                                              selected_present_queue_family_index );
        if ( !selected_physical_device ) {
            LOG_ERROR( VULKAN_TAG, "%1", "Could not select physical device based on the chosen properties!" );
            return {};
        }

        ctx.physical_device = selected_physical_device.value( );

        auto device =
            create_logical_device( ctx.physical_device, selected_graphics_queue_family_index, selected_present_queue_family_index );
        if ( !device )
            return {};

        ctx.device = device.value( );

        vkGetDeviceQueue( ctx.device, ctx.graphics_queue.family_index, 0, &ctx.graphics_queue.handle );
        vkGetDeviceQueue( ctx.device, ctx.present_queue.family_index, 0, &ctx.present_queue.handle );

        create_swap_chain( ctx.physical_device, ctx.device, ctx.presentation_surface );

        return ctx;
    }

    auto cleanup( context &ctx ) noexcept {
        LOG_DEBUG_CHECKPOINT( VULKAN_TAG );

        using namespace detail;

        if ( ctx.debugging )
            debugging::cleanup( ctx.instance );

        destroy_logical_device( ctx.device );

        destroy_instance( ctx.instance );
    }

} // namespace vulkan

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

namespace app {

#if defined( VK_USE_PLATFORM_WIN32_KHR )
#elif defined( VK_USE_PLATFORM_XCB_KHR )
#elif defined( VK_USE_PLATFORM_XLIB_KHR )
#endif

    typedef struct settings_type {
        settings_type( ) = default;

        bool validation = false;
        bool vsync = false;
        bool fullscreen = false;
        bool overlay = false;
    } settings_t;

    typedef struct instance_type {
        instance_type( ) = default;

        xcb_connection_t *connection = nullptr;
        xcb_window_t window = 0;

        xcb_intern_atom_reply_t *protocols_reply;
        xcb_intern_atom_reply_t *atom_wm_delete_window;

        settings_t settings;
    } instance_t;

    namespace syswm {} // namespace syswm

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

        struct on_present {
            on_present( ) = default;

            on_present( const float interpolation, const uint64_t timesteps ) : interpolation{interpolation}, timesteps{timesteps} {
            }

            float interpolation = 0.f;
            uint64_t timesteps = 0ull;
        };

        using event = std::variant<on_init, on_done, on_update, on_present>;

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

            xcb_change_property( instance.connection, XCB_PROP_MODE_REPLACE, instance.window, ( *instance.protocols_reply ).atom, 4, 32, 1,
                                 &( *instance.atom_wm_delete_window ).atom );

            if ( instance.settings.fullscreen ) {
                xcb_intern_atom_reply_t *atom_wm_state = intern_atom_helper( instance.connection, false, "_NET_WM_STATE" );
                xcb_intern_atom_reply_t *atom_wm_fullscreen = intern_atom_helper( instance.connection, false, "_NET_WM_STATE_FULLSCREEN" );
                xcb_change_property( instance.connection, XCB_PROP_MODE_REPLACE, instance.window, atom_wm_state->atom, XCB_ATOM_ATOM, 32, 1,
                                     &( atom_wm_fullscreen->atom ) );
                free( atom_wm_fullscreen );
                free( atom_wm_state );
            }

            // Display window
            xcb_map_window( instance.connection, instance.window );
            xcb_flush( instance.connection );

            return instance;
        }

        auto destroy_instance( instance_t &instance ) noexcept {
            LOG_DEBUG_CHECKPOINT( APP_TAG );

            xcb_destroy_window( instance.connection, instance.window );
            xcb_disconnect( instance.connection );

            instance.connection = nullptr;
            instance.window = 0;
        }

    } // namespace detail

    auto run( std::function<void( const detail::event & )> on_event ) {
        LOG_INFO( APP_TAG, "%1", "Start running" );

        auto instance = detail::create_instance( );
        if ( !instance ) {
            LOG_ERROR( APP_TAG, "%1", "Couldn't create application instance" );
            return EXIT_FAILURE;
        }

        auto vk_ctx = vulkan::init( instance.value( ).connection, instance.value( ).window );
        if ( !vk_ctx ) {
            LOG_ERROR( APP_TAG, "%1", "Couldn't init vulkan" );
            return EXIT_FAILURE;
        }

        on_event( detail::on_init{} );

        constexpr auto timestep = 0.005f;
        auto current_time = std::chrono::high_resolution_clock::now( );
        auto last_time = current_time;
        auto dt_accumulator = 0.0;
        auto timesteps = 0ull;

        // Main message loop
        bool running = true;
        while ( running ) {
            xcb_generic_event_t *event;
            while ( event = xcb_poll_for_event( instance.value( ).connection ) ) {
                switch ( event->response_type & 0x7f ) {
                case XCB_CLIENT_MESSAGE: {
                    if ( ( *(xcb_client_message_event_t *)event ).data.data32[0] == ( *instance.value( ).atom_wm_delete_window ).atom ) {
                        running = false;
                        free( instance.value( ).atom_wm_delete_window );
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
                    running = false;
                    break;
                }
                case XCB_KEY_RELEASE: {
                    const auto key_release = reinterpret_cast<const xcb_key_release_event_t *>( event );
                    break;
                }
                case XCB_DESTROY_NOTIFY: {
                    running = false;
                    break;
                }
                }

                free( event );
            }

            last_time = current_time;
            current_time = std::chrono::high_resolution_clock::now( );
            const auto diff_time = std::chrono::duration<double, std::milli>( current_time - last_time ).count( );
            dt_accumulator += std::clamp( diff_time, 0.0, 0.2 );

            while ( dt_accumulator >= timestep ) {
                dt_accumulator -= timestep;

                on_event( detail::on_update{} );

                timesteps++;
            }

            on_event( detail::on_present{dt_accumulator / timestep, timesteps} );
        }

        on_event( detail::on_done{} );

        vulkan::cleanup( vk_ctx.value( ) );

        detail::destroy_instance( instance.value( ) );

        LOG_INFO( APP_TAG, "%1", "App exit" );

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
                    // LOG_INFO( APP_TAG, "%1", "App update" );
                } else if constexpr ( std::is_same_v<T, app::detail::on_present> ) {
                    // LOG_INFO( APP_TAG, "App present %1 %2", arg.interpolation, arg.timesteps );
                } else if constexpr ( std::is_same_v<T, app::detail::on_done> ) {
                    LOG_INFO( APP_TAG, "%1", "App done" );
                }
            },
            ev );
    } );
}
