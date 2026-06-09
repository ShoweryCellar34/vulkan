#ifndef LAYERS_H
#define LAYERS_H

// System Headers
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Library Headers

// Project Headers

// Validate the presence of an array of vulkan layers in vulkan
bool validateVulkanLayers(const char** layers, uint32_t layerCount);

#endif // LAYERS_H
