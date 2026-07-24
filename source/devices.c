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

void destroyVulkanPhysicalDeviceInfo(PhysicalDeviceInfo* physicalDeviceInfo) {
    SLICE_DESTROY_IF(physicalDeviceInfo->surfaceFormats)
    if(physicalDeviceInfo->queueFamilyProperties != NULL) {
        free(physicalDeviceInfo->queueFamilyProperties);
        physicalDeviceInfo->queueFamilyProperties = NULL;

        free(physicalDeviceInfo->queueFamiliesPresentationSupport);
        physicalDeviceInfo->queueFamiliesPresentationSupport = NULL;

        physicalDeviceInfo->queueFamilyCount = 0;
    }
}

static PhysicalDeviceInfo getVulkanPhysicalDeviceInfo(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
    // Define the physical device info structure and the pointer chain for physical device properties 2 to get all the properties
    PhysicalDeviceInfo physicalDeviceInfo = {
        .physicalDevice = physicalDevice,
        .surface        = surface,

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
        }
    };

    // get the properties of the physical device we are currently checking
    vkGetPhysicalDeviceProperties2(physicalDevice, &physicalDeviceInfo.physicalDeviceProperties);

    // Get the supported device extensions array and register its cleanup method
    physicalDeviceInfo.supportedExtensions = getSupportedVulkanDeviceExtensions(physicalDevice);

    // Get the length of the supported queue families array
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &physicalDeviceInfo.queueFamilyCount, NULL);

    // Allocate the supported physical devices array
    physicalDeviceInfo.queueFamilyProperties = malloc(sizeof(*physicalDeviceInfo.queueFamilyProperties) * physicalDeviceInfo.queueFamilyCount);
    if(physicalDeviceInfo.queueFamilyProperties == NULL) {
        fprintf(stderr, "Failed to allocate supported queue families array of size %zu\n", sizeof(*physicalDeviceInfo.queueFamilyProperties) * physicalDeviceInfo.queueFamilyCount);
        destroyVulkanPhysicalDeviceInfo(&physicalDeviceInfo);
        exit(EXIT_FAILURE);
    }

    // initialize the structure type member variable for every variable in the queue family properties array
    for(uint32_t i = 0; i < physicalDeviceInfo.queueFamilyCount; i++) {
        physicalDeviceInfo.queueFamilyProperties[i] = (VkQueueFamilyProperties2){
            .sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2
        };
    }

    // Fill the supported queue families array with the list of supported queue families for the physical device
    vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, &physicalDeviceInfo.queueFamilyCount, physicalDeviceInfo.queueFamilyProperties);

    // This variable will be set to true if at least one queue family supports presentation
    bool checkForSurfaceFormats = false;

    // Check for queue families that support presentation
    physicalDeviceInfo.queueFamiliesPresentationSupport = malloc(sizeof(*physicalDeviceInfo.queueFamiliesPresentationSupport) * physicalDeviceInfo.queueFamilyCount);

    if(physicalDeviceInfo.queueFamiliesPresentationSupport == NULL) {
        fprintf(stderr, "Failed to allocate queue families presentation support array of size %zu\n", sizeof(*physicalDeviceInfo.queueFamiliesPresentationSupport) * physicalDeviceInfo.queueFamilyCount);
        exit(EXIT_FAILURE);
    }

    for(uint32_t i = 0; i < physicalDeviceInfo.queueFamilyCount; i++) {
        VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &physicalDeviceInfo.queueFamiliesPresentationSupport[i]);
        if(result != VK_SUCCESS) {
            fprintf(stdout, "Failed to check if a physical device queue family supports presenting to the window surface: %i\n", result);
            destroyVulkanPhysicalDeviceInfo(&physicalDeviceInfo);
            exit(EXIT_FAILURE);
        }

        if(physicalDeviceInfo.queueFamiliesPresentationSupport[i] == true) {
            checkForSurfaceFormats = true;
        }
    }

    if(checkForSurfaceFormats == true) {
        // Get the length of the supported surface formats array
        VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, physicalDeviceInfo.surface, &physicalDeviceInfo.surfaceFormats.count, NULL);
        if(result != VK_SUCCESS) {
            fprintf(stderr, "Failed to get number of supported surface formats: %i\n", result);
            exit(EXIT_FAILURE);
        }

        // Allocate the supported surface formats array
        physicalDeviceInfo.surfaceFormats.data = malloc(SLICE_SIZE(physicalDeviceInfo.surfaceFormats));
        if(physicalDeviceInfo.surfaceFormats.data == NULL) {
            fprintf(stderr, "Failed to allocate supported surface formats array of size %zu\n", SLICE_SIZE(physicalDeviceInfo.surfaceFormats));
            exit(EXIT_FAILURE);
        }

        // Fill the supported surface formats array with the list of supported surface formats
        result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, physicalDeviceInfo.surface, &physicalDeviceInfo.surfaceFormats.count, physicalDeviceInfo.surfaceFormats.data);
        if(result != VK_SUCCESS) {
            fprintf(stderr, "Failed to get  supported surface formats: %i\n", result);
            destroyVulkanPhysicalDeviceInfo(&physicalDeviceInfo);
            exit(EXIT_FAILURE);
        }
    }

    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDeviceInfo.physicalDevice, physicalDeviceInfo.surface, &physicalDeviceInfo.surfaceCapabilities);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to surface capabilities: %i\n", result);
        exit(EXIT_FAILURE);
    }

    return physicalDeviceInfo;
}

static LogicalDeviceCreateInfo getVulkanPhysicalDeviceSuitability(PhysicalDeviceInfo physicalDeviceInfo, StringSlice deviceExtensions, SurfaceFormatKHRSlice surfaceFormats) {
    LogicalDeviceCreateInfo logicalDeviceCreateInfo = {
        .physicalDeviceInfo = physicalDeviceInfo,
        .deviceExtensions   = deviceExtensions,
    };

    // Ensure the current physical device supports at least vulkan 1.4
    if(physicalDeviceInfo.physicalDeviceProperties.properties.apiVersion < VK_API_VERSION_1_4) {
        return (LogicalDeviceCreateInfo){
            .physicalDeviceInfo = physicalDeviceInfo
        };
    }

    // Ensure all required surface formats are supported
    for(uint32_t i = 0; i < physicalDeviceInfo.surfaceFormats.count; i++) {
        bool surfaceFormatDesired = false;
        for(uint32_t j = 0; j < surfaceFormats.count; j++) {
            if(surfaceFormats.data[j].format == physicalDeviceInfo.surfaceFormats.data[i].format && surfaceFormats.data[j].colorSpace == physicalDeviceInfo.surfaceFormats.data[i].colorSpace) {
                surfaceFormatDesired = true;
            }
        }
        if(surfaceFormatDesired) {
            logicalDeviceCreateInfo.surfaceFormatIndex = i;
            break;
        }
    }

    // Ensure all required device extensions are supported
    for(uint32_t i = 0; i < deviceExtensions.count; i++) {
        bool deviceExtensionSupported = false;
        for(uint32_t j = 0; j < physicalDeviceInfo.supportedExtensions.count; j++) {
            if(!strcmp(deviceExtensions.data[i], physicalDeviceInfo.supportedExtensions.data[j].extensionName)) {
                deviceExtensionSupported = true;
            }
        }
        // Ensure that the device extension were checking is supported by the physical device
        if(deviceExtensionSupported == false) {
            return (LogicalDeviceCreateInfo){
                .physicalDeviceInfo = physicalDeviceInfo
            };
        }
    }

    // Select the queue families we are going to use
    uint8_t breakFlag = 0;
    for(uint32_t i = 0; i < physicalDeviceInfo.queueFamilyCount; i++) {
        if((physicalDeviceInfo.queueFamilyProperties[i].queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 1) {
            logicalDeviceCreateInfo.graphicsQueueFamilyIndex = i;
            breakFlag = 1;
            for(uint32_t j = 0; j < physicalDeviceInfo.queueFamilyCount; j++) {
                if(physicalDeviceInfo.queueFamiliesPresentationSupport[j] == VK_TRUE) {
                    logicalDeviceCreateInfo.presentationQueueFamilyIndex = j;
                    breakFlag = 2;
                    if(i == j) {
                        breakFlag = 3;
                        break;
                    }
                }
            }
            if(breakFlag != 0) {
                break;
            }
        }
    }

    // If we haven't found every needed queue family on the same physical device we set the graphics and presentation queues to not found
    if(breakFlag < 2) {
        logicalDeviceCreateInfo.graphicsQueueFamilyIndex     = 0;
        logicalDeviceCreateInfo.presentationQueueFamilyIndex = 0;
        return (LogicalDeviceCreateInfo){
            .physicalDeviceInfo = physicalDeviceInfo
        };
    }

    // If there is one queue family that supports both graphics and presentation we add 1 the score
    if(breakFlag == 3) {
        logicalDeviceCreateInfo.score += 1;
    }

    // If the physical device is a discrete gpu add 2 the score
    if(physicalDeviceInfo.physicalDeviceProperties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        logicalDeviceCreateInfo.score += 2;
    }

    return logicalDeviceCreateInfo;
}

LogicalDeviceCreateInfo getSuitableVulkanPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, StringSlice deviceExtensions, SurfaceFormatKHRSlice surfaceFormats) {
    // Get the physical devices array and register its cleanup method
    PhysicalDeviceInfoSlice physicalDeviceInfos = getVulkanPhysicalDeviceInfos(instance, surface);

    // Ensure there is atleast one physical device before continuing
    if(physicalDeviceInfos.count == 0) {
        fprintf(stdout, "No physical devices supported\n");
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "Successfully got list of physical devices:\n");

    // Print all the physical devices supported
    for(uint32_t i = 0; i < physicalDeviceInfos.count; i++) {
        fprintf(stdout, "    %s\n", physicalDeviceInfos.data[i].physicalDeviceProperties.properties.deviceName);
    }

    // Create a handle for our selected physical device and create variables to store the index of our selected queue families and number of images in our swapchain
    LogicalDeviceCreateInfo mostSuitablePhysicalDevice = {0};

    // Iterate through all physical devices and select the best one
    for(uint32_t i = 0; i < physicalDeviceInfos.count; i++) {
        LogicalDeviceCreateInfo currentPhysicalDeviceCreateInfo = getVulkanPhysicalDeviceSuitability(physicalDeviceInfos.data[i], deviceExtensions, surfaceFormats);
        if(currentPhysicalDeviceCreateInfo.score > mostSuitablePhysicalDevice.score) {
            mostSuitablePhysicalDevice = currentPhysicalDeviceCreateInfo;
        }
    }

    // Return the most suitable physical device, this will be empty if no suitable device was found
    return mostSuitablePhysicalDevice;
}

PhysicalDeviceInfoSlice getVulkanPhysicalDeviceInfos(VkInstance instance, VkSurfaceKHR surface) {
    PhysicalDeviceInfoSlice physicalDeviceInfos = {0};

    // Get the length of the supported physical devices array
    VkResult result = vkEnumeratePhysicalDevices(instance, &physicalDeviceInfos.count, NULL);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to get number of supported physical devices: %i\n", result);
        exit(EXIT_FAILURE);
    }

    // If no physical devices are supported we exit early to avoid allocating 0 bytes and doing unnecessary operations
    if(physicalDeviceInfos.count == 0) {
        return (PhysicalDeviceInfoSlice){0};
    }

    // Allocate the supported physical devices array
    VkPhysicalDevice* physicalDevices = malloc(sizeof(VkPhysicalDevice) * physicalDeviceInfos.count);
    physicalDeviceInfos.data          = malloc(SLICE_SIZE(physicalDeviceInfos));
    if(physicalDevices == NULL) {
        fprintf(stderr, "Failed to allocate supported physical devices array of size %zu\n", sizeof(VkPhysicalDevice) * physicalDeviceInfos.count);
        exit(EXIT_FAILURE);
    }

    // Fill the supported physical device array with the list of physical devices
    result = vkEnumeratePhysicalDevices(instance, &physicalDeviceInfos.count, physicalDevices);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to get number of supported physical devices: %i\n", result);
        exit(EXIT_FAILURE);
    }

    // Fill the physical device infos slice with the physical device info for each physical device
    for(uint32_t i = 0; i < physicalDeviceInfos.count; i++) {
        physicalDeviceInfos.data[i] = getVulkanPhysicalDeviceInfo(physicalDevices[i], surface);
    }

    return physicalDeviceInfos;
}

LogicalDeviceInfo createVulkanDevice(LogicalDeviceCreateInfo logicalDeviceCreateInfo) {
    LogicalDeviceInfo logicalDevice = {
        .physicalDeviceInfo           = logicalDeviceCreateInfo.physicalDeviceInfo,
        .surfaceFormatIndex           = logicalDeviceCreateInfo.surfaceFormatIndex,
        .graphicsQueueFamilyIndex     = logicalDeviceCreateInfo.graphicsQueueFamilyIndex,
        .presentationQueueFamilyIndex = logicalDeviceCreateInfo.presentationQueueFamilyIndex
    };

    fprintf(
        stdout,
        "Creating logical device from physical device:\n    Name:                      %s\n    API Version:               %" PRIu32 ".%" PRIu32 ".%" PRIu32 "\n    Graphics Queue Family:     %" PRIu32 "\n    Presentation Queue Family: %" PRIu32 "\n",
        logicalDeviceCreateInfo.physicalDeviceInfo.physicalDeviceProperties.properties.deviceName,
        VK_VERSION_MAJOR(logicalDeviceCreateInfo.physicalDeviceInfo.physicalDeviceProperties.properties.apiVersion),
        VK_VERSION_MINOR(logicalDeviceCreateInfo.physicalDeviceInfo.physicalDeviceProperties.properties.apiVersion),
        VK_VERSION_PATCH(logicalDeviceCreateInfo.physicalDeviceInfo.physicalDeviceProperties.properties.apiVersion),
        logicalDeviceCreateInfo.graphicsQueueFamilyIndex,
        logicalDeviceCreateInfo.presentationQueueFamilyIndex
    );

    // Add our the graphics and presentation queue families to an array
    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueFamilies[2];
    uint32_t queueFamiliesCount = 0;
    queueFamilies[queueFamiliesCount++] = (VkDeviceQueueCreateInfo){
            .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = logicalDeviceCreateInfo.graphicsQueueFamilyIndex,
            .queueCount       = 1,
            .pQueuePriorities = &queuePriority,
        };
    if(logicalDeviceCreateInfo.graphicsQueueFamilyIndex != logicalDeviceCreateInfo.presentationQueueFamilyIndex) {
        queueFamilies[queueFamiliesCount++] = (VkDeviceQueueCreateInfo){
            .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = logicalDeviceCreateInfo.presentationQueueFamilyIndex,
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
        .enabledExtensionCount = logicalDeviceCreateInfo.deviceExtensions.count,
        .ppEnabledExtensionNames = logicalDeviceCreateInfo.deviceExtensions.data
    };

    // Create the logical device
    VkResult result = vkCreateDevice(logicalDeviceCreateInfo.physicalDeviceInfo.physicalDevice, &deviceInfo, NULL, &logicalDevice.logicalDevice);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to create logical device: %i\n", result);
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "Created logical device\n");

    // Load device level vulkan functions
    volkLoadDevice(logicalDevice.logicalDevice);
    fprintf(stdout, "Loaded device level vulkan functions using volk\n");

    // Get the the handles to the graphics and presentation queues
    vkGetDeviceQueue(logicalDevice.logicalDevice, logicalDevice.graphicsQueueFamilyIndex, 0, &logicalDevice.graphicsQueue);
    vkGetDeviceQueue(logicalDevice.logicalDevice, logicalDevice.presentationQueueFamilyIndex, 0, &logicalDevice.presentationQueue);

    // Return the logical device
    return logicalDevice;
}
