#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

// System Headers

// Library Headers
#include <volk.h>

// Project Headers
#include <macros.h>

typedef struct {
    VkSwapchainKHR swapchain;
    VkExtent2D     swapchainExtent;
    ImageSlice     swapchainImages;
    ImageViewSlice swapchainImageViews;
} SwapchainInfo;

#endif // SWAPCHAIN_H
