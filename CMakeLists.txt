cmake_minimum_required(VERSION 3.12)
project(mtcr_demo VERSION 1.0 LANGUAGES C)

# General boilerplate
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# We want to make sure to set up platform defines for acfutils and XPLM
add_definitions(-DXPLM200=1 -DXPLM210=1 -DXPLM300=1 -DXPLM301=1 -DDEBUG=1)
if(APPLE)
	set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Build architectures for Mac OS X" FORCE)
	add_definitions(-DAPL=1 -DIBM=0 -DLIN=0)
    set(ARCH "mac64")
elseif(WIN32)
	add_definitions(-DAPL=0 -DIBM=1 -DLIN=0)
    add_definitions(-D_WIN32_WINNT=0x0600)
    set(ARCH "mingw64")
    
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
    set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "${CMAKE_SHARED_LIBRARY_LINK_C_FLAGS} -static-libgcc -s")
    set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "${CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS}  -static-libgcc -static-libstdc++ -s")
else()
	add_definitions(-DAPL=0 -DIBM=0 -DLIN=1)
    set(ARCH "lin64")
endif()

# Set up acfutils

option(LIBACFUTILS "Directory where libacfutils is installed" "")

file(GLOB LIBACFUTILS_LIBS ${LIBACFUTILS}/libacfutils-redist/${ARCH}/lib/*.a)
if(WIN32)
    list(APPEND LIBACFUTILS_LIBS crypt32 ws2_32 gdi32 dbghelp psapi winmm)
else()
    list(APPEND LIBACFUTILS_LIBS pthread)
endif()
set(SDK_ROOT "${LIBACFUTILS}/SDK")

include_directories(
    "${LIBACFUTILS}/libacfutils-redist/include"
    "${LIBACFUTILS}/libacfutils-redist/${ARCH}/include"
    "${SDK_ROOT}/CHeaders/XPLM"
    "${SDK_ROOT}/CHeaders/Widgets"
    "${SDK_ROOT}/CHeaders/Wrappers"
)

# Set up required libraries

find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED)

# define our plugin target
add_library(mtcr_demo SHARED src/xplane.c)
target_compile_options(mtcr_demo PUBLIC -Wall -Wextra -Werror)
target_link_libraries(mtcr_demo PUBLIC ${LIBACFUTILS_LIBS} ${OPENGL_LIBRARIES})

# X-Plane Plugin stuff 
if(APPLE)
    find_library(XPLM_LIBRARY XPLM ${SDK_ROOT}/Libraries/Mac/)
    find_library(XPWIDGETS_LIBRARY XPWidgets ${SDK_ROOT}/Libraries/Mac/)
    find_library(CocoaLib Cocoa)
    
    target_link_libraries(mtcr_demo PUBLIC ${XPLM_LIBRARY})
    target_link_libraries(mtcr_demo PUBLIC ${XPWIDGETS_LIBRARY})
    target_link_libraries(mtcr_demo PUBLIC ${CocoaLib})
elseif(WIN32)
    set(CMAKE_LIBRARY_PATH ${SDK_ROOT}/Libraries/Win/)
    find_library(XPLM_LIBRARY
        NAMES XPLM_64
        PATHS ${SDK_ROOT}/Libraries/Win/
        NO_DEFAULT_PATH
        NO_CMAKE_FIND_ROOT_PATH
    )
    find_library(XPWIDGETS_LIBRARY
        XPWidgets_64
        ${SDK_ROOT}/Libraries/Win/
        NO_DEFAULT_PATH
        NO_CMAKE_FIND_ROOT_PATH
    )
    target_link_libraries(mtcr_demo PUBLIC ${XPLM_LIBRARY} ${XPWIDGETS_LIBRARY})
endif()

set_target_properties(mtcr_demo PROPERTIES PREFIX "")
set_target_properties(mtcr_demo PROPERTIES SUFFIX "")
set_target_properties(mtcr_demo PROPERTIES OUTPUT_NAME "mtcr_demo.xpl")

set(PLUGIN_BUILD_DIR "${PROJECT_SOURCE_DIR}/mtcr_demo")

if(WIN32)
	set_target_properties(mtcr_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PLUGIN_BUILD_DIR}/win_x64" )
	set_target_properties(mtcr_demo PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PLUGIN_BUILD_DIR}/win_x64" )
    # set(OUT_PLUGIN_PATH "Aircraft/Q4XP/plugins/efb/win_x64")
elseif(APPLE)
	set_target_properties(mtcr_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PLUGIN_BUILD_DIR}/mac_x64" )
	set_target_properties(mtcr_demo PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PLUGIN_BUILD_DIR}/mac_x64" )
    # set(OUT_PLUGIN_PATH "Aircraft/Q4XP/plugins/efb/mac_x64")
else()
	set_target_properties(mtcr_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PLUGIN_BUILD_DIR}/lin_x64" )
	set_target_properties(mtcr_demo PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PLUGIN_BUILD_DIR}/lin_x64" )
    # set(OUT_PLUGIN_PATH "Aircraft/Q4XP/plugins/efb/lin_x64")
endif()

set_target_properties(mtcr_demo PROPERTIES LINK_FLAGS
    "${CMAKE_SHARED_LINKER_FLAGS} -fno-stack-protector -fPIC -fvisibility=hidden"
)
