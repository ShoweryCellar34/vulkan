#include <devices.h>

// System Headers
#include <stdio.h>
#include <stdlib.h>

// Library Headers
#include <GLFW/glfw3.h>

// Project Headers

VkPhysicalDevice* getVulkanPhysicalDevices(VkInstance instance, uint32_t* physicalDeviceCount) {
    // Create a variable to store the result of vulkan functions for error checking and reporting
    VkResult result = VK_SUCCESS;

    // Get the length of the supported vulkan physical devices array
    result = vkEnumeratePhysicalDevices(instance, physicalDeviceCount, NULL);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to get number of supported vulkan physical devices: %i\n", result);
        exit(EXIT_FAILURE);
    }

    // If no vulkan physical devices are supported we exit early to avoid allocating 0 bytes and doing unnecessary operations
    if(*physicalDeviceCount == 0) {
        return NULL;
    }

    // Allocate the supported vulkan physical devices array
    VkPhysicalDevice* physicalDevices = malloc(sizeof(VkPhysicalDevice) * *physicalDeviceCount);
    if(physicalDevices == NULL) {
        fprintf(stderr, "Failed to allocate supported vulkan physical devices array of size %zu\n", *physicalDeviceCount * sizeof(VkPhysicalDevice));
        exit(EXIT_FAILURE);
    }

    // Fill the supported vulkan physical device array with the list of vulkan physical devices
    result = vkEnumeratePhysicalDevices(instance, physicalDeviceCount, physicalDevices);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to get number of supported vulkan physical devices: %i\n", result);
        exit(EXIT_FAILURE);
    }

    return physicalDevices;
}

VkQueueFamilyProperties* getVulkanPhysicalDeviceQueueFamilies(VkPhysicalDevice physicalDevice, uint32_t* queueFamilyCount) {
    // Get the length of the supported vulkan queue families array
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, queueFamilyCount, NULL);
    if(*queueFamilyCount == 0) {
        return NULL;
    }

    // Allocate the supported vulkan physical devices array
    VkQueueFamilyProperties* queueFamilies = malloc(sizeof(VkQueueFamilyProperties) * *queueFamilyCount);
    if(queueFamilies == NULL) {
        fprintf(stderr, "Failed to allocate supported vulkan queue families array of size %zu\n", *queueFamilyCount * sizeof(VkQueueFamilyProperties));
        exit(EXIT_FAILURE);
    }

    // Fill the supported vulkan queue families array with the list of supported vulkan queue families for the vulkan physical device
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, queueFamilyCount, queueFamilies);

    return queueFamilies;
}
