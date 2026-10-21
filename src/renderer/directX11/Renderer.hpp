#include "core/Simulation.hpp"

#include <cstdint>
#include <memory>

#include <windows.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <GeometricPrimitive.h>

using namespace std;

namespace sil::renderer
{
    //StateとStageの情報から3D表示を行うクラス
    class Renderer
    {
    public:
        Renderer(HWND window, std::uint32_t width, std::uint32_t height);
        ~Renderer() = default;

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        void resize(std::uint32_t width, std::uint32_t height);

        void render(const Stage& stage, const State& state);
        void present();

        //DirectX 11 バックエンドを初期化するときに
        ID3D11Device* device() const noexcept { return device_.Get(); }
        ID3D11DeviceContext* context() const noexcept { return context_.Get(); }

    private:
        void createDeviceAndSwapChain(HWND window);
        void createSizeDependentResources(std::uint32_t width, std::uint32_t height);

        Microsoft::WRL::ComPtr<ID3D11Device> device_;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
        Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain_;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView_;
        Microsoft::WRL::ComPtr<ID3D11Texture2D> depthTexture_;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView_;

        D3D11_VIEWPORT viewport_{};
        std::uint32_t width_ = 0;
        std::uint32_t height_ = 0;

        std::unique_ptr<DirectX::GeometricPrimitive> cube_;
        std::unique_ptr<DirectX::GeometricPrimitive> sphere_;
    };