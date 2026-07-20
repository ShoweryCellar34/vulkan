#include <layers.h>

// System Headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Library Headers

// Project Headers
#include <cleanup.h>

StringSlice getVulkanLayersAndValidate(bool debug) {
    // Get the layers array
    StringSlice layers = getVulkanLayers(debug);
    fprintf(stdout, "Successfully got list of required layers:\n");

    // Print all the layers we will use
    for(uint32_t i = 0; i < layers.count; i++) {
        fprintf(stdout, "    %s\n", layers.data[i]);
    }

    // Get the supported layers array and register its cleanup method
    LayerPropertiesSlice supportedLayers = getSupportedVulkanLayers();
    fprintf(stdout, "Successfully got list of supported layers:\n");

    // Print all the layers supported
    for(uint32_t i = 0; i < supportedLayers.count; i++) {
        fprintf(stdout, "    %s\n", supportedLayers.data[i].layerName);
    }

    // Ensure all requested layers are supported
    for(uint32_t i = 0; i < layers.count; i++) {
        bool layerSupported = false;
        for(uint32_t j = 0; j < supportedLayers.count; j++) {
            if(!strcmp(layers.data[i], supportedLayers.data[j].layerName)) {
                layerSupported = true;
            }
        }
        if(layerSupported == false) {
            fprintf(stderr, "Not all requested layers are supported\n");
            free(supportedLayers.data);
            exit(EXIT_FAILURE);
        }
    }
    fprintf(stdout, "All required layers are supported\n");

    // Free the supported layers array
    free(supportedLayers.data);

    // Return the list of layers
    return layers;
}

StringSlice getVulkanLayers(bool debug) {
    static const char* layerNames[] = {
        "VK_LAYER_KHRONOS_validation"
    };
    return (StringSlice){
        .count = (debug == true ? 1 : 0),
        .data  = layerNames
    };
}

LayerPropertiesSlice getSupportedVulkanLayers() {
    // Get the length of the supported layers array
    LayerPropertiesSlice supportedLayers = {0};
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
    supportedLayers.data = malloc(sizeof(*supportedLayers.data) * supportedLayers.count);
    if(supportedLayers.data == NULL) {
        fprintf(stderr, "Failed to allocate supported layers array of size %zu\n", SLICE_SIZE(supportedLayers));
        exit(EXIT_FAILURE);
    }

    // Fill the supported layers array with the list of supported layers
    result = vkEnumerateInstanceLayerProperties(&supportedLayers.count, supportedLayers.data);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to get supported layers: %i\n", result);
        exit(EXIT_FAILURE);
    }

    // Return the suported layers slice
    return supportedLayers;
}
