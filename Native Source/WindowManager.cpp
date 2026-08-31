//  This file is licenced under the GNU Affero General Public License. (See file LICENSE at <https://github.com/98ahni/UnityWindowing?tab=GPL-3.0-1-ov-file>)
//  <Copyright (C) 2026 98ahni> Original file author

#include "pch.h"
#include "WindowManager.h"
#include "UnityWindow.h"
#include "DebugLog.h"
#include "GLFW/include/GLFW/glfw3.h"

void WindowManager::SetRenderer(IRenderer* aRenderer)
{
    _renderer = aRenderer;
}

void WindowManager::SetupNextWindow(const WindowCreationProperties& someProperties)
{
    glfwWindowHint(GLFW_RESIZABLE, someProperties.Resizable);
    glfwWindowHint(GLFW_DECORATED, someProperties.Decorated);
    glfwWindowHint(GLFW_FOCUSED, someProperties.Focused);
    glfwWindowHint(GLFW_FLOATING, someProperties.AlwaysOnTop);
    glfwWindowHint(GLFW_MAXIMIZED, someProperties.Maximized);
    glfwWindowHint(GLFW_POSITION_X, someProperties.PositionX);
    glfwWindowHint(GLFW_POSITION_Y, someProperties.PositionY);
}

Window* WindowManager::CreateNewWindow(int aWidth, int aHeight, const char* aTitle)
{
	if (_renderer == nullptr)
	{
		Debug::LogError("[Unity Windowing] Renderer not set!");
	}
	Window* wnd = new Window(aWidth, aHeight, aTitle);
    glfwSetJoystickCallback(OnJoypadEvent);
	return wnd;
}

void WindowManager::RegisterWindow(Window* aWindow)
{
    if (_windowMap.contains(aWindow->_window))
    {
        Debug::LogError("Window is already registered!");
    }
    _windowMap[aWindow->_window] = aWindow;
    SetCallbacks(aWindow->_window);
}

void WindowManager::UnregisterWindow(Window* aWindow)
{
    if (_windowMap.contains(aWindow->_window))
    {
        _windowMap.erase(aWindow->_window);
    }
    else
    {
        Debug::LogError("Window is not registered!");
    }
}

void WindowManager::SetCallbacks(GLFWwindow* aWindow)
{
    glfwSetWindowPosCallback(aWindow, OnWindowMoved);
    glfwSetWindowSizeCallback(aWindow, OnWindowResized);
    glfwSetWindowCloseCallback(aWindow, OnWindowClose);
    glfwSetWindowRefreshCallback(aWindow, OnWindowRefresh);
    glfwSetWindowFocusCallback(aWindow, OnWindowFocused);
    glfwSetKeyCallback(aWindow, OnKeyEvent);
    glfwSetCursorEnterCallback(aWindow, OnCursorEntered);
    glfwSetCursorPosCallback(aWindow, OnCursorMoved);
    glfwSetMouseButtonCallback(aWindow, OnMouseButtonEvent);
    glfwSetScrollCallback(aWindow, OnScrollEvent);
    glfwSetDropCallback(aWindow, OnDragDropEvent);
}

void WindowManager::DestroyWindow(Window* aWindow)
{
    aWindow->DestroyInternal();
    delete aWindow;
}

void WindowManager::Shutdown()
{
    for (const auto& [ID, wnd] : _windowMap)
    {
        wnd->DestroyInternal();
    }
    _windowMap.clear();
}

void WindowManager::StartFrame()
{
    glfwPollEvents();   // Not needed (on Windows at least) as Unity already polls each window in the process
}

bool WindowManager::CheckWindow(Window* aWindow)
{
    return _windowMap.contains(aWindow->_window);
}

bool WindowManager::PlatformUsesInvertedY()
{
    return _renderer->UsesInvertedY();
}

void WindowManager::OnWindowMoved(GLFWwindow* aWindow, int aPosX, int aPosY)
{
    if (_windowMap.contains(aWindow))
        _windowMap[aWindow]->OnMoved(aPosX, aPosY);
}

void WindowManager::OnWindowResized(GLFWwindow* aWindow, int aSizeX, int aSizeY)
{
    if (_windowMap.contains(aWindow))
        _windowMap[aWindow]->OnResized(aSizeX, aSizeY);
}

void WindowManager::OnWindowClose(GLFWwindow* aWindow)
{
    if (_windowMap.contains(aWindow))
        _windowMap[aWindow]->OnClose();
}

void WindowManager::OnWindowRefresh(GLFWwindow* aWindow)
{
    if (_windowMap.contains(aWindow))
        _windowMap[aWindow]->OnRefresh();
}

void WindowManager::OnWindowFocused(GLFWwindow* aWindow, int anIsFocused)
{
    if (_windowMap.contains(aWindow))
        _windowMap[aWindow]->OnFocused(anIsFocused != 0);
}

void WindowManager::OnKeyEvent(GLFWwindow* aWindow, int aKey, int aScancode, int anAction, int someMods)
{
    if (_windowMap.contains(aWindow))
        _windowMap[aWindow]->OnKeyEvent(aKey, aScancode, anAction, someMods);
}

void WindowManager::OnCursorEntered(GLFWwindow* aWindow, int anEntered)
{
    if (_windowMap.contains(aWindow))
        _windowMap[aWindow]->OnCursorEntered(anEntered != 0);
}

void WindowManager::OnCursorMoved(GLFWwindow* aWindow, double aPosX, double aPosY)
{
    if (_windowMap.contains(aWindow))
        _windowMap[aWindow]->OnCursorMoved(aPosX, aPosY);
}

void WindowManager::OnMouseButtonEvent(GLFWwindow* aWindow, int aButton, int anAction, int someMods)
{
    if (_windowMap.contains(aWindow))
        _windowMap[aWindow]->OnMouseButtonEvent(aButton, anAction, someMods);
}

void WindowManager::OnScrollEvent(GLFWwindow* aWindow, double anOffsetX, double anOffsetY)
{
    if (_windowMap.contains(aWindow))
        _windowMap[aWindow]->OnScrollEvent(anOffsetX, anOffsetY);
}

void WindowManager::OnDragDropEvent(GLFWwindow* aWindow, int aCount, const char** somePaths)
{
    if (_windowMap.contains(aWindow))
        _windowMap[aWindow]->OnDragDropEvent(aCount, somePaths);
}

void WindowManager::OnJoypadEvent(int aJoyID, int anEvent)
{
}

UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API WindowManager_SetupNextWindow(const WindowCreationProperties& someProperties)
{
    WindowManager::SetupNextWindow(someProperties);
}

UNITY_INTERFACE_EXPORT Window* UNITY_INTERFACE_API WindowManager_CreateNewWindow(int aWidth, int aHeight, const char* aTitle)
{
    return WindowManager::CreateNewWindow(aWidth, aHeight, aTitle);
}

UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API WindowManager_DestroyWindow(Window* aWindow)
{
    WindowManager::DestroyWindow(aWindow);
}

UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API WindowManager_StartFrame()
{
    WindowManager::StartFrame();
}

UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API WindowManager_CheckWindow(Window* aWindow)
{
    return WindowManager::CheckWindow(aWindow);
}

UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API WindowManager_PlatformUsesInvertedY()
{
    return WindowManager::PlatformUsesInvertedY();
}
