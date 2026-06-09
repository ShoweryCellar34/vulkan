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

    // Get the vulkan extensions array
    uint32_t extensionCount = 0;
    const char** extensions = getVulkanExtensions(&extensionCount, true);

    // Validate the presence of our desired vulkan extensions in vulkan
    if(validateVulkanExtensions(extensions, extensionCount) == false) {
        fprintf(stderr, "Not all requested vulkan extensions are supported\n");
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "All requested vulkan extensions are supported:\n");

    // Print all the vulkan extensions we will use
    for(uint32_t i = 0; i < extensionCount; i++) {
        fprintf(stdout, "    %s\n", extensions[i]);
    }

    // Create the array of vulkan layers we will use
    const char* layers[] = {
        "VK_LAYER_KHRONOS_validation"
    };
    const uint32_t layerCount = ARRAY_COUNT(layers);

    // Validate the presence of our desired vulkan layers in vulkan
    if(validateVulkanLayers(layers, layerCount) == false) {
        fprintf(stderr, "Not all requested vulkan layers are supported\n");
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "All requested vulkan layers are supported:\n");

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

    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
    result = vkCreateDebugUtilsMessengerEXT(instance, &debugInfo, NULL, &debugMessenger);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to set vulkan debug callback: %i\n", result);
        return EXIT_FAILURE;
    }
    fprintf(stdout, "Added vulkan debug callback\n");

    // Get the length of the array of vulkan physical devices
    uint32_t physicalDeviceCount = 0;
    result = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, NULL);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to get number of vulkan physical devices: %i\n", result);
        return EXIT_FAILURE;
    }

    // Ensure that there is at least one vulkan physical device before continuing
    if(physicalDeviceCount < 1) {
        fprintf(stderr, "There must be at least one vulkan physical device to continue\n");
        return EXIT_FAILURE;
    }

    // Fill an array with the list of supported vulkan extensions
    VkPhysicalDevice* physicalDevices = malloc(physicalDeviceCount * sizeof(VkPhysicalDevice));
    if(physicalDevices == NULL) {
        fprintf(stderr, "Failed to allocate vulkan physical devices array of length %zu\n", physicalDeviceCount * sizeof(VkPhysicalDevice));
        return EXIT_FAILURE;
    }
    result = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to get number of vulkan physical devices: %i\n", result);
        return EXIT_FAILURE;
    }
    fprintf(stdout, "Got array of vulkan physical devices\n");

    for(uint32_t i = 0; i < physicalDeviceCount; i++) {
        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevices[i], &physicalDeviceProperties);
        VkPhysicalDeviceFeatures physicalDeviceFeatures;
        vkGetPhysicalDeviceFeatures(physicalDevices[i], &physicalDeviceFeatures);

        fprintf(stdout, "%s\n", physicalDeviceProperties.deviceName);
    }

    // Main application loop
    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    // Cleanup vulkan
    vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, NULL);
    vkDestroyInstance(instance, NULL);
    volkFinalize();

    // Cleanup GLFW
    glfwDestroyWindow(window);
    glfwTerminate();

    // Exit with success
    return EXIT_SUCCESS;
}
