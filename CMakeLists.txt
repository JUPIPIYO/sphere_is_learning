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

    # PolicyTest.cppは現在ai/Policy.cppを直接includeしているため、Policy.cppはどのターゲットにも追加しない。
    # Policy.hppへ宣言を分けたら、Policy.cppをsphere_coreへ追加し、テストのincludeをPolicy.hppへ変える。
    add_executable(policy_tests tests/PolicyTest.cpp)
    target_link_libraries(policy_tests PRIVATE sphere_core)
    add_test(NAME policy COMMAND policy_tests)
endif()

if(WIN32)
    set(BUILD_TOOLS OFF CACHE BOOL "Build DirectXTK auxiliary tools" FORCE)
    set(BUILD_XAUDIO_WIN8 OFF CACHE BOOL "Build DirectXTK audio" FORCE)
    add_subdirectory(external/DirectXTK)

    add_executable(sphere_game WIN32
        src/app/WindowsMain.cpp
        src/renderer/directX11/Renderer.cpp
        src/renderer/directX11/Renderer.hpp)

    #GUI関連
    add_library(sil_imgui STATIC
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_win32.cpp
    external/imgui/backends/imgui_impl_dx11.cpp)
    target_include_directories(sil_imgui PUBLIC external/imgui)
    target_compile_definitions(sil_imgui PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX UNICODE _UNICODE)
    target_link_libraries(sil_imgui PRIVATE d3d11 dxgi d3dcompiler dwmapi user32 gdi32 imm32)
    target_link_libraries(sphere_game PRIVATE sil_imgui)
    target_compile_definitions(sphere_game PRIVATE SIL_WITH_IMGUI=1)

    target_include_directories(sphere_game PRIVATE src)
    target_compile_definitions(sphere_game PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX UNICODE _UNICODE)
    target_link_libraries(sphere_game PRIVATE sphere_core DirectXTK user32 d3d11 dxgi)
    if(MSVC)
        target_compile_options(sphere_game PRIVATE /W4 /permissive- /utf-8)
    endif()
endif()