//  This file is licenced under the GNU Affero General Public License. (See file LICENSE at <https://github.com/98ahni/UnityWindowing?tab=GPL-3.0-1-ov-file>)
//  <Copyright (C) 2026 98ahni> Original file author

#include "pch.h"
#include "DX12_Impl.h"
#if HAS_DX12
#include <d3d12.h>
#include <dxgi1_5.h>
#include <d3dcompiler.h>
#include "UnityAPI/IUnityGraphicsD3D12.h"
#include <windows.h>
#include <string>
#include <sstream>
#include "DebugLog.h"
#include "UnityWindow.h"
#include <comdef.h>
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3dcompiler.lib")

IUnityGraphicsD3D12v8* g_UnityGraphics;
ID3D12Device* g_device;

#define DEVICE g_UnityGraphics->GetDevice()
constexpr int NUM_B_BUFFERS = 2;

// Helper to throw on failure
#define THROW_IF_FAILED(NAME, hr) \
    HRESULT NAME = hr; \
    if ((NAME) != S_OK) { \
        _com_error err(NAME);\
        Debug::ThrowException((std::stringstream() << "[D3D12][" #NAME "] (0x" << std::hex << NAME << ") " << err.ErrorMessage()).str().c_str()); \
    }

DX12_Renderer::DX12_Renderer(IUnityInterfaces* unityInterfaces)
{
    g_UnityGraphics = unityInterfaces->Get<IUnityGraphicsD3D12v8>();
}

void DX12_Renderer::GraphicsCreate(Window* aWindow)
{
    RenderData* rd = (RenderData*)aWindow->GetRenderData();
    bool firstTime = rd == nullptr;
    if (firstTime)
    {
        rd = new RenderData();
        aWindow->SetRenderData(rd);
    }

    THROW_IF_FAILED(Factory, CreateDXGIFactory2(0, IID_PPV_ARGS(&rd->Factory)));

    THROW_IF_FAILED(Allocator, DEVICE->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&rd->Allocator)));
    THROW_IF_FAILED(IntermediateAllocator, DEVICE->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&rd->IntermediateAllocator)));

    {
        D3D12_COMMAND_QUEUE_DESC desc = {};
        desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.NodeMask = 1;
        THROW_IF_FAILED(CommandQueue, DEVICE->CreateCommandQueue(&desc, IID_PPV_ARGS(&rd->CommandQueue)));
        rd->CommandQueue->SetName(L"CommandQueue");
        desc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
        THROW_IF_FAILED(IntermediateCommandQueue, DEVICE->CreateCommandQueue(&desc, IID_PPV_ARGS(&rd->IntermediateCommandQueue)));
        rd->IntermediateCommandQueue->SetName(L"IntermediateCommandQueue");
    }

    int width, height;
    aWindow->GetFrameSize(width, height);
    ID3D12Resource* texture = (ID3D12Resource*)aWindow->GetUnityTexture(); // Create the texture
    D3D12_RESOURCE_DESC textureDesc = texture->GetDesc();

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

    DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsDesc = {};
    fsDesc.Windowed = TRUE;

    HWND hwnd = (HWND)aWindow->GetWindowHandle();
    IDXGISwapChain1* swapChain = nullptr;
    THROW_IF_FAILED(SwapChain, rd->Factory->CreateSwapChainForHwnd(
        rd->CommandQueue,
        hwnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain
    ));
    THROW_IF_FAILED(SwapChain2, swapChain->QueryInterface(IID_PPV_ARGS(&rd->SwapChain)));

    for (UINT n = 0; n < NUM_B_BUFFERS; n++)
    {
        THROW_IF_FAILED(GetBuffer, rd->SwapChain->GetBuffer(n, IID_PPV_ARGS(&rd->RenderTargets[n])));
        rd->RenderTargets[n]->SetName(L"RenderTarget");
    }
    
    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

    D3D12_RESOURCE_DESC intermediateDesc = textureDesc;
    intermediateDesc.Flags = D3D12_RESOURCE_FLAG_NONE; // strip any cross-adapter flag
    intermediateDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN; // normal layout, not ROW_MAJOR

    DEVICE->CreateCommittedResource(
        &heapProps, D3D12_HEAP_FLAG_NONE, &intermediateDesc,
        D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
        IID_PPV_ARGS(&rd->IntermediateTexture));
    rd->IntermediateTexture->SetName(L"IntermediateTexture");

    THROW_IF_FAILED(CommandList, DEVICE->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, rd->Allocator, nullptr, IID_PPV_ARGS(&rd->CommandList)));
    rd->CommandList->SetName(L"CommandList");
    THROW_IF_FAILED(IntermediateCommandList, DEVICE->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, rd->IntermediateAllocator, nullptr, IID_PPV_ARGS(&rd->IntermediateCommandList)));
    rd->IntermediateCommandList->SetName(L"IntermediateCommandList");

    // Close the command list and execute it to begin the initial GPU setup.
    THROW_IF_FAILED(CommandListClose, rd->CommandList->Close());
    THROW_IF_FAILED(IntermediateCommandListClose, rd->IntermediateCommandList->Close());
    ID3D12CommandList* ppCommandLists[] = { rd->CommandList };
    rd->CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    rd->ShouldInvalidate = false;
    THROW_IF_FAILED(Fence, DEVICE->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&rd->Fence)));
    rd->Fence->SetName(L"Fence");
    rd->FenceVal = 1;
    THROW_IF_FAILED(IntermediateFence, DEVICE->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&rd->IntermediateFence)));
    rd->IntermediateFence->SetName(L"IntermediateFence");
    rd->IntermediateFenceVal = 1;

    WaitForPreviousFrame(aWindow);
}

void DX12_Renderer::GraphicsNewFrame(Window* aWindow)
{
    RenderData* rd = (RenderData*)aWindow->GetRenderData();

    rd->CurrentRTIndex = rd->SwapChain->GetCurrentBackBufferIndex();
}

void DX12_Renderer::GraphicsRender(Window* aWindow)
{
    RenderData* rd = (RenderData*)aWindow->GetRenderData();
    
    // Get current back buffer for this frame
    ID3D12Resource* backBuffer = rd->RenderTargets[rd->CurrentRTIndex];

    // Reset allocator + command list for new recording
    THROW_IF_FAILED(PreChangeAllocator, rd->Allocator->Reset());
    THROW_IF_FAILED(PreChangeCommandListReset, rd->CommandList->Reset(rd->Allocator, nullptr));
    THROW_IF_FAILED(IntermediateAllocator, rd->IntermediateAllocator->Reset());
    THROW_IF_FAILED(CopyCommandListReset, rd->IntermediateCommandList->Reset(rd->IntermediateAllocator, nullptr));

    ID3D12Resource* texture = (ID3D12Resource*)aWindow->GetUnityTexture();
    
//    ID3D12DebugCommandQueue* debugQueue = nullptr;
//    if (SUCCEEDED(rd->CommandQueue->QueryInterface(IID_PPV_ARGS(&debugQueue))))
//    {
//#define CheckState(STATE) if (debugQueue->AssertResourceState(texture, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, STATE)) { Debug::Log("IT IS IN STATE " #STATE); }
//        CheckState(D3D12_RESOURCE_STATE_COMMON)
//        CheckState(D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
//        CheckState(D3D12_RESOURCE_STATE_INDEX_BUFFER)
//        CheckState(D3D12_RESOURCE_STATE_RENDER_TARGET)
//        CheckState(D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
//        CheckState(D3D12_RESOURCE_STATE_DEPTH_WRITE)
//        CheckState(D3D12_RESOURCE_STATE_DEPTH_READ)
//        CheckState(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)
//        CheckState(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
//        CheckState(D3D12_RESOURCE_STATE_STREAM_OUT)
//        CheckState(D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT)
//        CheckState(D3D12_RESOURCE_STATE_COPY_DEST)
//        CheckState(D3D12_RESOURCE_STATE_COPY_SOURCE)
//        CheckState(D3D12_RESOURCE_STATE_RESOLVE_DEST)
//        CheckState(D3D12_RESOURCE_STATE_RESOLVE_SOURCE)
//        CheckState(D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE)
//        CheckState(D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE)
//        CheckState(D3D12_RESOURCE_STATE_RESERVED_INTERNAL_8000)
//        CheckState(D3D12_RESOURCE_STATE_RESERVED_INTERNAL_4000)
//        CheckState(D3D12_RESOURCE_STATE_RESERVED_INTERNAL_100000)
//        CheckState(D3D12_RESOURCE_STATE_RESERVED_INTERNAL_40000000)
//        CheckState(D3D12_RESOURCE_STATE_RESERVED_INTERNAL_80000000)
//        CheckState(D3D12_RESOURCE_STATE_GENERIC_READ)
//        CheckState(D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE)
//        CheckState(D3D12_RESOURCE_STATE_PRESENT)
//        CheckState(D3D12_RESOURCE_STATE_PREDICATION)
//        CheckState(D3D12_RESOURCE_STATE_VIDEO_DECODE_READ)
//        CheckState(D3D12_RESOURCE_STATE_VIDEO_DECODE_WRITE)
//        CheckState(D3D12_RESOURCE_STATE_VIDEO_PROCESS_READ)
//        CheckState(D3D12_RESOURCE_STATE_VIDEO_PROCESS_WRITE)
//        CheckState(D3D12_RESOURCE_STATE_VIDEO_ENCODE_READ)
//        CheckState(D3D12_RESOURCE_STATE_VIDEO_ENCODE_WRITE)
//#undef CheckState
//
//        debugQueue->Release();
//    }
    
    D3D12_RESOURCE_BARRIER preChangeBarriers[1] = {};
    preChangeBarriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    preChangeBarriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    preChangeBarriers[0].Transition.pResource = texture;
    preChangeBarriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    preChangeBarriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    preChangeBarriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
    
    rd->CommandList->ResourceBarrier(1, preChangeBarriers);
    
    THROW_IF_FAILED(PreChangeCommandListClose, rd->CommandList->Close());
    ID3D12CommandList* preChangecommandLists[] = { rd->CommandList };
    rd->CommandQueue->ExecuteCommandLists(1, preChangecommandLists);
    
    // Signal the direct fence, and make the COPY queue wait on it before proceeding
    rd->FenceVal++;
    THROW_IF_FAILED(PreChangeCommandQueueSignal, rd->CommandQueue->Signal(rd->Fence, rd->FenceVal));
    THROW_IF_FAILED(PreChangeIntermediateCommandQueueWait, rd->IntermediateCommandQueue->Wait(rd->Fence, rd->FenceVal)); // GPU-side wait, no CPU stall
    
    THROW_IF_FAILED(Allocator, rd->Allocator->Reset());
    THROW_IF_FAILED(CommandListReset, rd->CommandList->Reset(rd->Allocator, nullptr));

    // Copy texture -> IntermediateTexture
    D3D12_RESOURCE_BARRIER preIntermediateCopyBarriers[2] = {};
    preIntermediateCopyBarriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    preIntermediateCopyBarriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    preIntermediateCopyBarriers[0].Transition.pResource = texture;
    preIntermediateCopyBarriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    preIntermediateCopyBarriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
    preIntermediateCopyBarriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;

    preIntermediateCopyBarriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    preIntermediateCopyBarriers[1].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    preIntermediateCopyBarriers[1].Transition.pResource = rd->IntermediateTexture;
    preIntermediateCopyBarriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    preIntermediateCopyBarriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
    preIntermediateCopyBarriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
    rd->CommandList->ResourceBarrier(2, preIntermediateCopyBarriers);

    rd->IntermediateCommandList->CopyResource(rd->IntermediateTexture, texture);

    D3D12_RESOURCE_BARRIER postIntermediateCopyBarriers[2] = {};
    postIntermediateCopyBarriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    postIntermediateCopyBarriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    postIntermediateCopyBarriers[0].Transition.pResource = rd->IntermediateTexture;
    postIntermediateCopyBarriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    postIntermediateCopyBarriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    postIntermediateCopyBarriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;

    postIntermediateCopyBarriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    postIntermediateCopyBarriers[1].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    postIntermediateCopyBarriers[1].Transition.pResource = texture;
    postIntermediateCopyBarriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    postIntermediateCopyBarriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
    postIntermediateCopyBarriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
    rd->CommandList->ResourceBarrier(2, postIntermediateCopyBarriers);

    THROW_IF_FAILED(IntermediateCommandListClose, rd->IntermediateCommandList->Close());
    ID3D12CommandList* copyLists[] = { rd->IntermediateCommandList };
    rd->IntermediateCommandQueue->ExecuteCommandLists(1, copyLists);

    // Signal the copy fence, and make the DIRECT queue wait on it before proceeding
    rd->IntermediateFenceVal++;
    THROW_IF_FAILED(IntermediateCommandQueueSignal, rd->IntermediateCommandQueue->Signal(rd->IntermediateFence, rd->IntermediateFenceVal));
    THROW_IF_FAILED(CommandQueueWait, rd->CommandQueue->Wait(rd->IntermediateFence, rd->IntermediateFenceVal)); // GPU-side wait, no CPU stall

    // Transition source texture -> COPY_SOURCE, back buffer -> COPY_DEST
    D3D12_RESOURCE_BARRIER preCopyBarriers[2] = {};
    preCopyBarriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    preCopyBarriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    preCopyBarriers[0].Transition.pResource = rd->IntermediateTexture;
    preCopyBarriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    preCopyBarriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
    preCopyBarriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;

    preCopyBarriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    preCopyBarriers[1].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    preCopyBarriers[1].Transition.pResource = backBuffer;
    preCopyBarriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    preCopyBarriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    preCopyBarriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
    rd->CommandList->ResourceBarrier(2, preCopyBarriers);

    // Copy IntermediateTexture -> back buffer
    rd->CommandList->CopyResource(backBuffer, rd->IntermediateTexture);

    // Transition back buffer -> PRESENT, source texture back to its normal state
    D3D12_RESOURCE_BARRIER postCopyBarriers[3] = {};
    postCopyBarriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    postCopyBarriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    postCopyBarriers[0].Transition.pResource = backBuffer;
    postCopyBarriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    postCopyBarriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    postCopyBarriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

    postCopyBarriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    postCopyBarriers[1].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    postCopyBarriers[1].Transition.pResource = rd->IntermediateTexture;
    postCopyBarriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    postCopyBarriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
    postCopyBarriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;

    postCopyBarriers[2].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    postCopyBarriers[2].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    postCopyBarriers[2].Transition.pResource = texture;
    postCopyBarriers[2].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    postCopyBarriers[2].Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
    postCopyBarriers[2].Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    rd->CommandList->ResourceBarrier(3, postCopyBarriers);

    // Close and execute
    THROW_IF_FAILED(CommandListClose, rd->CommandList->Close());
    ID3D12CommandList* commandLists[] = { rd->CommandList };
    rd->CommandQueue->ExecuteCommandLists(1, commandLists);

    // Present
    THROW_IF_FAILED(SwapChain, rd->SwapChain->Present(1, 0));
    THROW_IF_FAILED(RemoveReason, DEVICE->GetDeviceRemovedReason());

    // Wait for this frame's GPU work to finish before the next Render() reuses the allocator
    WaitForPreviousFrame(aWindow);
}

void DX12_Renderer::GraphicsDestroy(Window* aWindow)
{
    RenderData* rd = (RenderData*)aWindow->GetRenderData();

    WaitForPreviousFrame(aWindow);
    for (UINT n = 0; n < rd->CurrentRTIndex; n++)
    {
        rd->RenderTargets[n]->Release();
    }
    rd->IntermediateTexture->Release();
    rd->IntermediateFence->Release();
    rd->CommandList->Release();
    rd->IntermediateCommandList->Release();
    rd->Allocator->Release();
    rd->IntermediateAllocator->Release();
    rd->SwapChain->Release();
    rd->CommandQueue->Release();
    rd->IntermediateCommandQueue->Release();
    rd->Factory->Release();
    rd->Fence->Release();
    rd->IntermediateFence->Release();
    delete rd;
    aWindow->SetRenderData(nullptr);
}

void DX12_Renderer::GraphicsInvalidate(Window* aWindow)
{
    RenderData* rd = (RenderData*)aWindow->GetRenderData();

    WaitForPreviousFrame(aWindow);

    for (UINT n = 0; n < NUM_B_BUFFERS; n++)
    {
        rd->RenderTargets[n]->Release();
        rd->RenderTargets[n] = nullptr;
    }
    rd->IntermediateTexture->Release();
    rd->IntermediateTexture = nullptr;

    DXGI_SWAP_CHAIN_DESC1 desc = {};
    rd->SwapChain->GetDesc1(&desc);
    int width, height;
    aWindow->GetFrameSize(width, height);
    THROW_IF_FAILED(Resize, rd->SwapChain->ResizeBuffers(
        NUM_B_BUFFERS, width, height, desc.Format, desc.Flags));

    for (UINT n = 0; n < NUM_B_BUFFERS; n++)
    {
        THROW_IF_FAILED(GetBuffer, rd->SwapChain->GetBuffer(n, IID_PPV_ARGS(&rd->RenderTargets[n])));
    }
    rd->CurrentRTIndex = 0;

    ID3D12Resource* texture = (ID3D12Resource*)aWindow->GetUnityTexture(width, height); // Creates the Unity texture

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

    D3D12_RESOURCE_DESC intermediateDesc = texture->GetDesc();
    intermediateDesc.Flags = D3D12_RESOURCE_FLAG_NONE; // strip any cross-adapter flag
    intermediateDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN; // normal layout, not ROW_MAJOR

    THROW_IF_FAILED(IntermediateTexture, DEVICE->CreateCommittedResource(
        &heapProps, D3D12_HEAP_FLAG_NONE, &intermediateDesc,
        D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
        IID_PPV_ARGS(&rd->IntermediateTexture)));
    rd->IntermediateTexture->SetName(L"IntermediateTexture");
}

bool DX12_Renderer::UsesInvertedY()
{
    return true;
}

void DX12_Renderer::WaitForPreviousFrame(Window* aWindow)
{
    RenderData* rd = (RenderData*)aWindow->GetRenderData();

    // Signal and increment the fence value.
    ID3D12Fence* fence = rd->Fence;
    rd->FenceVal++;
    THROW_IF_FAILED(CommandQueue, rd->CommandQueue->Signal(fence, rd->FenceVal));
    
    // Wait until the previous frame is finished.
    if (fence && fence->GetCompletedValue() < rd->FenceVal)
    {
        HANDLE fenceEvent = CreateEvent(nullptr, false, false, nullptr);
        THROW_IF_FAILED(OnCompletion, fence->SetEventOnCompletion(rd->FenceVal, fenceEvent));
        WaitForSingleObject(fenceEvent, INFINITE);
        CloseHandle(fenceEvent);
    }
}
#endif // HAS_DX12
