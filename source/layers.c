#include <layers.h>

// System Headers
#include <stdio.h>
#include <stdlib.h>

// Library Headers

// Project Headers
#include <cleanup.h>

VkLayerProperties* getSupportedVulkanLayers(uint32_t* supportedLayerCount) {
    // Create a variable to store the result of vulkan functions for error checking and reporting
    VkResult result = VK_SUCCESS;

    // Get the length of the supported vulkan layers array
    result = vkEnumerateInstanceLayerProperties(supportedLayerCount, NULL);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to get number of supported vulkan layers: %i\n", result);
        exit(EXIT_FAILURE);
    }

    // If no vulkan layers are supported we exit early to avoid allocating 0 bytes and doing unnecessary operations
    if(*supportedLayerCount == 0) {
        return NULL;
    }

    // Allocate the supported vulkan layers array
    VkLayerProperties* supportedLayers = malloc(sizeof(VkLayerProperties) * *supportedLayerCount);
    if(supportedLayers == NULL) {
        fprintf(stderr, "Failed to allocate supported vulkan layers array of size %zu\n", *supportedLayerCount * sizeof(VkLayerProperties));
        exit(EXIT_FAILURE);
    }

    // Fill the supported vulkan layers array with the list of supported vulkan layers
    result = vkEnumerateInstanceLayerProperties(supportedLayerCount, supportedLayers);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to get number of supported vulkan layers: %i\n", result);
        exit(EXIT_FAILURE);
    }

    return supportedLayers;
}
