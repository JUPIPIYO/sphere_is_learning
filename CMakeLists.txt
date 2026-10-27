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

# AI。sphere_coreを純粋なシミュレーションのまま保つため、別ライブラリにする。
# 新しい.cppを作ったら、この一覧にだけ追加する。
add_library(sphere_ai
    src/ai/common/Observation.cpp
    src/ai/common/Episode.cpp
    src/ai/ga/GAAgent.cpp 
    src/ai/ga/GenomeFile.cpp
    src/ai/ga/GATrainer.cpp)
target_link_libraries(sphere_ai PUBLIC sphere_core)
if(MSVC)
    target_compile_options(sphere_ai PRIVATE /W4 /permissive- /utf-8)
else()
    target_compile_options(sphere_ai PRIVATE -Wall -Wextra -Wpedantic)
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

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/AiTests.cpp")
        add_executable(ai_tests tests/AiTests.cpp)
        target_link_libraries(ai_tests PRIVATE sphere_ai)
        add_test(NAME ai COMMAND ai_tests)
    else()
        message(STATUS "AI tests are not implemented yet (tests/AiTests.cpp)")
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

add_executable(sphere_experiment src/app/AgentMain.cpp)
target_link_libraries(sphere_experiment PRIVATE sphere_ai)

endif()