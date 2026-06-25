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
} extensionNames;

// This struct stores an array of vulkan extension properties and the number of them
typedef struct {
    VkExtensionProperties* properties;
    uint32_t               count;
} extensionProperties;

// Get the list of vulkan extensions and validate them
extensionNames getVulkanExtensionsAndValidate(bool debug);

// Get the list of extensions we are going to use
extensionNames getVulkanExtensions(bool debug);

// Get the list of supported extensions
extensionProperties getSupportedVulkanExtensions();

// Get the list of device extensions we are going to use
extensionNames getVulkanDeviceExtensions();

// Get the list of supported device extensions
extensionProperties getSupportedVulkanDeviceExtensions(VkPhysicalDevice physicalDevice);

#endif // EXTENSIONS_H
