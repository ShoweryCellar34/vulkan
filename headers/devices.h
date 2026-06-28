#ifndef DEVICES_H
#define DEVICES_H

// System Headers
#include <stdbool.h>
#include <stdint.h>

// Library Headers
#include <volk.h>

// Project Headers
#include <extensions.h>

// This struct stores a vulkan physical device and the graphics and presentation queue family indicies
typedef struct {
    VkPhysicalDevice            physicalDevice;
    VkPhysicalDeviceProperties2 physicalDeviceProperties;
    ExtensionProperties         supportedExtensions;
    VkSurfaceKHR                surface;
    uint32_t                    surfaceFormatsCount; 
    VkSurfaceFormatKHR*         surfaceFormats;
    uint32_t                    queueFamilyPropertiesCount;
    VkQueueFamilyProperties2*   queueFamilyProperties;
    VkBool32*                   queueFamilyPresentationSupport;
} PhysicalDeviceInfo;

// Get the most suitable vulkan physical device and return it and its configuration
PhysicalDeviceInfo getVulkanSuitablePhysicalDevice(VkInstance instance, VkSurfaceKHR surface, ExtensionNames deviceExtensions);

// Get the list of supported physical devices
VkPhysicalDevice* getVulkanPhysicalDevices(VkInstance instance, uint32_t* physicalDevicesCount);

// Creates and return a vulkan logical device
VkDevice createVulkanDevice(PhysicalDeviceInfo physicalDevice, ExtensionNames deviceExtensions);

#endif // DEVICES_H
