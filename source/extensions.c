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

const char** getVulkanExtensions(uint32_t* extensionCount, bool debug) {
    // Declare the variable to store the vulkan extensions array
    const char** extensions = NULL;

    // Get the array of vulkan extensions requested by GLFW
    const uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions((uint32_t*)&glfwExtensionCount);
    if(glfwExtensions == NULL || glfwExtensionCount == 0) {
        fprintf(stderr, "Failed to get array of vulkan extensions required by GLFW\n");
        exit(EXIT_FAILURE);
    }

    // Allocate memory for an array to store both glfw and maybe debug vulkan extensions
    *extensionCount = glfwExtensionCount + (debug == true ? 1 : 0);
    extensions = malloc(*extensionCount * sizeof(const char*));
    if(extensions == NULL) {
        fprintf(stderr, "Failed to allocate vulkan extensions array of length %zu\n", *extensionCount * sizeof(const char*));
        exit(EXIT_FAILURE);
    }

    // Copy glfw vulkan extensions to joined array and maybe add debug extension to the end
    memcpy(extensions, glfwExtensions, sizeof(const char*) * glfwExtensionCount);
    if(debug == true) {
        extensions[glfwExtensionCount] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
        fprintf(stdout, "Appended vulkan debug extensions to vulkan extensions array\n");
    }

    // Return the list of extensions 
    return extensions;
}

VkExtensionProperties* getSupportedVulkanExtensions(uint32_t* supportedExtensionCount) {
    // Create a variable to store the result of vulkan functions for error checking and reporting
    VkResult result = VK_SUCCESS;

    // Get the length of the supported vulkan extensions array
    result = vkEnumerateInstanceExtensionProperties(NULL, supportedExtensionCount, NULL);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to get number of supported vulkan extensions: %i\n", result);
        exit(EXIT_FAILURE);
    }

    // If no vulkan extensions are supported we exit early to avoid allocating 0 bytes and doing unnecessary operations
    if(*supportedExtensionCount == 0) {
        return NULL;
    }

    // Allocate the supported vulkan extensions array
    VkExtensionProperties* supportedExtensions = malloc(sizeof(VkExtensionProperties) * *supportedExtensionCount);
    if(supportedExtensions == NULL) {
        fprintf(stderr, "Failed to allocate supported vulkan extensions array of size %zu\n", *supportedExtensionCount * sizeof(VkExtensionProperties));
        exit(EXIT_FAILURE);
    }

    // Fill the supported vulkan extensions array with the list of supported vulkan extensions
    result = vkEnumerateInstanceExtensionProperties(NULL, supportedExtensionCount, supportedExtensions);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to get number of supported vulkan extensions: %i\n", result);
        exit(EXIT_FAILURE);
    }

    return supportedExtensions;
}
