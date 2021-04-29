# Add the subdirectories
add_subdirectory(camera)
add_subdirectory(compute)
add_subdirectory(frame)
add_subdirectory(world)

# Carry the list to the parent scope
set(EXTRA_LIBS "${EXTRA_LIBS}" PARENT_SCOPE)
