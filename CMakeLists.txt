cmake_minimum_required(VERSION 3.14)
project(TPowerFisher) # Change this to your projects/Scripts name

# Select all source files
set(SOURCE_FILES main.cpp)

# C++ flags
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Os -s -m32")
set(CMAKE_SHARED_LINKER_FLAGS "-Os -s -m32 -static-libgcc -static-libstdc++ -static")

# Set directories
set(ALPACABOT_DIRECTORY $ENV{HOMEDRIVE}$ENV{HOMEPATH}\\AlpacaBot)
set(SCRIPT_DIRECTORY ${ALPACABOT_DIRECTORY}\\Scripts\\Local\\${PROJECT_NAME})
message("AlpacaBot directory: " ${ALPACABOT_DIRECTORY})
message("Script directory: " ${SCRIPT_DIRECTORY})

# Set the output folder of the script, and make the directory if it doesn't exist
file(MAKE_DIRECTORY ${SCRIPT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${SCRIPT_DIRECTORY})
add_custom_target(${PROJECT_NAME}_MakeDir ALL COMMAND ${CMAKE_COMMAND} -E make_directory ${SCRIPT_DIRECTORY})

# Set the BUILD_DLL #define
add_definitions(-DBUILD_DLL)

# Build the .dll
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

# Include and Library directories
include_directories(${ALPACABOT_DIRECTORY}\\Include)
target_link_directories(${PROJECT_NAME} PRIVATE ${ALPACABOT_DIRECTORY}/Library)

# Link libs
target_link_libraries(${PROJECT_NAME} AlpacaLibrary)