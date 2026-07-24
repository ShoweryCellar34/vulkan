#include <swapchain.h>

// System Headers

// Library Headers
#include <GLFW/glfw3.h>

// Project Headers

// void CreateVulkanSwapchain(SwapchainInfo* swapchainInfo, VkExtent2D desiredExtent) {
//     // Set the number of images that should be in the swapchain
//     uint32_t swapchainImageCount = swapchainInfo->physicalDeviceCreateInfo.physicalDeviceInfo.surfaceCapabilities.minImageCount + (swapchainInfo->physicalDeviceCreateInfo.physicalDeviceInfo.surfaceCapabilities.maxImageCount > swapchainInfo->physicalDeviceCreateInfo.physicalDeviceInfo.surfaceCapabilities.minImageCount || swapchainInfo->physicalDeviceCreateInfo.physicalDeviceInfo.surfaceCapabilities.maxImageCount == 0 ? 1 : 0);

//     // Set the dimensions of the swapchain
//     if(swapchainInfo->physicalDeviceCreateInfo.physicalDeviceInfo.surfaceCapabilities.currentExtent.width == UINT32_MAX) {
//         int windowWidth, windowHeight;
//         glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

//         swapchainExtent.width = CLAMP((uint32_t)windowWidth, physicalDeviceCreateInfo.physicalDeviceInfo.surfaceCapabilities.minImageExtent.width, physicalDeviceCreateInfo.physicalDeviceInfo.surfaceCapabilities.maxImageExtent.width);
//         swapchainExtent.height = CLAMP((uint32_t)windowHeight, physicalDeviceCreateInfo.physicalDeviceInfo.surfaceCapabilities.minImageExtent.height, physicalDeviceCreateInfo.physicalDeviceInfo.surfaceCapabilities.maxImageExtent.height);
//     } else {
//         swapchainExtent = physicalDeviceCreateInfo.physicalDeviceInfo.surfaceCapabilities.currentExtent;
//     }

//     // Create a struct containing the settings for the swapchain
//     VkSwapchainCreateInfoKHR swapchainCreateInfo = {
//         .sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
//         .surface          = surface,
//         .minImageCount    = swapchainImageCount,
//         .imageFormat      = physicalDeviceCreateInfo.physicalDeviceInfo.surfaceFormats.data[physicalDeviceCreateInfo.surfaceFormatIndex].format,
//         .imageColorSpace  = physicalDeviceCreateInfo.physicalDeviceInfo.surfaceFormats.data[physicalDeviceCreateInfo.surfaceFormatIndex].colorSpace,
//         .imageExtent      = swapchainExtent,
//         .imageArrayLayers = 1,
//         .imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
//         .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
//         .preTransform     = physicalDeviceCreateInfo.physicalDeviceInfo.surfaceCapabilities.currentTransform,
//         .compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
//         .presentMode      = VK_PRESENT_MODE_FIFO_KHR,
//         .clipped          = true
//     };

//     // Change the image sharing mode in the swapchain create info struct
//     if(physicalDeviceCreateInfo.graphicsQueueFamilyIndex != physicalDeviceCreateInfo.presentationQueueFamilyIndex) {
//         swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
//         swapchainCreateInfo.queueFamilyIndexCount = 2;
//         swapchainCreateInfo.pQueueFamilyIndices = (const uint32_t[]){
//             physicalDeviceCreateInfo.graphicsQueueFamilyIndex,
//             physicalDeviceCreateInfo.presentationQueueFamilyIndex
//         };
//     }

//     // Create the swapchain
//     VkSwapchainKHR swapchain = VK_NULL_HANDLE;
//     result = vkCreateSwapchainKHR(device, &swapchainCreateInfo, NULL, &swapchain);
//     if(result != VK_SUCCESS) {
//         fprintf(stdout, "Failed to create swapchain: %i\n", result);
//         exit(EXIT_FAILURE);
//     }
//     PUSH_CLEANUP_ARGS_3(vkDestroySwapchainKHR, device, swapchain, NULL);
//     fprintf(stdout, "Created swapchain\n");

//     // Get the length of the swapchain images array
//     uint32_t swapchainImagesCount = 0;
//     result = vkGetSwapchainImagesKHR(device, swapchain, &swapchainImagesCount, NULL);
//     if(result != VK_SUCCESS) {
//         fprintf(stderr, "Failed to get number of swapchain images: %i\n", result);
//         exit(EXIT_FAILURE);
//     }

//     // Allocate the swapchain images array
//     VkImage* swapchainImages = malloc(sizeof(VkImage) * swapchainImagesCount);
//     if(swapchainImages == NULL) {
//         fprintf(stderr, "Failed to allocate swapchain images array of size %zu\n", swapchainImagesCount * sizeof(VkImage));
//         exit(EXIT_FAILURE);
//     }
//     pushCleanupCallback(free, swapchainImages);

//     // Fill the swapchain images array with the list of swapchain images
//     result = vkGetSwapchainImagesKHR(device, swapchain, &swapchainImagesCount, swapchainImages);
//     if(result != VK_SUCCESS) {
//         fprintf(stderr, "Failed to get number of swapchain images: %i\n", result);
//         exit(EXIT_FAILURE);
//     }

//     // Create a struct containing the settings for the swapchain image views
//     VkImageViewCreateInfo swapchainImageViewsCreateInfo = {
//         .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
//         .viewType = VK_IMAGE_VIEW_TYPE_2D,
//         .format = physicalDeviceCreateInfo.physicalDeviceInfo.surfaceFormats.data[physicalDeviceCreateInfo.surfaceFormatIndex].format,
//         .components = {
//             VK_COMPONENT_SWIZZLE_IDENTITY,
//             VK_COMPONENT_SWIZZLE_IDENTITY,
//             VK_COMPONENT_SWIZZLE_IDENTITY,
//             VK_COMPONENT_SWIZZLE_IDENTITY
//         },
//         .subresourceRange = {
//             .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
//             .levelCount = 1,
//             .layerCount = 1
//         }
//     };

//     // Allocate the swapchain image views array
//     VkImageView* swapchainImageViews = malloc(sizeof(VkImageView) * swapchainImagesCount);
//     if(swapchainImageViews == NULL) {
//         fprintf(stderr, "Failed to allocate swapchain image views array of size %zu\n", swapchainImagesCount * sizeof(VkImageView));
//         exit(EXIT_FAILURE);
//     }
//     pushCleanupCallback(free, swapchainImageViews);

//     // Create the swapchain image views
//     for(uint32_t i = 0; i < swapchainImagesCount; i++) {
//         swapchainImageViewsCreateInfo.image = swapchainImages[i];
//         result = vkCreateImageView(device, &swapchainImageViewsCreateInfo, NULL, &swapchainImageViews[i]);
//         if(result != VK_SUCCESS) {
//             fprintf(stderr, "Failed to create swapchain image view %" PRIu32 ": %i\n", i, result);
//             exit(EXIT_FAILURE);
//         }
//         PUSH_CLEANUP_ARGS_3(vkDestroyImageView, device, swapchainImageViews[i], NULL);
//         fprintf(stderr, "Created swapchain image view %" PRIu32 "\n", i);
//     }
// }
