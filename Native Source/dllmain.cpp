//  This file is licenced under the GNU Affero General Public License. (See file LICENSE at <https://github.com/98ahni/UnityWindowing?tab=GPL-3.0-1-ov-file>)
//  <Copyright (C) 2026 98ahni> Original file author

#include "pch.h"
#include "UnityAPI/IUnityInterface.h"
#include "UnityAPI/IUnityGraphics.h"
#include "GLFW/include/GLFW/glfw3.h"
#include "DebugLog.h"
#include "WindowManager.h"
#include <sstream>
#include "DX11/DX11_Impl.h"
#include "DX12/DX12_Impl.h"
#include "OpenGL/OpenGL_Impl.h"
#include "Vulkan/Vulkan_Impl.h"
#include "Metal/Metal_Impl.h"

IUnityInterfaces* g_Interfaces;
void OnGraphicsDeviceEvent(UnityGfxDeviceEventType anEventType);
void SetRenderer(UnityGfxRenderer aRenderer);

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

// Entrypoint for Unity (Awake)
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces* unityInterfaces)
{
    g_Interfaces = unityInterfaces;
    IUnityGraphics* graphics = unityInterfaces->Get<IUnityGraphics>();
    UnityGfxRenderer renderer = graphics->GetRenderer();
    Debug::Init(unityInterfaces->Get<IUnityLog>());

    // Initialise GLFW
    if (!glfwInit())
    {
        // TODO: Panic
    }

    glfwSetErrorCallback([](int error, const char* desc) { Debug::Log((std::stringstream() << "[GLFW] (0x" << std::hex << error << ") " << desc).str().c_str()); });

    SetRenderer(renderer);
    graphics->RegisterDeviceEventCallback(OnGraphicsDeviceEvent);

    Debug::Log(("[Unity Windowing] Loaded! (Renderer: " + std::to_string(renderer) + ")").c_str());

    return;
}

// Exitpoint for Unity (OnDestroy)
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginUnload()
{
    Debug::LogWarning("[Unity Windowing] Shutdown!");
    WindowManager::Shutdown();
    return;
}

void UNITY_INTERFACE_API OnGraphicsDeviceEvent(UnityGfxDeviceEventType anEventType)
{
    IUnityGraphics* graphics = g_Interfaces->Get<IUnityGraphics>();
    UnityGfxRenderer renderer = graphics->GetRenderer();
    bool print = true;
    switch (anEventType)
    {
    case kUnityGfxDeviceEventAfterReset:
        Debug::Log(("[Unity Windowing] [UnityGfxDeviceEventAfterReset] Renderer: " + std::to_string(renderer)).c_str());
        print = false;
    case kUnityGfxDeviceEventInitialize:
    {
        if (print) Debug::Log(("[Unity Windowing] [UnityGfxDeviceEventInitialize] Renderer: " + std::to_string(renderer)).c_str());
        SetRenderer(renderer);
    }
        break;
    case kUnityGfxDeviceEventBeforeReset:
        Debug::Log(("[Unity Windowing] [UnityGfxDeviceEventBeforeReset] Renderer: " + std::to_string(renderer)).c_str());
        print = false;
    case kUnityGfxDeviceEventShutdown:
        if (print) Debug::Log(("[Unity Windowing] [UnityGfxDeviceEventShutdown] Renderer: " + std::to_string(renderer)).c_str());
        delete WindowManager::GetRenderer();
        break;
    }
}

void SetRenderer(UnityGfxRenderer aRenderer)
{
    // Don't let GLFW use it's own graphics
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    switch (aRenderer)
    {
#if HAS_DX11
    case UnityGfxRenderer::kUnityGfxRendererD3D11:
        WindowManager::SetRenderer(new DX11_Renderer(g_Interfaces));
        break;
#endif
#if HAS_DX12
    case UnityGfxRenderer::kUnityGfxRendererD3D12:
        WindowManager::SetRenderer(new DX12_Renderer(g_Interfaces));
        break;
#endif
#if HAS_METAL
    case UnityGfxRenderer::kUnityGfxRendererMetal:
        WindowManager::SetRenderer(new Metal_Renderer(g_Interfaces));
        break;
#endif
#if HAS_VK
    case UnityGfxRenderer::kUnityGfxRendererVulkan:
        WindowManager::SetRenderer(new Vulkan_Renderer(g_Interfaces));
        break;
#endif
#if HAS_GL
    case UnityGfxRenderer::kUnityGfxRendererOpenGLCore:
    case UnityGfxRenderer::kUnityGfxRendererOpenGLES30:
        WindowManager::SetRenderer(new OpenGL_Renderer(g_Interfaces));
        break;
#endif
    }
}
