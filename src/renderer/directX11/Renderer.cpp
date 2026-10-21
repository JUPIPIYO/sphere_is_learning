#include "renderer/d3d11/Renderer.hpp"

#include <algorithm>
#include <iomanip>
#include <iterator>
#include <sstream>
#include <stdexcept>

#include <DirectXColors.h>
#include <DirectXMath.h>
#include "core/Simulation.hpp"

namespace {

    void throwIfFailed(HRESULT result, const char* operation) {
        if (SUCCEEDED(result)) {
            return;
        }

        std::ostringstream message;
        message << operation << " failed (HRESULT=0x"
            << std::hex << std::uppercase << static_cast<unsigned long>(result) << ')';
        throw std::runtime_error(message.str());
    }

    // DirectXTK の単位立方体を、XZ平面上の Rect と指定した高さへ変換して描く。
    void drawRect(
        DirectX::GeometricPrimitive& cube,
        const sil::Rect& rect,
        float centerY,
        float height,
        DirectX::FXMVECTOR color,
        DirectX::FXMMATRIX view,
        DirectX::FXMMATRIX projection) {
        using namespace DirectX;

        const XMMATRIX world =
            XMMatrixScaling(rect.half.x * 2.0F, height, rect.half.z * 2.0F) *
            XMMatrixTranslation(rect.center.x, centerY, rect.center.z);
        cube.Draw(world, view, projection, color);
    }

} // namespace

namespace sil::renderer {

    Renderer::Renderer(HWND window, std::uint32_t width, std::uint32_t height) {
        createDeviceAndSwapChain(window);

        // DirectXTK の図形生成はデバイスコンテキストを受け取る。false は左手系座標。
        cube_ = DirectX::GeometricPrimitive::CreateCube(context_.Get(), 1.0F, false);
        sphere_ = DirectX::GeometricPrimitive::CreateSphere(context_.Get(), 1.0F, 24, false);

        resize(width, height);
    }

    void Renderer::createDeviceAndSwapChain(HWND window) {
        DXGI_SWAP_CHAIN_DESC swapChainDescription{};
        swapChainDescription.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDescription.SampleDesc.Count = 1;
        swapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDescription.BufferCount = 1;
        swapChainDescription.OutputWindow = window;
        swapChainDescription.Windowed = TRUE;
        swapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG)
        // Visual Studio の Graphics Diagnostics / D3D11 debug layer を使うための指定。
        flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        constexpr D3D_FEATURE_LEVEL requestedLevels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
        };
        D3D_FEATURE_LEVEL actualLevel{};

        throwIfFailed(
            D3D11CreateDeviceAndSwapChain(
                nullptr,
                D3D_DRIVER_TYPE_HARDWARE,
                nullptr,
                flags,
                requestedLevels,
                static_cast<UINT>(std::size(requestedLevels)),
                D3D11_SDK_VERSION,
                &swapChainDescription,
                swapChain_.GetAddressOf(),
                device_.GetAddressOf(),
                &actualLevel,
                context_.GetAddressOf()),
            "D3D11CreateDeviceAndSwapChain");
    }

    void Renderer::resize(std::uint32_t width, std::uint32_t height) {
        if (width == 0 || height == 0) {
            return;
        }

        // 初回も ResizeBuffers 後も、描画先と深度バッファを作り直す。
        if (width_ != 0 && height_ != 0) {
            context_->OMSetRenderTargets(0, nullptr, nullptr);
            renderTargetView_.Reset();
            depthStencilView_.Reset();
            depthTexture_.Reset();

            throwIfFailed(
                swapChain_->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0),
                "IDXGISwapChain::ResizeBuffers");
        }

        createSizeDependentResources(width, height);
        width_ = width;
        height_ = height;
    }

    void Renderer::createSizeDependentResources(std::uint32_t width, std::uint32_t height) {
        Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
        throwIfFailed(
            swapChain_->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())),
            "IDXGISwapChain::GetBuffer");
        throwIfFailed(
            device_->CreateRenderTargetView(backBuffer.Get(), nullptr, renderTargetView_.GetAddressOf()),
            "ID3D11Device::CreateRenderTargetView");

        D3D11_TEXTURE2D_DESC depthDescription{};
        depthDescription.Width = width;
        depthDescription.Height = height;
        depthDescription.MipLevels = 1;
        depthDescription.ArraySize = 1;
        depthDescription.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthDescription.SampleDesc.Count = 1;
        depthDescription.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        throwIfFailed(
            device_->CreateTexture2D(&depthDescription, nullptr, depthTexture_.GetAddressOf()),
            "ID3D11Device::CreateTexture2D for depth");
        throwIfFailed(
            device_->CreateDepthStencilView(
                depthTexture_.Get(), nullptr, depthStencilView_.GetAddressOf()),
            "ID3D11Device::CreateDepthStencilView");

        viewport_.TopLeftX = 0.0F;
        viewport_.TopLeftY = 0.0F;
        viewport_.Width = static_cast<float>(width);
        viewport_.Height = static_cast<float>(height);
        viewport_.MinDepth = 0.0F;
        viewport_.MaxDepth = 1.0F;
    }

    void Renderer::render(const Stage& stage, const State& state) {
        if (width_ == 0 || height_ == 0) {
            return;
        }

        const float clearColor[] = { 0.06F, 0.08F, 0.12F, 1.0F };
        context_->OMSetRenderTargets(1, renderTargetView_.GetAddressOf(), depthStencilView_.Get());
        context_->RSSetViewports(1, &viewport_);
        context_->ClearRenderTargetView(renderTargetView_.Get(), clearColor);
        context_->ClearDepthStencilView(
            depthStencilView_.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0F, 0);

        using namespace DirectX;
        const float aspect = static_cast<float>(width_) / static_cast<float>(height_);

        // 左手系で -Z 側から原点を見る。画面の右はワールドの +X、上は +Z になる。
        const XMMATRIX view = XMMatrixLookAtLH(
            XMVectorSet(0.0F, 16.0F, -18.0F, 0.0F),
            XMVectorSet(0.0F, 0.0F, 0.0F, 0.0F),
            XMVectorSet(0.0F, 1.0F, 0.0F, 0.0F));
        const XMMATRIX projection = XMMatrixPerspectiveFovLH(
            XMConvertToRadians(50.0F), aspect, 0.1F, 100.0F);

        constexpr float floorThickness = 0.20F;
        constexpr float wallHeight = 1.00F;
        constexpr float obstacleHeight = 0.75F;

        // 床、ゴール、壁、障害物は単位立方体を拡大して描く。
        drawRect(*cube_, stage.bounds, -floorThickness * 0.5F, floorThickness,
            Colors::DarkSlateGray, view, projection);
        drawRect(*cube_, stage.goal, 0.025F, 0.05F, Colors::LimeGreen, view, projection);

        for (const Rect& wall : stage.walls) {
            drawRect(*cube_, wall, wallHeight * 0.5F, wallHeight, Colors::SlateGray, view, projection);
        }
        for (const Rect& obstacle : state.obstacles) {
            drawRect(*cube_, obstacle, obstacleHeight * 0.5F, obstacleHeight,
                Colors::Crimson, view, projection);
        }

        // CreateSphere の直径は1なので、半径の2倍で拡大する。
        const XMMATRIX playerWorld =
            XMMatrixScaling(stage.radius * 2.0F, stage.radius * 2.0F, stage.radius * 2.0F) *
            XMMatrixTranslation(state.player.x, stage.radius, state.player.z);
        sphere_->Draw(playerWorld, view, projection, Colors::CornflowerBlue);
    }

    void Renderer::present() {
        // 1 は垂直同期を待つ指定。学習用のヘッドレス実行はこの関数を呼ばない。
        throwIfFailed(swapChain_->Present(1, 0), "IDXGISwapChain::Present");
    }

}