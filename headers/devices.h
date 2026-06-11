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

// Get the list of queue families supported by a vulkan physical device
VkQueueFamilyProperties* getVulkanPhysicalDeviceQueueFamilies(VkPhysicalDevice physicalDevice, uint32_t* queueFamilyCount);

#endif // DEVICES_H
