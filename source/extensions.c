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

extensionNames getVulkanExtensionsAndValidate(bool debug) {
    // Get the extensions array
    extensionNames extensions = getVulkanExtensions(debug);
    fprintf(stdout, "Successfully got list of required extensions:\n");

    // Print all the extensions we will use
    for(uint32_t i = 0; i < extensions.count; i++) {
        fprintf(stdout, "    %s\n", extensions.names[i]);
    }

    // Get the supported extensions array and register its cleanup method
    extensionProperties supportedExtensions = getSupportedVulkanExtensions();
    fprintf(stdout, "Successfully got list of supported extensions:\n");

    // Print all the extensions supported
    for(uint32_t i = 0; i < supportedExtensions.count; i++) {
        fprintf(stdout, "    %s\n", supportedExtensions.properties[i].extensionName);
    }

    // Ensure all requested extensions are supported
    for(uint32_t i = 0; i < extensions.count; i++) {
        bool extensionSupported = false;
        for(uint32_t j = 0; j < supportedExtensions.count; j++) {
            if(!strcmp(extensions.names[i], supportedExtensions.properties[j].extensionName)) {
                extensionSupported = true;
            }
        }
        if(extensionSupported == false) {
            fprintf(stderr, "Not all requested extensions are supported\n");
            free(supportedExtensions.properties);
            exit(EXIT_FAILURE);
        }
    }
    fprintf(stdout, "All required extensions are supported\n");

    // Free the supported extensions array
    free(supportedExtensions.properties);

    // Return the list of extensions
    return extensions;
}

extensionNames getVulkanExtensions(bool debug) {
    // Get the array of extensions requested by GLFW
    extensionNames glfwExtensions;
    glfwExtensions.names = glfwGetRequiredInstanceExtensions((uint32_t*)&glfwExtensions.count);
    if(glfwExtensions.names == NULL) {
        const char* errorMessage = NULL;
        int errorCode = glfwGetError(&errorMessage);
        fprintf(stderr, "Failed to get array of extensions required by GLFW:\n    Error Code:    %i\n    Error Message: %s\n", errorCode, errorMessage);
        exit(EXIT_FAILURE);
    }

    // Allocate memory for an array to store both glfw and maybe the debug extension
    extensionNames extensions = {
        .count = glfwExtensions.count + (debug == true ? 1 : 0),
    };
    extensions.names = malloc(sizeof(*extensions.names) * extensions.count);
    if(extensions.names == NULL) {
        fprintf(stderr, "Failed to allocate extensions array of length %zu\n", sizeof(*extensions.names) * extensions.count);
        exit(EXIT_FAILURE);
    }

    // Copy glfw extensions to joined array and maybe add debug extension to the end
    memcpy(extensions.names, glfwExtensions.names, sizeof(*glfwExtensions.names) * glfwExtensions.count);
    if(debug == true) {
        extensions.names[glfwExtensions.count] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    }

    // Return the list of extensions 
    return extensions;
}

extensionProperties getSupportedVulkanExtensions() {
    // Get the length of the supported extensions array
    extensionProperties supportedExtensions = {0};
    VkResult result = vkEnumerateInstanceExtensionProperties(NULL, &supportedExtensions.count, NULL);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to get number of supported extensions: %i\n", result);
        exit(EXIT_FAILURE);
    }

    // If no extensions are supported we exit early to avoid allocating 0 bytes and doing unnecessary operations
    if(supportedExtensions.count == 0) {
        return supportedExtensions;
    }

    // Allocate the supported extensions array
    supportedExtensions.properties = malloc(sizeof(*supportedExtensions.properties) * supportedExtensions.count);
    if(supportedExtensions.properties == NULL) {
        fprintf(stderr, "Failed to allocate supported extensions array of size %zu\n", sizeof(*supportedExtensions.properties) * supportedExtensions.count);
        exit(EXIT_FAILURE);
    }

    // Fill the supported extensions array with the list of supported extensions
    result = vkEnumerateInstanceExtensionProperties(NULL, &supportedExtensions.count, supportedExtensions.properties);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to get supported extensions: %i\n", result);
        exit(EXIT_FAILURE);
    }

    // Return the suported extensions
    return supportedExtensions;
}

extensionNames getVulkanDeviceExtensions() {
    static const char* deviceExtensionNamesArray[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    static extensionNames deviceExtensions = {
        .names = deviceExtensionNamesArray,
        .count = 1,
    };
    // Retrun the list of debug ayers
    return deviceExtensions;
}

extensionProperties getSupportedVulkanDeviceExtensions(VkPhysicalDevice physicalDevice) {
    // Get the length of the supported device extensions array
    extensionProperties supportedDeviceExtensions = {0};
    VkResult result = vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &supportedDeviceExtensions.count, NULL);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to get number of supported device extensions: %i\n", result);
        exit(EXIT_FAILURE);
    }

    // If no device extensions are supported we exit early to avoid allocating 0 bytes and doing unnecessary operations
    if(supportedDeviceExtensions.count == 0) {
        return supportedDeviceExtensions;
    }

    // Allocate the supported device extensions array
    supportedDeviceExtensions.properties = malloc(sizeof(*supportedDeviceExtensions.properties) * supportedDeviceExtensions.count);
    if(supportedDeviceExtensions.properties == NULL) {
        fprintf(stderr, "Failed to allocate supported device extensions array of size %zu\n", sizeof(*supportedDeviceExtensions.properties) * supportedDeviceExtensions.count);
        exit(EXIT_FAILURE);
    }

    // Fill the supported device extensions array with the list of supported device extensions
    result = vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &supportedDeviceExtensions.count, supportedDeviceExtensions.properties);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to get supported device extensions: %i\n", result);
        exit(EXIT_FAILURE);
    }

    // Return the suported extensions
    return supportedDeviceExtensions;
}
