#include <layers.h>

// System Headers
#include <stdio.h>
#include <stdlib.h>

// Library Headers

// Project Headers
#include <cleanup.h>

const char** getVulkanLayers(uint32_t* layersCount, bool debug) {
    static const char* layers[] = {
        "VK_LAYER_KHRONOS_validation"
    };
    *layersCount = debug ? sizeof(layers) / sizeof(*layers) : 0;
    return debug ? layers : NULL;
}

VkLayerProperties* getSupportedVulkanLayers(uint32_t* supportedLayersCount) {
    // Create a variable to store the result of vulkan functions for error checking and reporting
    VkResult result = VK_SUCCESS;

    // Get the length of the supported layers array
    result = vkEnumerateInstanceLayerProperties(supportedLayersCount, NULL);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to get number of supported layers: %i\n", result);
        exit(EXIT_FAILURE);
    }

    // If no layers are supported we exit early to avoid allocating 0 bytes and doing unnecessary operations
    if(*supportedLayersCount == 0) {
        return NULL;
    }

    // Allocate the supported layers array
    VkLayerProperties* supportedLayers = malloc(sizeof(VkLayerProperties) * *supportedLayersCount);
    if(supportedLayers == NULL) {
        fprintf(stderr, "Failed to allocate supported layers array of size %zu\n", *supportedLayersCount * sizeof(VkLayerProperties));
        exit(EXIT_FAILURE);
    }

    // Fill the supported layers array with the list of supported layers
    result = vkEnumerateInstanceLayerProperties(supportedLayersCount, supportedLayers);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to get number of supported layers: %i\n", result);
        exit(EXIT_FAILURE);
    }

    return supportedLayers;
}
