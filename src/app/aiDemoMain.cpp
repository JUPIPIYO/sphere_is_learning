#include <windows.h>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <string>
#include <iostream>

#include "core/Simulation.hpp"
#include "renderer/directX11/Renderer.hpp"
#include "ai/ga/GenomeFile.hpp"

//ImGui関連
#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.h>

//AIファイルの読み込み関連
#include <filesystem>
#include <string_view>
#include <charconv>
#include <optional>
#include <format>

#include <shellapi.h>
#include <memory>

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


    void advanceGame(Environment& env, GAAgent* agent, GameState& lastState, uint32_t initialSeed, float& lastReward, vector<string>& lastResults, vector<float>& lastRewards)
    {
        //ここでnextDirecを呼ぶ
        Direc d = agent->nextDirec(env);
        auto result = env.step(d);

        if (result.end != GameState::Running)
        {
            lastState = result.end;
            lastReward = result.reward;

            lastResults.push_back(StateToStr(lastState));
            lastRewards.push_back(lastReward);

            env.reset(initialSeed);
            agent->reset();
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

    //追加：保存したAIファイルを探す
    //ファイル名は'results/GA/seedXX/{epoch}_distance.sil' or 'seedXX/g{epoch}_fitness.sil'
    //seedXXはpath時点で指定
    //kind : 「fitness」 or 「distance」
    std::optional<std::filesystem::path> findLatestGenome(const std::filesystem::path& folder,std::string_view kind)
    {
        //ファイルの存在を確認
        if (!std::filesystem::exists(folder)) return std::nullopt;

        std::filesystem::path path;
        int maxEpisode = 0;
        //フォルダ内を一つずつ精査
        for (auto& entry : std::filesystem::directory_iterator(folder))
        {
            if (!entry.is_regular_file()) continue;

            //1文字目のepochが最大のものを設定
            auto currentPath = entry.path();
            string endPrefix = "_" + std::string(kind) + ".sil";
            if (!currentPath.filename().string().ends_with(endPrefix)) continue;

            int currentEp = std::stoi(entry.path().filename().string().substr(0, 1));
            
            if (currentEp >= maxEpisode)
            {
                maxEpisode = currentEp;
                path = currentPath;
            }

        }

        if (path.empty()) return std::nullopt;

        return path;
    }

    struct LocalFreeDeleter {
        void operator()(wchar_t** p) const { LocalFree(p); }
    };

    // 起動引数の1つ目（なければ空）を返す
    std::filesystem::path firstArgument() {
        int count = 0;
        std::unique_ptr<wchar_t*, LocalFreeDeleter> args(CommandLineToArgvW(GetCommandLineW(), &count));
        if (!args || count < 2) return {};
        return args.get()[1];   // [0]はexe自身のパス
    }
}


int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand)
{
    HWND window = nullptr;
    try {

        std::filesystem::path genomePath = firstArgument();
        if (genomePath.empty()) {
            const auto latest = findLatestGenome("results/GA/seed42/","distance");
            if (!latest) throw std::runtime_error("results/GA/seed42/ has no *_distance.sil");
            genomePath = *latest;
        }

        //現ディレクトリ上のファイルを全取得
        std::filesystem::path dirPath = "results/GA/seed42";
        vector<filesystem::path> files;
        for (auto& entry : std::filesystem::directory_iterator(dirPath))
        {
            if (!entry.is_regular_file()) continue;

            files.push_back(entry.path());
        }

        //savedをちゃんと変数として宣言しないと消える
        SavedGenome saved = loadGenome(genomePath.string());  
        GAAgent agent{ saved.genes };                                
        std::string genomeName = genomePath.filename().string();

        vector<int> speeds = { 1, 2, 4 };
        int currentSpeed = 1;


        //ここらへんガチでなにやってるかわからん
        constexpr wchar_t className[] = L"SphereIsLearningAIDemoWindow";
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
        window = CreateWindowExW(0, className, L"sphere is learning - AI DEMO", WS_OVERLAPPEDWINDOW,
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

        Environment env{ makeStage() };
        env.reset(initialSeed);
        agent.reset();

        GameState lastEnd = GameState::Running;
        float lastReward = 0;
        typedef chrono::steady_clock Clock;
        //using Clock = chrono::steady_clock;

        auto previousTime = Clock::now();
        double accumulator = 0.0;
        bool running = true;
        ShowWindow(window, showCommand);

        //ステージ選択部分を表示するための変数
        int selectedStage = 0;
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
                //ステージの選択ボタンを表示
                ImGui::RadioButton("Stage 1", &selectedStage, 0);

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


                static size_t selected_idx = 0;
                if (ImGui::BeginCombo("Select File", genomeName.c_str())) 
                {
                    for (size_t n = 0; n < files.size(); n++) {
                        bool is_selected = (selected_idx == n);

                        // リストに表示する文字列（ここではファイル名だけを表示）
                        std::filesystem::path filePath =  files[n];

                        if (ImGui::Selectable(filePath.filename().string().c_str(), is_selected))
                        {
                            selected_idx = n; // インデックスを更新

                            //ここで指定されているファイルにGenomeを更新し、全てreset
                            saved = loadGenome(filePath.string());
                            GAAgent agent{ saved.genes };
                            genomeName = filePath.filename().string();

                            env.reset(initialSeed);
                            agent.reset();
                        }

                        if (is_selected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }

                static size_t speed_idx = 0;
                //倍速
                if (ImGui::BeginCombo("SPEED", to_string(currentSpeed).c_str()))
                {
                    for (size_t i = 0; i < speeds.size(); i++)
                    {
                        bool is_selected = (speed_idx == i);

                        // リストに表示する文字列（ここではファイル名だけを表示）
                        int speed = speeds[i];

                        if (ImGui::Selectable(to_string(speed).c_str(), is_selected))
                        {
                            speed_idx = i; // インデックスを更新
                            currentSpeed = speed;
                        }

                        if (is_selected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }


                //AI表示
                ImGui::Text("read File : %s, epoch : %d, seed : %d", genomeName, saved.generation, saved.seed);
                ImGui::Text("bestZ : %d, survivedSec : %d, result : %s", saved.metrics.bestZ, saved.metrics.surviveSeconds, StateToStr(saved.metrics.end));

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

                //ImGui::Text("Result : %s Reward : %.1f", StateToStr(lastEnd).c_str(), lastReward);
            }

            ImGui::End();

            if (mainPlaying)
            {
                accumulator += elapsed * currentSpeed;

                while (accumulator >= sil::Environment::dt)
                {
                    advanceGame(env, &agent, lastEnd, initialSeed, lastReward, lastResults, lastRewards);
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
        MessageBoxA(nullptr, error.what(), "sphere is learning - AIDEMO", MB_OK | MB_ICONERROR);
        return 1;
    }
    return 0;
}
