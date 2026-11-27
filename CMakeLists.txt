cmake_minimum_required(VERSION 3.21)

project(tgbot VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenCV CONFIG REQUIRED COMPONENTS core imgproc imgcodecs highgui)
find_package(nlohmann_json CONFIG REQUIRED)

add_executable(tgbot
    src/main.cpp
    src/core/Logger.cpp
    src/core/Randomizer.cpp
)

target_include_directories(tgbot PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(tgbot PRIVATE
    opencv_core
    opencv_imgproc
    opencv_imgcodecs
    opencv_highgui
    nlohmann_json::nlohmann_json
)

if(MSVC)
    target_compile_options(tgbot PRIVATE /W4 /permissive- /utf-8)
    target_compile_definitions(tgbot PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
endif()
