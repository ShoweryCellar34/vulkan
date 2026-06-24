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

uint32_t getVulkanPhysicalDeviceSuitability(physicalDeviceAndConfiguration* physicalDevice, VkSurfaceKHR surface, const char** deviceExtensions, uint32_t deviceExtensionsCount) {
    // Create a variable to store the result of vulkan functions for error checking and reporting
    VkResult result = VK_SUCCESS;

    // get the properties of the physical device we are currently checking
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice->physicalDevice, &physicalDeviceProperties);

    // Ensure the current physical device supports at least vulkan 1.4
    if(physicalDeviceProperties.apiVersion < VK_API_VERSION_1_4) {
        return 0;
    }

    // Get the length of the supported surface formats array
    uint32_t surfaceFormatsCount = 0;
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice->physicalDevice, surface, &surfaceFormatsCount, NULL);
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

    // Fill the supported surface formats array with the list of supported surface formats
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice->physicalDevice, surface, &surfaceFormatsCount, surfaceFormats);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to get  supported surface formats: %i\n", result);
        free(surfaceFormats);
        exit(EXIT_FAILURE);
    }

    // Ensure all required surface formats are supported
    bool surfaceFormatSupported = false;
    for(uint32_t i = 0; i < surfaceFormatsCount; i++) {
        if(surfaceFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB && surfaceFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            surfaceFormatSupported = true;
            physicalDevice->format = surfaceFormats[i];
        }
    }
    free(surfaceFormats);
    if(surfaceFormatSupported == false) {
        return 0;
    }

    // Get the supported device extensions array and register its cleanup method
    uint32_t supportedDeviceExtensionsCount = 0;
    VkExtensionProperties* supportedDeviceExtensions = getSupportedVulkanDeviceExtensions(physicalDevice->physicalDevice, &supportedDeviceExtensionsCount);

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
    // Free the supported device extensions array because we don't need it anymore
    free(supportedDeviceExtensions);

    // Get the length of the supported queue families array
    uint32_t queueFamiliesCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice->physicalDevice, &queueFamiliesCount, NULL);
    if(queueFamiliesCount == 0) {
        return 0;
    }

    // Allocate the supported physical devices array
    VkQueueFamilyProperties* queueFamilies = malloc(sizeof(VkQueueFamilyProperties) * queueFamiliesCount);
    if(queueFamilies == NULL) {
        fprintf(stderr, "Failed to allocate supported queue families array of size %zu\n", queueFamiliesCount * sizeof(VkQueueFamilyProperties));
        exit(EXIT_FAILURE);
    }

    // Fill the supported queue families array with the list of supported queue families for the physical device
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice->physicalDevice, &queueFamiliesCount, queueFamilies);

    bool graphicsQueueFamilyFound = false, presentationQueueFamilyFound = false;

    for(uint32_t i = 0; i < queueFamiliesCount; i++) {
        // Check if the current queue family supports graphics
        if(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphicsQueueFamilyFound = true;
            physicalDevice->graphicsQueueFamilyIndex = i;
        }

        // Check if the current queue family supports presentation
        VkBool32 presentationSupport = VK_FALSE;
        result = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice->physicalDevice, i, surface, &presentationSupport);
        if(result != VK_SUCCESS) {
            fprintf(stdout, "Failed to check if a physical device queue family supports presenting to the window surface: %i\n", result);
            free(queueFamilies);
            exit(EXIT_FAILURE);
        }
        if(presentationSupport == VK_TRUE) {
            presentationQueueFamilyFound = true;
            physicalDevice->presentationQueueFamilyIndex = i;
        }

        // If there is one queue family that supports both graphics and presentation we can exit the loop early as this is the best case scenario
        if(graphicsQueueFamilyFound == presentationQueueFamilyFound) {
            break;
        }
    }
    // Free the queue families array because we don't need it anymore
    free(queueFamilies);

    // If we haven't found every needed queue family on the same physical device we set the graphics and presentation queues to not found
    if(graphicsQueueFamilyFound == false || presentationQueueFamilyFound == false) {
        physicalDevice->graphicsQueueFamilyIndex = 0;
        physicalDevice->presentationQueueFamilyIndex = 0;
        return 0;
    }

    // Create a score variable to store the suitability of the physical device, set it to 1 by default because the device is atleast minimally suitable
    uint32_t score = 1;

    // If there is one queue family that supports both graphics and presentation we add 1 the score
    if(physicalDevice->graphicsQueueFamilyIndex == physicalDevice->presentationQueueFamilyIndex) {
        score += 1;
    }

    // If the physical device is a discrete gpu add 2 the score
    if(physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 2;
    }

    return score;
}

physicalDeviceAndConfiguration getVulkanSuitablePhysicalDevice(VkInstance instance, VkSurfaceKHR surface, const char** deviceExtensions, uint32_t deviceExtensionsCount) {
    // Get the physical devices array and register its cleanup method
    uint32_t physicalDevicesCount = 0;
    VkPhysicalDevice* physicalDevices = getVulkanPhysicalDevices(instance, &physicalDevicesCount);
    pushCleanupCallback(free, physicalDevices);
    // Ensure there is atleast one physical device before continuing
    if(physicalDevicesCount == 0) {
        fprintf(stdout, "No physical devices supported\n");
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "Successfully got list of physical devices:\n");

    // Print all the physical devices supported
    for(uint32_t i = 0; i < physicalDevicesCount; i++) {
        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevices[i], &physicalDeviceProperties);
        fprintf(stdout, "    %s\n", physicalDeviceProperties.deviceName);
    }

    // Create a handle for our selected physical device and create variables to store the index of our selected queue families and number of images in our swapchain
    uint32_t highestScore = 0;
    physicalDeviceAndConfiguration mostSuitablePhysicalDevice = {
        .physicalDevice         = VK_NULL_HANDLE
    };

    // Iterate through all physical devices and select the best one
    for(uint32_t i = 0; i < physicalDevicesCount; i++) {
        physicalDeviceAndConfiguration tempPhysicalDevice = {
            .physicalDevice         = physicalDevices[i]
        };
        uint32_t score = getVulkanPhysicalDeviceSuitability(&tempPhysicalDevice, surface, deviceExtensions, deviceExtensionsCount);
        if(score > highestScore) {
            highestScore = score;
            mostSuitablePhysicalDevice = tempPhysicalDevice;
        }
    }

    // Free the physical devices array
    popCleanupCallback(0);

    // Return the most suitable physical device, this will be empty if no suitable device was found
    return mostSuitablePhysicalDevice;
}

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

VkDevice createVulkanDevice(physicalDeviceAndConfiguration physicalDevice) {
    
}
