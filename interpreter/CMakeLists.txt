# CMakeList.txt : CMake project for CMakeTest, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_library (Interpreter
	"../gc/include/value.h"
	"../gc/value.cpp"
	"../gc/garbageCollector.h"
	"../gc/garbageCollector.cpp"
	"scope.h"
	"scope.cpp"
	"calculator.h"
	"calculator.cpp"
	"interpreter.h"
	"interpreter.cpp"
	"list.h" 
	"list.cpp"
	"object.h"
	"object.cpp"
	"include/IFunc.h"
	"IFunc.cpp"
	"func.h"
	"func.cpp"
	"include/ISession.h"
	"ISession.cpp"
	"session.h"
	"session.cpp"
)

target_include_directories(Interpreter INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(Interpreter PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(Interpreter PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(Interpreter PUBLIC Parser)
target_link_libraries(Interpreter PUBLIC GC)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Interpreter PROPERTY CXX_STANDARD 20)
endif()
