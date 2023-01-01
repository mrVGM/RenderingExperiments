# CMakeList.txt : CMake project for CMakeTest, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_library (Parser
	"codeSource.cpp"
	"include/codeSource.h"
	"grammar.cpp"
	"include/grammar.h"
	"parser.cpp"
	"include/parser.h"
	"parserState.cpp"
	"parserState.h"
	"parserTable.cpp"
	"parserTable.h"
	"rule.cpp"
	"rule.h"
	"symbol.cpp"
	"include/symbol.h"
	"tokenizer.cpp"
	"tokenizer.h"
 )

target_include_directories(Parser INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(Parser PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(Parser PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(Parser PUBLIC DataLib)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Parser PROPERTY CXX_STANDARD 20)
endif()
