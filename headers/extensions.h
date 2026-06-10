#ifndef EXTENSIONS_H
#define EXTENSIONS_H

// System Headers
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Library Headers
#include <volk.h>

// Project Headers

// Get the list of extensions we are going to use for vulkan
const char** getVulkanExtensions(uint32_t* extensionCount, bool debug);

// Get the list of extensions supported by vulkan
VkExtensionProperties* getSupportedVulkanExtensions(uint32_t* extensionCount);

#endif // EXTENSIONS_H
