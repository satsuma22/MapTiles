cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_GENERATOR_PLATFORM x64)

project(MapTiles)

file(GLOB SOURCES "MapTiles/*.cpp" "MapTiles/render/src/*.cpp" "MapTiles/render/src/opengl/*.cpp" "MapTiles/data/src/*.cpp")

file(GLOB HEADERS "MapTiles/*.h" "MapTiles/render/include/*.h" "MapTiles/render/include/opengl/*.h" "MapTiles/data/include/*.h")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the build type" FORCE)
endif()

link_directories("3rd/glew/x64" "3rd/glfw/lib-vc2022")

add_executable(MapTiles ${SOURCES} "3rd/tinyxml2-master/tinyxml2.cpp" ${HEADERS})

target_compile_definitions(MapTiles PRIVATE GLEW_STATIC)

target_include_directories(MapTiles PUBLIC "MapTiles/data/include" "MapTiles/render/include" "3rd/WGS84toCartesian-master" "3rd/stb_image" "3rd/glm-master/glm" "3rd/tinyxml2-master" "3rd/earcut.hpp-master/include" "3rd/cpp-httplib-master" "3rd/glew/include" "3rd/glfw/include")

target_link_libraries(MapTiles PRIVATE glew32s opengl32 glfw3)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT MapTiles)