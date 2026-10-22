cmake_minimum_required(VERSION 3.24)
project(SphereIsLearning LANGUAGES CXX)

add_library(sphere_core src/core/Simulation.cpp)
target_include_directories(sphere_core PUBLIC src)
target_compile_features(sphere_core PUBLIC cxx_std_20)
if(MSVC)
    target_compile_options(sphere_core PRIVATE /W4 /permissive- /utf-8)
else()
    target_compile_options(sphere_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(sphere_headless src/app/HeadlessMain.cpp)
target_link_libraries(sphere_headless PRIVATE sphere_core)

option(BUILD_TESTING "Build the tests" ON)
if(BUILD_TESTING)
    enable_testing()
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/SimulationTests.cpp")
        add_executable(sphere_tests tests/SimulationTests.cpp)
        target_link_libraries(sphere_tests PRIVATE sphere_core)
        add_test(NAME simulation COMMAND sphere_tests)
    else()
        message(STATUS "Simulation tests are not implemented yet")
    endif()
endif()

if(WIN32)
    set(BUILD_TOOLS OFF CACHE BOOL "Build DirectXTK auxiliary tools" FORCE)
    set(BUILD_XAUDIO_WIN8 OFF CACHE BOOL "Build DirectXTK audio" FORCE)
    add_subdirectory(external/DirectXTK)

    add_executable(sphere_game WIN32
        src/app/WindowsMain.cpp
        src/renderer/directX11/Renderer.cpp
        src/renderer/directX11/Renderer.hpp)
    target_include_directories(sphere_game PRIVATE src)
    target_compile_definitions(sphere_game PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX UNICODE _UNICODE)
    target_link_libraries(sphere_game PRIVATE sphere_core DirectXTK user32 d3d11 dxgi)
    if(MSVC)
        target_compile_options(sphere_game PRIVATE /W4 /permissive- /utf-8)
    endif()
endif()