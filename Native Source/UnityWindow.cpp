//  This file is licenced under the GNU Affero General Public License. (See file LICENSE at <https://github.com/98ahni/UnityWindowing?tab=GPL-3.0-1-ov-file>)
//  <Copyright (C) 2026 98ahni> Original file author

#include "pch.h"
#include "UnityWindow.h"
#include <functional>
#include <sstream>
#include "GLFW/include/GLFW/glfw3.h"
#include "IRenderer.h"
#include "WindowManager.h"
#include "DebugLog.h"

#if defined(__has_include)
#if __has_include(<X11/Xlib.h>) || __has_include(<X11/extensions/Xrandr.h>)
#define CAN_USE_X11
#endif
#if __has_include(<wayland-client.h>)
#define CAN_USE_WAYLAND
#endif
#endif

#if CAN_USE_X11 && (defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__))
#define GLFW_HAS_X11        1
#else
#define GLFW_HAS_X11        0
#endif
#if CAN_USE_WAYLAND && (defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__))
#define GLFW_HAS_WAYLAND    1
#else
#define GLFW_HAS_WAYLAND    0
#endif
#ifdef _WIN32
#undef APIENTRY
#ifndef GLFW_EXPOSE_NATIVE_WIN32    // for glfwGetWin32Window()
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include "GLFW/include/GLFW/glfw3native.h"
#elif defined(__APPLE__)
#ifndef GLFW_EXPOSE_NATIVE_COCOA    // for glfwGetCocoaWindow()
#define GLFW_EXPOSE_NATIVE_COCOA
#endif
#include "GLFW/include/GLFW/glfw3native.h"
#elif GLFW_HAS_WAYLAND
#ifndef GLFW_EXPOSE_NATIVE_WAYLAND    // for glfwGetWaylandWindow()
#define GLFW_EXPOSE_NATIVE_WAYLAND      // OBS! No idea if more includes are necessary
#endif
#include "GLFW/include/GLFW/glfw3native.h"
#elif GLFW_HAS_X11
#ifndef GLFW_EXPOSE_NATIVE_X11      // for glfwGetX11Display(), glfwGetX11Window() on Freedesktop (Linux, BSD, etc.)
#define GLFW_EXPOSE_NATIVE_X11
#include "X11/Xatom.h"
#include "dlfcn.h"              // for dlopen()
#endif
#include "GLFW/include/GLFW/glfw3native.h"
#undef Status                   // X11 headers are leaking this.
#endif

Window::Window(int aWidth, int aHeight, const char* aTitle)
{
    _window = glfwCreateWindow(aWidth, aHeight, aTitle, NULL, NULL);
    WindowManager::RegisterWindow(this);
}

Window::~Window()
{
}

void* Window::GetWindowHandle()
{
#ifdef GLFW_EXPOSE_NATIVE_WIN32
    return glfwGetWin32Window(_window);
#elif GLFW_EXPOSE_NATIVE_COCOA
    return (void*)glfwGetCocoaWindow(_window);
#elif GLFW_EXPOSE_NATIVE_WAYLAND
    return (void*)glfwGetX11Window(_window);
#elif GLFW_EXPOSE_NATIVE_X11
    return (void*)glfwGetWaylandWindow(_window);
#else
    return nullptr
#endif
}

GLFWwindow* Window::GetGLFWHandle()
{
    return _window;
}

void* Window::GetRenderData()
{
    return _renderData;
}

void Window::SetRenderData(void* someRenderData)
{
    _renderData = someRenderData;
}

void Window::GetPosition(int& aPosX, int& aPosY)
{
    glfwGetWindowPos(_window, &aPosX, &aPosY);
}

void Window::GetSize(int& aSizeX, int& aSizeY)
{
    glfwGetWindowSize(_window, &aSizeX, &aSizeY);
}

void Window::GetFrameSize(int& aSizeX, int& aSizeY)
{
    glfwGetFramebufferSize(_window, &aSizeX, &aSizeY);
}

void Window::SetPosition(int aPosX, int aPosY)
{
    glfwSetWindowPos(_window, aPosX, aPosY);
}

void Window::SetSize(int aSizeX, int aSizeY)
{
    glfwSetWindowSize(_window, aSizeX, aSizeY);
}

const char* Window::GetTitle()
{
    return glfwGetWindowTitle(_window);
}

void Window::SetTitle(const char* aTitle)
{
    glfwSetWindowTitle(_window, aTitle);
}

void Window::SetSizeLimits(int aMinWidth, int aMinHeight, int aMaxWidth, int aMaxHeight)
{
    glfwSetWindowSizeLimits(_window, aMinWidth, aMinHeight, aMaxWidth, aMaxHeight);
}

void Window::SetIcon(int aWidth, int aHeight, void* someData)
{
    if (someData == nullptr)
    {
        glfwSetWindowIcon(_window, 0, nullptr);
        return;
    }
    GLFWimage img { aWidth, aHeight, (unsigned char*)someData };
    glfwSetWindowIcon(_window, 1, &img);
}

void Window::SetAspectRatio(int aDividend, int aDivisor)
{
    glfwSetWindowAspectRatio(_window, aDividend, aDivisor);
}

void Window::Minimize()
{
    glfwIconifyWindow(_window);
}

void Window::RestoreSize()
{
    glfwRestoreWindow(_window);
}

void Window::Maximize()
{
    glfwMaximizeWindow(_window);
}

void Window::Alert()
{
    glfwRequestWindowAttention(_window);
}

void Window::Show()
{
    glfwShowWindow(_window);
}

void Window::Focus()
{
    glfwFocusWindow(_window);
}

void Window::Hide()
{
    glfwHideWindow(_window);
}

void Window::Create()
{
    WindowManager::GetRenderer()->GraphicsCreate(this);
}

void Window::Recreate(int aWidth, int aHeight, const char* aTitle)
{
    DestroyInternal();
    _window = glfwCreateWindow(aWidth, aHeight, aTitle, NULL, NULL);
    WindowManager::RegisterWindow(this);
    Create();
}

void Window::NewFrame()
{
    if (glfwWindowShouldClose(_window))
    {
        return;
    }
    WindowManager::GetRenderer()->GraphicsNewFrame(this);
}

void Window::Render()
{
    WindowManager::GetRenderer()->GraphicsRender(this);
}

void Window::Destroy()
{
    WindowManager::DestroyWindow(this);
}

UnityTexturePtr Window::GetUnityTexture(int aSizeX, int aSizeY)
{
    return _unityGetTexture(aSizeX, aSizeY);
}

UnityTexturePtr Window::GetUnityTexture()
{
    int x, y;
    GetFrameSize(x, y);
    return _unityGetTexture(x, y);
}

void Window::DestroyInternal()
{
    WindowManager::GetRenderer()->GraphicsDestroy(this);
    delete _renderData;
    WindowManager::UnregisterWindow(this);
    glfwDestroyWindow(_window);
}

void Window::OnMoved(int aPosX, int aPosY)
{
    _unityOnMoved(aPosX, aPosY);
}

void Window::OnResized(int anX, int aY)
{
    _unityOnResized(anX, aY);
    WindowManager::GetRenderer()->GraphicsInvalidate(this);
}

void Window::OnClose()
{
    if (!_unityOnClose())
    {
        glfwSetWindowShouldClose(_window, 0);
        return;
    }
    //Destroy();    // Unity Handles the actual closing in the _unityOnClose event
}

void Window::OnRefresh()
{
    _unityOnRefresh();
}

void Window::OnFocused(bool anIsFocused)
{
    _unityOnFocused(anIsFocused);
}

void Window::OnKeyEvent(int aKey, int aScancode, int anAction, int someMods)
{
    UnityKeyCode keyCode = GLFW2UnityKeyCode(aKey);
    if (keyCode == UnityKeyCode::None)
    {
        keyCode = NativeScan2UnityKeyCode(aScancode);
    }
    _unityOnKeyEvent(keyCode, (UnityKeyAction)anAction, someMods);
}

void Window::OnCursorEntered(bool anEntered)
{
    _unityOnCursorEntered(anEntered);
}

void Window::OnCursorMoved(double aPosX, double aPosY)
{
    _unityOnCursorMoved(aPosX, aPosY);
}

void Window::OnMouseButtonEvent(int aButton, int anAction, int someMods)
{
    _unityOnMouseButtonEvent(aButton, anAction, someMods);
}

void Window::OnScrollEvent(double anOffsetX, double anOffsetY)
{
    _unityOnScrollEvent(anOffsetX, anOffsetY);
}

void Window::OnDragDropEvent(int aCount, const char** somePaths)
{
    _unityOnDragDropEvent(aCount, somePaths);
}

void Window_SetCallbacksInternal(
    Window* aWindow,
    UnityOnMoved aUnityOnMoved,
    UnityOnResized aUnityOnResized,
    UnityOnClose aUnityOnClose,
    UnityOnRefresh aUnityOnRefresh,
    UnityOnFocused aUnityOnFocused,
    UnityOnKeyEvent aUnityOnKeyEvent,
    UnityOnCursorEntered aUnityOnCursorEntered,
    UnityOnCursorMoved aUnityOnCursorMoved,
    UnityOnMouseButtonEvent aUnityOnMouseButtonEvent,
    UnityOnScrollEvent aUnityOnScrollEvent,
    UnityOnDragDropEvent aUnityOnDragDropEvent,
    UnityGetTexture aUnityOnInvalidate
)
{
    aWindow->_unityOnMoved = aUnityOnMoved;
    aWindow->_unityOnResized = aUnityOnResized;
    aWindow->_unityOnClose = aUnityOnClose;
    aWindow->_unityOnRefresh = aUnityOnRefresh;
    aWindow->_unityOnFocused = aUnityOnFocused;
    aWindow->_unityOnKeyEvent = aUnityOnKeyEvent;
    aWindow->_unityOnCursorEntered = aUnityOnCursorEntered;
    aWindow->_unityOnCursorMoved = aUnityOnCursorMoved;
    aWindow->_unityOnMouseButtonEvent = aUnityOnMouseButtonEvent;
    aWindow->_unityOnScrollEvent = aUnityOnScrollEvent;
    aWindow->_unityOnDragDropEvent = aUnityOnDragDropEvent;
    aWindow->_unityGetTexture = aUnityOnInvalidate;
}

extern "C" UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Window_SetCallbacks(
    Window* aWindow,
    UnityOnMoved aUnityOnMoved,
    UnityOnResized aUnityOnResized,
    UnityOnClose aUnityOnClose,
    UnityOnRefresh aUnityOnRefresh,
    UnityOnFocused aUnityOnFocused,
    UnityOnKeyEvent aUnityOnKeyEvent,
    UnityOnCursorEntered aUnityOnCursorEntered,
    UnityOnCursorMoved aUnityOnCursorMoved,
    UnityOnMouseButtonEvent aUnityOnMouseButtonEvent,
    UnityOnScrollEvent aUnityOnScrollEvent,
    UnityOnDragDropEvent aUnityOnDragDropEvent,
    UnityGetTexture aUnityOnInvalidate
)
{
    Window_SetCallbacksInternal(aWindow,
        aUnityOnMoved,
        aUnityOnResized,
        aUnityOnClose,
        aUnityOnRefresh,
        aUnityOnFocused,
        aUnityOnKeyEvent,
        aUnityOnCursorEntered,
        aUnityOnCursorMoved,
        aUnityOnMouseButtonEvent,
        aUnityOnScrollEvent,
        aUnityOnDragDropEvent,
        aUnityOnInvalidate
    );
}

UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Window_Initialize(Window* aWindow)
{
    aWindow->Create();
}

UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Window_Recreate(Window* aWindow, int aWidth, int aHeight, const char* aTitle)
{
    aWindow->Recreate(aWidth, aHeight, aTitle);
}

UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Window_NewFrame(Window* aWindow)
{
    aWindow->NewFrame();
}

UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Window_Render(Window* aWindow)
{
    aWindow->Render();
}

UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Window_GetPosition(Window* aWindow, int& aPosX, int& aPosY)
{
    aWindow->GetPosition(aPosX, aPosY);
}

UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Window_GetSize(Window* aWindow, int& aSizeX, int& aSizeY)
{
    aWindow->GetSize(aSizeX, aSizeY);
}

UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Window_GetFrameSize(Window* aWindow, int& aSizeX, int& aSizeY)
{
    aWindow->GetFrameSize(aSizeX, aSizeY);
}

UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Window_SetPosition(Window* aWindow, int aPosX, int aPosY)
{
    aWindow->SetPosition(aPosX, aPosY);
}

UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Window_SetSize(Window* aWindow, int aSizeX, int aSizeY)
{
    aWindow->SetSize(aSizeX, aSizeY);
}

UNITY_INTERFACE_EXPORT const char* UNITY_INTERFACE_API Window_GetTitle(Window* aWindow)
{
    return aWindow->GetTitle();
}

UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Window_SetTitle(Window* aWindow, const char* aTitle)
{
    aWindow->SetTitle(aTitle);
}

UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Window_SetSizeLimits(Window* aWindow, int aMinWidth, int aMinHeight, int aMaxWidth, int aMaxHeight)
{
    aWindow->SetSizeLimits(aMinWidth, aMinHeight, aMaxWidth, aMaxHeight);
}

UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Window_SetIcon(Window* aWindow, int aWidth, int aHeight, void* someData)
{
    aWindow->SetIcon(aWidth, aHeight, someData);
}

UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Window_SetAspectRatio(Window* aWindow, int aDividend, int aDivisor)
{
    aWindow->SetAspectRatio(aDividend, aDivisor);
}

UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Window_Minimize(Window* aWindow)
{
    aWindow->Minimize();
}

UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Window_RestoreSize(Window* aWindow)
{
    aWindow->RestoreSize();
}

UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Window_Maximize(Window* aWindow)
{
    aWindow->Maximize();
}

UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Window_Alert(Window* aWindow)
{
    aWindow->Alert();
}

UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Window_Show(Window* aWindow)
{
    aWindow->Show();
}

UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Window_Focus(Window* aWindow)
{
    aWindow->Focus();
}

UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Window_Hide(Window* aWindow)
{
    aWindow->Hide();
}
