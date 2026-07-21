#include <window.h>

// System Headers
#include <stdio.h>
#include <stdlib.h>

// Library Headers

// Project Headers

bool windowResizedFlag = false;

void resizeCallback(GLFWwindow* window, int width, int height) {
    window; width; height;
    windowResizedFlag = true;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)  {
    window; key; scancode; action; mods;
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)  {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

GLFWwindow* createGLFWWindow(const char* title, int width, int height) {
    // Disable the creation of an OpenGL Context and disable window resizing
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // Create the window
    GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
    if(window == NULL) {
        const char* errorMessage = NULL;
        int errorCode = glfwGetError(&errorMessage);
        fprintf(stderr, "Failed to create GLFW window:\n    Error Code:    %i\n    Error Message: %s\n", errorCode, errorMessage);
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "Created GLFW window\n");

    // Set the kay callback for the window so we can capture keyboard events
    glfwSetKeyCallback(window, keyCallback);
    glfwSetFramebufferSizeCallback(window, resizeCallback);

    // Return the window
    return window;
}

VkSurfaceKHR createGLFWWidnowSurface(VkInstance instance, GLFWwindow* window) {
    // Create the window surface
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkResult result = glfwCreateWindowSurface(instance, window, NULL, &surface);
    if(result != VK_SUCCESS) {
        fprintf(stderr, "Failed to create surface for GLFW window: %i\n", result);
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "Created surface for GLFW window\n");

    // Return the window surface
    return surface;
}
