// System Headers
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Library Headers
#include <volk.h>
// #include <vk_mem_alloc.h>
#include <GLFW/glfw3.h>

// Project Headers
#include <cleanup.h>
#include <devices.h>
#include <extensions.h>
#include <layers.h>
#include <macros.h>
#include <projectData.h>
#include <window.h>

DEFINE_CALLBACK_ARGS_0(glfwTerminate)
DEFINE_CALLBACK_ARGS_1(glfwDestroyWindow, GLFWwindow*)
DEFINE_CALLBACK_ARGS_0(volkFinalize)
DEFINE_CALLBACK_ARGS_2(vkDestroyInstance, VkInstance, const VkAllocationCallbacks*)
DEFINE_CALLBACK_ARGS_3(vkDestroyDebugUtilsMessengerEXT, VkInstance, VkDebugUtilsMessengerEXT, const VkAllocationCallbacks*)
DEFINE_CALLBACK_ARGS_3(vkDestroySurfaceKHR, VkInstance, VkSurfaceKHR, const VkAllocationCallbacks*)
DEFINE_CALLBACK_ARGS_2(vkDestroyDevice, VkDevice, const VkAllocationCallbacks*)
// DEFINE_CALLBACK_ARGS_1(vmaDestroyAllocator, VmaAllocator)
DEFINE_CALLBACK_ARGS_3(vkDestroySwapchainKHR, VkDevice, VkSwapchainKHR, const VkAllocationCallbacks*)
DEFINE_CALLBACK_ARGS_3(vkDestroyImageView, VkDevice, VkImageView, const VkAllocationCallbacks*)
DEFINE_CALLBACK_ARGS_1(fclose, FILE*)
DEFINE_CALLBACK_ARGS_3(vkDestroyShaderModule, VkDevice, VkShaderModule, const VkAllocationCallbacks*)
DEFINE_CALLBACK_ARGS_3(vkDestroyPipelineLayout, VkDevice, VkPipelineLayout, const VkAllocationCallbacks*)
DEFINE_CALLBACK_ARGS_3(vkDestroyPipeline, VkDevice, VkPipeline, const VkAllocationCallbacks*)
DEFINE_CALLBACK_ARGS_3(vkDestroyCommandPool, VkDevice, VkCommandPool, const VkAllocationCallbacks*)
DEFINE_CALLBACK_ARGS_3(vkDestroySemaphore, VkDevice, VkSemaphore, const VkAllocationCallbacks*)
DEFINE_CALLBACK_ARGS_3(vkDestroyFence, VkDevice, VkFence, const VkAllocationCallbacks*)

#define WINDOW_WIDTH         800
#define WINDOW_HEIGHT        800
#define MAX_FRAMES_IN_FLIGHT 2

static VKAPI_ATTR VkBool32 VKAPI_CALL vulkanDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT      severity,
    VkDebugUtilsMessageTypeFlagsEXT             type,
    const VkDebugUtilsMessengerCallbackDataEXT* data,
    void*                                       user_data
) {
    (void)severity; (void)type; (void)user_data;
    fprintf(stderr, "Vulkan error: %s\n", data->pMessage);
    return VK_FALSE;
}

static void transitionImageLayout(
    VkCommandBuffer         commandBuffer,
    VkImage                 image,
    VkImageLayout           oldLayout,
    VkImageLayout           newLayout,
    VkAccessFlags2          srcAccessMask,
    VkAccessFlags2          dstAccessMask,
    VkPipelineStageFlags2   srcStageMask,
    VkPipelineStageFlags2   dstStageMask
) {
    VkImageMemoryBarrier2 barrier = {
        .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask        = srcStageMask,
        .srcAccessMask       = srcAccessMask,
        .dstStageMask        = dstStageMask,
        .dstAccessMask       = dstAccessMask,
        .oldLayout           = oldLayout,
        .newLayout           = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image               = image,
        .subresourceRange    = {
                .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel   = 0,
                .levelCount     = 1,
                .baseArrayLayer = 0,
                .layerCount     = 1
        }
    };
    VkDependencyInfo dependencyInfo = {
        .sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers    = &barrier
    };
    vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);
}

static VkInstance createVulkanInstance(const char* name, uint32_t version, StringSlice extensions, StringSlice layers, bool debug, VkDebugUtilsMessengerEXT* debugMessenger) {
    // Create a struct containing the settings for the debug messenger
    VkDebugUtilsMessengerCreateInfoEXT debugInfo = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = vulkanDebugCallback
    };

    // Create a struct containing the settings for the application
    VkApplicationInfo applicationInfo = {
        .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName   = name,
        .applicationVersion = version,
        .pEngineName        = "No Engine",
        .engineVersion      = VK_MAKE_VERSION(0, 0, 0),
        .apiVersion         = VK_API_VERSION_1_4
    };

    // Create a struct containing the settings for the instance
    VkInstanceCreateInfo instanceInfo = {
        .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext                   = debug == true ? &debugInfo : NULL,
        .pApplicationInfo        = &applicationInfo,
        .enabledLayerCount       = layers.count,
        .ppEnabledLayerNames     = layers.data,
        .enabledExtensionCount   = extensions.count,
        .ppEnabledExtensionNames = extensions.data
    };

    // Create the instance
    VkInstance instance = VK_NULL_HANDLE;
    VkResult result = vkCreateInstance(&instanceInfo, NULL, &instance);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to create instance: %i\n", result);
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "Created instance\n");

    // Load instance level vulkan functions
    volkLoadInstance(instance);
    fprintf(stdout, "Loaded instance level vulkan functions using volk\n");

    // Create the debug callback if this is in debug mode
    if(debug == true) {
        result = vkCreateDebugUtilsMessengerEXT(instance, &debugInfo, NULL, debugMessenger);
        if(result != VK_SUCCESS) {
            fprintf(stderr, "Failed to set vulkan debug callback: %i\n", result);
            vkDestroyInstance(instance, NULL);
            exit(EXIT_FAILURE);
        }
        fprintf(stdout, "Added vulkan debug callback\n");
    }

    // Return the instance
    return instance;
}

int main(int argc, char* argv[]) {
    // Output project info and argument count
    printf("Project Name:    %s\nProject Version: %s\nArg Count:       %i\n", PROJECT_NAME, PROJECT_VERSION, argc);

    // Output arguments
    for(int i = 0; i < argc; i++) {
        printf("Arg %i:           %s\n", i, argv[i]);
    }

    // Setup the exit callbacks stack for cleaning up resources upon exit
    if(atexit(startCleanupCallbacks) != 0) {
        fprintf(stdout, "Failed to register cleanup callback stack with atExit()\n");
        exit(EXIT_FAILURE);
    }

    // initialize GLFW
    if(glfwInit() != GLFW_TRUE) {
        const char* errorMessage = NULL;
        int errorCode = glfwGetError(&errorMessage);
        fprintf(stderr, "Failed to initialize GLFW:\n    Error Code:    %i\n    Error Message: %s\n", errorCode, errorMessage);
        return EXIT_FAILURE;
    }
    PUSH_CLEANUP_ARGS_0(glfwTerminate);
    fprintf(stdout, "Initialized GLFW\n");

    // Check for minimal vulkan support
    if(glfwVulkanSupported() != GLFW_TRUE) {
        fprintf(stderr, "Minimal vulkan functionality test failed\n");
        return EXIT_FAILURE;
    }
    fprintf(stdout, "Vulkan supported minimally\n");

    // Create the GLFW window
    GLFWwindow* window = createGLFWWindow(PROJECT_NAME, WINDOW_WIDTH, WINDOW_HEIGHT);
    PUSH_CLEANUP_ARGS_1(glfwDestroyWindow, window);

    // Create a variable to store the result of vulkan functions for error checking and reporting
    VkResult result = VK_SUCCESS;

    // Load the vulkan library and global-level vulkan functions
    result = volkInitialize();
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to load vulkan with volk: %i\n", result);
        return EXIT_FAILURE;
    }
    PUSH_CLEANUP_ARGS_0(volkFinalize);
    fprintf(stdout, "Loaded global level vulkan functions using volk\n");

    // Get the extensions array
    StringSlice extensions = getVulkanExtensionsAndValidate(DEBUG_BUILD);

    // Get the layers array
    StringSlice layers = getVulkanLayersAndValidate(DEBUG_BUILD);

    // Create the instance
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
    VkInstance instance = createVulkanInstance(PROJECT_NAME, VK_MAKE_VERSION(PROJECT_VERSION_MAJOR, PROJECT_VERSION_MINOR, PROJECT_VERSION_PATCH), extensions, layers, DEBUG_BUILD, &debugMessenger);
    PUSH_CLEANUP_ARGS_2(vkDestroyInstance, instance, NULL);
    #if DEBUG_BUILD == true
        PUSH_CLEANUP_ARGS_3(vkDestroyDebugUtilsMessengerEXT, instance, debugMessenger, NULL);
    #endif

    // Create the window surface
    VkSurfaceKHR surface = createGLFWWidnowSurface(instance, window);
    PUSH_CLEANUP_ARGS_3(vkDestroySurfaceKHR, instance, surface, NULL);

    // Get the device extensions array, it is static and does not need to be freed
    StringSlice deviceExtensions = getVulkanDeviceExtensions();

    // Print all the device extensions we will use
    fprintf(stdout, "Successfully got list of required device extensions:\n");
    for(uint32_t i = 0; i < deviceExtensions.count; i++) {
        fprintf(stdout, "    %s\n", deviceExtensions.data[i]);
    }

    // Get the most suitable physical device
    LogicalDeviceCreateInfo physicalDeviceCreateInfo = getSuitableVulkanPhysicalDevice(instance, surface, deviceExtensions, (SurfaceFormatKHRSlice){
        .count = 1,
        .data = (VkSurfaceFormatKHR[]){
            (VkSurfaceFormatKHR){
                .format     = VK_FORMAT_B8G8R8A8_SRGB,
                .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
            }
        }
    });

    // Ensure we have a suitable physical device before preceding
    if(physicalDeviceCreateInfo.score == 0) {
        fprintf(stdout, "Failed to find suitable physical device\n");
        exit(EXIT_FAILURE);
    }

    // Create the logical device
    LogicalDeviceInfo device = createVulkanDevice(physicalDeviceCreateInfo);
    PUSH_CLEANUP_ARGS_2(vkDestroyDevice, device.logicalDevice, NULL);

    // // Create a struct containing the settings for the VMA allocator
    // VmaAllocatorCreateInfo allocatorCreateInfo = {
    //     .physicalDevice = physicalDeviceCreateInfo.physicalDeviceInfo.physicalDevice,
    //     .device = device.logicalDevice,
    //     .instance = instance,
    //     .vulkanApiVersion = VK_API_VERSION_1_4
    // };

    // // Get the list of vulkan functions for VMA from volk
    // VmaVulkanFunctions vulkanFunctions;
    // result = vmaImportVulkanFunctionsFromVolk(&allocatorCreateInfo, &vulkanFunctions);
    // if(result != VK_SUCCESS) {
    //     fprintf(stderr, "Failed to import vulkan functions from volk for VMA: %i\n", result);
    //     return EXIT_FAILURE;
    // }
    // allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

    // // Create the VMA allocator
    // VmaAllocator allocator;
    // result = vmaCreateAllocator(&allocatorCreateInfo, &allocator);
    // if(result != VK_SUCCESS) {
    //     fprintf(stderr, "Failed to create the VMA allocator: %i\n", result);
    //     return EXIT_FAILURE;
    // }
    // PUSH_CLEANUP_ARGS_1(vmaDestroyAllocator, allocator);
    // fprintf(stdout, "Created the VMA allocator\n");

    // Set the number of images that should be in the swapchain
    uint32_t swapchainImageCount = physicalDeviceCreateInfo.physicalDeviceInfo.surfaceCapabilities.minImageCount + (physicalDeviceCreateInfo.physicalDeviceInfo.surfaceCapabilities.maxImageCount > physicalDeviceCreateInfo.physicalDeviceInfo.surfaceCapabilities.minImageCount || physicalDeviceCreateInfo.physicalDeviceInfo.surfaceCapabilities.maxImageCount == 0 ? 1 : 0);

    // Set the dimensions of the swapchain
    VkExtent2D swapchainExtent;
    if(physicalDeviceCreateInfo.physicalDeviceInfo.surfaceCapabilities.currentExtent.width == UINT32_MAX) {
        int windowWidth, windowHeight;
        glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

        swapchainExtent.width = CLAMP((uint32_t)windowWidth, physicalDeviceCreateInfo.physicalDeviceInfo.surfaceCapabilities.minImageExtent.width, physicalDeviceCreateInfo.physicalDeviceInfo.surfaceCapabilities.maxImageExtent.width);
        swapchainExtent.height = CLAMP((uint32_t)windowHeight, physicalDeviceCreateInfo.physicalDeviceInfo.surfaceCapabilities.minImageExtent.height, physicalDeviceCreateInfo.physicalDeviceInfo.surfaceCapabilities.maxImageExtent.height);
    } else {
        swapchainExtent = physicalDeviceCreateInfo.physicalDeviceInfo.surfaceCapabilities.currentExtent;
    }

    // Create a struct containing the settings for the swapchain
    VkSwapchainCreateInfoKHR swapchainCreateInfo = {
        .sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface          = surface,
        .minImageCount    = swapchainImageCount,
        .imageFormat      = physicalDeviceCreateInfo.physicalDeviceInfo.surfaceFormats.data[physicalDeviceCreateInfo.surfaceFormatIndex].format,
        .imageColorSpace  = physicalDeviceCreateInfo.physicalDeviceInfo.surfaceFormats.data[physicalDeviceCreateInfo.surfaceFormatIndex].colorSpace,
        .imageExtent      = swapchainExtent,
        .imageArrayLayers = 1,
        .imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform     = physicalDeviceCreateInfo.physicalDeviceInfo.surfaceCapabilities.currentTransform,
        .compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode      = VK_PRESENT_MODE_FIFO_KHR,
        .clipped          = true
    };

    // Change the image sharing mode in the swapchain create info struct
    if(physicalDeviceCreateInfo.graphicsQueueFamilyIndex != physicalDeviceCreateInfo.presentationQueueFamilyIndex) {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCreateInfo.queueFamilyIndexCount = 2;
        swapchainCreateInfo.pQueueFamilyIndices = (const uint32_t[]){
            physicalDeviceCreateInfo.graphicsQueueFamilyIndex,
            physicalDeviceCreateInfo.presentationQueueFamilyIndex
        };
    }

    // Create the swapchain
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    result = vkCreateSwapchainKHR(device.logicalDevice, &swapchainCreateInfo, NULL, &swapchain);
    if(result != VK_SUCCESS) {
        fprintf(stdout, "Failed to create swapchain: %i\n", result);
        exit(EXIT_FAILURE);
    }
    PUSH_CLEANUP_ARGS_3(vkDestroySwapchainKHR, device.logicalDevice, swapchain, NULL);
    fprintf(stdout, "Created swapchain\n");

    // Get the length of the swapchain images array
    uint32_t swapchainImagesCount = 0;
    result = vkGetSwapchainImagesKHR(device.logicalDevice, swapchain, &swapchainImagesCount, NULL);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to get number of swapchain images: %i\n", result);
        exit(EXIT_FAILURE);
    }

    // Allocate the swapchain images array
    VkImage* swapchainImages = malloc(sizeof(VkImage) * swapchainImagesCount);
    if(swapchainImages == NULL) {
        fprintf(stderr, "Failed to allocate swapchain images array of size %zu\n", swapchainImagesCount * sizeof(VkImage));
        exit(EXIT_FAILURE);
    }
    pushCleanupCallback(free, swapchainImages);

    // Fill the swapchain images array with the list of swapchain images
    result = vkGetSwapchainImagesKHR(device.logicalDevice, swapchain, &swapchainImagesCount, swapchainImages);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to get number of swapchain images: %i\n", result);
        exit(EXIT_FAILURE);
    }

    // Create a struct containing the settings for the swapchain image views
    VkImageViewCreateInfo swapchainImageViewsCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = physicalDeviceCreateInfo.physicalDeviceInfo.surfaceFormats.data[physicalDeviceCreateInfo.surfaceFormatIndex].format,
        .components = {
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY
        },
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .levelCount = 1,
            .layerCount = 1
        }
    };

    // Allocate the swapchain image views array
    VkImageView* swapchainImageViews = malloc(sizeof(VkImageView) * swapchainImagesCount);
    if(swapchainImageViews == NULL) {
        fprintf(stderr, "Failed to allocate swapchain image views array of size %zu\n", swapchainImagesCount * sizeof(VkImageView));
        exit(EXIT_FAILURE);
    }
    pushCleanupCallback(free, swapchainImageViews);

    // Create the swapchain image views
    for(uint32_t i = 0; i < swapchainImagesCount; i++) {
        swapchainImageViewsCreateInfo.image = swapchainImages[i];
        result = vkCreateImageView(device.logicalDevice, &swapchainImageViewsCreateInfo, NULL, &swapchainImageViews[i]);
        if(result != VK_SUCCESS) {
            fprintf(stderr, "Failed to create swapchain image view %" PRIu32 ": %i\n", i, result);
            exit(EXIT_FAILURE);
        }
        PUSH_CLEANUP_ARGS_3(vkDestroyImageView, device.logicalDevice, swapchainImageViews[i], NULL);
        fprintf(stderr, "Created swapchain image view %" PRIu32 "\n", i);
    }

    // Open the shader file
    FILE* shaderFile = fopen("resources/shaders/basic.spv", "rb");
    if(shaderFile == NULL) {
        fprintf(stderr, "Failed to open file: %s\n", "resources/shaders/basic.spv");
        exit(EXIT_FAILURE);
    }
    PUSH_CLEANUP_ARGS_1(fclose, shaderFile);
    fprintf(stdout, "Opened file: %s", "resources/shaders/basic.spv\n");

    // Set our position in the shader file to the end to get the shader file length
    if(fseek(shaderFile, 0, SEEK_END) != 0) {
        fprintf(stderr, "Failed to seek to end of file: %s\n", "resources/shaders/basic.spv");
        return EXIT_FAILURE;
    }

    // Get the length of the supported extensions array
    size_t shaderFileDataCount = (size_t)ftell(shaderFile);
    if(shaderFileDataCount < 0) {
        fprintf(stderr, "Failed to get length of file: %s\n", "resources/shaders/basic.spv");
        return EXIT_FAILURE;
    }
    if(shaderFileDataCount % 4 != 0) {
        fprintf(stderr, "Length of file data is not a multiple of 4 (required for SPIR-V shaders) for file: %s\n", "resources/shaders/basic.spv");
        return EXIT_FAILURE;
    }

    // Set our position in the shader file to the begining for reading
    if(fseek(shaderFile, 0, SEEK_SET) != 0) {
        fprintf(stderr, "Failed to seek to begining of file: %s\n", "resources/shaders/basic.spv");
        return EXIT_FAILURE;
    }

    // Allocate the shader file data array
    unsigned char* shaderFileData = malloc(sizeof(unsigned char) * shaderFileDataCount);
    if(shaderFileData == NULL) {
        fprintf(stderr, "Failed to allocate file data array of size %zu for file: %s\n", shaderFileDataCount * sizeof(unsigned char), "resources/shaders/basic.spv");
        exit(EXIT_FAILURE);
    }
    pushCleanupCallback(free, shaderFileData);

    // Fill the shader file data array with the shader file data
    size_t shaderFileDataCountRead = fread(shaderFileData, 1, shaderFileDataCount, shaderFile);
    if(shaderFileDataCountRead < (size_t)shaderFileDataCount) {
        if(ferror(shaderFile)) {
            fprintf(stderr, "Error reading data for file: %s\n", "resources/shaders/basic.spv");
        } else if(feof(shaderFile)) {
            fprintf(stderr, "Unexpected end of file for file: %s\n", "resources/shaders/basic.spv");
        }
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "Read file contents for file: %s", "resources/shaders/basic.spv\n");

    // Close the file handle
    popAndCallCleanupCallback(1);

    // Create a struct containing the settings for the shader module
    VkShaderModuleCreateInfo shaderModuleCreateInfo = {
        .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = (size_t)shaderFileDataCount,
        .pCode    = (uint32_t*)(void*)shaderFileData
    };

    // Create the shader module
    VkShaderModule shaderModule = VK_NULL_HANDLE;
    result = vkCreateShaderModule(device.logicalDevice, &shaderModuleCreateInfo, NULL, &shaderModule);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to create shader module: %i\n", result);
        exit(EXIT_FAILURE);
    }
    PUSH_CLEANUP_ARGS_3(vkDestroyShaderModule, device.logicalDevice, shaderModule, NULL);
    fprintf(stdout, "Created shader module\n");

    VkPipelineShaderStageCreateInfo shaderStages[] = {
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = shaderModule,
            .pName = "vertMain"
        },
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = shaderModule,
            .pName = "fragMain"
        }
    };

    VkPipelineVertexInputStateCreateInfo   vertexInputInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
    };
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
        .sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
    };
    VkPipelineViewportStateCreateInfo      viewportState = {
        .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount  = 1
    };

    VkPipelineRasterizationStateCreateInfo rasterizer = {
        .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable        = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode             = VK_POLYGON_MODE_FILL,
        .cullMode                = VK_CULL_MODE_BACK_BIT,
        .frontFace               = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable         = VK_FALSE,
        .lineWidth               = 1.0f
    };

    VkPipelineMultisampleStateCreateInfo multisampling = {
        .sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable  = VK_FALSE
    };

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
        .blendEnable    = VK_FALSE,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    };

    VkPipelineColorBlendStateCreateInfo colorBlending = {
        .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable   = VK_FALSE,
        .logicOp         = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments    = &colorBlendAttachment
    };

    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamicState = {
        .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = (uint32_t)(sizeof(dynamicStates) / sizeof(*dynamicStates)),
        .pDynamicStates    = dynamicStates
    };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount         = 0,
        .pushConstantRangeCount = 0
    };

    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    result = vkCreatePipelineLayout(device.logicalDevice, &pipelineLayoutInfo, NULL, &pipelineLayout);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to create graphics pipeline layout: %i\n", result);
        exit(EXIT_FAILURE);
    }
    PUSH_CLEANUP_ARGS_3(vkDestroyPipelineLayout, device.logicalDevice, pipelineLayout, NULL);
    fprintf(stdout, "Created graphics pipeline layout\n");

    VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo = {
        .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount    = 1,
        .pColorAttachmentFormats = &physicalDeviceCreateInfo.physicalDeviceInfo.surfaceFormats.data[physicalDeviceCreateInfo.surfaceFormatIndex].format
    };

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {
        .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext               = (void*)&pipelineRenderingCreateInfo,
        .stageCount          = 2,
        .pStages             = shaderStages,
        .pVertexInputState   = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState      = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState   = &multisampling,
        .pColorBlendState    = &colorBlending,
        .pDynamicState       = &dynamicState,
        .layout              = pipelineLayout,
        .renderPass          = NULL
    };

    VkPipeline pipeline = VK_NULL_HANDLE;
    result = vkCreateGraphicsPipelines(device.logicalDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, NULL, &pipeline);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to create graphics pipeline: %i\n", result);
        exit(EXIT_FAILURE);
    }
    PUSH_CLEANUP_ARGS_3(vkDestroyPipeline, device.logicalDevice, pipeline, NULL);
    fprintf(stdout, "Created graphics pipeline\n");

    VkCommandPoolCreateInfo commandPoolCreateInfo = {
        .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = physicalDeviceCreateInfo.graphicsQueueFamilyIndex
    };

    VkCommandPool graphicsCommandPool = VK_NULL_HANDLE;
    VkCommandPool presentationCommandPool = VK_NULL_HANDLE;
    result = vkCreateCommandPool(device.logicalDevice, &commandPoolCreateInfo, NULL, &graphicsCommandPool);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to create graphics command pool: %i\n", result);
        exit(EXIT_FAILURE);
    }
    PUSH_CLEANUP_ARGS_3(vkDestroyCommandPool, device.logicalDevice, graphicsCommandPool, NULL);
    fprintf(stdout, "Created graphics command pool\n");

    if(physicalDeviceCreateInfo.graphicsQueueFamilyIndex != physicalDeviceCreateInfo.presentationQueueFamilyIndex) {
        result = vkCreateCommandPool(device.logicalDevice, &commandPoolCreateInfo, NULL, &presentationCommandPool);
        if(result != VK_SUCCESS) {
            fprintf(stderr, "Failed to create presentation command pool: %i\n", result);
            exit(EXIT_FAILURE);
        }
        PUSH_CLEANUP_ARGS_3(vkDestroyCommandPool, device.logicalDevice, presentationCommandPool, NULL);
        fprintf(stdout, "Created presentation command pool\n");
    } else {
        presentationCommandPool = graphicsCommandPool;
        fprintf(stdout, "Reusing graphics command pool as presentation command pool\n");
    }

    VkCommandBufferAllocateInfo commandBufferAllocationInfo = {
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool        = graphicsCommandPool,
        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = MAX_FRAMES_IN_FLIGHT
    };

    VkCommandBuffer graphicsCommandBuffers[MAX_FRAMES_IN_FLIGHT]     = {VK_NULL_HANDLE};
    VkCommandBuffer presentationCommandBuffers[MAX_FRAMES_IN_FLIGHT] = {VK_NULL_HANDLE};
    result = vkAllocateCommandBuffers(device.logicalDevice, &commandBufferAllocationInfo, graphicsCommandBuffers);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to allocate %i graphics command buffer(s): %i\n", MAX_FRAMES_IN_FLIGHT, result);
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "Allocated graphics command buffer\n");

    if(physicalDeviceCreateInfo.graphicsQueueFamilyIndex != physicalDeviceCreateInfo.presentationQueueFamilyIndex) {
        commandBufferAllocationInfo.commandPool = presentationCommandPool;
        result = vkAllocateCommandBuffers(device.logicalDevice, &commandBufferAllocationInfo, presentationCommandBuffers);
        if(result != VK_SUCCESS) {
            fprintf(stderr, "Failed to allocate %i presentation command buffer(s): %i\n", MAX_FRAMES_IN_FLIGHT, result);
            exit(EXIT_FAILURE);
        }
        fprintf(stdout, "Allocated presentation command buffer\n");
    } else {
        for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            presentationCommandBuffers[i] = graphicsCommandBuffers[i];
        }
        fprintf(stdout, "Reused graphics command buffer as presentation command buffer\n");
    }

    VkSemaphoreCreateInfo semaphoreCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };
    VkFenceCreateInfo fenceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    VkSemaphore  presentCompleteSemaphores[MAX_FRAMES_IN_FLIGHT] = {VK_NULL_HANDLE};
    VkFence      drawFences[MAX_FRAMES_IN_FLIGHT]                = {VK_NULL_HANDLE};

    for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        result = vkCreateSemaphore(device.logicalDevice, &semaphoreCreateInfo, NULL, &presentCompleteSemaphores[i]);
        if(result != VK_SUCCESS) {
            fprintf(stderr, "Failed to create presentation semaphore %" PRIu32 ": %i\n", i, result);
            exit(EXIT_FAILURE);
        }
        PUSH_CLEANUP_ARGS_3(vkDestroySemaphore, device.logicalDevice, presentCompleteSemaphores[i], NULL);

        result = vkCreateFence(device.logicalDevice, &fenceCreateInfo, NULL, &drawFences[i]);
        if(result != VK_SUCCESS) {
            fprintf(stderr, "Failed to create draw fence %" PRIu32 ": %i\n", i, result);
            exit(EXIT_FAILURE);
        }
        PUSH_CLEANUP_ARGS_3(vkDestroyFence, device.logicalDevice, drawFences[i], NULL);
    }

    VkSemaphore* renderFinishedSemaphores = malloc(sizeof(VkSemaphore) * swapchainImagesCount);
    if(renderFinishedSemaphores == NULL) {
        fprintf(stderr, "Failed to allocate presentation semaphores array of size %zu\n", sizeof(VkSemaphore) * swapchainImagesCount);
        exit(EXIT_FAILURE);
    }
    pushCleanupCallback(free, renderFinishedSemaphores);
    for(uint32_t i = 0; i < swapchainImagesCount; i++) {
        result = vkCreateSemaphore(device.logicalDevice, &semaphoreCreateInfo, NULL, &renderFinishedSemaphores[i]);
        if(result != VK_SUCCESS) {
            fprintf(stderr, "Failed to create render semaphore %" PRIu32 ": %i\n", i, result);
            exit(EXIT_FAILURE);
        }
        PUSH_CLEANUP_ARGS_3(vkDestroySemaphore, device.logicalDevice, renderFinishedSemaphores[i], NULL);
    }

    fprintf(stdout, "Created essential semaphores/fences\n");

    // Store the current frame were on for frames in flight support
    uint32_t frameIndex = 0;

    // Main application loop
    while(!glfwWindowShouldClose(window)) {
        // Poll for new inputs
        glfwPollEvents();

        // Wait for previous frame to finish drawing
        result = vkWaitForFences(device.logicalDevice, 1, &drawFences[frameIndex], VK_TRUE, UINT64_MAX);
        if(result != VK_SUCCESS) {
            fprintf(stderr, "Failed to wait for drawing fence %" PRIu32 ": %i\n", frameIndex, result);
            exit(EXIT_FAILURE);
        }
        result = vkResetFences(device.logicalDevice, 1, &drawFences[frameIndex]);
        if(result != VK_SUCCESS) {
            fprintf(stderr, "Failed to reset drawing fence %" PRIu32 ": %i\n", frameIndex, result);
            exit(EXIT_FAILURE);
        }

        VkAcquireNextImageInfoKHR swapchainNextImageAcquisitionInfo = {
            .sType      = VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR,
            .semaphore  = presentCompleteSemaphores[frameIndex],
            .swapchain  = swapchain,
            .timeout    = UINT64_MAX,
            .deviceMask = 0x1
        };

        uint32_t swapchainIndex = 0;
        result = vkAcquireNextImage2KHR(device.logicalDevice, &swapchainNextImageAcquisitionInfo, &swapchainIndex);
        if(result != VK_SUCCESS) {
            fprintf(stderr, "Failed to acquire next swapchain image: %i\n", result);
            exit(EXIT_FAILURE);
        }

        VkCommandBufferBeginInfo graphicsCommandBufferBeginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
        };
        result = vkBeginCommandBuffer(graphicsCommandBuffers[frameIndex], &graphicsCommandBufferBeginInfo);
        if(result != VK_SUCCESS) {
            fprintf(stderr, "Failed to begin graphics command buffer: %i\n", result);
            exit(EXIT_FAILURE);
        }

        transitionImageLayout(
            graphicsCommandBuffers[frameIndex],
            swapchainImages[swapchainIndex],
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            0,
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT
        );

        VkClearValue clearColor = {
            .color.float32 = {
                0.0f,
                0.0f,
                0.0f,
                1.0f
            }
        };
        VkRenderingAttachmentInfo attachmentInfo = {
            .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView   = swapchainImageViews[swapchainIndex],
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp     = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue  = clearColor
        };

        VkRenderingInfo renderingInfo = {
            .sType                = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .renderArea           = {
                .offset = {
                    0,
                    0
                },
                .extent = swapchainExtent
            },
            .layerCount           = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments    = &attachmentInfo
        };

        vkCmdBeginRendering(graphicsCommandBuffers[frameIndex], &renderingInfo);

        vkCmdBindPipeline(graphicsCommandBuffers[frameIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        VkViewport viewport = {
            0.0f,
            0.0f,
            (float)swapchainExtent.width,
            (float)swapchainExtent.height,
            0.0f,
            1.0f
        };
        VkRect2D scissor = {
            {
                0,
                0
            },
            swapchainExtent
        };

        vkCmdSetViewport(graphicsCommandBuffers[frameIndex], 0, 1, &viewport);
        vkCmdSetScissor(graphicsCommandBuffers[frameIndex], 0, 1, &scissor);

        vkCmdDraw(graphicsCommandBuffers[frameIndex], 6, 1, 0, 0);

        vkCmdEndRendering(graphicsCommandBuffers[frameIndex]);

        transitionImageLayout(
            graphicsCommandBuffers[frameIndex],
            swapchainImages[swapchainIndex],
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            0,
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT
        );

        result = vkEndCommandBuffer(graphicsCommandBuffers[frameIndex]);
        if(result != VK_SUCCESS) {
            fprintf(stderr, "Failed to end graphics command buffer: %i\n", result);
            exit(EXIT_FAILURE);
        }

        VkPipelineStageFlags waitDestinationStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        const VkSubmitInfo submitInfo = {
            .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount   = 1,
            .pWaitSemaphores      = &presentCompleteSemaphores[frameIndex],
            .pWaitDstStageMask    = &waitDestinationStageMask,
            .commandBufferCount   = 1,
            .pCommandBuffers      = &graphicsCommandBuffers[frameIndex],
            .signalSemaphoreCount = 1,
            .pSignalSemaphores    = &renderFinishedSemaphores[swapchainIndex]
        };

        vkQueueWaitIdle(device.presentationQueue);

        result = vkQueueSubmit(device.graphicsQueue, 1, &submitInfo, drawFences[frameIndex]);
        if(result != VK_SUCCESS) {
            fprintf(stderr, "Failed to submit graphics command buffer queue: %i\n", result);
            exit(EXIT_FAILURE);
        }

        frameIndex = (frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;

        const VkPresentInfoKHR presentInfo = {
            .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores    = &renderFinishedSemaphores[swapchainIndex],
            .swapchainCount     = 1,
            .pSwapchains        = &swapchain,
            .pImageIndices      = &swapchainIndex
        };

        result = vkQueuePresentKHR(device.presentationQueue, &presentInfo);
    }

    vkDeviceWaitIdle(device.logicalDevice);

    // Exit with success, the cleanup stack will clean everything up
    return EXIT_SUCCESS;
}
