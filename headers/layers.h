#ifndef LAYERS_H
#define LAYERS_H

// System Headers
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Library Headers
#include <volk.h>

// Project Headers

// Get the list of layers supported by vulkan
VkLayerProperties* getSupportedVulkanLayers(uint32_t* layerCount);

#endif // LAYERS_H
