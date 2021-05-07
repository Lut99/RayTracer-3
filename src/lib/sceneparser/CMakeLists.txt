# Specify the libraries in this directory
add_library(SceneParser STATIC ${CMAKE_CURRENT_SOURCE_DIR}/SceneParser.cpp)

# Enter any subdirectory for this library
# Usage: target_sources(SceneParser PRIVATE ${extra_file})

# Set the dependencies for this library:
target_include_directories(SceneParser PUBLIC
                           "${INCLUDE_DIRS}")

# Add it to the list of includes & linked libraries
list(APPEND EXTRA_LIBS SceneParser)

# Carry the list to the parent scope
set(EXTRA_LIBS "${EXTRA_LIBS}" PARENT_SCOPE)
