#include <windows.h>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <string>

#include "core/Simulation.hpp"
#include "renderer/directX11/Renderer.hpp"
using namespace sil;
using namespace std;
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
	};


// Rendererより後までウィンドウを生かし例外時にも閉じる
struct WindowLifetime {
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
	//TODO : WASDを読む形に実装
	return Direc::Stop;
}

void advanceGame(Environment&, Direc, GameState&, uint32_t)
{

}

LRESULT CALLBACK windowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    auto* state = reinterpret_cast<WindowState*>(GetWindowLongPtrW(window, GWLP_USERDATA));
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
    }
    return DefWindowProcW(window, message, wParam, lParam);
}
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

        constexpr uint32_t initialSeed = 42;

        Environment env{ makeStage() };
        env.reset(initialSeed);

        GameState lastEnd = GameState::Running;
        typedef chrono::steady_clock Clock;
        //using Clock = chrono::steady_clock;

        auto previousTime = Clock::now();
        double accumulator = 0.0;
        bool running = true;
        ShowWindow(window, showCommand);

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

            // 5-Iに進んだら、この位置へ保留中のサイズ変更処理を追加する。
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

            // GUIを追加するときは、この位置でNewFrameとUI作成を行う。
            accumulator += elapsed;
            while (accumulator >= Environment::dt) 
            {
                //ゲームが実際に進む
                advanceGame(env, readDirec(), lastEnd, initialSeed);
                accumulator -= Environment::dt;
            }

            renderer.render(env.stage(), env.state());
            // GUIを追加するときは、この位置でGUIを描画する。
            renderer.present();
        }
    }
    catch (const std::exception& error) {
        MessageBoxA(nullptr, error.what(), "sphere is learning", MB_OK | MB_ICONERROR);
        return 1;
    }
    return 0;
}
