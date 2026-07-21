#ifndef WINDOW_H
#define WINDOW_H

// System Headers
#include <stdbool.h>

// Library Headers
#include <volk.h>
#include <GLFW/glfw3.h>

// Project Headers

extern bool windowResizedFlag;

// Creates and returns a window
GLFWwindow* createGLFWWindow(const char* title, int width, int height);

// Creates and returns a vulkan surface for a GLFW window
VkSurfaceKHR createGLFWWidnowSurface(VkInstance instance, GLFWwindow* window);

#endif // WINDOW_H
