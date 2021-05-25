# Add the subdirectories
add_subdirectory(renderer)
add_subdirectory(camera)
add_subdirectory(entities)
add_subdirectory(compute)
add_subdirectory(tools)

# Carry the list to the parent scope
set(EXTRA_LIBS "${EXTRA_LIBS}" PARENT_SCOPE)
