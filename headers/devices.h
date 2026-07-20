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
    VkSurfaceKHR                surface;

    VkPhysicalDeviceProperties2 physicalDeviceProperties;
    ExtensionPropertiesSlice    supportedExtensions;
    SurfaceFormatKHRSlice       surfaceFormats;
    uint32_t                    queueFamilyCount;
    VkQueueFamilyProperties2*   queueFamilyProperties;
    VkBool32*                   queueFamiliesPresentationSupport;
} PhysicalDeviceInfo;

MAKE_SLICE(PhysicalDeviceInfo, uint32_t, PhysicalDeviceInfo)

typedef struct {
    PhysicalDeviceInfo physicalDeviceInfo;
    uint32_t           score;

    uint32_t           surfaceFormatIndex;
    uint32_t           graphicsQueueFamilyIndex;
    uint32_t           presentationQueueFamilyIndex;
} PhysicalDeviceCreateInfo;

// Destroys the members of a valid physical device info structure safely
void destroyVulkanPhysicalDeviceInfo(PhysicalDeviceInfo* physicalDeviceInfo);

// Get the most suitable vulkan physical device and return it and its configuration
PhysicalDeviceCreateInfo getSuitableVulkanPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, StringSlice deviceExtensions, SurfaceFormatKHRSlice surfaceFormats);

// Get the list of supported physical devices
PhysicalDeviceInfoSlice getVulkanPhysicalDeviceInfos(VkInstance instance, VkSurfaceKHR surface);

// Creates and return a vulkan logical device
VkDevice createVulkanDevice(PhysicalDeviceCreateInfo physicalDeviceCreateInfo, StringSlice deviceExtensions);

#endif // DEVICES_H
