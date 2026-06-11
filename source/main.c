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
#include <devices.h>
#include <extensions.h>
#include <layers.h>
#include <projectData.h>

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 450

#define ARRAY_COUNT(x) sizeof(x) / sizeof(x[0])
#define NULLFREE(x)    free(x); x = NULL

void testCallback(void* data) {
    fprintf(stdout, (const char*)data);
}

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
    atexit(startCleanupCallbacks);

    // initialize GLFW
    if(glfwInit() != GLFW_TRUE) {
        const char* errorMessage = NULL;
        int errorCode = glfwGetError(&errorMessage);
        fprintf(stderr, "Failed to initialize GLFW:\n    Error Code:    %i\n    Error Message: %s\n", errorCode, errorMessage);
        return EXIT_FAILURE;
    }
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
    fprintf(stdout, "Loaded vulkan using volk\n");

    // Get the vulkan extensions array and register its cleanup method
    uint32_t extensionCount = 0;
    const char** extensions = getVulkanExtensions(&extensionCount, DEBUG_BUILD);
    pushCleanupCallback(free, extensions);
    fprintf(stdout, "Successfully got list of required vulkan extensions\n");

    // Get the supported vulkan extensions array and register its cleanup method
    uint32_t supportedExtensionCount = 0;
    VkExtensionProperties* supportedExtensions = getSupportedVulkanExtensions(&supportedExtensionCount);
    pushCleanupCallback(free, supportedExtensions);
    fprintf(stdout, "Successfully got list of supported vulkan extensions\n");

    // Ensure all requested vulkan extensions are supported
    uint32_t extensionsSupported = 0;
    for(uint32_t i = 0; i < supportedExtensionCount; i++) {
        for(uint32_t j = 0; j < extensionCount; j++) {
            if(!strcmp(supportedExtensions[i].extensionName, extensions[j])) {
                extensionsSupported++;
            }
        }
    }
    if(extensionsSupported != extensionCount) {
        fprintf(stderr, "Not all requested vulkan extensions are supported\n");
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "All required vulkan extensions are supported:\n");

    // Print all the vulkan extensions we will use
    for(uint32_t i = 0; i < extensionCount; i++) {
        fprintf(stdout, "    %s\n", extensions[i]);
    }

    // Create the array of vulkan layers we will use
    #if DEBUG_BUILD == 1
        const char* layers[] = {
            "VK_LAYER_KHRONOS_validation"
        };
        const uint32_t layerCount = ARRAY_COUNT(layers);
    #else
        const char* layers[1] = {NULL};
        const uint32_t layerCount = 0;
    #endif

    // Get the supported vulkan layers array and register its cleanup method
    uint32_t supportedLayerCount = 0;
    VkLayerProperties* supportedLayers = getSupportedVulkanLayers(&supportedLayerCount);
    pushCleanupCallback(free, supportedLayers);
    fprintf(stdout, "Successfully got list of supported vulkan layers\n");

    // Ensure all requested vulkan layers are supported
    uint32_t layersSupported = 0;
    for(uint32_t i = 0; i < supportedLayerCount; i++) {
        for(uint32_t j = 0; j < layerCount; j++) {
            if(!strcmp(supportedLayers[i].layerName, layers[j])) {
                layersSupported++;
            }
        }
    }
    if(layersSupported != layerCount) {
        fprintf(stderr, "Not all requested vulkan layers are supported\n");
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "All required vulkan layers are supported:\n");

    // Print all the vulkan layers we will use
    for(uint32_t i = 0; i < layerCount; i++) {
        fprintf(stdout, "    %s\n", layers[i]);
    }

    /*                         ___       __ ___           _  _ 
    \  / | | |  |/  /\  |\ |    |  |\ | (_   |  /\  |\ | /  |_ 
     \/  |_| |_ |\ /--\ | \|   _|_ | \| __)  | /--\ | \| \_ |_ 
    */

    VkDebugUtilsMessengerCreateInfoEXT debugInfo = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .pNext = NULL,
        .flags = 0,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = vulkanDebugCallback,
        .pUserData = NULL
    };

    // Create a struct containing the information about the application
    VkApplicationInfo applicationInfo = {
        .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext              = NULL,
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
        .flags                   = 0,
        .pApplicationInfo        = &applicationInfo,
        .enabledLayerCount       = layerCount,
        .ppEnabledLayerNames     = layers,
        .enabledExtensionCount   = extensionCount,
        .ppEnabledExtensionNames = extensions
    };

    // Create the instance with the instance info and application instance structs we created
    VkInstance instance = VK_NULL_HANDLE;
    result = vkCreateInstance(&instanceInfo, NULL, &instance);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to create vulkan instance: %i\n", result);
        return EXIT_FAILURE;
    }
    fprintf(stdout, "Created vulkan instance\n");

    // Load the vulkan library and instance-level vulkan functions
    volkLoadInstance(instance);

    #if DEBUG_BUILD == 1
        VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
        result = vkCreateDebugUtilsMessengerEXT(instance, &debugInfo, NULL, &debugMessenger);
        if(result != VK_SUCCESS) {
            fprintf(stderr, "Failed to set vulkan debug callback: %i\n", result);
            return EXIT_FAILURE;
        }
        fprintf(stdout, "Added vulkan debug callback\n");
    #endif

    VkSurfaceKHR windowSurface = VK_NULL_HANDLE;
    result = glfwCreateWindowSurface(instance, window, NULL, &windowSurface);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to create vulkan surface for GLFW window: %i\n", result);
        return EXIT_FAILURE;
    }
    fprintf(stdout, "Created vulkan surface for GLFW window\n");

    // Get the physical devices array and register its cleanup method
    uint32_t physicalDeviceCount = 0;
    VkPhysicalDevice* physicalDevices = getVulkanPhysicalDevices(instance, &physicalDeviceCount);
    pushCleanupCallback(free, physicalDevices);
    // Ensure there is atleast one vulkan physical device before continuing
    if(physicalDeviceCount == 0) {
        fprintf(stdout, "No vulkan physical devices supported\n");
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "Successfully got list of vulkan physical devices\n");

    // Create a handle for our selected vulkan physical device and create variables to store the index of our selected queue families
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    bool graphicsQueueFamilyFound = false, presentationQueueFamilyFound = false;
    uint32_t graphicsQueueFamily = 0, presentationQueueFamily = 0;

    // Iterate through all vulkan physical devices and queue families
    for(uint32_t i = 0; i < physicalDeviceCount; i++) {
        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevices[i], &physicalDeviceProperties);

        uint32_t queueFamilyCount = 0;
        VkQueueFamilyProperties* queueFamilies = getVulkanPhysicalDeviceQueueFamilies(physicalDevices[i], &queueFamilyCount);
        for(uint32_t j = 0; j < queueFamilyCount; j++) {
            VkBool32 presentationSupport = VK_FALSE;
            result = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevices[i], j, windowSurface, &presentationSupport);
            if(result != VK_SUCCESS) {
                fprintf(stdout, "Failed to check if a physical device queue family supports presenting to the GLFW windows surface\n");
                exit(EXIT_FAILURE);
            }
            if(presentationSupport == VK_TRUE) {
                presentationQueueFamilyFound = true;
                presentationQueueFamily = j;
            }
            if(queueFamilies[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                graphicsQueueFamilyFound = true;
                graphicsQueueFamily = j;
            }

            if(graphicsQueueFamilyFound == true && presentationQueueFamilyFound == true && physicalDeviceProperties.apiVersion >= VK_API_VERSION_1_4) {
                physicalDevice = physicalDevices[i];
                break;
            }
        }
    }

    if(physicalDevice == VK_NULL_HANDLE) {
        fprintf(stdout, "Failed to find suitable vulkan physical device\n");
        exit(EXIT_FAILURE);
    }

    // Print the vulkan physical device we will use
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
    VkPhysicalDeviceFeatures physicalDeviceFeatures;
    vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);
    fprintf(
        stdout,
        "Found suitable vulkan physical device:\n    Name:                      %s\n    API Version:               %" PRIu32 ".%" PRIu32 ".%" PRIu32 "\n    Graphics Queue Family:     %" PRIu32 "\n    Presentation Queue Family: %" PRIu32 "\n",
        physicalDeviceProperties.deviceName,
        VK_VERSION_MAJOR(physicalDeviceProperties.apiVersion),
        VK_VERSION_MINOR(physicalDeviceProperties.apiVersion),
        VK_VERSION_PATCH(physicalDeviceProperties.apiVersion),
        graphicsQueueFamily,
        presentationQueueFamily
    );

    // Main application loop
    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    // Cleanup vulkan
    vkDestroySurfaceKHR(instance, windowSurface, NULL);
    #if DEBUG_BUILD == 1
        vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, NULL);
    #endif
    vkDestroyInstance(instance, NULL);
    volkFinalize();

    // Cleanup GLFW
    glfwDestroyWindow(window);
    glfwTerminate();

    // Exit with success
    return EXIT_SUCCESS;
}
