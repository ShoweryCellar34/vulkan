#include <extensions.h>

// System Headers
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Library Headers
#include <GLFW/glfw3.h>

// Project Headers

const char** getVulkanExtensions(uint32_t* extensionsCount, bool debug) {
    // Declare the variable to store the extensions array
    const char** extensions = NULL;

    // Get the array of extensions requested by GLFW
    const uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions((uint32_t*)&glfwExtensionCount);
    if(glfwExtensions == NULL || glfwExtensionCount == 0) {
        fprintf(stderr, "Failed to get array of extensions required by GLFW\n");
        exit(EXIT_FAILURE);
    }

    // Allocate memory for an array to store both glfw and maybe debug extensions
    *extensionsCount = glfwExtensionCount + (debug == true ? 1 : 0);
    extensions = malloc(*extensionsCount * sizeof(const char*));
    if(extensions == NULL) {
        fprintf(stderr, "Failed to allocate extensions array of length %zu\n", *extensionsCount * sizeof(const char*));
        exit(EXIT_FAILURE);
    }

    // Copy glfw extensions to joined array and maybe add debug extension to the end
    memcpy(extensions, glfwExtensions, sizeof(const char*) * glfwExtensionCount);
    if(debug == true) {
        extensions[glfwExtensionCount] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    }

    // Return the list of extensions 
    return extensions;
}

VkExtensionProperties* getSupportedVulkanExtensions(uint32_t* supportedExtensionsCount) {
    // Create a variable to store the result of vulkan functions for error checking and reporting
    VkResult result = VK_SUCCESS;

    // Get the length of the supported extensions array
    result = vkEnumerateInstanceExtensionProperties(NULL, supportedExtensionsCount, NULL);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to get number of supported extensions: %i\n", result);
        exit(EXIT_FAILURE);
    }

    // If no extensions are supported we exit early to avoid allocating 0 bytes and doing unnecessary operations
    if(*supportedExtensionsCount == 0) {
        return NULL;
    }

    // Allocate the supported extensions array
    VkExtensionProperties* supportedExtensions = malloc(sizeof(VkExtensionProperties) * *supportedExtensionsCount);
    if(supportedExtensions == NULL) {
        fprintf(stderr, "Failed to allocate supported extensions array of size %zu\n", *supportedExtensionsCount * sizeof(VkExtensionProperties));
        exit(EXIT_FAILURE);
    }

    // Fill the supported extensions array with the list of supported extensions
    result = vkEnumerateInstanceExtensionProperties(NULL, supportedExtensionsCount, supportedExtensions);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to get number of supported extensions: %i\n", result);
        exit(EXIT_FAILURE);
    }

    return supportedExtensions;
}

VkQueueFamilyProperties* getVulkanPhysicalDeviceQueueFamilies(VkPhysicalDevice physicalDevice, uint32_t* queueFamiliesCount) {
    // Get the length of the supported queue families array
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, queueFamiliesCount, NULL);
    if(*queueFamiliesCount == 0) {
        return NULL;
    }

    // Allocate the supported physical devices array
    VkQueueFamilyProperties* queueFamilies = malloc(sizeof(VkQueueFamilyProperties) * *queueFamiliesCount);
    if(queueFamilies == NULL) {
        fprintf(stderr, "Failed to allocate supported queue families array of size %zu\n", *queueFamiliesCount * sizeof(VkQueueFamilyProperties));
        exit(EXIT_FAILURE);
    }

    // Fill the supported queue families array with the list of supported queue families for the physical device
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, queueFamiliesCount, queueFamilies);

    return queueFamilies;
}

static const char* deviceExtensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

const char** getVulkanDeviceExtensions(uint32_t* deviceExtensionsCount) {
    *deviceExtensionsCount = sizeof(deviceExtensions) / sizeof(*deviceExtensions);
    return deviceExtensions;
}

VkExtensionProperties* getSupportedVulkanDeviceExtensions(VkPhysicalDevice physicalDevice, uint32_t* supportedDeviceExtensionsCount) {
    // Create a variable to store the result of vulkan functions for error checking and reporting
    VkResult result = VK_SUCCESS;

    // Get the length of the supported device extensions array
    result = vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, supportedDeviceExtensionsCount, NULL);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to get number of supported device extensions: %i\n", result);
        exit(EXIT_FAILURE);
    }

    // If no device extensions are supported we exit early to avoid allocating 0 bytes and doing unnecessary operations
    if(*supportedDeviceExtensionsCount == 0) {
        return NULL;
    }

    // Allocate the supported device extensions array
    VkExtensionProperties* supportedDeviceExtensions = malloc(sizeof(VkExtensionProperties) * *supportedDeviceExtensionsCount);
    if(supportedDeviceExtensions == NULL) {
        fprintf(stderr, "Failed to allocate supported extensions array of size %zu\n", *supportedDeviceExtensionsCount * sizeof(VkExtensionProperties));
        exit(EXIT_FAILURE);
    }

    // Fill the supported device extensions array with the list of supported device extensions
    result = vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, supportedDeviceExtensionsCount, supportedDeviceExtensions);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to get number of supported device extensions: %i\n", result);
        exit(EXIT_FAILURE);
    }

    return supportedDeviceExtensions;
}
