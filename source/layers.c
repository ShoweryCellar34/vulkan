#include <layers.h>

// System Headers
#include <stdio.h>
#include <stdlib.h>

// Library Headers
#include <volk.h>

// Project Headers
#include <cleanup.h>

bool validateVulkanLayers(const char** layers, uint32_t layerCount) {
    // If we try to validate no layers we return true, indicating that there were no unsupported requested layers
    if(layerCount == 0) {
        return true;
    }

    // Create a variable to store the result of vulkan functions for error checking and reporting
    VkResult result = VK_SUCCESS;

    // Get the length of the supported vulkan layers array
    const uint32_t supportedLayerCount = 0;
    result = vkEnumerateInstanceLayerProperties((uint32_t*)&supportedLayerCount, NULL);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to get number of supported vulkan layers: %i\n", result);
        exit(EXIT_FAILURE);
    }

    // Ensure that there is at least one vulkan layer supported before continuing
    if(supportedLayerCount < layerCount) {
        fprintf(stderr, "Less vulkan layers supported than number of vulkan layers requested\n");
        return false;
    }

    // Allocate the supported vulkan layers array
    VkLayerProperties* supportedLayers = malloc(sizeof(VkLayerProperties) * supportedLayerCount);
    if(supportedLayers == NULL) {
        fprintf(stderr, "Failed to allocate supported vulkan layers array of size %zu\n", supportedLayerCount * sizeof(const char*));
        exit(EXIT_FAILURE);
    }

    // Temporarily add a callback that frees the supported layers array when we exit, this will be popped from the stack when we finish with the array
    pushCleanupCallback((cleanupCallback){
        .callback     = free,
        .callbackData = (void*)supportedLayers
    });

    // Fill the supported vulkan layers array with the list of supported vulkan layers
    result = vkEnumerateInstanceLayerProperties((uint32_t*)&supportedLayerCount, supportedLayers);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to get number of supported vulkan layers: %i\n", result);
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "Got array of layers supported by vulkan\n");

    // Count how many vulkan layers we requested are supported
    uint32_t layersSupported = 0;
    for(uint32_t i = 0; i < supportedLayerCount; i++) {
        for(uint32_t j = 0; j < layerCount; j++) {
            if(!strcmp(supportedLayers[i].layerName, layers[j])) {
                layersSupported++;
            }
        }
    }

    // Pop the callback to free the supported vulkan layers array and call it, this is because we no longer need the array and can free it now
    popAndCallCleanupCallback();

    // Check if the number of vulkan layers we requested that are supported is equal to the number of requested vulkan layers
    if(layersSupported != layerCount) {
        return false;
    }

    return true;
}
