// System Headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <errno.h>

// Library Headers
#include <volk.h>
#include <GLFW/glfw3.h>

// Project Headers
#include <projectData.h>

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 450

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT *data,
    void *user_data
) {
    (void)severity; (void)type; (void)user_data;
    fprintf(stderr, "validation error: %s\n", data->pMessage);
    return VK_FALSE;
}

int main(int argc, char* argv[]) {
    // Output project info and argument count
    printf("Project Name:    %s\nProject Version: %s\nArg Count:       %i\n", PROJECT_NAME, PROJECT_VERSION, argc);

    // Output arguments
    for(int i = 0; i < argc; i++) {
        printf("Arg %i:           %s\n", i, argv[i]);
    }

    // Create a variable to store the result of vulkan functions for error checking and reporting
    VkResult result = VK_SUCCESS;

    // Load the vulkan library and global-level vulkan functions
    result = volkInitialize();
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to load vulkan with volk: %d", result);
        return EXIT_FAILURE;
    }

    // initialize GLFW and check for vulkan support, exit if initialization fails or vulkan not supported
    if(glfwInit() != GLFW_TRUE || glfwVulkanSupported() != GLFW_TRUE) {
        fprintf(stderr, "Failed to initialize glfw or vulkan not supported");
        return EXIT_FAILURE;
    }

    // Disable the creation of an OpenGL Context and disable window resizing
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // Create the window
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, PROJECT_NAME, NULL, NULL);

    // Get the list of vulkan extensions needed by GLFW
    const uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions((uint32_t*)&glfwExtensionCount);
    if(glfwExtensions == NULL) {
        fprintf(stderr, "Failed to get required vulkan extensions for glfw");
        return EXIT_FAILURE;
    }

    // list the vulkan extension we will use
    const char* userExtensions[] = {
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME
    };
    const uint32_t userExtensionCount = sizeof(userExtensions) / sizeof(const char*);

    // Allocate memory for an array to store both glfw and user vulkan extensions
    const char** extensions = malloc((glfwExtensionCount + userExtensionCount) * sizeof(const char*));
    if(extensions == NULL) {
        fprintf(stderr, "Failed to allocate vulkan extension list");
        return EXIT_FAILURE;
    }
    const uint32_t extensionCount = glfwExtensionCount + userExtensionCount;

    // Copy glfw and user vulkan extensions to joined array
    memcpy(extensions, glfwExtensions, sizeof(const char*) * glfwExtensionCount);
    memcpy(&extensions[glfwExtensionCount], &userExtensions, sizeof(const char*) * userExtensionCount);

    // Get the length of the list of supported vulkan extensions
    uint32_t supportedExtensionCount = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &supportedExtensionCount, NULL);

    // Fill an array with the list of supported vulkan extensions
    VkExtensionProperties* supportedExtensions = malloc(sizeof(VkExtensionProperties) * supportedExtensionCount);
    vkEnumerateInstanceExtensionProperties(NULL, &supportedExtensionCount, supportedExtensions);

    // Check if the vulkan extensions needed are supported
    uint32_t extensionsSupported = 0;
    for(uint32_t i = 0; i < supportedExtensionCount; i++) {
        for(uint32_t j = 0; j < extensionCount; j++) {
            const char* name = supportedExtensions[supportedExtensionCount].extensionName;
            if(strcmp(name, extensions[extensionCount])) {
                extensionsSupported++;
            }
        }
    }
    if(extensionsSupported != extensionCount) {
        fprintf(stderr, "Not all vulkan extensions are supported");
        return EXIT_FAILURE;
    }

    // Create the array of vulkan layers we will use
    const char* layers[] = {
        "VK_LAYER_KHRONOS_validation"
    };
    const uint32_t layerCount = sizeof(layers) / sizeof(const char*);

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
        .pNext                   = NULL,
        .flags                   = 0,
        .pApplicationInfo        = &applicationInfo,
        .enabledLayerCount       = layerCount,
        .ppEnabledLayerNames     = layers,
        .enabledExtensionCount   = extensionCount,
        .ppEnabledExtensionNames = extensions
    };

    // Create the instance with the instance info and application instance structs we created
    VkInstance instance;
    result = vkCreateInstance(&instanceInfo, NULL, &instance);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to create vulkan instance: %d", result);
        return EXIT_FAILURE;
    }

    // Load the vulkan library and instance-level vulkan functions
    volkLoadInstance(instance);

    // Main application loop
    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    // Cleanup GLFW
    glfwDestroyWindow(window);
    glfwTerminate();

    // Exit with success
    return EXIT_SUCCESS;
}
