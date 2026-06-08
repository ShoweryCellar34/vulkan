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
#include <projectData.h>

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 450

#define ARRAY_COUNT(x) sizeof(x) / sizeof(x[0])
#define NULLFREE(x)    free(x); x = NULL

void testCallback(void* data) {
    fprintf(stdout, "Test callback!");
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

    /*                          _   ___ _       __ ___  _        __ 
    \  / | | |  |/  /\  |\ |   |_ \/ | |_ |\ | (_   |  / \ |\ | (_  
     \/  |_| |_ |\ /--\ | \|   |_ /\ | |_ | \| __) _|_ \_/ | \| __) 
    */

    // // Get the array of vulkan extensions requested by GLFW
    // const uint32_t glfwExtensionCount = 0;
    // const char** glfwExtensions = glfwGetRequiredInstanceExtensions((uint32_t*)&glfwExtensionCount);
    // if(glfwExtensions == NULL) {
    //     fprintf(stderr, "Failed to get required vulkan extensions for GLFW\n");
    //     return EXIT_FAILURE;
    // }
    // fprintf(stdout, "Got array of vulkan extensions requested by GLFW\n");

    // // list the vulkan extension we will use
    // const char* userExtensions[] = {
    //     VK_EXT_DEBUG_UTILS_EXTENSION_NAME
    // };
    // const uint32_t userExtensionCount = ARRAY_COUNT(userExtensions);

    // // Allocate memory for an array to store both glfw and user vulkan extensions
    // const uint32_t extensionCount = glfwExtensionCount + userExtensionCount;
    // const char** extensions = malloc(extensionCount * sizeof(const char*));
    // if(extensions == NULL) {
    //     fprintf(stderr, "Failed to allocate required vulkan extension array of length %zu\n", extensionCount * sizeof(const char*));
    //     return EXIT_FAILURE;
    // }

    // // Copy glfw and user vulkan extensions to joined array
    // memcpy(extensions, glfwExtensions, sizeof(const char*) * glfwExtensionCount);
    // memcpy(&extensions[glfwExtensionCount], &userExtensions, sizeof(const char*) * userExtensionCount);
    // fprintf(stdout, "Created concatenated array of all requested vulkan extensions\n");

    // // Get the length of the array of supported vulkan extensions
    // const uint32_t supportedExtensionCount = 0;
    // result = vkEnumerateInstanceExtensionProperties(NULL, (uint32_t*)&supportedExtensionCount, NULL);
    // if(result != VK_SUCCESS) {
    //     fprintf(stderr, "Failed to get number of supported vulkan extensions: %i\n", result);
    //     return EXIT_FAILURE;
    // }

    // // Ensure that there is at least one vulkan extension supported before continuing
    // if(extensionCount < 1) {
    //     fprintf(stderr, "There must be at least one vulkan extension supported to continue\n");
    //     return EXIT_FAILURE;
    // }

    // // Fill an array with the list of supported vulkan extensions
    // VkExtensionProperties* supportedExtensions = malloc(sizeof(VkExtensionProperties) * supportedExtensionCount);
    // if(supportedExtensions == NULL) {
    //     fprintf(stderr, "Failed to allocate supported vulkan extensions array of length %zu\n", supportedExtensionCount * sizeof(const char*));
    //     return EXIT_FAILURE;
    // }
    // result = vkEnumerateInstanceExtensionProperties(NULL, (uint32_t*)&supportedExtensionCount, supportedExtensions);
    // if(result != VK_SUCCESS) {
    //     fprintf(stderr, "Failed to get number of supported vulkan extensions: %i\n", result);
    //     return EXIT_FAILURE;
    // }
    // fprintf(stdout, "Got array of extensions supported by vulkan\n");

    // // Check if the requested vulkan extensions are supported
    // uint32_t extensionsSupported = 0;
    // for(uint32_t i = 0; i < supportedExtensionCount; i++) {
    //     for(uint32_t j = 0; j < extensionCount; j++) {
    //         if(!strcmp(supportedExtensions[i].extensionName, extensions[j])) {
    //             extensionsSupported++;
    //         }
    //     }
    // }
    // NULLFREE(supportedExtensions);
    // if(extensionsSupported != extensionCount) {
    //     fprintf(stderr, "Not all requested vulkan extensions are supported\n");
    //     return EXIT_FAILURE;
    // }
    // fprintf(stdout, "All requested vulkan extensions are supported\n");

    uint32_t extensionCount = 0;
    const char** extensions = getVulkanExtensions(&extensionCount, true);

    /*                                     _  _   __ 
    \  / | | |  |/  /\  |\ |   |   /\ \_/ |_ |_) (_  
     \/  |_| |_ |\ /--\ | \|   |_ /--\ |  |_ | \ __) 
    */

    // Create the array of vulkan layers we will use
    const char* layers[] = {
        "VK_LAYER_KHRONOS_validation"
    };
    const uint32_t layerCount = ARRAY_COUNT(layers);

    // Get the length of the array of supported vulkan layers
    const uint32_t supportedLayerCount = 0;
    result = vkEnumerateInstanceLayerProperties((uint32_t*)&supportedLayerCount, NULL);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to get number of supported vulkan layers: %i\n", result);
        return EXIT_FAILURE;
    }

    // Ensure that there is at least one vulkan layer before continuing
    if(layerCount < 1) {
        fprintf(stderr, "There must be at least one vulkan layer to continue\n");
        return EXIT_FAILURE;
    }

    // Fill an array with the list of supported vulkan layers
    VkLayerProperties* supportedLayers = malloc(sizeof(VkLayerProperties) * supportedLayerCount);
    if(supportedLayers == NULL) {
        fprintf(stderr, "Failed to allocate supported vulkan layers array of length %" PRIu32 "\n", supportedLayerCount);
        return EXIT_FAILURE;
    }
    result = vkEnumerateInstanceLayerProperties((uint32_t*)&supportedLayerCount, supportedLayers);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to get number of supported vulkan layers: %i\n", result);
        return EXIT_FAILURE;
    }
    fprintf(stdout, "Got array of layers supported by vulkan\n");

    // Check if the requested vulkan layers are supported
    uint32_t layersSupported = 0;
    for(uint32_t i = 0; i < supportedLayerCount; i++) {
        for(uint32_t j = 0; j < layerCount; j++) {
            if(!strcmp(supportedLayers[i].layerName, layers[j])) {
                layersSupported++;
            }
        }
    }
    NULLFREE(supportedLayers);
    if(layersSupported != layerCount) {
        fprintf(stderr, "Not all requested vulkan layers are supported\n");
        return EXIT_FAILURE;
    }
    fprintf(stdout, "All requested vulkan layers are supported\n");

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

    exitCallback callback = {
        .callback     = testCallback,
        .callbackData = NULL
    };
    pushExitCallback(callback);
    pushExitCallback(callback);
    pushExitCallback(callback);
    pushExitCallback(callback);
    pushExitCallback(callback);

    // Exit with success
    return EXIT_SUCCESS;
}
