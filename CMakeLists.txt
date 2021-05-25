# CMAKELIST for the RAYTRACER-3 project
#   by Lut99

# Specify the version required
cmake_minimum_required(VERSION 3.10)

# Set the project name & version
project(RayTracer-3 VERSION 1.0.0)

# If we're on Windows, prefix the GLFW path
IF(WIN32)
list(APPEND CMAKE_PREFIX_PATH "C:/Program Files (x86)/GLFW/lib/cmake/glfw3")
ENDIF()

# Get the VULKAN, GLM & CppDebugger library
find_package(Vulkan REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(CppDebugger REQUIRED)

# Specify the C++-standard to use
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set the option of the backend to use
set(RENDERER_BACKEND "Vulkan" CACHE STRING "Define which backend to use for renderer. Options are: 'Vulkan', 'VulkanOnline' or 'Sequential'")

# Convert the option to flags
set(FLAGS "")
if(RENDERER_BACKEND MATCHES "^Vulkan(Online)?$")
set(FLAGS "${FLAGS} -DENABLE_VULKAN")
endif()
if(RENDERER_BACKEND MATCHES "^VulkanOnline$")
set(FLAGS "${FLAGS} -DENABLE_ONLINE")
endif()

# Set the Wall Wextra flags
set(WARNING_FLAGS "-Wall -Wextra")
if(WIN32)
set(WARNING_FLAGS "")
endif()

# Add extra flags needed
message("Extra flags: '${FLAGS}'")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${WARNING_FLAGS} ${FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_DEBUG} ${FLAGS}")

# Define all include directories
get_target_property(GLFW_DIR glfw INTERFACE_INCLUDE_DIRECTORIES)
SET(INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/src/lib" "${Vulkan_INCLUDE_DIRS}" "${GLFW_DIR}")

# Load the libraries
add_subdirectory(src)



##### RAYTRACER TARGET #####
# Specify which file we want to compile
add_executable(raytracer ${PROJECT_SOURCE_DIR}/src/Main.cpp)
# Set the output to the bin directory
set_target_properties(raytracer
                      PROPERTIES 
                      RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/bin
                      RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin
                      )

# Add the include directories for this target
target_include_directories(raytracer PUBLIC "${INCLUDE_DIRS}")

# Add which libraries to link
target_link_libraries(raytracer PUBLIC
                      ${EXTRA_LIBS}
                      ${Vulkan_LIBRARIES}
                      glfw
                      cppdbg)



##### BUILDING SHADERS #####
# Define the custom commands to compile the shaders
add_custom_target(shaders
    COMMAND glslc -fshader-stage=compute -o ${PROJECT_SOURCE_DIR}/bin/shaders/raytracer_v3.spv ${PROJECT_SOURCE_DIR}/src/lib/shaders/raytracer_v3.glsl
    COMMAND glslc -fshader-stage=compute -o ${PROJECT_SOURCE_DIR}/bin/shaders/pre_render_sphere_v2_faces.spv ${PROJECT_SOURCE_DIR}/src/lib/shaders/pre_render_sphere_v2_faces.glsl
    COMMAND glslc -fshader-stage=compute -o ${PROJECT_SOURCE_DIR}/bin/shaders/pre_render_sphere_v2_vertices.spv ${PROJECT_SOURCE_DIR}/src/lib/shaders/pre_render_sphere_v2_vertices.glsl
    COMMENT "Building shaders..."
)
