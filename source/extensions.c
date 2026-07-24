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

StringSlice getVulkanExtensionsAndValidate(bool debug) {
    // Get the extensions array
    StringSlice extensions = getVulkanExtensions(debug);
    fprintf(stdout, "Successfully got list of required extensions:\n");

    // Print all the extensions we will use
    for(uint32_t i = 0; i < extensions.count; i++) {
        fprintf(stdout, "    %s\n", extensions.data[i]);
    }

    // Get the supported extensions array and register its cleanup method
    ExtensionPropertiesSlice supportedExtensions = getSupportedVulkanExtensions();
    fprintf(stdout, "Successfully got list of supported extensions:\n");

    // Print all the extensions supported
    for(uint32_t i = 0; i < supportedExtensions.count; i++) {
        fprintf(stdout, "    %s\n", supportedExtensions.data[i].extensionName);
    }

    // Ensure all requested extensions are supported
    for(uint32_t i = 0; i < extensions.count; i++) {
        bool extensionSupported = false;
        for(uint32_t j = 0; j < supportedExtensions.count; j++) {
            if(!strcmp(extensions.data[i], supportedExtensions.data[j].extensionName)) {
                extensionSupported = true;
            }
        }
        if(extensionSupported == false) {
            fprintf(stderr, "Not all requested extensions are supported\n");
            free(supportedExtensions.data);
            exit(EXIT_FAILURE);
        }
    }
    fprintf(stdout, "All required extensions are supported\n");

    // Free the supported extensions array
    free(supportedExtensions.data);

    // Return the list of extensions
    return extensions;
}

StringSlice getVulkanExtensions(bool debug) {
    StringSlice glfwExtensions;
    glfwExtensions.data = glfwGetRequiredInstanceExtensions(&glfwExtensions.count);

    static const char* extensions[] = {
        "",
        "",
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME
    };

    memcpy((void*)extensions, glfwExtensions.data, SLICE_SIZE(glfwExtensions));

    // Return the list of extensions 
    return (StringSlice){
        .count = (debug == true ? 3 : 2),
        .data  = extensions
    };
}

ExtensionPropertiesSlice getSupportedVulkanExtensions(void) {
    // Get the length of the supported extensions array
    ExtensionPropertiesSlice supportedExtensions = {0};
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
    supportedExtensions.data = malloc(SLICE_SIZE(supportedExtensions));
    if(supportedExtensions.data == NULL) {
        fprintf(stderr, "Failed to allocate supported extensions array of size %zu\n", SLICE_SIZE(supportedExtensions));
        exit(EXIT_FAILURE);
    }

    // Fill the supported extensions array with the list of supported extensions
    result = vkEnumerateInstanceExtensionProperties(NULL, &supportedExtensions.count, supportedExtensions.data);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to get supported extensions: %i\n", result);
        exit(EXIT_FAILURE);
    }

    // Return the suported extensions
    return supportedExtensions;
}

StringSlice getVulkanDeviceExtensions(void) {
    static const char* extensionNames[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    return (StringSlice){
        .count = 1,
        .data  = (const char**)&extensionNames
    };
}

ExtensionPropertiesSlice getSupportedVulkanDeviceExtensions(VkPhysicalDevice physicalDevice) {
    // Get the length of the supported device extensions array
    ExtensionPropertiesSlice supportedDeviceExtensions = {0};
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
    supportedDeviceExtensions.data = malloc(SLICE_SIZE(supportedDeviceExtensions));
    if(supportedDeviceExtensions.data == NULL) {
        fprintf(stderr, "Failed to allocate supported device extensions array of size %zu\n", SLICE_SIZE(supportedDeviceExtensions));
        exit(EXIT_FAILURE);
    }

    // Fill the supported device extensions array with the list of supported device extensions
    result = vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &supportedDeviceExtensions.count, supportedDeviceExtensions.data);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to get supported device extensions: %i\n", result);
        exit(EXIT_FAILURE);
    }

    // Return the suported extensions
    return supportedDeviceExtensions;
}
