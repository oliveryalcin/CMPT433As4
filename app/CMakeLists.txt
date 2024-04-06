# Build the app, using the HAL

include_directories(include)
include_directories(src)
file(GLOB MY_SOURCES "src/*.c")
add_executable(find_the_dot ${MY_SOURCES})

# Make use of the HAL library
# TODO note: m was added for 'sqrt' function
# We can maybe remove if not necessary
target_link_libraries(find_the_dot LINK_PRIVATE hal m)

# Copy executable to final location (change `find_the_dot` to project name as needed)
add_custom_command(TARGET find_the_dot POST_BUILD 
  COMMAND "${CMAKE_COMMAND}" -E copy 
     "$<TARGET_FILE:find_the_dot>"
     "/$ENV{HOME}/cmpt433/public/find_the_dot" 
  COMMENT "Copying ARM executable to public NFS directory")
