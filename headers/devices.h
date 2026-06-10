#ifndef DEVICES_H
#define DEVICES_H

// System Headers
#include <stdbool.h>
#include <stdint.h>

// Library Headers
#include <volk.h>

// Project Headers

// Get the list of physical devices supported by vulkan
VkPhysicalDevice* getVulkanPhysicalDevices(VkInstance instance, uint32_t* physicalDeviceCount);

// Check if a vulkan physical device is suitable for usage
bool isVulkanPhysicalDeviceSuitable(VkInstance instance, VkPhysicalDevice physicalDevice);

#endif // DEVICES_H
