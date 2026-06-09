#ifndef EXTENSIONS_H
#define EXTENSIONS_H

// System Headers
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Library Headers

// Project Headers

// Get the list of extensions we are going to use for vulkan
const char** getVulkanExtensions(uint32_t* extensionCount, bool debug);

// Validate the presence of an array of vulkan extensions in vulkan
bool validateVulkanExtensions(const char** extensions, uint32_t extensionCount);

#endif // EXTENSIONS_H
