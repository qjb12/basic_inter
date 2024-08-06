cmake_minimum_required(VERSION 3.10)

# Define the project name
project(MyProject)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the executable target
add_executable(main main.cpp)

# Include the nlohmann directory for header files
target_include_directories(main PRIVATE ${CMAKE_SOURCE_DIR}/nlohmann)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/..)

# Set the output directory for the executable
set_target_properties(main PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}
)

# add_custom_command(TARGET main POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E echo "Running post-build script..."
#     COMMAND ${CMAKE_COMMAND} -E env bash ${CMAKE_SOURCE_DIR}/locale_gen.sh
#     COMMAND ${CMAKE_COMMAND} -E echo "Post-build script complete."
# 