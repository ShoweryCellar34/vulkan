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
    VkSurfaceCapabilitiesKHR    surfaceCapabilities;
    uint32_t                    queueFamilyCount;
    VkQueueFamilyProperties2*   queueFamilyProperties;
    VkBool32*                   queueFamiliesPresentationSupport;
} PhysicalDeviceInfo;

MAKE_SLICE(PhysicalDeviceInfo, uint32_t, PhysicalDeviceInfo)

typedef struct {
    PhysicalDeviceInfo physicalDeviceInfo;
    StringSlice        deviceExtensions;
    uint32_t           score;

    uint32_t           surfaceFormatIndex;
    uint32_t           graphicsQueueFamilyIndex;
    uint32_t           presentationQueueFamilyIndex;
} LogicalDeviceCreateInfo;

typedef struct {
    PhysicalDeviceInfo physicalDeviceInfo;
    VkDevice           logicalDevice;

    VkQueue            graphicsQueue;
    VkQueue            presentationQueue;

    uint32_t           surfaceFormatIndex;
    uint32_t           graphicsQueueFamilyIndex;
    uint32_t           presentationQueueFamilyIndex;
} LogicalDeviceInfo;

// Destroys the members of a valid physical device info structure safely
void destroyVulkanPhysicalDeviceInfo(PhysicalDeviceInfo* physicalDeviceInfo);

// Get the most suitable vulkan physical device and return it and its configuration
LogicalDeviceCreateInfo getSuitableVulkanPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, StringSlice deviceExtensions, SurfaceFormatKHRSlice surfaceFormats);

// Get the list of supported physical devices
PhysicalDeviceInfoSlice getVulkanPhysicalDeviceInfos(VkInstance instance, VkSurfaceKHR surface);

// Creates and return a vulkan logical device
LogicalDeviceInfo createVulkanDevice(LogicalDeviceCreateInfo physicalDeviceCreateInfo);

#endif // DEVICES_H
