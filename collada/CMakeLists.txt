# CMakeList.txt : CMake project for CMakeTest, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_library (Collada
	"include/collada.h"
	"collada.cpp"
	"colladaTreeBuilder.h"
	"colladaTreeBuilder.cpp"
)

target_include_directories(Collada INTERFACE PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(Collada PRIVATE Interpreter DataLib)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Collada PROPERTY CXX_STANDARD 20)
endif()
