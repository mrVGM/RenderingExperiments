# CMakeList.txt : CMake project for CMakeTest, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_library (GC
	"include/value.h"
	"value.cpp"
	"include/garbageCollector.h"
	"garbageCollector.cpp"
	"include/nativeFunc.h"
	"nativeFunc.cpp"
)

target_include_directories(GC INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(GC PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(GC PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(GC PUBLIC Interpreter)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET GC PROPERTY CXX_STANDARD 20)
endif()
