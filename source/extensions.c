#include <extensions.h>

// System Headers
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Library Headers
#include <volk.h>
#include <GLFW/glfw3.h>

// Project Headers

static const char** extensions = NULL;

void freeVulkanExtensionsArray(void) {
    if(extensions != NULL) {
        free(extensions);
    }
}

const char** getVulkanExtensions(uint32_t* extensionCount, bool debug) {
    if(extensions != NULL) {
        free(extensions);
        extensions = NULL;
    }

    // Create a variable to store the result of vulkan functions for error checking and reporting
    VkResult result = VK_SUCCESS;

    // Get the array of vulkan extensions requested by GLFW
    const uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions((uint32_t*)&glfwExtensionCount);
    if(glfwExtensions == NULL || glfwExtensionCount == 0) {
        fprintf(stderr, "Failed to get array of vulkan extensions required by GLFW\n");
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "Got array of vulkan extensions required by GLFW\n");

    // Allocate memory for an array to store both glfw and maybe debug vulkan extensions
    *extensionCount = glfwExtensionCount + (debug == true ? 1 : 0);
    extensions = malloc(*extensionCount * sizeof(const char*));
    if(extensions == NULL) {
        fprintf(stderr, "Failed to allocate vulkan extensions array of length %zu\n", *extensionCount * sizeof(const char*));
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "Allocated vulkan extensions array of length %zu\n", *extensionCount * sizeof(const char*));

    // Add the function that frees the vulkan extensions array to the list of functions to be called when we exit
    atexit(freeVulkanExtensionsArray);

    // Copy glfw vulkan extensions to joined array and maybe add debug extension to the end
    memcpy(extensions, glfwExtensions, sizeof(const char*) * glfwExtensionCount);
    fprintf(stdout, "Filled vulkan extensions array with required GLFW extensions\n");
    if(debug == true) {
        extensions[glfwExtensionCount] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
        fprintf(stdout, "Appended vulkan debug extensions to vulkan extensions array\n");
    }

    // Get the length of the supported vulkan extensions array
    const uint32_t supportedExtensionCount = 0;
    result = vkEnumerateInstanceExtensionProperties(NULL, (uint32_t*)&supportedExtensionCount, NULL);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to get number of supported vulkan extensions: %i\n", result);
        exit(EXIT_FAILURE);
    }

    // Ensure that there is at least one vulkan extension supported before continuing
    if(supportedExtensionCount < *extensionCount) {
        fprintf(stderr, "Less vulkan extensions supported than number of vulkan extensions requested\n");
        exit(EXIT_FAILURE);
    }

    // Fill an array with the list of supported vulkan extensions
    VkExtensionProperties* supportedExtensions = malloc(sizeof(VkExtensionProperties) * supportedExtensionCount);
    if(supportedExtensions == NULL) {
        fprintf(stderr, "Failed to allocate supported vulkan extensions array of length %zu\n", supportedExtensionCount * sizeof(const char*));
        exit(EXIT_FAILURE);
    }
    result = vkEnumerateInstanceExtensionProperties(NULL, (uint32_t*)&supportedExtensionCount, supportedExtensions);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to get number of supported vulkan extensions: %i\n", result);
        free(supportedExtensions);
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "Got array of extensions supported by vulkan\n");

    // Check if the requested vulkan extensions are supported
    uint32_t extensionsSupported = 0;
    for(uint32_t i = 0; i < supportedExtensionCount; i++) {
        for(uint32_t j = 0; j < *extensionCount; j++) {
            if(!strcmp(supportedExtensions[i].extensionName, extensions[j])) {
                extensionsSupported++;
            }
        }
    }
    free(supportedExtensions);
    if(extensionsSupported != *extensionCount) {
        fprintf(stderr, "Not all requested vulkan extensions are supported\n");
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "All requested vulkan extensions are supported\n");

    return extensions;
}
