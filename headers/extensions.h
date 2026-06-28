#ifndef EXTENSIONS_H
#define EXTENSIONS_H

// System Headers
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Library Headers
#include <volk.h>

// Project Headers

// This struct stores an array of vulkan extension names and the number of them
typedef struct {
    const char** names;
    uint32_t     count;
} ExtensionNames;

// This struct stores an array of vulkan extension properties and the number of them
typedef struct {
    VkExtensionProperties* properties;
    uint32_t               count;
} ExtensionProperties;

// Get the list of vulkan extensions and validate them
ExtensionNames getVulkanExtensionsAndValidate(bool debug);

// Get the list of extensions we are going to use
ExtensionNames getVulkanExtensions(bool debug);

// Get the list of supported extensions
ExtensionProperties getSupportedVulkanExtensions();

// Get the list of device extensions we are going to use
ExtensionNames getVulkanDeviceExtensions();

// Get the list of supported device extensions
ExtensionProperties getSupportedVulkanDeviceExtensions(VkPhysicalDevice physicalDevice);

#endif // EXTENSIONS_H
