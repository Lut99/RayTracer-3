# Specify the libraries in this directory
add_library(Debugger STATIC ${CMAKE_CURRENT_SOURCE_DIR}/CppDebugger.cpp)

# Set the dependencies for this library:
target_include_directories(Debugger PUBLIC
                           "${INCLUDE_DIRS}")

# Add it to the list of includes & linked libraries
list(APPEND EXTRA_LIBS Debugger)

# Carry the list to the parent scope
set(EXTRA_LIBS "${EXTRA_LIBS}" PARENT_SCOPE)
