# Specify the libraries in this directory
add_library(Frames STATIC ${CMAKE_CURRENT_SOURCE_DIR}/Frame.cpp ${CMAKE_CURRENT_SOURCE_DIR}/LodePNG.cpp)

# Set the dependencies for this library:
target_include_directories(Frames PUBLIC
                           "${INCLUDE_DIRS}")

# Add it to the list of includes & linked libraries
list(APPEND EXTRA_LIBS Frames)

# Carry the list to the parent scope
set(EXTRA_LIBS "${EXTRA_LIBS}" PARENT_SCOPE)
