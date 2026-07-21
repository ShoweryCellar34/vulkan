#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

// System Headers

// Library Headers
#include <volk.h>

// Project Headers
#include <devices.h>
#include <macros.h>

typedef struct {
    PhysicalDeviceCreateInfo physicalDeviceCreateInfo;

    VkSwapchainKHR swapchain;
    VkExtent2D     swapchainExtent;
    ImageSlice     swapchainImages;
    ImageViewSlice swapchainImageViews;
} SwapchainInfo;

// Creates a vulkan swapchain
void CreateVulkanSwapchain(SwapchainInfo* swapchainInfo, VkExtent2D desiredExtent);

#endif // SWAPCHAIN_H
