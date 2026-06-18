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

DEFINE_CALLBACK_ARGS_0(glfwTerminate)
DEFINE_CALLBACK_ARGS_0(volkFinalize)
DEFINE_CALLBACK_ARGS_2(vkDestroyInstance, VkInstance, const VkAllocationCallbacks*)
DEFINE_CALLBACK_ARGS_3(vkDestroyDebugUtilsMessengerEXT, VkInstance, VkDebugUtilsMessengerEXT, const VkAllocationCallbacks*)
DEFINE_CALLBACK_ARGS_3(vkDestroySurfaceKHR, VkInstance, VkSurfaceKHR, const VkAllocationCallbacks*)
DEFINE_CALLBACK_ARGS_2(vkDestroyDevice, VkDevice, const VkAllocationCallbacks*)
DEFINE_CALLBACK_ARGS_1(vmaDestroyAllocator, VmaAllocator)

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

int main(int argc, char* argv[]) {
    // Output project info and argument count
    printf("Project Name:    %s\nProject Version: %s\nArg Count:       %i\n", PROJECT_NAME, PROJECT_VERSION, argc);

    // Output arguments
    for(int i = 0; i < argc; i++) {
        printf("Arg %i:           %s\n", i, argv[i]);
    }

    /*__  _ ___     _  
    (_  |_  | | | |_) 
    __) |_  | |_| |   
    */

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

    // Create a struct containing the information about the application
    VkApplicationInfo applicationInfo = {
        .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName   = PROJECT_NAME,
        .applicationVersion = VK_MAKE_VERSION(PROJECT_VERSION_MAJOR, PROJECT_VERSION_MINOR, PROJECT_VERSION_PATCH),
        .pEngineName        = "No Engine",
        .engineVersion      = VK_MAKE_VERSION(0, 0, 0),
        .apiVersion         = VK_API_VERSION_1_4
    };

    // Create a struct containing the information about the instance
    VkInstanceCreateInfo instanceInfo = {
        .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext                   = &debugInfo,
        .pApplicationInfo        = &applicationInfo,
        .enabledLayerCount       = layersCount,
        .ppEnabledLayerNames     = layers,
        .enabledExtensionCount   = extensionsCount,
        .ppEnabledExtensionNames = extensions
    };

    // Create the instance with the instance info and application instance structs we created
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

    VkSurfaceKHR windowSurface = VK_NULL_HANDLE;
    result = glfwCreateWindowSurface(instance, window, NULL, &windowSurface);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to create surface for GLFW window: %i\n", result);
        return EXIT_FAILURE;
    }
    PUSH_CLEANUP_ARGS_3(vkDestroySurfaceKHR, instance, windowSurface, NULL);
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

    // Create a handle for our selected physical device and create variables to store the index of our selected queue families
    uint32_t highestScore = 0;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    uint32_t presentationQueueFamily = 0, graphicsQueueFamily = 0;

    // Iterate through all physical devices and select the best one
    for(uint32_t i = 0; i < physicalDevicesCount; i++) {
        uint32_t tempPresentationQueueFamily = 0, tempGraphicsQueueFamily = 0;
        uint32_t score = getVulkanPhysicalDeviceSuitability(physicalDevices[i], windowSurface, deviceExtensions, deviceExtensionsCount, &tempPresentationQueueFamily, &tempGraphicsQueueFamily);
        if(score > highestScore) {
            highestScore = score;
            physicalDevice = physicalDevices[i];
            presentationQueueFamily = tempPresentationQueueFamily;
            graphicsQueueFamily = tempGraphicsQueueFamily;
        }
    }

    // Ensure we have a suitable physical device before preceding
    if(physicalDevice == VK_NULL_HANDLE || highestScore == 0) {
        fprintf(stdout, "Failed to find suitable physical device\n");
        exit(EXIT_FAILURE);
    }

    // Add our the presentation and graphics queue families to an array
    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueFamilies[2];
    uint32_t queueFamiliesCount = 0;
    queueFamilies[queueFamiliesCount++] = (VkDeviceQueueCreateInfo){
            .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = presentationQueueFamily,
            .queueCount       = 1,
            .pQueuePriorities = &queuePriority,
        };
    if(presentationQueueFamily != graphicsQueueFamily) {
        queueFamilies[queueFamiliesCount++] = (VkDeviceQueueCreateInfo){
            .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = graphicsQueueFamily,
            .queueCount       = 1,
            .pQueuePriorities = &queuePriority,
        };
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

    VkPhysicalDeviceVulkan11Features enabledDeviceFeatures_1_1 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES
    };
    VkPhysicalDeviceVulkan12Features enabledDeviceFeatures_1_2 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .pNext = &enabledDeviceFeatures_1_1
    };
    VkPhysicalDeviceVulkan13Features enabledDeviceFeatures_1_3 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = &enabledDeviceFeatures_1_2
    };
    VkPhysicalDeviceVulkan14Features enabledDeviceFeatures_1_4 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES,
        .pNext = &enabledDeviceFeatures_1_3
    };
    VkPhysicalDeviceFeatures2 enabledDeviceFeatures = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &enabledDeviceFeatures_1_4
    };

    VkDeviceCreateInfo deviceInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &enabledDeviceFeatures,
        .queueCreateInfoCount = queueFamiliesCount,
        .pQueueCreateInfos = queueFamilies,
        .enabledExtensionCount = deviceExtensionsCount,
        .ppEnabledExtensionNames = deviceExtensions
    };

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

    VkQueue presentationQueue = VK_NULL_HANDLE;
    VkQueue graphicsQueue = VK_NULL_HANDLE;

    vkGetDeviceQueue(device, presentationQueueFamily, 0, &presentationQueue);
    vkGetDeviceQueue(device, graphicsQueueFamily, 0, &graphicsQueue);

    VmaAllocatorCreateInfo allocatorCreateInfo = {
        .physicalDevice = physicalDevice,
        .device = device,
        .instance = instance,
        .vulkanApiVersion = VK_API_VERSION_1_4
    };

    VmaVulkanFunctions vulkanFunctions;
    result = vmaImportVulkanFunctionsFromVolk(&allocatorCreateInfo, &vulkanFunctions);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to import vulkan functions from volk for VMA: %i\n", result);
        return EXIT_FAILURE;
    }
    allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

    VmaAllocator allocator;
    result = vmaCreateAllocator(&allocatorCreateInfo, &allocator);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to create the VMA allocator: %i\n", result);
        return EXIT_FAILURE;
    }
    PUSH_CLEANUP_ARGS_1(vmaDestroyAllocator, allocator);
    fprintf(stdout, "Created the VMA allocator\n");

    // Main application loop
    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    // Exit with success, the cleanup stack will clean everything up
    return EXIT_SUCCESS;
}
