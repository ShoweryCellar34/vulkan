// System Headers
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Library Headers
#include <volk.h>
#include <vk_mem_alloc.h>
#include <GLFW/glfw3.h>

// Project Headers
#include <cleanup.h>
#include <cleanupMacros.h>
#include <devices.h>
#include <extensions.h>
#include <layers.h>
#include <projectData.h>

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 450

#define CLAMP(val, min, max) (((val) < (min)) ? (min) : (((val) > (max)) ? (max) : (val)))

DEFINE_CALLBACK_ARGS_0(glfwTerminate)
DEFINE_CALLBACK_ARGS_0(volkFinalize)
DEFINE_CALLBACK_ARGS_2(vkDestroyInstance, VkInstance, const VkAllocationCallbacks*)
DEFINE_CALLBACK_ARGS_3(vkDestroyDebugUtilsMessengerEXT, VkInstance, VkDebugUtilsMessengerEXT, const VkAllocationCallbacks*)
DEFINE_CALLBACK_ARGS_3(vkDestroySurfaceKHR, VkInstance, VkSurfaceKHR, const VkAllocationCallbacks*)
DEFINE_CALLBACK_ARGS_2(vkDestroyDevice, VkDevice, const VkAllocationCallbacks*)
DEFINE_CALLBACK_ARGS_1(vmaDestroyAllocator, VmaAllocator)
DEFINE_CALLBACK_ARGS_3(vkDestroySwapchainKHR, VkDevice, VkSwapchainKHR, const VkAllocationCallbacks*)
DEFINE_CALLBACK_ARGS_3(vkDestroyImageView, VkDevice, VkImageView, const VkAllocationCallbacks*)
DEFINE_CALLBACK_ARGS_1(fclose, FILE*)
DEFINE_CALLBACK_ARGS_3(vkDestroyShaderModule, VkDevice, VkShaderModule, const VkAllocationCallbacks*)
DEFINE_CALLBACK_ARGS_3(vkDestroyPipelineLayout, VkDevice, VkPipelineLayout, const VkAllocationCallbacks*)
DEFINE_CALLBACK_ARGS_3(vkDestroyPipeline, VkDevice, VkPipeline, const VkAllocationCallbacks*)
DEFINE_CALLBACK_ARGS_3(vkDestroyCommandPool, VkDevice, VkCommandPool, const VkAllocationCallbacks*)

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)  {
    window; key; scancode; action; mods;
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)  {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

static VKAPI_ATTR VkBool32 VKAPI_CALL vulkanDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT      severity,
    VkDebugUtilsMessageTypeFlagsEXT             type,
    const VkDebugUtilsMessengerCallbackDataEXT* data,
    void*                                       user_data
) {
    severity; type; user_data;
    fprintf(stderr, "Vulkan error: %s\n", data->pMessage);
    return VK_FALSE;
}

void transitionImageLayout(
    VkCommandBuffer         commandBuffer,
    VkImage                 image,
    VkImageLayout           oldLayout,
    VkImageLayout           newLayout,
    VkAccessFlags2          srcAccessMask,
    VkAccessFlags2          dstAccessMask,
    VkPipelineStageFlags2   srcStageMask,
    VkPipelineStageFlags2   dstStageMask
) {
    VkImageMemoryBarrier2 barrier = {
        .srcStageMask        = srcStageMask,
        .srcAccessMask       = srcAccessMask,
        .dstStageMask        = dstStageMask,
        .dstAccessMask       = dstAccessMask,
        .oldLayout           = oldLayout,
        .newLayout           = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image               = image,
        .subresourceRange    = {
                .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel   = 0,
                .levelCount     = 1,
                .baseArrayLayer = 0,
                .layerCount     = 1
        }
    };
    VkDependencyInfo dependencyInfo = {
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers    = &barrier
    };
    vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);
}

int main(int argc, char* argv[]) {
    // Output project info and argument count
    printf("Project Name:    %s\nProject Version: %s\nArg Count:       %i\n", PROJECT_NAME, PROJECT_VERSION, argc);

    // Output arguments
    for(int i = 0; i < argc; i++) {
        printf("Arg %i:           %s\n", i, argv[i]);
    }

    // Setup the exit callbacks stack for cleaning up resources upon exit
    if(atexit(startCleanupCallbacks) != 0) {
        fprintf(stdout, "Failed to register cleanup callback stack with atExit()\n");
        exit(EXIT_FAILURE);
    }

    // initialize GLFW
    if(glfwInit() != GLFW_TRUE) {
        const char* errorMessage = NULL;
        int errorCode = glfwGetError(&errorMessage);
        fprintf(stderr, "Failed to initialize GLFW:\n    Error Code:    %i\n    Error Message: %s\n", errorCode, errorMessage);
        return EXIT_FAILURE;
    }
    PUSH_CLEANUP_ARGS_0(glfwTerminate);
    fprintf(stdout, "Initialized GLFW\n");

    // Check for minimal vulkan support
    if(glfwVulkanSupported() != GLFW_TRUE) {
        fprintf(stderr, "Minimal vulkan functionality test failed\n");
        return EXIT_FAILURE;
    }
    fprintf(stdout, "Vulkan supported minimally\n");

    // Disable the creation of an OpenGL Context and disable window resizing
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // Create the window
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, PROJECT_NAME, NULL, NULL);
    if(window == NULL) {
        const char* errorMessage = NULL;
        int errorCode = glfwGetError(&errorMessage);
        fprintf(stderr, "Failed to create GLFW window:\n    Error Code:    %i\n    Error Message: %s\n", errorCode, errorMessage);
        return EXIT_FAILURE;
    }
    pushCleanupCallback(glfwDestroyWindow, window);
    fprintf(stdout, "Created GLFW window\n");

    // Set the kay callback for the window so we can capture keyboard events
    glfwSetKeyCallback(window, keyCallback);

    // Create a variable to store the result of vulkan functions for error checking and reporting
    VkResult result = VK_SUCCESS;

    // Load the vulkan library and global-level vulkan functions
    result = volkInitialize();
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to load vulkan with volk: %i", result);
        return EXIT_FAILURE;
    }
    PUSH_CLEANUP_ARGS_0(volkFinalize);
    fprintf(stdout, "Loaded global level vulkan functions using volk\n");

    // Get the extensions array and register its cleanup method
    uint32_t extensionsCount = 0;
    const char** extensions = getVulkanExtensions(&extensionsCount, DEBUG_BUILD);
    pushCleanupCallback(free, extensions);
    fprintf(stdout, "Successfully got list of required extensions:\n");

    // Print all the extensions we will use
    for(uint32_t i = 0; i < extensionsCount; i++) {
        fprintf(stdout, "    %s\n", extensions[i]);
    }

    // Get the supported extensions array and register its cleanup method
    uint32_t supportedExtensionsCount = 0;
    VkExtensionProperties* supportedExtensions = getSupportedVulkanExtensions(&supportedExtensionsCount);
    pushCleanupCallback(free, supportedExtensions);
    fprintf(stdout, "Successfully got list of supported extensions:\n");

    // Print all the extensions supported
    for(uint32_t i = 0; i < supportedExtensionsCount; i++) {
        fprintf(stdout, "    %s\n", supportedExtensions[i].extensionName);
    }

    // Ensure all requested extensions are supported
    for(uint32_t i = 0; i < extensionsCount; i++) {
        bool extensionSupported = false;
        for(uint32_t j = 0; j < supportedExtensionsCount; j++) {
            if(!strcmp(extensions[i], supportedExtensions[j].extensionName)) {
                extensionSupported = true;
            }
        }
        if(extensionSupported == false) {
            fprintf(stderr, "Not all requested extensions are supported\n");
            exit(EXIT_FAILURE);
        }
    }
    fprintf(stdout, "All required extensions are supported\n");

    // Create the array of layers we will use
    uint32_t layersCount = 0;
    const char** layers = getVulkanLayers(&layersCount, DEBUG_BUILD);
    fprintf(stdout, "Successfully got list of required layers:\n");

    // Print all the layers we will use
    for(uint32_t i = 0; i < layersCount; i++) {
        fprintf(stdout, "    %s\n", layers[i]);
    }

    // Get the supported layers array and register its cleanup method
    uint32_t supportedLayersCount = 0;
    VkLayerProperties* supportedLayers = getSupportedVulkanLayers(&supportedLayersCount);
    pushCleanupCallback(free, supportedLayers);
    fprintf(stdout, "Successfully got list of supported layers:\n");

    // Print all the layers supported
    for(uint32_t i = 0; i < supportedLayersCount; i++) {
        fprintf(stdout, "    %s\n", supportedLayers[i].layerName);
    }

    // Ensure all requested layers are supported
    for(uint32_t i = 0; i < layersCount; i++) {
        bool layerSupported = false;
        for(uint32_t j = 0; j < supportedLayersCount; j++) {
            if(!strcmp(layers[i], supportedLayers[j].layerName)) {
                layerSupported = true;
            }
        }
        if(layerSupported == false) {
            fprintf(stderr, "Not all requested layers are supported\n");
            exit(EXIT_FAILURE);
        }
    }
    fprintf(stdout, "All required layers are supported\n");

    VkDebugUtilsMessengerCreateInfoEXT debugInfo = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = vulkanDebugCallback
    };

    // Create a struct containing the settings for the application
    VkApplicationInfo applicationInfo = {
        .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName   = PROJECT_NAME,
        .applicationVersion = VK_MAKE_VERSION(PROJECT_VERSION_MAJOR, PROJECT_VERSION_MINOR, PROJECT_VERSION_PATCH),
        .pEngineName        = "No Engine",
        .engineVersion      = VK_MAKE_VERSION(0, 0, 0),
        .apiVersion         = VK_API_VERSION_1_4
    };

    // Create a struct containing the settings for the instance
    VkInstanceCreateInfo instanceInfo = {
        .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext                   = &debugInfo,
        .pApplicationInfo        = &applicationInfo,
        .enabledLayerCount       = layersCount,
        .ppEnabledLayerNames     = layers,
        .enabledExtensionCount   = extensionsCount,
        .ppEnabledExtensionNames = extensions
    };

    // Create the instance
    VkInstance instance = VK_NULL_HANDLE;
    result = vkCreateInstance(&instanceInfo, NULL, &instance);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to create instance: %i\n", result);
        return EXIT_FAILURE;
    }
    PUSH_CLEANUP_ARGS_2(vkDestroyInstance, instance, NULL);
    fprintf(stdout, "Created instance\n");

    // Load instance level vulkan functions
    volkLoadInstance(instance);
    fprintf(stdout, "Loaded instance level vulkan functions using volk\n");

    // Create the debug callback if this is a debug build
    #if DEBUG_BUILD == 1
        VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
        result = vkCreateDebugUtilsMessengerEXT(instance, &debugInfo, NULL, &debugMessenger);
        if(result != VK_SUCCESS) {
            fprintf(stderr, "Failed to set vulkan debug callback: %i\n", result);
            return EXIT_FAILURE;
        }
        PUSH_CLEANUP_ARGS_3(vkDestroyDebugUtilsMessengerEXT, instance, debugMessenger, NULL);
        fprintf(stdout, "Added vulkan debug callback\n");
    #endif

    // Create the GLFW window surface
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    result = glfwCreateWindowSurface(instance, window, NULL, &surface);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to create surface for GLFW window: %i\n", result);
        return EXIT_FAILURE;
    }
    PUSH_CLEANUP_ARGS_3(vkDestroySurfaceKHR, instance, surface, NULL);
    fprintf(stdout, "Created surface for GLFW window\n");

    // Get the device extensions array, it is static and does not need to be freed
    uint32_t deviceExtensionsCount = 0;
    const char** deviceExtensions = getVulkanDeviceExtensions(&deviceExtensionsCount);
    fprintf(stdout, "Successfully got list of required device extensions:\n");

    // Print all the device extensions we will use
    for(uint32_t i = 0; i < deviceExtensionsCount; i++) {
        fprintf(stdout, "    %s\n", deviceExtensions[i]);
    }

    // Get the physical devices array and register its cleanup method
    uint32_t physicalDevicesCount = 0;
    VkPhysicalDevice* physicalDevices = getVulkanPhysicalDevices(instance, &physicalDevicesCount);
    pushCleanupCallback(free, physicalDevices);
    // Ensure there is atleast one physical device before continuing
    if(physicalDevicesCount == 0) {
        fprintf(stdout, "No physical devices supported\n");
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "Successfully got list of physical devices:\n");

    // Print all the physical devices supported
    for(uint32_t i = 0; i < physicalDevicesCount; i++) {
        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevices[i], &physicalDeviceProperties);
        fprintf(stdout, "    %s\n", physicalDeviceProperties.deviceName);
    }

    // Create a handle for our selected physical device and create variables to store the index of our selected queue families and number of images in our swapchain
    uint32_t highestScore = 0;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    uint32_t graphicsQueueFamily = 0, presentationQueueFamily = 0;

    // Iterate through all physical devices and select the best one
    for(uint32_t i = 0; i < physicalDevicesCount; i++) {
        uint32_t tempGraphicsQueueFamily = 0, tempPresentationQueueFamily = 0;
        uint32_t score = getVulkanPhysicalDeviceSuitability(physicalDevices[i], surface, deviceExtensions, deviceExtensionsCount, &tempPresentationQueueFamily, &tempGraphicsQueueFamily);
        if(score > highestScore) {
            highestScore = score;
            physicalDevice = physicalDevices[i];
            graphicsQueueFamily = tempGraphicsQueueFamily;
            presentationQueueFamily = tempPresentationQueueFamily;
        }
    }

    // Ensure we have a suitable physical device before preceding
    if(physicalDevice == VK_NULL_HANDLE || highestScore == 0) {
        fprintf(stdout, "Failed to find suitable physical device\n");
        exit(EXIT_FAILURE);
    }

    // Print the physical device we will use
    VkPhysicalDeviceProperties2 physicalDeviceProperties = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2
    };
    vkGetPhysicalDeviceProperties2(physicalDevice, &physicalDeviceProperties);
    fprintf(
        stdout,
        "Using suitable physical device:\n    Name:                      %s\n    API Version:               %" PRIu32 ".%" PRIu32 ".%" PRIu32 "\n    Graphics Queue Family:     %" PRIu32 "\n    Presentation Queue Family: %" PRIu32 "\n",
        physicalDeviceProperties.properties.deviceName,
        VK_VERSION_MAJOR(physicalDeviceProperties.properties.apiVersion),
        VK_VERSION_MINOR(physicalDeviceProperties.properties.apiVersion),
        VK_VERSION_PATCH(physicalDeviceProperties.properties.apiVersion),
        graphicsQueueFamily,
        presentationQueueFamily
    );

    // Add our the graphics and presentation queue families to an array
    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueFamilies[2];
    uint32_t queueFamiliesCount = 0;
    queueFamilies[queueFamiliesCount++] = (VkDeviceQueueCreateInfo){
            .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = graphicsQueueFamily,
            .queueCount       = 1,
            .pQueuePriorities = &queuePriority,
        };
    if(graphicsQueueFamily != presentationQueueFamily) {
        queueFamilies[queueFamiliesCount++] = (VkDeviceQueueCreateInfo){
            .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = presentationQueueFamily,
            .queueCount       = 1,
            .pQueuePriorities = &queuePriority,
        };
    }

    // List the features we want our logical device to have
    VkPhysicalDeviceVulkan11Features enabledDeviceFeatures_1_1 = {
        .sType                = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
        .shaderDrawParameters = VK_TRUE
    };
    VkPhysicalDeviceVulkan12Features enabledDeviceFeatures_1_2 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .pNext = &enabledDeviceFeatures_1_1
    };
    VkPhysicalDeviceVulkan13Features enabledDeviceFeatures_1_3 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = &enabledDeviceFeatures_1_2,
        .dynamicRendering = VK_TRUE
    };
    VkPhysicalDeviceVulkan14Features enabledDeviceFeatures_1_4 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES,
        .pNext = &enabledDeviceFeatures_1_3
    };
    VkPhysicalDeviceFeatures2 enabledDeviceFeatures = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &enabledDeviceFeatures_1_4
    };

    // Create a struct containing the settings for the logical device
    VkDeviceCreateInfo deviceInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &enabledDeviceFeatures,
        .queueCreateInfoCount = queueFamiliesCount,
        .pQueueCreateInfos = queueFamilies,
        .enabledExtensionCount = deviceExtensionsCount,
        .ppEnabledExtensionNames = deviceExtensions
    };

    // Create the logical device
    VkDevice device = VK_NULL_HANDLE;
    result = vkCreateDevice(physicalDevice, &deviceInfo, NULL, &device);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to create logical device: %i\n", result);
        return EXIT_FAILURE;
    }
    PUSH_CLEANUP_ARGS_2(vkDestroyDevice, device, NULL);
    fprintf(stdout, "Created logical device\n");

    // Load device level vulkan functions
    volkLoadDevice(device);
    fprintf(stdout, "Loaded device level vulkan functions using volk\n");

    // Get the the handles to the graphics and presentation queues
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentationQueue = VK_NULL_HANDLE;
    vkGetDeviceQueue(device, graphicsQueueFamily, 0, &graphicsQueue);
    vkGetDeviceQueue(device, presentationQueueFamily, 0, &presentationQueue);

    // Create a struct containing the settings for the VMA allocator
    VmaAllocatorCreateInfo allocatorCreateInfo = {
        .physicalDevice = physicalDevice,
        .device = device,
        .instance = instance,
        .vulkanApiVersion = VK_API_VERSION_1_4
    };

    // Get the list of vulkan functions for VMA from volk
    VmaVulkanFunctions vulkanFunctions;
    result = vmaImportVulkanFunctionsFromVolk(&allocatorCreateInfo, &vulkanFunctions);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to import vulkan functions from volk for VMA: %i\n", result);
        return EXIT_FAILURE;
    }
    allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

    // Create the VMA allocator
    VmaAllocator allocator;
    result = vmaCreateAllocator(&allocatorCreateInfo, &allocator);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to create the VMA allocator: %i\n", result);
        return EXIT_FAILURE;
    }
    PUSH_CLEANUP_ARGS_1(vmaDestroyAllocator, allocator);
    fprintf(stdout, "Created the VMA allocator\n");

    // Get the capabilities of the surface
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);
    if(result != VK_SUCCESS) {
        fprintf(stdout, "Failed to get surface capabilities: %i\n", result);
        exit(EXIT_FAILURE);
    }

    // Set the number of images that should be in the swapchain
    uint32_t swapchainImageCount = surfaceCapabilities.minImageCount + (surfaceCapabilities.maxImageCount > surfaceCapabilities.minImageCount || surfaceCapabilities.maxImageCount == 0 ? 1 : 0);

    // Set the dimensions of the swapchain
    VkExtent2D swapchainExtent;
    if(surfaceCapabilities.currentExtent.width == UINT32_MAX) {
        int windowWidth, windowHeight;
        glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

        swapchainExtent.width = CLAMP((uint32_t)windowWidth, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
        swapchainExtent.height = CLAMP((uint32_t)windowHeight, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
    } else {
        swapchainExtent = surfaceCapabilities.currentExtent;
    }

    // Define the surface format we will use
    VkSurfaceFormatKHR swapchainFormat = {
        .format     = VK_FORMAT_B8G8R8A8_SRGB,
        .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
    };

    // Create a struct containing the settings for the swapchain
    VkSwapchainCreateInfoKHR swapchainCreateInfo = {
        .sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface          = surface,
        .minImageCount    = swapchainImageCount,
        .imageFormat      = swapchainFormat.format,
        .imageColorSpace  = swapchainFormat.colorSpace,
        .imageExtent      = swapchainExtent,
        .imageArrayLayers = 1,
        .imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform     = surfaceCapabilities.currentTransform,
        .compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode      = VK_PRESENT_MODE_FIFO_KHR,
        .clipped          = true
    };

    // Change the image sharing mode in the swapchain create info struct
    if(graphicsQueueFamily != presentationQueueFamily) {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCreateInfo.queueFamilyIndexCount = 2;
        swapchainCreateInfo.pQueueFamilyIndices = (const uint32_t[]){graphicsQueueFamily, presentationQueueFamily};
    }

    // Create the swapchain
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    result = vkCreateSwapchainKHR(device, &swapchainCreateInfo, NULL, &swapchain);
    if(result != VK_SUCCESS) {
        fprintf(stdout, "Failed to create swapchain: %i\n", result);
        exit(EXIT_FAILURE);
    }
    PUSH_CLEANUP_ARGS_3(vkDestroySwapchainKHR, device, swapchain, NULL);
    fprintf(stdout, "Created swapchain\n");

    // Get the length of the swapchain images array
    uint32_t swapchainImagesCount = 0;
    result = vkGetSwapchainImagesKHR(device, swapchain, &swapchainImagesCount, NULL);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to get number of swapchain images: %i\n", result);
        exit(EXIT_FAILURE);
    }

    // Allocate the swapchain images array
    VkImage* swapchainImages = malloc(sizeof(VkImage) * swapchainImagesCount);
    if(swapchainImages == NULL) {
        fprintf(stderr, "Failed to allocate swapchain images array of size %zu\n", swapchainImagesCount * sizeof(VkImage));
        exit(EXIT_FAILURE);
    }
    pushCleanupCallback(free, swapchainImages);

    // Fill the swapchain images array with the list of swapchain images
    result = vkGetSwapchainImagesKHR(device, swapchain, &swapchainImagesCount, swapchainImages);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to get number of swapchain images: %i\n", result);
        exit(EXIT_FAILURE);
    }

    // Create a struct containing the settings for the swapchain image views
    VkImageViewCreateInfo swapchainImageViewsCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = swapchainFormat.format,
        .components = {
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY
        },
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .levelCount = 1,
            .layerCount = 1
        }
    };

    // Allocate the swapchain image views array
    VkImageView* swapchainImageViews = malloc(sizeof(VkImageView) * swapchainImagesCount);
    if(swapchainImageViews == NULL) {
        fprintf(stderr, "Failed to allocate swapchain image views array of size %zu\n", swapchainImagesCount * sizeof(VkImageView));
        exit(EXIT_FAILURE);
    }
    pushCleanupCallback(free, swapchainImageViews);

    // Create the swapchain image views
    for(uint32_t i = 0; i < swapchainImagesCount; i++) {
        swapchainImageViewsCreateInfo.image = swapchainImages[i];
        result = vkCreateImageView(device, &swapchainImageViewsCreateInfo, NULL, &swapchainImageViews[i]);
        if(result != VK_SUCCESS) {
            fprintf(stderr, "Failed to create swapchain image view %" PRIu32 ": %i\n", i, result);
            exit(EXIT_FAILURE);
        }
        PUSH_CLEANUP_ARGS_3(vkDestroyImageView, device, swapchainImageViews[i], NULL);
        fprintf(stderr, "Created swapchain image view %" PRIu32 "\n", i);
    }

    // Open the shader file
    FILE* shaderFile = fopen("resources/shaders/basic.spv", "rb");
    if(shaderFile == NULL) {
        fprintf(stderr, "Failed to open file: %s\n", "resources/shaders/basic.spv");
        exit(EXIT_FAILURE);
    }
    PUSH_CLEANUP_ARGS_1(fclose, shaderFile);
    fprintf(stdout, "Opened file: %s", "resources/shaders/basic.spv\n");

    // Set our position in the shader file to the end to get the shader file length
    if(fseek(shaderFile, 0, SEEK_END) != 0) {
        fprintf(stderr, "Failed to seek to end of file: %s\n", "resources/shaders/basic.spv");
        return EXIT_FAILURE;
    }

    // Get the length of the supported extensions array
    long shaderFileDataCount = ftell(shaderFile);
    if(shaderFileDataCount < 0) {
        fprintf(stderr, "Failed to get length of file: %s\n", "resources/shaders/basic.spv");
        return EXIT_FAILURE;
    }
    if(shaderFileDataCount % 4 != 0) {
        fprintf(stderr, "Length of file data is not a multiple of 4 (required for SPIR-V shaders) for file: %s\n", "resources/shaders/basic.spv");
        return EXIT_FAILURE;
    }

    // Set our position in the shader file to the begining for reading
    if(fseek(shaderFile, 0, SEEK_SET) != 0) {
        fprintf(stderr, "Failed to seek to begining of file: %s\n", "resources/shaders/basic.spv");
        return EXIT_FAILURE;
    }

    // Allocate the shader file data array
    unsigned char* shaderFileData = malloc(sizeof(unsigned char) * shaderFileDataCount);
    if(shaderFileData == NULL) {
        fprintf(stderr, "Failed to allocate file data array of size %zu for file: %s\n", shaderFileDataCount * sizeof(unsigned char), "resources/shaders/basic.spv");
        exit(EXIT_FAILURE);
    }
    pushCleanupCallback(free, shaderFileData);

    // Fill the shader file data array with the shader file data
    size_t shaderFileDataCountRead = fread(shaderFileData, 1, shaderFileDataCount, shaderFile);
    if(shaderFileDataCountRead < (size_t)shaderFileDataCount) {
        if(ferror(shaderFile)) {
            fprintf(stderr, "Error reading data for file: %s\n", "resources/shaders/basic.spv");
        } else if(feof(shaderFile)) {
            fprintf(stderr, "Unexpected end of file for file: %s\n", "resources/shaders/basic.spv");
        }
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "Read file contents for file: %s", "resources/shaders/basic.spv\n");

    // Close the file handle
    popAndCallCleanupCallback(1);

    // Create a struct containing the settings for the shader module
    VkShaderModuleCreateInfo shaderModuleCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = (size_t)shaderFileDataCount,
        .pCode = (uint32_t*)shaderFileData
    };

    // Create the shader module
    VkShaderModule shaderModule = VK_NULL_HANDLE;
    result = vkCreateShaderModule(device, &shaderModuleCreateInfo, NULL, &shaderModule);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to create shader module %i\n", result);
        exit(EXIT_FAILURE);
    }
    PUSH_CLEANUP_ARGS_3(vkDestroyShaderModule, device, shaderModule, NULL);
    fprintf(stdout, "Created shader module\n");

    VkPipelineShaderStageCreateInfo shaderStages[] = {
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = shaderModule,
            .pName = "vertMain"
        },
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = shaderModule,
            .pName = "fragMain"
        }
    };

    VkPipelineVertexInputStateCreateInfo   vertexInputInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
    };
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
        .sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
    };
    VkPipelineViewportStateCreateInfo      viewportState = {
        .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount  = 1
    };

    VkPipelineRasterizationStateCreateInfo rasterizer = {
        .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable        = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode             = VK_POLYGON_MODE_FILL,
        .cullMode                = VK_CULL_MODE_BACK_BIT,
        .frontFace               = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable         = VK_FALSE,
        .lineWidth               = 1.0f
    };

    VkPipelineMultisampleStateCreateInfo multisampling = {
        .sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable  = VK_FALSE
    };

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
        .blendEnable    = VK_FALSE,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    };

    VkPipelineColorBlendStateCreateInfo colorBlending = {
        .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable   = VK_FALSE,
        .logicOp         = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments    = &colorBlendAttachment
    };

    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamicState = {
        .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = (uint32_t)(sizeof(dynamicStates) / sizeof(*dynamicStates)),
        .pDynamicStates    = dynamicStates
    };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount         = 0,
        .pushConstantRangeCount = 0
    };

    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    result = vkCreatePipelineLayout(device, &pipelineLayoutInfo, NULL, &pipelineLayout);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to create graphics pipeline layout %i\n", result);
        exit(EXIT_FAILURE);
    }
    PUSH_CLEANUP_ARGS_3(vkDestroyPipelineLayout, device, pipelineLayout, NULL);
    fprintf(stdout, "Created graphics pipeline layout\n");

    VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo = {
        .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &swapchainFormat.format
    };

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {
        .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext               = (void*)&pipelineRenderingCreateInfo,
        .stageCount          = 2,
        .pStages             = shaderStages,
        .pVertexInputState   = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState      = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState   = &multisampling,
        .pColorBlendState    = &colorBlending,
        .pDynamicState       = &dynamicState,
        .layout              = pipelineLayout,
        .renderPass          = NULL
    };

    VkPipeline pipeline = VK_NULL_HANDLE;
    result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, NULL, &pipeline);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to create graphics pipeline %i\n", result);
        exit(EXIT_FAILURE);
    }
    PUSH_CLEANUP_ARGS_3(vkDestroyPipeline, device, pipeline, NULL);
    fprintf(stdout, "Created graphics pipeline\n");

    VkCommandPoolCreateInfo commandPoolCreateInfo = {
        .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = graphicsQueueFamily
    };

    VkCommandPool graphicsCommandPool = VK_NULL_HANDLE;
    VkCommandPool presentationCommandPool = VK_NULL_HANDLE;
    result = vkCreateCommandPool(device, &commandPoolCreateInfo, NULL, &graphicsCommandPool);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to create graphics command pool %i\n", result);
        exit(EXIT_FAILURE);
    }
    PUSH_CLEANUP_ARGS_3(vkDestroyCommandPool, device, graphicsCommandPool, NULL);
    fprintf(stdout, "Created graphics command pool\n");

    if(graphicsQueueFamily != presentationQueueFamily) {
        result = vkCreateCommandPool(device, &commandPoolCreateInfo, NULL, &presentationCommandPool);
        if(result != VK_SUCCESS) {
            fprintf(stderr, "Failed to create presentation command pool %i\n", result);
            exit(EXIT_FAILURE);
        }
        PUSH_CLEANUP_ARGS_3(vkDestroyCommandPool, device, presentationCommandPool, NULL);
        fprintf(stdout, "Created presentation command pool\n");
    } else {
        presentationCommandPool = graphicsCommandPool;
        fprintf(stdout, "Reusing graphics command pool as presentation command pool\n");
    }

    VkCommandBufferAllocateInfo commandBufferAllocationInfo = {
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool        = graphicsCommandPool,
        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };

    VkCommandBuffer graphicsCommandBuffer = VK_NULL_HANDLE;
    VkCommandBuffer presentationCommandBuffer = VK_NULL_HANDLE;
    result = vkAllocateCommandBuffers(device, &commandBufferAllocationInfo, &graphicsCommandBuffer);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to allocate graphics command buffer %i\n", result);
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "Allocated graphics command buffer\n");

    if(graphicsQueueFamily != presentationQueueFamily) {
        commandBufferAllocationInfo.commandPool = presentationCommandPool;
        result = vkAllocateCommandBuffers(device, &commandBufferAllocationInfo, &presentationCommandBuffer);
        if(result != VK_SUCCESS) {
            fprintf(stderr, "Failed to allocate presentation command buffer %i\n", result);
            exit(EXIT_FAILURE);
        }
        fprintf(stdout, "Allocated presentation command buffer\n");
    } else {
        presentationCommandBuffer = graphicsCommandBuffer;
        fprintf(stdout, "Reusing graphics command buffer as presentation command buffer\n");
    }

    // Main application loop
    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        static uint32_t swapchainImageIndex = 0;

        VkCommandBufferBeginInfo graphicsCommandBufferBeginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
        };
        result = vkBeginCommandBuffer(graphicsCommandBuffer, &graphicsCommandBufferBeginInfo);
        if(result != VK_SUCCESS) {
            fprintf(stderr, "Failed to begin graphics command buffer %i\n", result);
            exit(EXIT_FAILURE);
        }

        // Before starting rendering, transition the swapchain image to vk::ImageLayout::eColorAttachmentOptimal
        transitionImageLayout(
            graphicsCommandBuffer,
            swapchainImages[swapchainImageIndex],
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            0,
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT
        );

        VkClearValue clearColor = {
            .color = {
                0.0f,
                0.0f,
                0.0f,
                1.0f
            }
        };
        VkRenderingAttachmentInfo attachmentInfo = {
            .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView   = swapchainImages[swapchainImageIndex],
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp     = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue  = clearColor
        };

    }

    // Exit with success, the cleanup stack will clean everything up
    return EXIT_SUCCESS;
}
