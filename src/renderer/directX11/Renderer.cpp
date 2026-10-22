#include "renderer/directX11/Renderer.hpp"

#include <algorithm>
#include <iomanip>
#include <iterator>
#include <sstream>
#include <stdexcept>

#include <DirectXColors.h>
#include <DirectXMath.h>
#include "core/Simulation.hpp"

using namespace std;
using namespace DirectX;

namespace {
	void throwIfFailed(HRESULT result,const char* operation)
	{
		//成功なら戻る、失敗ならその時の処理とruntime_errorを投げる
		if (FAILED(result))
		{
			ostringstream message;
			//失敗メッセージの組み立て
			message << "hex" << result << "in operation:" << operation;
			throw runtime_error(message.str());
		}
		else return;
	}
}

namespace sil::renderer {

	//コンストラクタ
	Renderer::Renderer(HWND window, uint32_t width, uint32_t height)
	{

		//デバイス・スワップチェインの作成
		createDeviceAndSwapChain(window);

		resize(width, height);
	}
	
	void Renderer::createDeviceAndSwapChain(HWND window)
	{
		//まずdescriptionを定義する
		DXGI_SWAP_CHAIN_DESC description{};
		//諸々の設定をしていく
		description.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		description.SampleDesc.Count = 1;
		description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		description.BufferCount = 1;
		description.OutputWindow = window;
		description.Windowed = true;
		description.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; //内容は基本保持しない
		UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
		//デバッグの時はフラグ追加
#ifdef _DEBUG
		flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		D3D_FEATURE_LEVEL levels[] = 
		{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0
		};

		D3D_FEATURE_LEVEL actualLevel{};
		throwIfFailed(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr
			, flags, levels, static_cast<UINT>(size(levels)), D3D11_SDK_VERSION, &description, swapChain_.GetAddressOf()
			, device_.GetAddressOf(), &actualLevel, context_.GetAddressOf()), "create_device_and_swapChain");
	}

	void Renderer::createSizeDependentResources(uint32_t width, uint32_t height)
	{
		Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
		//バックバッファ取得
		throwIfFailed(swapChain_->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())), "Get Buffer");
		//renderTargetView_に格納
		throwIfFailed(device_->CreateRenderTargetView(backBuffer.Get(), nullptr, renderTargetView_.GetAddressOf()), "Create RenderTargetView");

		//左上を(0, 0)、幅と高さを引数の値、深度範囲を[0, 1]に
		viewport_.TopLeftX = 0;
		viewport_.TopLeftY = 0;
		viewport_.Width = width;
		viewport_.Height = height;
		viewport_.MinDepth = 0;
		viewport_.MaxDepth = 1;

		//深度バッファ
		D3D11_TEXTURE2D_DESC depthDescription{};
		depthDescription.Width = width;
		depthDescription.Height = height;
		depthDescription.MipLevels = 1;
		depthDescription.ArraySize = 1;
		depthDescription.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthDescription.SampleDesc.Count = 1;
		depthDescription.Usage = D3D11_USAGE_DEFAULT;
		depthDescription.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		throwIfFailed(device_->CreateTexture2D(&depthDescription, nullptr, depthTexture_.GetAddressOf()), "Create depth texture");
		throwIfFailed(device_->CreateDepthStencilView(depthTexture_.Get(), nullptr, depthStencilView_.GetAddressOf()), "Create depth view");
	}

	//サイズの変更 = 全て再構成(コンストラクタでも呼ばれる)
	void Renderer::resize(uint32_t width, uint32_t height)
	{
		width_ = width;
		height_ = height;
		if (width == 0 || height == 0) {
			return;
		}

		if (renderTargetView_) {
			context_->OMSetRenderTargets(0, nullptr, nullptr);
			renderTargetView_.Reset();
			depthStencilView_.Reset();
			depthTexture_.Reset();
			throwIfFailed(swapChain_->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0), "ResizeBuffers");
		}
		createSizeDependentResources(width, height);
	}

	void Renderer::render(const Stage& stage, const State& state)
	{
		//とりあえず背景色だけ表示するのをやる
		//width, heightでまず判定
		if (width_ <= 0 || height_ <= 0) { return; }

		//描画先の設定
		ID3D11RenderTargetView* views[] = {
			renderTargetView_.Get()
		};
		context_->OMSetRenderTargets(1, views, depthStencilView_.Get());
		context_->ClearDepthStencilView(depthStencilView_.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0F, 0);
		context_->RSSetViewports(1, &viewport_);

		float RGBA[] = {
			1.0f, 0.0f, 0.0f, 1.0f
		};
		context_->ClearRenderTargetView(renderTargetView_.Get(), RGBA);
	}

	void Renderer::present()
	{
		throwIfFailed(swapChain_->Present(1, 0), "Present"); //垂直同期
	}
}