# CMakeList.txt : CMake project for CMakeTest, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (MainApp "main.cpp")

target_link_libraries(MainApp PUBLIC Interpreter)
target_link_libraries(MainApp PUBLIC Rendering)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET MainApp PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
