#include <layers.h>

// System Headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Library Headers

// Project Headers
#include <cleanup.h>

layerNames getVulkanLayersAndValidate(bool debug) {
    // Get the layers array
    layerNames layers = getVulkanLayers(debug);
    fprintf(stdout, "Successfully got list of required layers:\n");

    // Print all the layers we will use
    for(uint32_t i = 0; i < layers.count; i++) {
        fprintf(stdout, "    %s\n", layers.names[i]);
    }

    // Get the supported layers array and register its cleanup method
    layerProperties supportedLayers = getSupportedVulkanLayers();
    fprintf(stdout, "Successfully got list of supported layers:\n");

    // Print all the layers supported
    for(uint32_t i = 0; i < supportedLayers.count; i++) {
        fprintf(stdout, "    %s\n", supportedLayers.properties[i].layerName);
    }

    // Ensure all requested layers are supported
    for(uint32_t i = 0; i < layers.count; i++) {
        bool layerSupported = false;
        for(uint32_t j = 0; j < supportedLayers.count; j++) {
            if(!strcmp(layers.names[i], supportedLayers.properties[j].layerName)) {
                layerSupported = true;
            }
        }
        if(layerSupported == false) {
            fprintf(stderr, "Not all requested layers are supported\n");
            free(supportedLayers.properties);
            exit(EXIT_FAILURE);
        }
    }
    fprintf(stdout, "All required layers are supported\n");

    // Free the supported layers array
    free(supportedLayers.properties);

    // Return the list of layers
    return layers;
}

layerNames getVulkanLayers(bool debug) {
    // If we are in debug build return the khronos validation layer, and if we are not in a debug build we return NULL and set layer count to 1 and 0 respectively
    if(debug == true) {
        static const char* layerNamesArray[] = {
            "VK_LAYER_KHRONOS_validation"
        };
        static layerNames layers = {
            .names = layerNamesArray,
            .count = 1,
        };
        // Retrun the list of debug layers
        return layers;
    } else {
        static layerNames layers = {0};
        // Retrun the list of layers
        return layers;
    }
}

layerProperties getSupportedVulkanLayers() {
    // Get the length of the supported layers array
    layerProperties supportedLayers = {0};
    VkResult result = vkEnumerateInstanceLayerProperties(&supportedLayers.count, NULL);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to get number of supported layers: %i\n", result);
        exit(EXIT_FAILURE);
    }

    // If no layers are supported we exit early to avoid allocating 0 bytes and doing unnecessary operations
    if(supportedLayers.count == 0) {
        return supportedLayers;
    }

    // Allocate the supported layers array
    supportedLayers.properties = malloc(sizeof(*supportedLayers.properties) * supportedLayers.count);
    if(supportedLayers.properties == NULL) {
        fprintf(stderr, "Failed to allocate supported layers array of size %zu\n", sizeof(*supportedLayers.properties) * supportedLayers.count);
        exit(EXIT_FAILURE);
    }

    // Fill the supported layers array with the list of supported layers
    result = vkEnumerateInstanceLayerProperties(&supportedLayers.count, supportedLayers.properties);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to get supported layers: %i\n", result);
        exit(EXIT_FAILURE);
    }

    // Return the suported layers slice
    return supportedLayers;
}
