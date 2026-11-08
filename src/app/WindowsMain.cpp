#include <windows.h>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <string>

#include "core/Simulation.hpp"
#include "renderer/directX11/Renderer.hpp"

//ImGui関連
#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.h>

using namespace sil;
using namespace std;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

namespace
{
	struct WindowState
	{
		//ウィンドウサイズを定義
		uint32_t width = 1280;
		uint32_t height = 720;

		bool resizePending = false;
		bool minimized = false;
		bool hasFocus = true;
		bool resetClock = false;
        bool imguiReady = false;
	};


    // Rendererより後までウィンドウを生かし例外時にも閉じる
    struct WindowLifetime 
    {
	    HWND window;
	    ~WindowLifetime() 
	    {
		    if (IsWindow(window)) {
			    DestroyWindow(window);
		    }
	    }
    };

    Direc readDirec()
    {
	    //WASDの入力内容を下にDirecを作成
        int w = 0x57;
        int a = 0x41;
        int s = 0x53;
        int d = 0x44;
    
        int x = 0; int z = 0;

        if ((GetAsyncKeyState(d) && 0x8000) != 0) x += 1;
        if ((GetAsyncKeyState(a) && 0x8000) != 0) x -= 1;

        if ((GetAsyncKeyState(w) && 0x8000) != 0) z += 1;
        if ((GetAsyncKeyState(s) && 0x8000) != 0) z -= 1;

        auto direc = direcFromAxes(x, z);
        return direc;
    }

    void advanceGame(Environment& env, Direc nextDirec, GameState& lastState, uint32_t initialSeed, float& lastReward, vector<string>& lastResults, vector<float>& lastRewards )
    {
        auto result = env.step(nextDirec);

        if (result.end != GameState::Running)
        {
            lastState = result.end;
            lastReward = result.reward;
            
            lastResults.push_back(StateToStr(lastState));
            lastRewards.push_back(lastReward);

            env.reset(initialSeed);
        }
    }

    LRESULT CALLBACK windowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
    {
        auto* state = reinterpret_cast<WindowState*>(GetWindowLongPtrW(window, GWLP_USERDATA));

        const LRESULT guiHandled = state && state->imguiReady
            ? ImGui_ImplWin32_WndProcHandler(window, message, wParam, lParam) : 0;

        switch (message) {
        case WM_NCCREATE: {
            const auto* create = reinterpret_cast<const CREATESTRUCTW*>(lParam);
            SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(create->lpCreateParams));
            return TRUE;
        }
        case WM_SIZE:
            if (state) {
                state->width = LOWORD(lParam);
                state->height = HIWORD(lParam);
                state->minimized = wParam == SIZE_MINIMIZED;
                state->resizePending = !state->minimized;
                state->resetClock = true;
            }
            return 0;
        case WM_ACTIVATEAPP:
            if (state) {
                state->hasFocus = wParam != FALSE;
                state->resetClock = true;
            }
            return 0;
        case WM_CLOSE:
            DestroyWindow(window);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        };

        if (guiHandled) return guiHandled;

        return DefWindowProcW(window, message, wParam, lParam);
    }

    class ImGuiSession {
    public:
        ImGuiSession(HWND window, sil::renderer::Renderer& renderer, WindowState& state)
            : state_(state)
        {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            try {
                ImGui::StyleColorsDark();

                //各種バックエンドの初期化
                ImGui_ImplWin32_Init(window);
                ImGui_ImplDX11_Init(renderer.device(), renderer.context());

                win32Ready_ = true;
                dx11Ready_ = true;

                state_.imguiReady = true;
            }
            catch (...) {
                shutdown();
                throw;
            }
        }
        ~ImGuiSession() { shutdown(); }
        ImGuiSession(const ImGuiSession&) = delete;
        ImGuiSession& operator=(const ImGuiSession&) = delete;

    private:
        void shutdown() 
        {
            state_.imguiReady = false;
            if (dx11Ready_) 
            {
                ImGui_ImplDX11_Shutdown();
            }
            if (win32Ready_) 
            {
                ImGui_ImplWin32_Shutdown();
            }
            ImGui::DestroyContext();
        }
        WindowState& state_;
        bool win32Ready_ = false;
        bool dx11Ready_ = false;
    };


}


int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand)
{
    HWND window = nullptr;
    try {
        //ここらへんガチでなにやってるかわからん
        constexpr wchar_t className[] = L"SphereIsLearningWindow";
        WNDCLASSEXW windowClass{};
        windowClass.cbSize = sizeof(windowClass);
        windowClass.hInstance = instance;
        windowClass.lpszClassName = className;
        windowClass.lpfnWndProc = windowProcedure;
        windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        if (!RegisterClassExW(&windowClass)) 
        {
            const DWORD error = GetLastError();
            throw runtime_error("RegisterClassExW: " + std::to_string(error));
        }

        WindowState windowState;
        window = CreateWindowExW(0, className, L"sphere is learning", WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720, nullptr, nullptr, instance, &windowState);
        if (!window) 
        {
            const DWORD error = GetLastError();
            throw runtime_error("CreateWindowExW: " + to_string(error));
        }
        WindowLifetime windowLifetime{ window };

        RECT client{};
        if (!GetClientRect(window, &client)) {
            const DWORD error = GetLastError();
            throw runtime_error("GetClientRect: " + to_string(error));
        }
        windowState.width = static_cast<uint32_t>(client.right - client.left);
        windowState.height = static_cast<uint32_t>(client.bottom - client.top);
        windowState.resizePending = false;

        //Rendererを作成
        renderer::Renderer renderer(window, windowState.width, windowState.height);

        //imGuiを作成
        ImGuiSession imGui(window, renderer, windowState);

        constexpr uint32_t initialSeed = 42;

        vector<int> stages = { 1, 2, 3, 4 };
        int currentStage = 1;

        Environment env{ makeStage(currentStage) };
        env.reset(initialSeed);

        GameState lastEnd = GameState::Running;
        float lastReward = 0;
        typedef chrono::steady_clock Clock;
        //using Clock = chrono::steady_clock;

        auto previousTime = Clock::now();
        double accumulator = 0.0;
        bool running = true;
        ShowWindow(window, showCommand);

        bool mainPlaying = false;

        //過去のリザルトを5回分だけ保持して、表示する
        vector<string> lastResults;
        vector<float> lastRewards;

        while (running) {
            MSG message{};
            while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) 
            {
                if (message.message == WM_QUIT) {
                    running = false;
                    break;
                }
                TranslateMessage(&message);
                DispatchMessageW(&message);
            }

            if (!running)
            {
                break;
            }

            // サイズ変更処理
            if (windowState.resizePending && !windowState.minimized) {
                renderer.resize(windowState.width, windowState.height);
                windowState.resizePending = false;
            }

            //windowStateでここはなにをやっているんだろう
            const auto now = Clock::now();
            if (windowState.resetClock || windowState.minimized || !windowState.hasFocus) 
            {
                previousTime = now;
                accumulator = 0.0;
                windowState.resetClock = false;
            }
            if (windowState.minimized || !windowState.hasFocus) {
                Sleep(10);
                continue;
            }
            const double elapsed = min(std::chrono::duration<double>(now - previousTime).count(), 0.25);
            previousTime = now;

            // GUIの追加
            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            if (ImGui::Begin("Stage selection")) {

                if (!mainPlaying)
                {
                    if (ImGui::Button("Start"))
                    {
                        mainPlaying = true;
                    }
                }
                else
                {
                    if (ImGui::Button("Stop Game"))
                    {
                        mainPlaying = false;
                    }
                }
                
                //残り時間と前回の結果を表示

                int j = 0;
                ImGui::Text("Remaining %.1f second", env.remainSec());
                if (lastResults.size() > 0)
                {
                    for (int i = lastResults.size() - 1; i >= 0; i--)
                    {
                        ImGui::Text("Result : %s Reward : %.1f", lastResults[i].c_str(), lastRewards[i]);
                        j++;
                        if (j >= 15) break;
                    }
                }

                static size_t stage_idx = 0;
                //ステージ
                if (ImGui::BeginCombo("STAGE", to_string(currentStage).c_str()))
                {
                    for (size_t i = 0; i < stages.size(); i++)
                    {
                        bool is_selected = (stage_idx == i);

                        int stage = stages[i];

                        if (ImGui::Selectable(to_string(stage).c_str(), is_selected))
                        {
                            stage_idx = i; // インデックスを更新
                            //違うステージになるならハードリセット
                            if (currentStage != stage)
                            {
                                currentStage = stage;
                                env = Environment(makeStage(currentStage));
                                env.reset(initialSeed);
                            }
                        }

                        if (is_selected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }

                //ImGui::Text("Result : %s Reward : %.1f", StateToStr(lastEnd).c_str(), lastReward);
            }

            ImGui::End();

            if (mainPlaying) 
            {
                const auto direc = ImGui::GetIO().WantCaptureKeyboard
                    ? sil::Direc::Stop : readDirec();
                accumulator += elapsed;

                while (accumulator >= sil::Environment::dt) 
                {
                    advanceGame(env, direc, lastEnd, initialSeed, lastReward, lastResults, lastRewards);
                    accumulator -= sil::Environment::dt;
                }
            }
            else 
            {
                accumulator = 0.0;
            }

            //描画
            renderer.render(env.stage(), env.state());

            // GUI描画
            ImGui::Render();
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

            renderer.present();
        }
    }
    catch (const std::exception& error) {
        MessageBoxA(nullptr, error.what(), "sphere is learning", MB_OK | MB_ICONERROR);
        return 1;
    }
    return 0;
}
