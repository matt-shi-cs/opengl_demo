cmake_minimum_required (VERSION 3.0)
cmake_policy(VERSION 3.0)

project(guassian_blur)

set(CMAKE_CXX_STANDARD 17) # this does nothing for MSVC, use target_compile_options below
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)


option(BUILD_TARGET_DEMO "target type[on =demo]" OFF)
option(BUILD_TARGET_TEST "target type[on =test]" OFF)
option(BUILD_TARGET_LIB "target type[on =export lib]" OFF)

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

if(WIN32)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
endif(WIN32)

link_directories(${CMAKE_SOURCE_DIR}/lib)

# find the required packages
if(WIN32)
  set(OpenCV_INCLUDE_DIRS D:/work/3rdparts/opencv/4.5.2/build/win64/include)
  set(OpenCV_LIB_DIRS D:/work/3rdparts/opencv/4.5.2/build/win64/lib/Release)
else()
  find_package(OpenCV REQUIRED)
  if(OpenCV_FOUND)
    INCLUDE_DIRECTORIES(${OpenCV_INCLUDE_DIRS})
  endif()
endif()
INCLUDE_DIRECTORIES(${OpenCV_INCLUDE_DIRS})
link_directories(${OpenCV_LIB_DIRS})

if(WIN32)
  set(GLM_INCLUDE_DIRS D:/work/3rdparts/GLM/glm)
else()
  find_package(GLM REQUIRED)
endif()
message(STATUS "GLM included at ${GLM_INCLUDE_DIRS}")
INCLUDE_DIRECTORIES(${GLM_INCLUDE_DIRS})

if(WIN32)
  set(GLFW3_INCLUDE_DIRS D:/work/3rdparts/GLFW/glfw-3.3.8.bin.WIN64/include)
  set(GLFW3_LIB_DIRS D:/work/3rdparts/GLFW/glfw-3.3.8.bin.WIN64/lib-vc2019)
else()
  find_package(GLFW3 REQUIRED)
endif()
message(STATUS "Found GLFW3 in ${GLFW3_INCLUDE_DIRS}")
INCLUDE_DIRECTORIES(${GLFW3_INCLUDE_DIRS})
link_directories(${GLFW3_LIB_DIRS})


if(WIN32)
  set(ASSIMP_INCLUDE_DIRS "C:/Program Files/Assimp/include")
  set(ASSIMP_LIB_DIRS "C:/Program Files/Assimp/lib/x64")
else()
  find_package(ASSIMP REQUIRED)
endif()
message(STATUS "Found ASSIMP in ${ASSIMP_INCLUDE_DIRS}")
INCLUDE_DIRECTORIES(${ASSIMP_INCLUDE_DIRS})
link_directories(${ASSIMP_LIB_DIRS})


if(APPLE)
  find_package(Freetype REQUIRED)
  message(STATUS "Found Freetype in ${FREETYPE_INCLUDE_DIRS}")
endif(APPLE)

if(WIN32)
  find_package(OpenGL REQUIRED)
  set(LIBS glfw3 opengl32 assimp freetype irrKlang)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
elseif(UNIX AND NOT APPLE)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
  find_package(OpenGL REQUIRED)
  add_definitions(${OPENGL_DEFINITIONS})
  find_package(X11 REQUIRED)
  set(LIBS ${GLFW3_LIBRARY} X11 Xrandr Xinerama Xi Xxf86vm Xcursor GL dl pthread freetype ${ASSIMP_LIBRARY})
  set (CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -ldl")
elseif(APPLE)
  INCLUDE_DIRECTORIES(/System/Library/Frameworks)
  FIND_LIBRARY(OpenCV_LIBRARY OpenCV)
  FIND_LIBRARY(COCOA_LIBRARY Cocoa)
  FIND_LIBRARY(OpenGL_LIBRARY OpenGL)
  FIND_LIBRARY(IOKit_LIBRARY IOKit)
  FIND_LIBRARY(CoreVideo_LIBRARY CoreVideo)
  MARK_AS_ADVANCED(COCOA_LIBRARY OpenGL_LIBRARY)
  SET(APPLE_LIBS ${COCOA_LIBRARY} ${IOKit_LIBRARY} ${OpenGL_LIBRARY} ${CoreVideo_LIBRARY})
  SET(APPLE_LIBS ${APPLE_LIBS} ${GLFW3_LIBRARY} ${ASSIMP_LIBRARY} ${FREETYPE_LIBRARIES})
  SET(APPLE_LIBS ${APPLE_LIBS} glfw)
  set(LIBS ${LIBS} ${APPLE_LIBS} )
else()
  set(LIBS )
endif(WIN32)

configure_file(configuration/root_directory.h.in configuration/root_directory.h)
include_directories(${CMAKE_BINARY_DIR}/configuration)

add_library(STB_IMAGE "tools/stb_image.cpp")
set(LIBS ${LIBS} STB_IMAGE)

add_library(GLAD "tools/glad.c")
set(LIBS ${LIBS} GLAD)
if(WIN32)
  set(LIBS ${LIBS} opencv_core452 opencv_videoio452)
else()
  set(LIBS ${LIBS} opencv_core opencv_videoio)
endif()

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/includes)


set(source_for_core
    features/framebuffer/FrameBuffer.cpp
    features/framebuffer/glExtension.cpp
    features/gaussian_blur_core.cpp)
  
set(source_for_export
    features/exports/gaussian_blur_lib_export.cpp)

set(source_for_demo 
    features/framebuffer/FrameBuffer.cpp
    features/framebuffer/glExtension.cpp
    features/gaussian_blur_demo.cpp)

set(source_for_testlib
    features/gaussian_blur_main.cpp)

if(BUILD_TARGET_DEMO)
  set(source_code_files 
    ${source_for_demo})
elseif(BUILD_TARGET_LIB)
  set(source_code_files 
    ${source_for_core}
    ${source_for_export})
elseif(BUILD_TARGET_TEST)
  set(source_code_files 
    ${source_for_core}
    ${source_for_testlib})
endif()

message(STATUS "all source file: ${source_code_files}")

if(BUILD_TARGET_DEMO)
  add_executable(${PROJECT_NAME} ${source_code_files})
elseif(BUILD_TARGET_TEST)
  add_executable(${PROJECT_NAME} ${source_code_files})
elseif(BUILD_TARGET_LIB)
  add_library(${PROJECT_NAME} SHARED ${source_code_files})
endif()

target_link_libraries(${PROJECT_NAME} ${LIBS})
