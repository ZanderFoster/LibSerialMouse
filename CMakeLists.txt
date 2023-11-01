# Minimum CMake version required
cmake_minimum_required(VERSION 3.0)

# Name your project
project(SerialMouse)

# External Cmake Libs

# Additional compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++23 -std=gnu++11 -g")

# Determine the target platform (you can set this based on your build system)
if (WIN32)
    add_definitions(-D_WIN32)
else ()
    add_definitions(-D LINUX)
endif ()

# Include platform-specific directories
if (WIN32)
    include_directories(
        . libs libs/SerialMouse
        ${CMAKE_SOURCE_DIR}/includes
        ${PROJECT_SOURCE_DIR}/libs
    )
else (LINUX)
    include_directories(
        . libs libs/SerialMouse 
        ${CMAKE_SOURCE_DIR}/includes
        ${PROJECT_SOURCE_DIR}/libs
    )
endif ()


# Specify the source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Build an executable
add_executable(SerialMouse
    ${SOURCES}
)

# Link libraries and specify the runtime path
target_link_libraries(SerialMouse 
    -L${PROJECT_SOURCE_DIR} 
    -l:libserialport.a
)                           

set_target_properties(SerialMouse PROPERTIES INSTALL_RPATH "$ORIGIN")

# Install the binary to the specified directory
install(TARGETS SerialMouse DESTINATION ${CMAKE_BINARY_DIR}/bin)