// System Headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// Library Headers
#include <vulkan/vulkan.h>

// Project Headers
#include <projectData.h>

int main(int argc, char* argv[]) {
    // Output project info and argument count
    printf("Project Name:    %s\nProject Version: %s\nArg Count:       %i\n", PROJECT_NAME, PROJECT_VERSION, argc);

    // Output arguments
    for(int i = 0; i < argc; i++) {
        printf("Arg %i:           %s\n", i, argv[i]);
    }

    // Exit with success
    return EXIT_SUCCESS;
}
