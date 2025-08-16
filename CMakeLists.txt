cmake_minimum_required(VERSION 3.20)
project(ImageParser LANGUAGES C)

set(CMAKE_C_STANDARD 11)
if (MSVC)
    set(CMAKE_C_FLAGS "/W4 /WX")
else ()
    set(CMAKE_C_FLAGS "-Wall -Wextra")
endif ()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
file(GLOB_RECURSE C_SRC "${PROJECT_SOURCE_DIR}/*.c")

add_executable(imageParser ${C_SRC})

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)

target_link_libraries(imageParser PRIVATE OpenGL::GL glfw GLEW::GLEW)
