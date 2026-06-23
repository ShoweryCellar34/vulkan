#include <devices.h>

// System Headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Library Headers
#include <GLFW/glfw3.h>

// Project Headers
#include <cleanup.h>
#include <extensions.h>

VkPhysicalDevice* getVulkanPhysicalDevices(VkInstance instance, uint32_t* physicalDevicesCount) {
    // Create a variable to store the result of vulkan functions for error checking and reporting
    VkResult result = VK_SUCCESS;

    // Get the length of the supported physical devices array
    result = vkEnumeratePhysicalDevices(instance, physicalDevicesCount, NULL);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to get number of supported physical devices: %i\n", result);
        exit(EXIT_FAILURE);
    }

    // If no physical devices are supported we exit early to avoid allocating 0 bytes and doing unnecessary operations
    if(*physicalDevicesCount == 0) {
        return NULL;
    }

    // Allocate the supported physical devices array
    VkPhysicalDevice* physicalDevices = malloc(sizeof(VkPhysicalDevice) * *physicalDevicesCount);
    if(physicalDevices == NULL) {
        fprintf(stderr, "Failed to allocate supported physical devices array of size %zu\n", *physicalDevicesCount * sizeof(VkPhysicalDevice));
        exit(EXIT_FAILURE);
    }

    // Fill the supported physical device array with the list of physical devices
    result = vkEnumeratePhysicalDevices(instance, physicalDevicesCount, physicalDevices);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to get number of supported physical devices: %i\n", result);
        exit(EXIT_FAILURE);
    }

    return physicalDevices;
}

uint32_t getVulkanPhysicalDeviceSuitability(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const char** deviceExtensions, uint32_t deviceExtensionsCount, uint32_t* graphicsQueueFamily, uint32_t* presentationQueueFamily) {
    // Create a variable to store the result of vulkan functions for error checking and reporting
    VkResult result = VK_SUCCESS;

    // get the properties of the physical device we are currently checking
    VkPhysicalDeviceProperties2 physicalDeviceProperties = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2
    };
    vkGetPhysicalDeviceProperties2(physicalDevice, &physicalDeviceProperties);

    // Ensure the current physical device supports at least vulkan 1.4
    if(physicalDeviceProperties.properties.apiVersion < VK_API_VERSION_1_4) {
        return 0;
    }

    // Get the length of the supported surface formats array
    uint32_t surfaceFormatsCount = 0;
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatsCount, NULL);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to get number of supported surface formats: %i\n", result);
        exit(EXIT_FAILURE);
    }

    // If no surface formats are supported we exit early to avoid allocating 0 bytes and doing unnecessary operations
    if(surfaceFormatsCount == 0) {
        return 0;
    }

    // Allocate the supported surface formats array
    VkSurfaceFormatKHR* surfaceFormats = malloc(sizeof(VkSurfaceFormatKHR) * surfaceFormatsCount);
    if(surfaceFormats == NULL) {
        fprintf(stderr, "Failed to allocate supported surface formats array of size %zu\n", surfaceFormatsCount * sizeof(VkSurfaceFormatKHR));
        exit(EXIT_FAILURE);
    }
    pushCleanupCallback(free, surfaceFormats);

    // Fill the supported surface formats array with the list of supported surface formats
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatsCount, surfaceFormats);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to get number of supported surface formats: %i\n", result);
        exit(EXIT_FAILURE);
    }

    // Ensure all required surface formats are supported
    bool surfaceFormatSupported = false;
    for(uint32_t i = 0; i < surfaceFormatsCount; i++) {
        if(surfaceFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB && surfaceFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            surfaceFormatSupported = true;
        }
    }
    if(surfaceFormatSupported == false) {
        return 0;
    }
    popAndCallCleanupCallback(0);

    // Get the supported device extensions array and register its cleanup method
    uint32_t supportedDeviceExtensionsCount = 0;
    VkExtensionProperties* supportedDeviceExtensions = getSupportedVulkanDeviceExtensions(physicalDevice, &supportedDeviceExtensionsCount);
    pushCleanupCallback(free, supportedDeviceExtensions);

    // Ensure all requested device extensions are supported
    for(uint32_t i = 0; i < deviceExtensionsCount; i++) {
        bool deviceExtensionSupported = false;
        for(uint32_t j = 0; j < supportedDeviceExtensionsCount; j++) {
            if(!strcmp(deviceExtensions[i], supportedDeviceExtensions[j].extensionName)) {
                deviceExtensionSupported = true;
            }
        }
        if(deviceExtensionSupported == false) {
            return 0;
        }
    }
    // We can pop and call the most recent cleanup callback which frees the supported device extensions array because we don't need it anymore
    popAndCallCleanupCallback(0);

    // Get the queue families supported by the current physical device
    uint32_t queueFamilyCount = 0;
    VkQueueFamilyProperties* queueFamilies = getVulkanPhysicalDeviceQueueFamilies(physicalDevice, &queueFamilyCount);
    // Add a cleanup callback to free the queue families array
    pushCleanupCallback(free, queueFamilies);

    bool graphicsQueueFamilyFound = false, presentationQueueFamilyFound = false;

    for(uint32_t i = 0; i < queueFamilyCount; i++) {
        // Check if the current queue family supports graphics
        if(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphicsQueueFamilyFound = true;
            *graphicsQueueFamily = i;
        }

        // Check if the current queue family supports presentation
        VkBool32 presentationSupport = VK_FALSE;
        result = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentationSupport);
        if(result != VK_SUCCESS) {
            fprintf(stdout, "Failed to check if a physical device queue family supports presenting to the window surface: %i\n", result);
            exit(EXIT_FAILURE);
        }
        if(presentationSupport == VK_TRUE) {
            presentationQueueFamilyFound = true;
            *presentationQueueFamily = i;
        }

        // If there is one queue family that supports both graphics and presentation we can exit the loop early as this is the best case scenario
        if(graphicsQueueFamilyFound == presentationQueueFamilyFound) {
            break;
        }
    }
    // We can pop and call the most recent cleanup callback which frees the queue families array because we don't need it anymore
    popAndCallCleanupCallback(0);

    // If we haven't found every needed queue family on the same physical device we set the graphics and presentation queues to not found
    if(graphicsQueueFamilyFound == false || presentationQueueFamilyFound == false) {
        *graphicsQueueFamily = 0;
        *presentationQueueFamily = 0;
        return 0;
    }

    // Create a score variable to store the suitability of the physical device, set it to 1 by default because the device is atleast minimally suitable
    uint32_t score = 1;

    // If there is one queue family that supports both graphics and presentation we add 1 the score
    if(*graphicsQueueFamily == *presentationQueueFamily) {
        score += 1;
    }

    // If the physical device is a discrete gpu add 2 the score
    if(physicalDeviceProperties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 2;
    }

    return score;
}
