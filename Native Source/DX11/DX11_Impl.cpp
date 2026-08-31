//  This file is licenced under the GNU Affero General Public License. (See file LICENSE at <https://github.com/98ahni/UnityWindowing?tab=GPL-3.0-1-ov-file>)
//  <Copyright (C) 2026 98ahni> Original file author

#include "pch.h"
#include "DX11_Impl.h"
#if HAS_DX11
#include <d3d11.h>
#include <dxgi1_5.h>
#include "UnityAPI/IUnityGraphicsD3D11.h"
#include <windows.h>
#include <string>
#include <sstream>
#include "DebugLog.h"
#include "UnityWindow.h"
#include <comdef.h>
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

IUnityGraphicsD3D11* g_UnityGraphics;

#define DEVICE g_UnityGraphics->GetDevice()
constexpr int NUM_B_BUFFERS = 2;

// Helper to throw on failure
#define THROW_IF_FAILED(NAME, hr) \
    HRESULT NAME = hr; \
    if ((NAME) != S_OK) { \
        _com_error err(NAME);\
        Debug::ThrowException((std::stringstream() << "[D3D11][" #NAME "] (0x" << std::hex << NAME << ") " << err.ErrorMessage()).str().c_str()); \
    }

DX11_Renderer::DX11_Renderer(IUnityInterfaces* unityInterfaces)
{
    g_UnityGraphics = unityInterfaces->Get<IUnityGraphicsD3D11>();
}

void DX11_Renderer::GraphicsCreate(Window* aWindow)
{
    RenderData* rd = (RenderData*)aWindow->GetRenderData();
    bool firstTime = rd == nullptr;
    if (firstTime)
    {
        rd = new RenderData();
        aWindow->SetRenderData(rd);
    }

    THROW_IF_FAILED(Factory, CreateDXGIFactory2(0, IID_PPV_ARGS(&rd->Factory)));

    int width, height;
    aWindow->GetFrameSize(width, height);

    ID3D11Texture2D* texture = (ID3D11Texture2D*)aWindow->GetUnityTexture(); // Create the texture
    D3D11_TEXTURE2D_DESC textureDesc;
    texture->GetDesc(&textureDesc);

    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
    swapChainDesc.BufferCount = NUM_B_BUFFERS;
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    //swapChainDesc.Format = textureDesc.Format;    // Unity always sets the typeless version of its format
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapChainDesc.Scaling = DXGI_SCALING_NONE;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

    HWND hwnd = (HWND)aWindow->GetWindowHandle();
    THROW_IF_FAILED(SwapChain, rd->Factory->CreateSwapChainForHwnd(
        DEVICE,
        hwnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &rd->SwapChain
    ));

    DEVICE->GetImmediateContext(&rd->Context);

    rd->ShouldInvalidate = false;
}

void DX11_Renderer::GraphicsNewFrame(Window* aWindow)
{}

void DX11_Renderer::GraphicsRender(Window* aWindow)
{
    RenderData* rd = (RenderData*)aWindow->GetRenderData();

    ID3D11Texture2D* backBuffer = nullptr;
    THROW_IF_FAILED(GetBuffer, rd->SwapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)));

    ID3D11Texture2D* texture = (ID3D11Texture2D*)aWindow->GetUnityTexture();

    rd->Context->CopyResource(backBuffer, texture);

    backBuffer->Release();

    THROW_IF_FAILED(SwapChain, rd->SwapChain->Present(1, 0));

    ID3D11Device* device = DEVICE;
    THROW_IF_FAILED(RemoveReason, device->GetDeviceRemovedReason());
}

void DX11_Renderer::GraphicsDestroy(Window* aWindow)
{
    RenderData* rd = (RenderData*)aWindow->GetRenderData();

    rd->Context->Release();
    rd->SwapChain->Release();
    rd->Factory->Release();
    delete rd;
    aWindow->SetRenderData(nullptr);
}

void DX11_Renderer::GraphicsInvalidate(Window* aWindow)
{
    RenderData* rd = (RenderData*)aWindow->GetRenderData();

    rd->Context->Release();

    DXGI_SWAP_CHAIN_DESC1 desc = {};
    rd->SwapChain->GetDesc1(&desc);
    int width, height;
    aWindow->GetFrameSize(width, height);
    THROW_IF_FAILED(Resize, rd->SwapChain->ResizeBuffers(
        NUM_B_BUFFERS, width, height, desc.Format, desc.Flags));

    DEVICE->GetImmediateContext(&rd->Context);

    aWindow->GetUnityTexture(width, height); // Creates/recreates the Unity texture at the new size
}

bool DX11_Renderer::UsesInvertedY()
{
    return true;
}
#endif // HAS_DX11
