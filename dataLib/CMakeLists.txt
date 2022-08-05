# CMakeList.txt : CMake project for CMakeTest, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_library (DataLib "dataLib.cpp" "include/dataLib.h" "include/json.hpp")

target_include_directories(DataLib INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(DataLib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET DataLib PROPERTY CXX_STANDARD 20)
endif()
