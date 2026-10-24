//#pragma once
#include <cstdint>
#include <memory>
#include <windows.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <GeometricPrimitive.h>

using namespace std;

namespace sil { struct Stage; struct State; }

namespace sil::renderer {
    class Renderer{
    public:
        Renderer(HWND window, uint32_t width, uint32_t height);
        ~Renderer() = default;
        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        void resize(uint32_t width, uint32_t height);
        //stageとstateから現在の状況を実際に描画
        void render(const Stage& stage, const State& state);
        //実際に表示
        void present();
        ID3D11Device* device() const { return device_.Get(); }
        ID3D11DeviceContext* context() const { return context_.Get(); }

    private:
        void createDeviceAndSwapChain(HWND window);
        void createSizeDependentResources(uint32_t width, uint32_t height);

        // TODO: 所有する資源、ビューポート、幅と高さ、図形を宣言
        Microsoft::WRL::ComPtr<ID3D11Device> device_;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
        Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain_;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView_;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView_;

             Microsoft::WRL::ComPtr<ID3D11Texture2D> depthTexture_;
        D3D11_VIEWPORT viewport_{};
        uint32_t width_;
        uint32_t height_;

        unique_ptr<DirectX::GeometricPrimitive> cube_;
        unique_ptr<DirectX::GeometricPrimitive> sphere_;
    };
}