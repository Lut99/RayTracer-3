# CMAKELIST for the RAYTRACER-3 project
#   by Lut99

# Specify the version required
cmake_minimum_required(VERSION 3.10)

# Set the project name & version
project(RayTracer-3 VERSION 1.0.0)

# Get the VULKAN, GLM & CppDebugger library
find_package(Vulkan REQUIRED)
find_package(CppDebugger REQUIRED)

# Specify the C++-standard to use
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Define all include directories
SET(INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/src/lib" "${Vulkan_INCLUDE_DIRS}")

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
                      cppdbg)
