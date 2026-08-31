//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE at <https://github.com/98ahni/UnityWindowing?tab=GPL-3.0-1-ov-file>)
//  <Copyright (C) 2026 98ahni> Original file author

#pragma once

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <OpenGL/OpenGL.h>
#else // Linux / GLX
#include <GL/glx.h>
#endif

#if HAS_GL
struct UnityGLContext
{
    UnityGLContext();
    ~UnityGLContext();
private:
    friend struct WindowGLContext;
#if defined(_WIN32)
    HDC _unityDC;
    HGLRC _unityContext;
#elif defined(__APPLE__)
    CGLContextObj _unityContext;
#else // Linux / GLX
    Display* _unityDisplay;
    GLXDrawable _unityDrawable;
    GLXContext _unityContext;
#endif
};

struct WindowGLContext
{
    WindowGLContext(void* aNativeWindow, UnityGLContext* aUnityContext);
    void MakeCurrent();
    void SwapBuffers();
    void Reset();
    ~WindowGLContext();
private:
    UnityGLContext* _unityGL;
#if defined(_WIN32)
    HWND _windowHandle;
    HDC _windowDC;
#elif defined(__APPLE__)
    NSView* _windowView;
#else // Linux / GLX
    GLXDrawable _windowDrawable;
#endif
};
#endif // HAS_GL