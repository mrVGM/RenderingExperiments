# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("D3D12_Tests")

add_definitions(-DDEBUG=1)
add_definitions(-DUNICODE)

# Include sub-projects.
add_subdirectory ("main")
add_subdirectory ("dataLib")
add_subdirectory ("parser")
add_subdirectory ("interpreter")
add_subdirectory ("rendering")
add_subdirectory ("gc")
