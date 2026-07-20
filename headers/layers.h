#ifndef LAYERS_H
#define LAYERS_H

// System Headers
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Library Headers
#include <volk.h>

// Project Headers
#include <macros.h>


// Get the list of vulkan layers and validate them
StringSlice getVulkanLayersAndValidate(bool debug);

// Get the list of layers we are going to use
StringSlice getVulkanLayers(bool debug);

// Get the list of supported layers
LayerPropertiesSlice getSupportedVulkanLayers();

#endif // LAYERS_H
