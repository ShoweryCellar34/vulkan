#ifndef EXTENSIONS_H
#define EXTENSIONS_H

// System Headers
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Library Headers
#include <volk.h>

// Project Headers

// Get the list of extensions we are going to use
const char** getVulkanExtensions(uint32_t* extensionsCount, bool debug);

// Get the list of supported extensions
VkExtensionProperties* getSupportedVulkanExtensions(uint32_t* supportedExtensionsCount);

#endif // EXTENSIONS_H
