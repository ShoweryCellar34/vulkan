#include <devices.h>

// System Headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Library Headers
#include <GLFW/glfw3.h>

// Project Headers
#include <cleanup.h>

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

uint32_t getVulkanPhysicalDeviceSuitability(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const char** deviceExtensions, uint32_t deviceExtensionsCount, uint32_t* presentationQueueFamily, uint32_t* graphicsQueueFamily) {
    // get the properties of the physical device we are currently checking
    VkPhysicalDeviceProperties2 physicalDeviceProperties = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2
    };
    vkGetPhysicalDeviceProperties2(physicalDevice, &physicalDeviceProperties);

    // Ensure the current physical device supports at least vulkan 1.4
    if(physicalDeviceProperties.properties.apiVersion < VK_API_VERSION_1_4) {
        return 0;
    }

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
    popAndCallCleanupCallback();

    // Get the queue families supported by the current physical device
    uint32_t queueFamilyCount = 0;
    VkQueueFamilyProperties* queueFamilies = getVulkanPhysicalDeviceQueueFamilies(physicalDevice, &queueFamilyCount);
    // Add a cleanup callback to free the queue families array
    pushCleanupCallback(free, queueFamilies);

    bool presentationQueueFamilyFound = false, graphicsQueueFamilyFound = false;

    for(uint32_t i = 0; i < queueFamilyCount; i++) {
        // Check if the current queue family supports presentation
        VkBool32 presentationSupport = VK_FALSE;
        VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentationSupport);
        if(result != VK_SUCCESS) {
            fprintf(stdout, "Failed to check if a physical device queue family supports presenting to the window surface: %i\n", result);
        }
        if(presentationSupport == VK_TRUE) {
            presentationQueueFamilyFound = true;
            *presentationQueueFamily = i;
        }

        // Check if the current queue family supports graphics
        if(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphicsQueueFamilyFound = true;
            *graphicsQueueFamily = i;
        }

        // If there is one queue family that supports both graphics and presentation we can exit the loop early as this is the best case scenario
        if(presentationQueueFamilyFound == graphicsQueueFamilyFound) {
            break;
        }
    }
    // We can pop and call the most recent cleanup callback which frees the queue families array because we don't need it anymore
    popAndCallCleanupCallback();

    // If we haven't found every needed queue family on the same physical device we set the graphics and presentation queues to not found
    if(presentationQueueFamilyFound == false || graphicsQueueFamilyFound == false) {
        *presentationQueueFamily = 0;
        *graphicsQueueFamily = 0;
        return 0;
    }

    // Create a score variable to store the suitability of the physical device, set it to 1 by default because the device is atleast minimally suitable
    uint32_t score = 1;

    // If there is one queue family that supports both graphics and presentation we add 1 the score
    if(*presentationQueueFamily == *graphicsQueueFamily) {
        score += 1;
    }

    // If the physical device is a discrete gpu add 2 the score
    if(physicalDeviceProperties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 2;
    }

    return score;
}

VkQueueFamilyProperties* getVulkanPhysicalDeviceQueueFamilies(VkPhysicalDevice physicalDevice, uint32_t* queueFamiliesCount) {
    // Get the length of the supported queue families array
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, queueFamiliesCount, NULL);
    if(*queueFamiliesCount == 0) {
        return NULL;
    }

    // Allocate the supported physical devices array
    VkQueueFamilyProperties* queueFamilies = malloc(sizeof(VkQueueFamilyProperties) * *queueFamiliesCount);
    if(queueFamilies == NULL) {
        fprintf(stderr, "Failed to allocate supported queue families array of size %zu\n", *queueFamiliesCount * sizeof(VkQueueFamilyProperties));
        exit(EXIT_FAILURE);
    }

    // Fill the supported queue families array with the list of supported queue families for the physical device
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, queueFamiliesCount, queueFamilies);

    return queueFamilies;
}

static const char* deviceExtensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

const char** getVulkanDeviceExtensions(uint32_t* deviceExtensionsCount) {
    *deviceExtensionsCount = sizeof(deviceExtensions) / sizeof(*deviceExtensions);
    return deviceExtensions;
}

VkExtensionProperties* getSupportedVulkanDeviceExtensions(VkPhysicalDevice physicalDevice, uint32_t* supportedDeviceExtensionsCount) {
    // Create a variable to store the result of vulkan functions for error checking and reporting
    VkResult result = VK_SUCCESS;

    // Get the length of the supported device extensions array
    result = vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, supportedDeviceExtensionsCount, NULL);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to get number of supported device extensions: %i\n", result);
        exit(EXIT_FAILURE);
    }

    // If no device extensions are supported we exit early to avoid allocating 0 bytes and doing unnecessary operations
    if(*supportedDeviceExtensionsCount == 0) {
        return NULL;
    }

    // Allocate the supported device extensions array
    VkExtensionProperties* supportedDeviceExtensions = malloc(sizeof(VkExtensionProperties) * *supportedDeviceExtensionsCount);
    if(supportedDeviceExtensions == NULL) {
        fprintf(stderr, "Failed to allocate supported extensions array of size %zu\n", *supportedDeviceExtensionsCount * sizeof(VkExtensionProperties));
        exit(EXIT_FAILURE);
    }

    // Fill the supported device extensions array with the list of supported device extensions
    result = vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, supportedDeviceExtensionsCount, supportedDeviceExtensions);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to get number of supported device extensions: %i\n", result);
        exit(EXIT_FAILURE);
    }

    return supportedDeviceExtensions;
}
