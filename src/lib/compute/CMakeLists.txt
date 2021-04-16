# Specify the libraries in this directory
add_library(GPU STATIC ${CMAKE_CURRENT_SOURCE_DIR}/GPU.cpp)

# Set the dependencies for this library:
target_include_directories(GPU PUBLIC
                           "${INCLUDE_DIRS}")

# Add it to the list of includes & linked libraries
list(APPEND EXTRA_LIBS GPU)

# Carry the list to the parent scope
set(EXTRA_LIBS "${EXTRA_LIBS}" PARENT_SCOPE)
