//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE at <https://github.com/98ahni/UnityWindowing?tab=GPL-3.0-1-ov-file>)
//  <Copyright (C) 2026 98ahni> Original file author

#include "pch.h"
#include "Metal_Impl.h"
#if HAS_METAL
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <AppKit/AppKit.h>
#include "UnityAPI/IUnityGraphicsMetal.h"
#include <sstream>
#include "DebugLog.h"
#include "UnityWindow.h"

IUnityGraphicsMetal* g_UnityMetal;

Metal_Renderer::Metal_Renderer(IUnityInterfaces* unityInterfaces)
{
    g_UnityMetal = unityInterfaces->Get<IUnityGraphicsMetal>();
}

void Metal_Renderer::GraphicsCreate(Window* aWindow)
{
    RenderData* rd = (RenderData*)aWindow->GetRenderData();
    bool firstTime = rd == nullptr;
    if (firstTime)
    {
        rd = new RenderData();
        aWindow->SetRenderData(rd);
    }

    id<MTLDevice> device = g_UnityMetal->MetalDevice();

    NSView* view = (__bridge NSView*)aWindow->GetWindowHandle();
    id<MTLTexture> unityTexture = (__bridge id<MTLTexture>)aWindow->GetUnityTexture(); // Create the texture

    CAMetalLayer* layer = [CAMetalLayer layer];
    layer.device = device;
    //layer.pixelFormat = unityTexture.pixelFormat;     // Unity always sets the typeless version of its format
    layer.pixelFormat = MTLPixelFormatRGBA8Unorm; // must match the Unity texture's pixel format for the blit below to succeed
    layer.framebufferOnly = NO;                    // NO: we blit into the drawable's texture rather than rendering to it directly
    layer.displaySyncEnabled = YES;                 // vsync'd, matches the DXGI Present(1,0) / VK_PRESENT_MODE_FIFO_KHR used elsewhere

    int width, height;
    aWindow->GetFrameSize(width, height);
    layer.drawableSize = CGSizeMake(width, height);

    view.wantsLayer = YES;
    view.layer = layer;

    rd->Layer = (void*)CFBridgingRetain(layer);

    id<MTLCommandQueue> queue = [device newCommandQueue];
    rd->CommandQueue = (void*)CFBridgingRetain(queue);
}

void Metal_Renderer::GraphicsNewFrame(Window* aWindow)
{
}

void Metal_Renderer::GraphicsRender(Window* aWindow)
{
    RenderData* rd = (RenderData*)aWindow->GetRenderData();
    CAMetalLayer* layer = (__bridge CAMetalLayer*)rd->Layer;
    id<MTLCommandQueue> queue = (__bridge id<MTLCommandQueue>)rd->CommandQueue;

    id<CAMetalDrawable> drawable = [layer nextDrawable];
    if (drawable == nil)
    {
        // No drawable available right now (e.g. window occluded or
        // minimized) - skip this frame rather than block.
        return;
    }

    id<MTLTexture> unityTexture = (__bridge id<MTLTexture>)aWindow->GetUnityTexture();

    id<MTLCommandBuffer> commandBuffer = [queue commandBuffer];
    id<MTLBlitCommandEncoder> blit = [commandBuffer blitCommandEncoder];
    [blit copyFromTexture:unityTexture
              sourceSlice:0
              sourceLevel:0
             sourceOrigin:MTLOriginMake(0, 0, 0)
               sourceSize:MTLSizeMake(unityTexture.width, unityTexture.height, 1)
                toTexture:drawable.texture
         destinationSlice:0
         destinationLevel:0
        destinationOrigin:MTLOriginMake(0, 0, 0)];
    [blit endEncoding];

    [commandBuffer presentDrawable:drawable];
    [commandBuffer commit];
}

void Metal_Renderer::GraphicsDestroy(Window* aWindow)
{
    RenderData* rd = (RenderData*)aWindow->GetRenderData();

    CFRelease(rd->CommandQueue);
    CFRelease(rd->Layer);

    delete rd;
    aWindow->SetRenderData(nullptr);
}

void Metal_Renderer::GraphicsInvalidate(Window* aWindow)
{
    RenderData* rd = (RenderData*)aWindow->GetRenderData();
    CAMetalLayer* layer = (__bridge CAMetalLayer*)rd->Layer;

    int width, height;
    aWindow->GetFrameSize(width, height);
    layer.drawableSize = CGSizeMake(width, height);

    aWindow->GetUnityTexture(width, height); // Creates/recreates the Unity texture at the new size
}

bool Metal_Renderer::UsesInvertedY()
{
    return true;
}
#endif // HAS_METAL
