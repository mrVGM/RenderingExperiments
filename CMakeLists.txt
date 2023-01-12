# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("D3D12_Tests")

IF(CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DDEBUG=1)
ENDIF()

add_definitions(-DUNICODE)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/run.bat DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/runNoise.bat DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Include sub-projects.
add_subdirectory ("main")
add_subdirectory ("dataLib")
add_subdirectory ("parser")
add_subdirectory ("interpreter")
add_subdirectory ("rendering")
add_subdirectory ("gc")
add_subdirectory ("collada")
