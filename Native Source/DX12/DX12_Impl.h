//  This file is licenced under the GNU Affero General Public License. (See file LICENSE at <https://github.com/98ahni/UnityWindowing?tab=GPL-3.0-1-ov-file>)
//  <Copyright (C) 2026 98ahni> Original file author

#pragma once
#include "IRenderer.h"

#if !defined(HAS_DX12)
#if defined(__has_include)
#if __has_include(<d3d12.h>)
#define HAS_DX12 1
#else
#define HAS_DX12 0
#endif
#elif defined(_WIN32)
#define HAS_DX12 1
#else
#define HAS_DX12 0
#endif
#endif

#if HAS_DX12
struct IDXGIFactory4;
struct ID3D12CommandQueue;
struct IDXGISwapChain3;
struct ID3D12Resource;
struct ID3D12CommandAllocator;
struct ID3D12GraphicsCommandList;
struct ID3D12Fence;

class DX12_Renderer : public IRenderer
{
public:
	DX12_Renderer(IUnityInterfaces* unityInterfaces);
	void GraphicsCreate(Window* aWindow) override;
	void GraphicsNewFrame(Window* aWindow) override;
	void GraphicsRender(Window* aWindow) override;
	void GraphicsDestroy(Window* aWindow) override;
	void GraphicsInvalidate(Window* aWindow) override;

	bool UsesInvertedY() override;

private:
	void WaitForPreviousFrame(Window* aWindow);

	struct RenderData
	{
		IDXGIFactory4* Factory;
		ID3D12CommandQueue* CommandQueue;
		IDXGISwapChain3* SwapChain;
		ID3D12Resource* RenderTargets[2];
		UINT CurrentRTIndex;
		ID3D12CommandAllocator* Allocator;
		ID3D12GraphicsCommandList* CommandList;
		bool ShouldInvalidate;
		ID3D12Fence* Fence;
		UINT FenceVal;
		ID3D12CommandQueue* IntermediateCommandQueue;
		ID3D12CommandAllocator* IntermediateAllocator;
		ID3D12GraphicsCommandList* IntermediateCommandList;
		ID3D12Resource* IntermediateTexture;
		ID3D12Fence* IntermediateFence;
		UINT IntermediateFenceVal;
	};
};
#endif // HAS_DX12
