# Specify the libraries in this directory
if (RENDERER_BACKEND MATCHES "^Vulkan$")
    add_library(Renderer STATIC ${CMAKE_CURRENT_SOURCE_DIR}/Renderer.cpp ${CMAKE_CURRENT_SOURCE_DIR}/VulkanRenderer.cpp)
elseif(RENDERER_BACKEND MATCHES "^VulkanOnline$")
    add_library(Renderer STATIC ${CMAKE_CURRENT_SOURCE_DIR}/Renderer.cpp ${CMAKE_CURRENT_SOURCE_DIR}/VulkanOnlineRenderer.cpp)
elseif(RENDERER_BACKEND MATCHES "^Sequential$")
    add_library(Renderer STATIC ${CMAKE_CURRENT_SOURCE_DIR}/Renderer.cpp ${CMAKE_CURRENT_SOURCE_DIR}/SequentialRenderer.cpp)
else()
    message(FATAL_ERROR "Unknown rendering backend '${RENDERER_BACKEND}'")
endif()

message("Using rendering backend '${RENDERER_BACKEND}'")

# Set the dependencies for this library:
target_include_directories(Renderer PUBLIC
"${INCLUDE_DIRS}")

# Add it to the list of includes & linked libraries
list(APPEND EXTRA_LIBS Renderer)

# Carry the list to the parent scope
set(EXTRA_LIBS "${EXTRA_LIBS}" PARENT_SCOPE)
