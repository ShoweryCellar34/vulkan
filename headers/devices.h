#ifndef DEVICES_H
#define DEVICES_H

// System Headers
#include <stdbool.h>
#include <stdint.h>

// Library Headers
#include <volk.h>

// Project Headers

// This struct stores a vulkan physical device and the graphics and presentation queue family indicies
typedef struct {
    VkPhysicalDevice   physicalDevice;
    uint32_t           graphicsQueueFamilyIndex;
    uint32_t           presentationQueueFamilyIndex;
    VkSurfaceFormatKHR format;
} physicalDeviceAndConfiguration;

// Get the most suitable vulkan physical device and return it and its configuration
physicalDeviceAndConfiguration getVulkanSuitablePhysicalDevice(VkInstance instance, VkSurfaceKHR surface, const char** deviceExtensions, uint32_t deviceExtensionsCount);

// Get the list of supported physical devices
VkPhysicalDevice* getVulkanPhysicalDevices(VkInstance instance, uint32_t* physicalDevicesCount);

// Creates and return a vulkan logical device
VkDevice createVulkanDevice(physicalDeviceAndConfiguration physicalDevice);

#endif // DEVICES_H
