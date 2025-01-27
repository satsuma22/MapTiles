cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
if (WIN32)
	set(CMAKE_GENERATOR_PLATFORM x64)
endif()

if (WIN32)
	set(PATH_TO_VCPKG "D:/Package" CACHE STRING "Path to vcpkg")

	set(glfw3_DIR "${PATH_TO_VCPKG}/vcpkg/installed/x64-windows/share/glfw3/")
	set(GLEW_DIR "${PATH_TO_VCPKG}/vcpkg/installed/x64-windows/share/glew/")
	set(PACKAGE_MANAGER_DLL_DIR "${PATH_TO_VCPKG}/vcpkg/installed/x64-windows/bin")
	set(PACKAGE_MANAGER_DLL_DIR_DEBUG "${PATH_TO_VCPKG}/vcpkg/installed/x64-windows/debug/bin")
endif()

project(MapTiles)

file(GLOB SOURCES "MapTiles/*.cpp" "MapTiles/render/src/*.cpp" "MapTiles/render/src/opengl/*.cpp" "MapTiles/data/src/*.cpp")

file(GLOB HEADERS "MapTiles/*.h" "MapTiles/render/include/*.h" "MapTiles/render/include/opengl/*.h" "MapTiles/data/include/*.h")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the build type" FORCE)
endif()

find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)


add_executable(MapTiles ${SOURCES} "3rd/tinyxml2-master/tinyxml2.cpp" ${HEADERS})

#target_compile_definitions(MapTiles PRIVATE GLEW_STATIC)

target_include_directories(MapTiles PUBLIC "MapTiles/" "MapTiles/data/include" "MapTiles/render/include" "3rd/WGS84toCartesian-master" "3rd/stb_image" "3rd/glm-master/glm" "3rd/tinyxml2-master" "3rd/earcut.hpp-master/include" "3rd/cpp-httplib-master/")

target_link_libraries(MapTiles PRIVATE opengl32 glfw GLEW::GLEW)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT MapTiles)

if (WIN32)
	add_custom_command(TARGET MapTiles POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different 
		"$<$<CONFIG:Debug>:${PACKAGE_MANAGER_DLL_DIR_DEBUG}/glfw3.dll>"
		"$<$<CONFIG:Debug>:${PACKAGE_MANAGER_DLL_DIR_DEBUG}/glew32d.dll>"
		"$<$<CONFIG:Release>:${PACKAGE_MANAGER_DLL_DIR}/glfw3.dll>"
		"$<$<CONFIG:Release>:${PACKAGE_MANAGER_DLL_DIR}/glew32.dll>"
		$<TARGET_FILE_DIR:MapTiles>)
endif()