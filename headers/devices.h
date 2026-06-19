#ifndef DEVICES_H
#define DEVICES_H

// System Headers
#include <stdbool.h>
#include <stdint.h>

// Library Headers
#include <volk.h>

// Project Headers

// Get the list of supported physical devices
VkPhysicalDevice* getVulkanPhysicalDevices(VkInstance instance, uint32_t* physicalDevicesCount);

// get the suitability of a physical device
uint32_t getVulkanPhysicalDeviceSuitability(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const char** deviceExtensions, uint32_t deviceExtensionsCount, uint32_t* presentationQueueFamily, uint32_t* graphicsQueueFamily);

// Get the list of queue families supported by a physical device
VkQueueFamilyProperties* getVulkanPhysicalDeviceQueueFamilies(VkPhysicalDevice physicalDevice, uint32_t* queueFamiliesCount);

#endif // DEVICES_H
