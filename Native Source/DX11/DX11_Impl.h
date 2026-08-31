//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE at <https://github.com/98ahni/UnityWindowing?tab=GPL-3.0-1-ov-file>)
//  <Copyright (C) 2026 98ahni> Original file author

#pragma once
#include "IRenderer.h"

#if !defined(HAS_DX11)
#if defined(__has_include)
#if __has_include(<d3d11.h>)
#define HAS_DX11 1
#else
#define HAS_DX11 0
#endif
#elif defined(_WIN32)
#define HAS_DX11 1
#else
#define HAS_DX11 0
#endif
#endif

#if HAS_DX11
struct IDXGIFactory2;
struct IDXGISwapChain1;
struct ID3D11Texture2D;
struct ID3D11DeviceContext;

class DX11_Renderer : public IRenderer
{
public:
	DX11_Renderer(IUnityInterfaces* unityInterfaces);
	void GraphicsCreate(Window* aWindow) override;
	void GraphicsNewFrame(Window* aWindow) override;
	void GraphicsRender(Window* aWindow) override;
	void GraphicsDestroy(Window* aWindow) override;
	void GraphicsInvalidate(Window* aWindow) override;

	bool UsesInvertedY() override;

private:
	struct RenderData
	{
		IDXGIFactory2* Factory;
		IDXGISwapChain1* SwapChain;
		ID3D11DeviceContext* Context;
		bool ShouldInvalidate;
	};
};
#endif // HAS_DX11
