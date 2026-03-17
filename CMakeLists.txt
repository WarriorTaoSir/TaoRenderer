cmake_minimum_required(VERSION 3.10)
project(Renderer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find all source and header files in TaoRenderer-master
file(GLOB_RECURSE SOURCES "TaoRenderer-master/*.cpp")
file(GLOB_RECURSE HEADERS "TaoRenderer-master/*.h")

# Add resources like icons/cursors if any
file(GLOB_RECURSE RESOURCES "TaoRenderer-master/*.rc" "TaoRenderer-master/*.cur")

# Create the executable
add_executable(Renderer ${SOURCES} ${HEADERS} ${RESOURCES})

# If it uses standard Win32 libraries, they are linked by default on Windows/MSVC.
