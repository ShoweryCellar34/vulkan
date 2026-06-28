#ifndef LAYERS_H
#define LAYERS_H

// System Headers
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Library Headers
#include <volk.h>

// Project Headers

// This struct stores an array of vulkan layer names and the number of them
typedef struct {
    const char** names;
    uint32_t     count;
} LayerNames;

// This struct stores an array of vulkan layer properties and the number of them
typedef struct {
    VkLayerProperties* properties;
    uint32_t           count;
} LayerProperties;

// Get the list of vulkan layers and validate them
LayerNames getVulkanLayersAndValidate(bool debug);

// Get the list of layers we are going to use
LayerNames getVulkanLayers(bool debug);

// Get the list of supported layers
LayerProperties getSupportedVulkanLayers();

#endif // LAYERS_H
