//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE at <https://github.com/98ahni/UnityWindowing?tab=GPL-3.0-1-ov-file>)
//  <Copyright (C) 2026 98ahni> Original file author

#pragma once
#include <unordered_map>
#include "UnityAPI/IUnityInterface.h"
#include "IRenderer.h"
#include "WindowCreationProperties.h"

struct GLFWwindow;
class Window;
class WindowManager
{
public:
    static void SetRenderer(IRenderer* aRenderer);
	static IRenderer* GetRenderer() { return _renderer; }
    static void SetupNextWindow(const WindowCreationProperties& someProperties);
	static Window* CreateNewWindow(int aWidth, int aHeight, const char* aTitle);
    static void RegisterWindow(Window* aWindow);
    static void UnregisterWindow(Window* aWindow);
	static void DestroyWindow(Window* aWindow);
	static void Shutdown();

	static void StartFrame();
	static bool CheckWindow(Window* aWindow);

    static bool PlatformUsesInvertedY();

private:
	static inline IRenderer* _renderer = nullptr;
	static inline std::unordered_map<GLFWwindow*, Window*> _windowMap = {};

	static void SetCallbacks(GLFWwindow* aWindow);

	static void OnWindowMoved(GLFWwindow* aWindow, int aPosX, int aPosY);
    static void OnWindowResized(GLFWwindow* aWindow, int aSizeX, int aSizeY);
    static void OnWindowClose(GLFWwindow* aWindow);
    static void OnWindowRefresh(GLFWwindow* aWindow);
    static void OnWindowFocused(GLFWwindow* aWindow, int anIsFocused);
    static void OnKeyEvent(GLFWwindow* aWindow, int aKey, int aScancode, int anAction, int someMods);
    static void OnCursorEntered(GLFWwindow* aWindow, int anEntered);
    static void OnCursorMoved(GLFWwindow* aWindow, double aPosX, double aPosY);
    static void OnMouseButtonEvent(GLFWwindow* aWindow, int aButton, int anAction, int someMods);
    static void OnScrollEvent(GLFWwindow* aWindow, double anOffsetX, double anOffsetY);
    static void OnDragDropEvent(GLFWwindow* aWindow, int aCount, const char** somePaths);
    static void OnJoypadEvent(int aJoyID, int anEvent);
};

extern "C" UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API WindowManager_SetupNextWindow(const WindowCreationProperties& someProperties);
extern "C" UNITY_INTERFACE_EXPORT Window* UNITY_INTERFACE_API WindowManager_CreateNewWindow(int aWidth, int aHeight, const char* aTitle);
extern "C" UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API WindowManager_DestroyWindow(Window* aWindow);
extern "C" UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API WindowManager_StartFrame();
extern "C" UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API WindowManager_CheckWindow(Window* aWindow);
extern "C" UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API WindowManager_PlatformUsesInvertedY();