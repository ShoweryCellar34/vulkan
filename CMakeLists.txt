# Set required cmake version and define project
cmake_minimum_required(VERSION 3.15)
project(vulkanTestApp LANGUAGES C VERSION 0.0.1)

# Set the C Standard Settings
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Set the CXX Standard Settings 
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Define all source files and add executable
set(SOURCES
    source/cleanup.c
    source/devices.c
    source/extensions.c
    source/layers.c
    source/main.c
)
add_executable(${PROJECT_NAME} ${SOURCES})

# Set compile flags and enable static linking
set(BUILD_SHARED_LIBS OFF)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic
        $<$<CONFIG:Debug>:-g3 -ggdb -Og -fno-omit-frame-pointer -Wshadow $<$<PLATFORM_ID:Windows>:-fsanitize=address,undefined>>
        $<$<CONFIG:Release>:-O3 -march=native -flto -DNDEBUG -D_FORTIFY_SOURCE=3 -fstack-protector-strong -fstack-clash-protection -fPIE -pie -Wl,-z,relro,-z,now -Werror -s>
    )

    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:-fsanitize=address,undefined>
        $<$<CONFIG:Release>:-static>
    )
elseif(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /analyze
        $<$<CONFIG:Debug>:/Od /Zi /RTC1 /MDd /GS /JMC /fsanitize=address /D_DEBUG>
        $<$<CONFIG:Release>:/O2 /GL /Zi /GS /DNDEBUG>
    )

    target_link_options(${PROJECT_NAME} PRIVATE /INCREMENTAL:NO
        $<$<CONFIG:Release>:/LTCG /OPT:REF,ICF>
    )

    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "_CRT_SECURE_NO_WARNINGS"
    )

    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG_BUILD=$<CONFIG:Debug>)

# Include fetch content 
include(FetchContent)

# Configure options for dependencies
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(GLFW_LIBRARY_TYPE "STATIC" CACHE STRING "" FORCE)
set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF CACHE BOOL "" FORCE)

set(CGLM_SHARED OFF CACHE BOOL "" FORCE)
set(CGLM_STATIC ON CACHE BOOL "" FORCE)
set(CGLM_USE_C99 OFF CACHE BOOL "" FORCE)
set(CGLM_TEST OFF CACHE BOOL "" FORCE)

# Declare dependencies
FetchContent_Declare(
    volk
    GIT_REPOSITORY https://github.com/zeux/volk.git
    GIT_TAG        47cddf7ed97b94118a08aacb548a411188e016cc
)

FetchContent_Declare(
    vma
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
    GIT_TAG        3aa921224c154a0d2c43912bc88e1c42ce1f7607
)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        b00e6a8a88ad1b60c0a045e696301deb92c9a13e
)

FetchContent_Declare(
    cglm
    GIT_REPOSITORY https://github.com/recp/cglm.git
    GIT_TAG        83d5b2c97378e61a4666825fd3a5548955c83add
)

# Download dependencies
FetchContent_MakeAvailable(volk vma glfw cglm)

# Link dependencies to executable
target_link_libraries(${PROJECT_NAME} PRIVATE volk VulkanMemoryAllocator glfw cglm)

# Set the include directories
target_include_directories(${PROJECT_NAME} PRIVATE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/headers>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated>
)

# Generate a header with cmake project information
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/headers/projectData.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/generated/projectData.h"
)

# Try to find the slang shader compiler
find_program(SLANG_COMPILER
    NAMES slangc
    HINTS "$ENV{VULKAN_SDK}/bin" "$ENV{VULKAN_SDK}/Bin"
    NO_CACHE
)

# Check if the slang shader compiler was found
if(NOT SLANG_COMPILER)
    message(FATAL_ERROR "Cannot find slangc program for compiling slang shaders")
else()
    message(STATUS "Found slangc for compiling shaders")
endif()

# Copy new resources and compile shaders
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD

    COMMAND ${CMAKE_COMMAND} -E echo "Removing old resources from build folder"
    COMMAND ${CMAKE_COMMAND} -E remove_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources"

    COMMAND ${CMAKE_COMMAND} -E echo "Copying new resources to build folder"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/resources/assets" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources/assets"

    COMMAND ${CMAKE_COMMAND} -E echo "Compiling shaders to build folder"
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources/shaders"
    COMMAND ${SLANG_COMPILER} ${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders/test.slang -profile spirv_1_6 -o $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources/shaders/test.spv
)
