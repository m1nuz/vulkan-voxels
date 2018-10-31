#pragma once

#include <cstring>

#include <vulkan/vulkan.h>

constexpr char VULKAN_TAG[] = "Vulkan";
constexpr char VULKAN_DBG_TAG[] = "Vulkan.Dbg";

namespace common {

    template <typename T> constexpr inline auto make_version( const T major, const T minor, const T patch ) -> T {
        return ( ( ( major ) << 22 ) | ( ( minor ) << 12 ) | ( patch ) );
    }

} // namespace common

namespace vulkan {

    struct queue_parameters {
        VkQueue handle = VK_NULL_HANDLE;
        uint32_t family_index = 0;
    };

    struct image_info {
        VkImage image = VK_NULL_HANDLE;
        VkImageView image_view = VK_NULL_HANDLE;
        VkSampler sampler = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
    };

    struct swap_chain_info {
        VkSwapchainKHR swap_chain = VK_NULL_HANDLE;
        VkFormat format = VK_FORMAT_UNDEFINED;
        std::vector<image_info> images;
        VkExtent2D extent = {};
    };

    struct context {
        context( ) = default;

        VkInstance instance = VK_NULL_HANDLE;
        VkPhysicalDevice physical_device = VK_NULL_HANDLE;
        VkDevice device = VK_NULL_HANDLE;
        queue_parameters graphics_queue = {};
        queue_parameters present_queue = {};
        VkSurfaceKHR presentation_surface = VK_NULL_HANDLE;
        VkCommandPool present_queue_command_pool = VK_NULL_HANDLE;
        std::vector<VkCommandBuffer> present_queue_command_buffers;
        swap_chain_info swap_chain = {};
        VkSemaphore image_available;
        VkSemaphore rendering_finished;
        bool is_debugging = false;
    };

    namespace debugging {

        const char *validation_layer_names[] = {"VK_LAYER_LUNARG_standard_validation"};

        VkDebugUtilsMessengerEXT debug_utils_messenger_cb;

        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                             VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                             const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                             void *pUserData ) noexcept {
            (void)messageSeverity, (void)messageType, (void)pUserData;

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
                if ( std::strcmp( ext.extensionName, extension_name.data( ) ) == 0 ) {
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

            auto selected_device_index = 0u;
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

        auto destroy_logical_device( VkDevice device ) noexcept {
            LOG_DEBUG_CHECKPOINT( VULKAN_TAG );

            if ( const auto res = vkDeviceWaitIdle( device ); res != VK_SUCCESS )
                LOG_ERROR( VULKAN_TAG, "Couldn't wait device %1", error_string( res ) );

            vkDestroyDevice( device, nullptr );
        }

        inline auto get_swap_chain_num_images( VkSurfaceCapabilitiesKHR &surface_capabilities ) {
            uint32_t image_count = surface_capabilities.minImageCount + 2;
            if ( ( surface_capabilities.maxImageCount > 0 ) && ( image_count > surface_capabilities.maxImageCount ) ) {
                image_count = surface_capabilities.maxImageCount;
            }

            return image_count;
        }

        inline auto get_swap_chain_format( std::vector<VkSurfaceFormatKHR> &surface_formats ) {
            if ( ( surface_formats.size( ) == 1 ) && ( surface_formats[0].format == VK_FORMAT_UNDEFINED ) ) {
                return VkSurfaceFormatKHR{VK_FORMAT_R8G8B8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR};
            }

            for ( VkSurfaceFormatKHR &surface_format : surface_formats ) {
                if ( surface_format.format == VK_FORMAT_R8G8B8A8_UNORM ) {
                    return surface_format;
                }
            }

            return surface_formats[0];
        }

        inline auto get_swap_chain_extent( VkSurfaceCapabilitiesKHR &surface_capabilities ) {
            if ( surface_capabilities.currentExtent.width == static_cast<uint32_t>( -1 ) ) {
                VkExtent2D swap_chain_extent = {640, 480};
                if ( swap_chain_extent.width < surface_capabilities.minImageExtent.width ) {
                    swap_chain_extent.width = surface_capabilities.minImageExtent.width;
                }
                if ( swap_chain_extent.height < surface_capabilities.minImageExtent.height ) {
                    swap_chain_extent.height = surface_capabilities.minImageExtent.height;
                }
                if ( swap_chain_extent.width > surface_capabilities.maxImageExtent.width ) {
                    swap_chain_extent.width = surface_capabilities.maxImageExtent.width;
                }
                if ( swap_chain_extent.height > surface_capabilities.maxImageExtent.height ) {
                    swap_chain_extent.height = surface_capabilities.maxImageExtent.height;
                }
                return swap_chain_extent;
            }

            return surface_capabilities.currentExtent;
        }

        inline auto get_swap_chain_usage_flags( VkSurfaceCapabilitiesKHR &surface_capabilities ) {
            if ( surface_capabilities.supportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT ) {
                return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            }

            LOG_ERROR( VULKAN_TAG, "%1", "VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT image usage is not supported by the swap chain!" );

            return static_cast<VkImageUsageFlagBits>( -1 );
        }

        inline auto get_swap_chain_transform( VkSurfaceCapabilitiesKHR &surface_capabilities ) {
            if ( surface_capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR ) {
                return VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
            } else {
                return surface_capabilities.currentTransform;
            }
        }

        inline auto get_swap_chain_present_mode( const std::vector<VkPresentModeKHR> &present_modes ) {
            for ( const auto &present_mode : present_modes ) {
                if ( present_mode == VK_PRESENT_MODE_MAILBOX_KHR ) {
                    return present_mode;
                }
            }

            for ( const auto &present_mode : present_modes ) {
                if ( present_mode == VK_PRESENT_MODE_IMMEDIATE_KHR ) {
                    return present_mode;
                }
            }

            for ( const auto &present_mode : present_modes ) {
                if ( present_mode == VK_PRESENT_MODE_FIFO_KHR ) {
                    return present_mode;
                }
            }

            LOG_ERROR( VULKAN_TAG, "%1", "FIFO present mode is not supported by the swap chain!" );

            return static_cast<VkPresentModeKHR>( -1 );
        }

        auto create_swap_chain_image_views( VkDevice device, const VkFormat format, const std::vector<VkImage> &images )
            -> std::vector<image_info> {
            if ( images.empty( ) )
                return {};

            std::vector<image_info> image_infos;
            image_infos.resize( images.size( ) );

            size_t ix = 0;
            for ( const auto img : images ) {
                image_infos[ix].image = img;

                VkImageViewCreateInfo image_view_create_info = {};
                image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                image_view_create_info.pNext = nullptr;
                image_view_create_info.flags = 0;
                image_view_create_info.image = images[ix];
                image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
                image_view_create_info.format = format;
                image_view_create_info.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                                     VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};
                image_view_create_info.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

                if ( const auto res = vkCreateImageView( device, &image_view_create_info, nullptr, &image_infos[ix].image_view );
                     res != VK_SUCCESS ) {
                    LOG_ERROR( VULKAN_TAG, "%1 Could not create image view for framebuffer!", res );
                }

                ix++;
            }

            return image_infos;
        }

        [[nodiscard]] auto create_swap_chain( VkPhysicalDevice physical_device, VkDevice device, VkSurfaceKHR presentation_surface,
                                              VkSwapchainKHR swap_chain ) -> std::optional<swap_chain_info> {
            LOG_DEBUG_CHECKPOINT( VULKAN_TAG );

            if ( device != VK_NULL_HANDLE ) {
                vkDeviceWaitIdle( device );
            }

            VkSurfaceCapabilitiesKHR surface_capabilities;
            if ( const auto res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR( physical_device, presentation_surface, &surface_capabilities );
                 res != VK_SUCCESS ) {
                LOG_ERROR( VULKAN_TAG, "(%1) Could not check presentation surface capabilities!", error_string( res ) );

                return {};
            }

            uint32_t formats_count;
            if ( const auto res = vkGetPhysicalDeviceSurfaceFormatsKHR( physical_device, presentation_surface, &formats_count, nullptr );
                 ( res != VK_SUCCESS ) || ( formats_count == 0 ) ) {

                LOG_ERROR( VULKAN_TAG, "(%1) Error occurred during presentation surface formats enumeration!", error_string( res ) );

                return {};
            }

            std::vector<VkSurfaceFormatKHR> surface_formats;
            surface_formats.resize( formats_count );
            if ( const auto res =
                     vkGetPhysicalDeviceSurfaceFormatsKHR( physical_device, presentation_surface, &formats_count, surface_formats.data( ) );
                 res != VK_SUCCESS ) {
                LOG_ERROR( VULKAN_TAG, "(%1) Error occurred during presentation surface formats enumeration!", error_string( res ) );
                return {};
            }

            uint32_t present_modes_count;
            if ( const auto res =
                     vkGetPhysicalDeviceSurfacePresentModesKHR( physical_device, presentation_surface, &present_modes_count, nullptr );
                 ( res != VK_SUCCESS ) || ( present_modes_count == 0 ) ) {
                LOG_ERROR( VULKAN_TAG, "(%1) Error occurred during presentation surface present modes enumeration!", error_string( res ) );
                return {};
            }

            std::vector<VkPresentModeKHR> present_modes;
            present_modes.resize( present_modes_count );

            if ( const auto res = vkGetPhysicalDeviceSurfacePresentModesKHR( physical_device, presentation_surface, &present_modes_count,
                                                                             present_modes.data( ) );
                 res != VK_SUCCESS ) {
                LOG_ERROR( VULKAN_TAG, "(%1) Error occurred during presentation surface present modes enumeration!", error_string( res ) );
                return {};
            }

            const auto desired_number_of_images = get_swap_chain_num_images( surface_capabilities );
            const auto desired_format = get_swap_chain_format( surface_formats );
            const auto desired_extent = get_swap_chain_extent( surface_capabilities );
            const auto desired_usage = get_swap_chain_usage_flags( surface_capabilities );
            const auto desired_transform = get_swap_chain_transform( surface_capabilities );
            const auto desired_present_mode = get_swap_chain_present_mode( present_modes );

            if ( static_cast<int>( desired_usage ) == -1 ) {
                return {};
            }

            if ( static_cast<int>( desired_present_mode ) == -1 ) {
                return {};
            }

            if ( ( desired_extent.width == 0 ) || ( desired_extent.height == 0 ) ) {
                LOG_ERROR( VULKAN_TAG, "%1", "Can't create swap chain" );
                return {};
            }

            auto old_swap_chain = swap_chain;

            VkSwapchainCreateInfoKHR swap_chain_create_info;
            swap_chain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            swap_chain_create_info.pNext = nullptr;
            swap_chain_create_info.flags = 0;
            swap_chain_create_info.surface = presentation_surface;
            swap_chain_create_info.minImageCount = desired_number_of_images;
            swap_chain_create_info.imageFormat = desired_format.format;
            swap_chain_create_info.imageColorSpace = desired_format.colorSpace;
            swap_chain_create_info.imageExtent = desired_extent;
            swap_chain_create_info.imageArrayLayers = 1;
            swap_chain_create_info.imageUsage = desired_usage;
            swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            swap_chain_create_info.queueFamilyIndexCount = 0;
            swap_chain_create_info.pQueueFamilyIndices = nullptr;
            swap_chain_create_info.preTransform = desired_transform;
            swap_chain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            swap_chain_create_info.presentMode = desired_present_mode;
            swap_chain_create_info.clipped = VK_TRUE;
            swap_chain_create_info.oldSwapchain = old_swap_chain;

            if ( surface_capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT ) {
                swap_chain_create_info.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
            }

            if ( surface_capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT ) {
                swap_chain_create_info.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            }

            if ( vkCreateSwapchainKHR( device, &swap_chain_create_info, nullptr, &swap_chain ) != VK_SUCCESS ) {
                LOG_ERROR( VULKAN_TAG, "%1", "Could not create swap chain!" );
                return {};
            }

            if ( old_swap_chain != VK_NULL_HANDLE ) {
                vkDestroySwapchainKHR( device, old_swap_chain, nullptr );
            }

            uint32_t image_count = 0;
            if ( const auto res = vkGetSwapchainImagesKHR( device, swap_chain, &image_count, nullptr );
                 ( res != VK_SUCCESS ) || ( image_count == 0 ) ) {
                LOG_ERROR( VULKAN_TAG, "%1", "Could not get swap chain images!" );
                return {};
            }

            std::vector<VkImage> images;
            images.resize( image_count );
            if ( const auto res = vkGetSwapchainImagesKHR( device, swap_chain, &image_count, images.data( ) ); res != VK_SUCCESS ) {
                LOG_ERROR( VULKAN_TAG, "%1", "Could not get swap chain images!" );
                return {};
            }

            swap_chain_info info;
            info.swap_chain = swap_chain;
            info.format = desired_format.format;
            info.extent = desired_extent;
            info.images = create_swap_chain_image_views( device, desired_format.format, images );

            return info;
        }

        auto destroy_swap_chain( VkDevice device, swap_chain_info &swap_chain ) {
            LOG_DEBUG_CHECKPOINT( VULKAN_TAG );

            if ( swap_chain.swap_chain != VK_NULL_HANDLE ) {
                vkDestroySwapchainKHR( device, swap_chain.swap_chain, nullptr );
                swap_chain.swap_chain = VK_NULL_HANDLE;
            }

            for ( auto &img : swap_chain.images ) {
                vkDestroyImageView( device, img.image_view, nullptr );
                img.image_view = VK_NULL_HANDLE;
            }
        }

        auto create_semaphores( VkDevice device ) -> std::tuple<VkSemaphore, VkSemaphore> {
            LOG_DEBUG_CHECKPOINT( VULKAN_TAG );

            VkSemaphoreCreateInfo semaphore_create_info;
            semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            semaphore_create_info.pNext = nullptr;
            semaphore_create_info.flags = 0;

            VkSemaphore sems[2] = {VK_NULL_HANDLE, VK_NULL_HANDLE};

            if ( const auto res = vkCreateSemaphore( device, &semaphore_create_info, nullptr, &sems[0] ); res != VK_SUCCESS ) {
                LOG_ERROR( VULKAN_TAG, "%1", "Could not create semaphores!" );
            }

            if ( const auto res = vkCreateSemaphore( device, &semaphore_create_info, nullptr, &sems[1] ); res != VK_SUCCESS ) {
                LOG_ERROR( VULKAN_TAG, "%1", "Could not create semaphores!" );
            }

            return {sems[0], sems[1]};
        }

        auto create_command_buffers( VkDevice device, VkSwapchainKHR swap_chain, const uint32_t present_queue_family_index )
            -> std::optional<std::tuple<VkCommandPool, std::vector<VkCommandBuffer>>> {

            VkCommandPoolCreateInfo command_pool_create_info;
            command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            command_pool_create_info.pNext = nullptr;
            command_pool_create_info.flags = 0;
            command_pool_create_info.queueFamilyIndex = present_queue_family_index;

            VkCommandPool present_queue_command_pool;
            if ( const auto res = vkCreateCommandPool( device, &command_pool_create_info, nullptr, &present_queue_command_pool );
                 res != VK_SUCCESS ) {
                LOG_ERROR( VULKAN_TAG, "%1 Could not create a command pool!", res );
                return {};
            }

            uint32_t image_count = 0;
            if ( const auto res = vkGetSwapchainImagesKHR( device, swap_chain, &image_count, nullptr );
                 ( res != VK_SUCCESS ) || ( image_count == 0 ) ) {
                LOG_ERROR( VULKAN_TAG, "%1", "Could not get swap chain images!" );
                return {};
            }

            std::vector<VkCommandBuffer> present_queue_command_buffers;
            present_queue_command_buffers.resize( image_count );

            VkCommandBufferAllocateInfo command_buffer_allocate_info;
            command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            command_buffer_allocate_info.pNext = nullptr;
            command_buffer_allocate_info.commandPool = present_queue_command_pool;
            command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            command_buffer_allocate_info.commandBufferCount = image_count;

            if ( vkAllocateCommandBuffers( device, &command_buffer_allocate_info, present_queue_command_buffers.data( ) ) != VK_SUCCESS ) {
                LOG_ERROR( VULKAN_TAG, "%1", "Could not allocate command buffers!" );
                return {};
            }

            return {std::make_tuple( present_queue_command_pool, present_queue_command_buffers )};
        }

        //
        auto record_command_buffers( VkDevice device, VkSwapchainKHR swap_chain,
                                     const std::vector<VkCommandBuffer> &present_queue_command_buffers ) {
            auto image_count = static_cast<uint32_t>( present_queue_command_buffers.size( ) );

            std::vector<VkImage> swap_chain_images( image_count );
            if ( const auto res = vkGetSwapchainImagesKHR( device, swap_chain, &image_count, swap_chain_images.data( ) );
                 res != VK_SUCCESS ) {
                LOG_ERROR( VULKAN_TAG, "%1", "Could not get swap chain images!" );
                return false;
            }

            VkCommandBufferBeginInfo command_buffer_begin_info = {};
            command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            command_buffer_begin_info.pNext = nullptr;
            command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
            command_buffer_begin_info.pInheritanceInfo = nullptr;

            const VkClearColorValue clear_color = {{0.23f, 0.23f, 0.23f, 0.0f}};

            VkImageSubresourceRange image_subresource_range = {};
            image_subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            image_subresource_range.baseMipLevel = 0;
            image_subresource_range.levelCount = 1;
            image_subresource_range.baseArrayLayer = 0;
            image_subresource_range.layerCount = 1;

            for ( uint32_t i = 0; i < image_count; ++i ) {
                VkImageMemoryBarrier barrier_from_present_to_clear = {};
                barrier_from_present_to_clear.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier_from_present_to_clear.pNext = nullptr;
                barrier_from_present_to_clear.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
                barrier_from_present_to_clear.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier_from_present_to_clear.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                barrier_from_present_to_clear.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                barrier_from_present_to_clear.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier_from_present_to_clear.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier_from_present_to_clear.image = swap_chain_images[i];
                barrier_from_present_to_clear.subresourceRange = image_subresource_range;

                VkImageMemoryBarrier barrier_from_clear_to_present = {};
                barrier_from_clear_to_present.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier_from_clear_to_present.pNext = nullptr;
                barrier_from_clear_to_present.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier_from_clear_to_present.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
                barrier_from_clear_to_present.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                barrier_from_clear_to_present.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                barrier_from_clear_to_present.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier_from_clear_to_present.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier_from_clear_to_present.image = swap_chain_images[i];
                barrier_from_clear_to_present.subresourceRange = image_subresource_range;

                vkBeginCommandBuffer( present_queue_command_buffers[i], &command_buffer_begin_info );
                vkCmdPipelineBarrier( present_queue_command_buffers[i], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                                      0, nullptr, 0, nullptr, 1, &barrier_from_present_to_clear );

                vkCmdClearColorImage( present_queue_command_buffers[i], swap_chain_images[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                      &clear_color, 1, &image_subresource_range );

                vkCmdPipelineBarrier( present_queue_command_buffers[i], VK_PIPELINE_STAGE_TRANSFER_BIT,
                                      VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier_from_clear_to_present );
                if ( vkEndCommandBuffer( present_queue_command_buffers[i] ) != VK_SUCCESS ) {
                    LOG_ERROR( VULKAN_TAG, "%1", "Could not record command buffers!" );
                    return false;
                }
            }

            return true;
        }

    } // namespace detail

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
        ctx.is_debugging = debugging_flag;

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

        const auto swap_chain = create_swap_chain( ctx.physical_device, ctx.device, ctx.presentation_surface, ctx.swap_chain.swap_chain );
        if ( !swap_chain )
            return {};

        ctx.swap_chain = swap_chain.value( );

        auto [image_available, rendering_finished] = create_semaphores( ctx.device );
        if ( ctx.image_available = image_available; image_available == VK_NULL_HANDLE )
            return {};

        if ( ctx.rendering_finished = rendering_finished; rendering_finished == VK_NULL_HANDLE )
            return {};

        auto command_buffers = create_command_buffers( ctx.device, ctx.swap_chain.swap_chain, ctx.present_queue.family_index );
        if ( !command_buffers )
            return {};

        auto [command_pool, buffers] = command_buffers.value( );
        ctx.present_queue_command_pool = command_pool;
        ctx.present_queue_command_buffers = buffers;

        if ( !record_command_buffers( ctx.device, ctx.swap_chain.swap_chain, ctx.present_queue_command_buffers ) ) {
            return {};
        }

        return ctx;
    }

    auto cleanup( context &ctx ) noexcept {
        LOG_DEBUG_CHECKPOINT( VULKAN_TAG );

        using namespace detail;

        vkDeviceWaitIdle( ctx.device );

        if ( ( ctx.present_queue_command_buffers.size( ) > 0 ) && ( ctx.present_queue_command_buffers[0] != VK_NULL_HANDLE ) ) {
            vkFreeCommandBuffers( ctx.device, ctx.present_queue_command_pool,
                                  static_cast<uint32_t>( ctx.present_queue_command_buffers.size( ) ),
                                  ctx.present_queue_command_buffers.data( ) );
            ctx.present_queue_command_buffers.clear( );
        }

        if ( ctx.present_queue_command_pool != VK_NULL_HANDLE ) {
            vkDestroyCommandPool( ctx.device, ctx.present_queue_command_pool, nullptr );
            ctx.present_queue_command_pool = VK_NULL_HANDLE;
        }

        if ( ctx.image_available != VK_NULL_HANDLE )
            vkDestroySemaphore( ctx.device, ctx.image_available, nullptr );

        if ( ctx.rendering_finished != VK_NULL_HANDLE )
            vkDestroySemaphore( ctx.device, ctx.rendering_finished, nullptr );

        destroy_swap_chain( ctx.device, ctx.swap_chain );

        if ( ctx.is_debugging )
            debugging::cleanup( ctx.instance );

        destroy_logical_device( ctx.device );

        destroy_instance( ctx.instance );
    }

    auto submit_and_present( context &ctx ) {
        LOG_DEBUG_CHECKPOINT_ONCE( VULKAN_TAG );

        uint32_t image_index = 0;
        switch ( const auto result = vkAcquireNextImageKHR( ctx.device, ctx.swap_chain.swap_chain, UINT64_MAX, ctx.image_available,
                                                            VK_NULL_HANDLE, &image_index );
                 result ) {
        case VK_SUCCESS:
        case VK_SUBOPTIMAL_KHR:
            break;
        case VK_ERROR_OUT_OF_DATE_KHR:
            break;
        default:
            LOG_ERROR( VULKAN_TAG, "%1", "Problem occurred during swap chain image acquisition!", result );
        }

        const VkPipelineStageFlags wait_dst_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT;
        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.pNext = nullptr;
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &ctx.image_available;
        submit_info.pWaitDstStageMask = &wait_dst_stage_mask;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &ctx.present_queue_command_buffers[image_index];
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &ctx.rendering_finished;

        if ( const auto res = vkQueueSubmit( ctx.present_queue.handle, 1, &submit_info, VK_NULL_HANDLE ); res != VK_SUCCESS ) {
            LOG_ERROR( VULKAN_TAG, "%1 Submit error!", res );
        }

        VkPresentInfoKHR present_info = {};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.pNext = nullptr;
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &ctx.rendering_finished;
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &ctx.swap_chain.swap_chain;
        present_info.pImageIndices = &image_index;
        present_info.pResults = nullptr;

        switch ( const auto result = vkQueuePresentKHR( ctx.present_queue.handle, &present_info ); result ) {
        case VK_SUCCESS:
            break;
        case VK_ERROR_OUT_OF_DATE_KHR:
        case VK_SUBOPTIMAL_KHR:
            break;
        default:
            LOG_ERROR( VULKAN_TAG, "%1 Problem occurred during image presentation!", result );
        }
    }

} // namespace vulkan
