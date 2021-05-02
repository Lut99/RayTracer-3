# Specify the libraries in this directory
add_library(Entities STATIC ${CMAKE_CURRENT_SOURCE_DIR}/RenderEntity.cpp ${CMAKE_CURRENT_SOURCE_DIR}/Triangle.cpp)

# Set the dependencies for this library:
target_include_directories(Entities PUBLIC
                           "${INCLUDE_DIRS}")

# Add it to the list of includes & linked libraries
list(APPEND EXTRA_LIBS Entities)

# Carry the list to the parent scope
set(EXTRA_LIBS "${EXTRA_LIBS}" PARENT_SCOPE)
