#ifndef LAYERS_H
#define LAYERS_H

// System Headers
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Library Headers
#include <volk.h>

// Project Headers

// Get the list of vulkan layers and validate them
const char** getVulkanLayersAndValidate(uint32_t* layersCount, bool debug);

// Get the list of layers we are going to use
const char** getVulkanLayers(uint32_t* layersCount, bool debug);

// Get the list of supported layers
VkLayerProperties* getSupportedVulkanLayers(uint32_t* supportedLayersCount);

#endif // LAYERS_H
