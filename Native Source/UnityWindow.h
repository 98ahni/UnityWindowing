//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE at <https://github.com/98ahni/UnityWindowing?tab=GPL-3.0-1-ov-file>)
//  <Copyright (C) 2026 98ahni> Original file author

#pragma once
#include "UnityAPI/IUnityInterface.h"
#include "IRenderer.h"
#include "UnityEventTranslation.h"

typedef void (UNITY_INTERFACE_API* UnityOnMoved)(int aPosX, int aPosY);
typedef void (UNITY_INTERFACE_API* UnityOnResized)(int aSizeX, int aSizeY);
typedef bool (UNITY_INTERFACE_API* UnityOnClose)(); // If 'false' is returned then don't close
typedef void (UNITY_INTERFACE_API* UnityOnRefresh)();
typedef void (UNITY_INTERFACE_API* UnityOnFocused)(bool anIsFocused);
typedef void (UNITY_INTERFACE_API* UnityOnKeyEvent)(UnityKeyCode aKey, UnityKeyAction anAction, int someMods);
typedef void (UNITY_INTERFACE_API* UnityOnCursorEntered)(bool anEntered);
typedef void (UNITY_INTERFACE_API* UnityOnCursorMoved)(double aPosX, double aPosY);
typedef void (UNITY_INTERFACE_API* UnityOnMouseButtonEvent)(int aButton, int anAction, int someMods);
typedef void (UNITY_INTERFACE_API* UnityOnScrollEvent)(double anOffsetX, double anOffsetY);
typedef void (UNITY_INTERFACE_API* UnityOnDragDropEvent)(int aCount, const char** somePaths);
typedef UnityTexturePtr (UNITY_INTERFACE_API* UnityGetTexture)(int aSizeX, int aSizeY);	// The renderer requests the texture

struct GLFWwindow;
class Window
{
public:
	Window(int aWidth, int aHeight, const char* aTitle);
	~Window();

	void* GetWindowHandle();
	GLFWwindow* GetGLFWHandle();
	void* GetRenderData();
	void SetRenderData(void* someRenderData);
	void GetPosition(int& aPosX, int& aPosY);
	void GetSize(int& aSizeX, int& aSizeY);
	void GetFrameSize(int& aSizeX, int& aSizeY);
	void SetPosition(int aPosX, int aPosY);
	void SetSize(int aSizeX ,int aSizeY);
	const char* GetTitle();
	void SetTitle(const char* aTitle);
	void SetSizeLimits(int aMinWidth, int aMinHeight, int aMaxWidth, int aMaxHeight);
	void SetIcon(int aWidth, int aHeight, void* someData);
	void SetAspectRatio(int aDividend, int aDivisor);
	void Minimize();
	void RestoreSize();
	void Maximize();
	void Alert();

	void Show();
	void Focus();
	void Hide();

	void Create();
	void Recreate(int aWidth, int aHeight, const char* aTitle);
	void NewFrame();
	void Render();
	void Destroy();
	UnityTexturePtr GetUnityTexture(int aSizeX, int aSizeY);
	UnityTexturePtr GetUnityTexture();

private:
	friend class WindowManager;
	GLFWwindow* _window;
	void* _renderData = nullptr;

	void DestroyInternal();

	void OnMoved(int aPosX, int aPosY);
	void OnResized(int aSizeX, int aSizeY);
	void OnClose();
	void OnRefresh();
	void OnFocused(bool anIsFocused);
	void OnKeyEvent(int aKey, int aScancode, int anAction, int someMods);
	void OnCursorEntered(bool anEntered);
	void OnCursorMoved(double aPosX, double aPosY);
	void OnMouseButtonEvent(int aButton, int anAction, int someMods);
	void OnScrollEvent(double anOffsetX, double anOffsetY);
	void OnDragDropEvent(int aCount, const char** somePaths);

	friend void Window_SetCallbacksInternal(
		Window*,
		UnityOnMoved,
		UnityOnResized,
		UnityOnClose,
		UnityOnRefresh,
		UnityOnFocused,
		UnityOnKeyEvent,
		UnityOnCursorEntered,
		UnityOnCursorMoved,
		UnityOnMouseButtonEvent,
		UnityOnScrollEvent,
		UnityOnDragDropEvent,
		UnityGetTexture
	);
	UnityOnMoved _unityOnMoved;
	UnityOnResized _unityOnResized;
	UnityOnClose _unityOnClose;
	UnityOnRefresh _unityOnRefresh;
	UnityOnFocused _unityOnFocused;
	UnityOnKeyEvent _unityOnKeyEvent;
	UnityOnCursorEntered _unityOnCursorEntered;
	UnityOnCursorMoved _unityOnCursorMoved;
	UnityOnMouseButtonEvent _unityOnMouseButtonEvent;
	UnityOnScrollEvent _unityOnScrollEvent;
	UnityOnDragDropEvent _unityOnDragDropEvent;
	UnityGetTexture _unityGetTexture;
};

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
	UnityGetTexture aUnitygetTexture
);
extern "C" UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Window_Initialize(Window* aWindow);
extern "C" UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Window_Recreate(Window* aWindow, int aWidth, int aHeight, const char* aTitle);
extern "C" UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Window_NewFrame(Window* aWindow);
extern "C" UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Window_Render(Window* aWindow);
extern "C" UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Window_GetPosition(Window* aWindow, int& aPosX, int& aPosY);
extern "C" UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Window_GetSize(Window* aWindow, int& aSizeX, int& aSizeY);
extern "C" UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Window_GetFrameSize(Window* aWindow, int& aSizeX, int& aSizeY);
extern "C" UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Window_SetPosition(Window* aWindow, int aPosX, int aPosY);
extern "C" UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Window_SetSize(Window* aWindow, int aSizeX, int aSizeY);
extern "C" UNITY_INTERFACE_EXPORT const char* UNITY_INTERFACE_API Window_GetTitle(Window* aWindow);
extern "C" UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Window_SetTitle(Window* aWindow, const char* aTitle);
extern "C" UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Window_SetSizeLimits(Window* aWindow, int aMinWidth, int aMinHeight, int aMaxWidth, int aMaxHeight);
extern "C" UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Window_SetIcon(Window* aWindow, int aWidth, int aHeight, void* someData);
extern "C" UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Window_SetAspectRatio(Window* aWindow, int aDividend, int aDivisor);
extern "C" UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Window_Minimize(Window* aWindow);
extern "C" UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Window_RestoreSize(Window* aWindow);
extern "C" UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Window_Maximize(Window* aWindow);
extern "C" UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Window_Alert(Window* aWindow);
extern "C" UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Window_Show(Window* aWindow);
extern "C" UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Window_Focus(Window* aWindow);
extern "C" UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Window_Hide(Window* aWindow);
