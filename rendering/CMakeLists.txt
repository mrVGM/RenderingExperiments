# CMakeList.txt : CMake project for CMakeTest, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_library (Rendering
	"window.h"
	"window.cpp"
	"include/api.h"
	"api.cpp"
	"d3dx12.h"
	"dxDevice.h"
	"dxDevice.cpp"
	"dxVertexShader.h"
	"dxVertexShader.cpp"
	"dxPixelShader.h"
	"dxPixelShader.cpp"
	"dxHeap.h"
	"dxHeap.cpp"
	"dxFence.h"
	"dxFence.cpp"
)

target_include_directories(Rendering INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(Rendering PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(Rendering PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(Rendering PUBLIC GC)
target_link_libraries(Rendering PRIVATE "d3d12.lib" "dxgi.lib" "d3dcompiler.lib")

add_definitions(-D_AFXDLL)
set(CMAKE_MFC_FLAG 1)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Rendering PROPERTY CXX_STANDARD 20)
endif()
