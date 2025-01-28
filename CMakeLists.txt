cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
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

file(GLOB SOURCES 
            "MapTiles/*.cpp" 
            "MapTiles/render/src/*.cpp" 
            "MapTiles/render/src/opengl/*.cpp" 
            "MapTiles/data/src/*.cpp"
    )

file(GLOB HEADERS 
            "MapTiles/*.h" 
            "MapTiles/render/include/*.h" 
            "MapTiles/render/include/opengl/*.h" 
            "MapTiles/data/include/*.h"
    )

set(IMGUI_SOURCES 
            "3rd/imgui/imgui.cpp" 
            "3rd/imgui/imgui_demo.cpp" 
            "3rd/imgui/imgui_draw.cpp" 
            "3rd/imgui/imgui_tables.cpp"
            "3rd/imgui/imgui_widgets.cpp" 
            "3rd/imgui/backends/imgui_impl_glfw.cpp" 
            "3rd/imgui/backends/imgui_impl_opengl3.cpp"
    )

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the build type" FORCE)
endif()

find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)

if (UNIX)
	find_package(Threads REQUIRED)
endif()

add_executable(MapTiles 
            ${SOURCES} 
            ${HEADERS}
            "3rd/tinyxml2-master/tinyxml2.cpp"
            ${IMGUI_SOURCES} 
    )

#target_compile_definitions(MapTiles PRIVATE GLEW_STATIC)

target_include_directories(MapTiles PUBLIC 
            "MapTiles/" 
            "MapTiles/data/include" 
            "MapTiles/render/include" 
            "3rd/WGS84toCartesian-master" 
            "3rd/stb_image" 
            "3rd/glm-master/glm" 
            "3rd/tinyxml2-master" 
            "3rd/earcut.hpp-master/include" 
            "3rd/cpp-httplib-master" 
            "3rd/imgui/"
    )

target_link_libraries(MapTiles PRIVATE OpenGL::GL glfw GLEW::GLEW)

if (UNIX)
	target_link_libraries(MapTiles PRIVATE Threads::Threads)
endif()

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