#ifndef EXTENSIONS_H
#define EXTENSIONS_H

// System Headers
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Library Headers
#include <volk.h>

// Project Headers
#include <macros.h>


// Get the list of vulkan extensions and validate them
StringSlice getVulkanExtensionsAndValidate(bool debug);

// Get the list of extensions we are going to use
StringSlice getVulkanExtensions(bool debug);

// Get the list of supported extensions
ExtensionPropertiesSlice getSupportedVulkanExtensions(void);

// Get the list of device extensions we are going to use
StringSlice getVulkanDeviceExtensions(void);

// Get the list of supported device extensions
ExtensionPropertiesSlice getSupportedVulkanDeviceExtensions(VkPhysicalDevice physicalDevice);

#endif // EXTENSIONS_H
