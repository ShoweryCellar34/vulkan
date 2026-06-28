#include <devices.h>

// System Headers
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Library Headers
#include <GLFW/glfw3.h>

// Project Headers
#include <cleanup.h>

PhysicalDeviceInfo getVulkanPhysicalDeviceInfo(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
    // Define the physical device info structure and the pointer chain for physical device properties 2 to get all the properties
    PhysicalDeviceInfo physicalDeviceInfo = {
        .physicalDevice = physicalDevice,
        .physicalDeviceProperties = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
            .pNext = &(VkPhysicalDeviceVulkan11Properties){
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES,
                .pNext = &(VkPhysicalDeviceVulkan12Properties){
                    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES,
                    .pNext = &(VkPhysicalDeviceVulkan13Properties){
                        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES,
                        .pNext = &(VkPhysicalDeviceVulkan14Properties){
                            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_PROPERTIES
                        }
                    }
                }
            }
        },
        .surface = surface
    };

    // get the properties of the physical device we are currently checking
    vkGetPhysicalDeviceProperties2(physicalDevice, &physicalDeviceInfo.physicalDeviceProperties);

    // Get the supported device extensions array and register its cleanup method
    physicalDeviceInfo.supportedExtensions = getSupportedVulkanDeviceExtensions(physicalDevice);

    // Get the length of the supported queue families array
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &physicalDeviceInfo.queueFamilyPropertiesCount, NULL);

    // Allocate the supported physical devices array
    physicalDeviceInfo.queueFamilyProperties = malloc(sizeof(*physicalDeviceInfo.queueFamilyProperties) * physicalDeviceInfo.queueFamilyPropertiesCount);
    if(physicalDeviceInfo.queueFamilyProperties == NULL) {
        fprintf(stderr, "Failed to allocate supported queue families array of size %zu\n", sizeof(*physicalDeviceInfo.queueFamilyProperties) * physicalDeviceInfo.queueFamilyPropertiesCount);
        exit(EXIT_FAILURE);
    }

    // initialize the structure type member variable for every variable in the queue family properties array
    for(uint32_t i = 0; i < physicalDeviceInfo.queueFamilyPropertiesCount; i++) {
        physicalDeviceInfo.queueFamilyProperties[i] = (VkQueueFamilyProperties2){
            .sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2
        };
    }

    // Fill the supported queue families array with the list of supported queue families for the physical device
    vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, &physicalDeviceInfo.queueFamilyPropertiesCount, &physicalDeviceInfo.queueFamilyProperties);

    bool graphicsQueueFamilyFound = false, presentationQueueFamilyFound = false;

    // Check for queue families that support presentation
    physicalDeviceInfo.queueFamilyPresentationSupport = malloc(sizeof(*physicalDeviceInfo.queueFamilyPresentationSupport) * physicalDeviceInfo.queueFamilyPropertiesCount);
    for(uint32_t i = 0; i < physicalDeviceInfo.queueFamilyPropertiesCount; i++) {
        result = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, physicalDeviceInfo.queueFamilyPresentationSupport[i]);
        if(result != VK_SUCCESS) {
            fprintf(stdout, "Failed to check if a physical device queue family supports presenting to the window surface: %i\n", result);
            free(physicalDeviceInfo.queueFamilyPresentationSupport);
            free(physicalDeviceInfo.queueFamilyProperties);
            free(physicalDeviceInfo.surfaceFormats);
            exit(EXIT_FAILURE);
        }
    }

    // // Get the length of the supported surface formats array
    // VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, physicalDeviceInfo.surface, &physicalDeviceInfo.surfaceFormatsCount, NULL);
    // if(result != VK_SUCCESS) {
    //     fprintf(stderr, "Failed to get number of supported surface formats: %i\n", result);
    //     exit(EXIT_FAILURE);
    // }

    // // If at least one surface format is supported enumerate the surface formats
    // if(physicalDeviceInfo.surfaceFormatsCount != 0) {
    //     // Allocate the supported surface formats array
    //     physicalDeviceInfo.surfaceFormats = malloc(sizeof(*physicalDeviceInfo.surfaceFormats) * physicalDeviceInfo.surfaceFormatsCount);
    //     if(physicalDeviceInfo.surfaceFormats == NULL) {
    //         fprintf(stderr, "Failed to allocate supported surface formats array of size %zu\n", sizeof(*physicalDeviceInfo.surfaceFormats) * physicalDeviceInfo.surfaceFormatsCount);
    //         exit(EXIT_FAILURE);
    //     }

    //     // Fill the supported surface formats array with the list of supported surface formats
    //     result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, physicalDeviceInfo.surface, &physicalDeviceInfo.surfaceFormatsCount, physicalDeviceInfo.surfaceFormats);
    //     if(result != VK_SUCCESS) {
    //         fprintf(stderr, "Failed to get  supported surface formats: %i\n", result);
    //         free(physicalDeviceInfo.surfaceFormats);
    //         exit(EXIT_FAILURE);
    //     }
    // }
}

uint32_t getVulkanPhysicalDeviceSuitability(physicalDeviceInfo* physicalDevice, VkSurfaceKHR surface, ExtensionNames deviceExtensions) {
    // get the properties of the physical device we are currently checking
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice->physicalDevice, &physicalDeviceProperties);

    // Ensure the current physical device supports at least vulkan 1.4
    if(physicalDeviceProperties.apiVersion < VK_API_VERSION_1_4) {
        return 0;
    }

    // Get the length of the supported surface formats array
    uint32_t surfaceFormatsCount = 0;
    VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice->physicalDevice, surface, &surfaceFormatsCount, NULL);
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
    ExtensionProperties supportedDeviceExtensions = getSupportedVulkanDeviceExtensions(physicalDevice->physicalDevice);

    // Ensure all requested device extensions are supported
    for(uint32_t i = 0; i < deviceExtensions.count; i++) {
        bool deviceExtensionSupported = false;
        for(uint32_t j = 0; j < supportedDeviceExtensions.count; j++) {
            if(!strcmp(deviceExtensions.names[i], supportedDeviceExtensions.properties[j].extensionName)) {
                deviceExtensionSupported = true;
            }
        }
        if(deviceExtensionSupported == false) {
            return 0;
        }
    }
    // Free the supported device extensions array because we don't need it anymore
    free(supportedDeviceExtensions.properties);

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

physicalDeviceInfo getVulkanSuitablePhysicalDevice(VkInstance instance, VkSurfaceKHR surface, ExtensionNames deviceExtensions) {
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
    physicalDeviceInfo mostSuitablePhysicalDevice = {
        .physicalDevice         = VK_NULL_HANDLE
    };

    // Iterate through all physical devices and select the best one
    for(uint32_t i = 0; i < physicalDevicesCount; i++) {
        physicalDeviceInfo tempPhysicalDevice = {
            .physicalDevice         = physicalDevices[i]
        };
        uint32_t score = getVulkanPhysicalDeviceSuitability(&tempPhysicalDevice, surface, deviceExtensions);
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
    // Get the length of the supported physical devices array
    VkResult result = vkEnumeratePhysicalDevices(instance, physicalDevicesCount, NULL);
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

VkDevice createVulkanDevice(physicalDeviceInfo physicalDevice, ExtensionNames deviceExtensions) {
    // Print the physical device we will use
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice.physicalDevice, &physicalDeviceProperties);
    fprintf(
        stdout,
        "Creating logical device from physical device:\n    Name:                      %s\n    API Version:               %" PRIu32 ".%" PRIu32 ".%" PRIu32 "\n    Graphics Queue Family:     %" PRIu32 "\n    Presentation Queue Family: %" PRIu32 "\n",
        physicalDeviceProperties.deviceName,
        VK_VERSION_MAJOR(physicalDeviceProperties.apiVersion),
        VK_VERSION_MINOR(physicalDeviceProperties.apiVersion),
        VK_VERSION_PATCH(physicalDeviceProperties.apiVersion),
        physicalDevice.graphicsQueueFamilyIndex,
        physicalDevice.presentationQueueFamilyIndex
    );

    // Add our the graphics and presentation queue families to an array
    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueFamilies[2];
    uint32_t queueFamiliesCount = 0;
    queueFamilies[queueFamiliesCount++] = (VkDeviceQueueCreateInfo){
            .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = physicalDevice.graphicsQueueFamilyIndex,
            .queueCount       = 1,
            .pQueuePriorities = &queuePriority,
        };
    if(physicalDevice.graphicsQueueFamilyIndex != physicalDevice.presentationQueueFamilyIndex) {
        queueFamilies[queueFamiliesCount++] = (VkDeviceQueueCreateInfo){
            .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = physicalDevice.presentationQueueFamilyIndex,
            .queueCount       = 1,
            .pQueuePriorities = &queuePriority,
        };
    }

    // List the features we want our logical device to have
    VkPhysicalDeviceVulkan11Features enabledDeviceFeatures_1_1 = {
        .sType                = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
        .shaderDrawParameters = VK_TRUE
    };
    VkPhysicalDeviceVulkan12Features enabledDeviceFeatures_1_2 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .pNext = &enabledDeviceFeatures_1_1
    };
    VkPhysicalDeviceVulkan13Features enabledDeviceFeatures_1_3 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = &enabledDeviceFeatures_1_2,
        .dynamicRendering = VK_TRUE,
        .synchronization2 = VK_TRUE
    };
    VkPhysicalDeviceVulkan14Features enabledDeviceFeatures_1_4 = {
        .sType            = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES,
        .pNext            = &enabledDeviceFeatures_1_3
    };
    VkPhysicalDeviceFeatures2 enabledDeviceFeatures = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &enabledDeviceFeatures_1_4
    };

    // Create a struct containing the settings for the logical device
    VkDeviceCreateInfo deviceInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &enabledDeviceFeatures,
        .queueCreateInfoCount = queueFamiliesCount,
        .pQueueCreateInfos = queueFamilies,
        .enabledExtensionCount = deviceExtensions.count,
        .ppEnabledExtensionNames = deviceExtensions.names
    };

    // Create the logical device
    VkDevice device = VK_NULL_HANDLE;
    VkResult result = vkCreateDevice(physicalDevice.physicalDevice, &deviceInfo, NULL, &device);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to create logical device: %i\n", result);
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "Created logical device\n");

    // Load device level vulkan functions
    volkLoadDevice(device);
    fprintf(stdout, "Loaded device level vulkan functions using volk\n");

    // Return the logical device
    return device;
}
