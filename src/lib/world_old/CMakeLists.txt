# Specify the libraries in this directory
add_library(World STATIC ${CMAKE_CURRENT_SOURCE_DIR}/World.cpp ${CMAKE_CURRENT_SOURCE_DIR}/RenderObject.cpp ${CMAKE_CURRENT_SOURCE_DIR}/Sphere.cpp)

# Set the dependencies for this library:
target_include_directories(World PUBLIC
                           "${INCLUDE_DIRS}")

# Add it to the list of includes & linked libraries
list(APPEND EXTRA_LIBS World)

# Carry the list to the parent scope
set(EXTRA_LIBS "${EXTRA_LIBS}" PARENT_SCOPE)
