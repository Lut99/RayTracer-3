# Specify the libraries in this directory
add_library(Compute STATIC ${CMAKE_CURRENT_SOURCE_DIR}/GPU.cpp ${CMAKE_CURRENT_SOURCE_DIR}/MemoryPool.cpp ${CMAKE_CURRENT_SOURCE_DIR}/DescriptorSetLayout.cpp ${CMAKE_CURRENT_SOURCE_DIR}/DescriptorPool.cpp)

# Set the dependencies for this library:
target_include_directories(Compute PUBLIC
                           "${INCLUDE_DIRS}")

# Add it to the list of includes & linked libraries
list(APPEND EXTRA_LIBS Compute)

# Carry the list to the parent scope
set(EXTRA_LIBS "${EXTRA_LIBS}" PARENT_SCOPE)
