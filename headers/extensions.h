#ifndef EXTENSIONS_H
#define EXTENSIONS_H

// System Headers
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Library Headers
#include <volk.h>

// Project Headers

// Get the list of vulkan extensions and validate them
const char** getVulkanExtensionsAndValidate(uint32_t* extensionsCount, bool debug);

// Get the list of extensions we are going to use
const char** getVulkanExtensions(uint32_t* extensionsCount, bool debug);

// Get the list of supported extensions
VkExtensionProperties* getSupportedVulkanExtensions(uint32_t* supportedExtensionsCount);

// Get the list of device extensions we are going to use
const char** getVulkanDeviceExtensions(uint32_t* deviceExtensionsCount);

// Get the list of supported device extensions
VkExtensionProperties* getSupportedVulkanDeviceExtensions(VkPhysicalDevice physicalDevice, uint32_t* supportedDeviceExtensionsCount);

#endif // EXTENSIONS_H
