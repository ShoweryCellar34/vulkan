// System Headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <errno.h>

// Library Headers
#include <volk.h>
#include <GLFW/glfw3.h>

// Project Headers
#include <projectData.h>

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 450

int main(int argc, char* argv[]) {
    // Output project info and argument count
    printf("Project Name:    %s\nProject Version: %s\nArg Count:       %i\n", PROJECT_NAME, PROJECT_VERSION, argc);

    // Output arguments
    for(int i = 0; i < argc; i++) {
        printf("Arg %i:           %s\n", i, argv[i]);
    }

    // initialize GLFW and check for vulkan support, exit if initialization fails or vulkan not supported
    if(!glfwInit() || !glfwVulkanSupported()) {
        return EXIT_FAILURE;
    }

    // Disable the creation of an OpenGL Context and disable window resizing
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // Create the window
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, PROJECT_NAME, NULL, NULL);

    // Main application loop
    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    // Cleanup GLFW
    glfwDestroyWindow(window);
    glfwTerminate();

    // Exit with success
    return EXIT_SUCCESS;
}
