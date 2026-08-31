//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE at <https://github.com/98ahni/UnityWindowing?tab=GPL-3.0-1-ov-file>)
//  <Copyright (C) 2026 98ahni> Original file author

#include "OpenGL_Impl.h" // For the HAS_GL macro
#include "ScopedGLContext.h"
#include <string>
#include <sstream>
#include "DebugLog.h"

#if HAS_GL
#if defined(_WIN32)
UnityGLContext::UnityGLContext() : _unityDC(wglGetCurrentDC()), _unityContext(wglGetCurrentContext())
{
    UNITY_ASSERT("[GL] Init was not done on main thread!" && (int)_unityContext);
}
UnityGLContext::~UnityGLContext()
{
}

WindowGLContext::WindowGLContext(void* aNativeWindow, UnityGLContext* aUnityContext) : _unityGL(aUnityContext)
{
    _windowHandle = (HWND)aNativeWindow;
    _windowDC = GetDC(_windowHandle);
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    int pixelFormat = ChoosePixelFormat(_windowDC, &pfd);
    SetPixelFormat(_windowDC, pixelFormat, &pfd);
}
void WindowGLContext::MakeCurrent()
{
    wglMakeCurrent(_windowDC, _unityGL->_unityContext);
}
void WindowGLContext::SwapBuffers()
{
    ::SwapBuffers(_windowDC);
}
void WindowGLContext::Reset()
{
    wglMakeCurrent(_unityGL->_unityDC, _unityGL->_unityContext);
}
WindowGLContext::~WindowGLContext()
{
    ReleaseDC(_windowHandle, _windowDC);
}
#elif defined(__APPLE__)
UnityGLContext::UnityGLContext() : _unityContext([NSOpenGLContext currentContext])
{
    UNITY_ASSERT("[GL] Init was not done on main thread!" && (int)_unityContext);
}
UnityGLContext::~UnityGLContext()
{
}

WindowGLContext::WindowGLContext(void* aNativeWindow, UnityGLContext* aUnityContext) : _unityGL(aUnityContext)
{
    _windowView = (NSView*)aNativeWindow;
}
void WindowGLContext::MakeCurrent()
{
    [_unityGL->_unityContext setView : _windowView];
    [_unityGL->_unityContext makeCurrentContext];
}
void WindowGLContext::SwapBuffers()
{
    // Handled in Reset()
}
void WindowGLContext::Reset()
{
    [_unityGL->_unityContext flushBuffer];
    [_unityGL->_unityContext makeCurrentContext];
}
WindowGLContext::~WindowGLContext()
{
    // Nothing to return
}
#else // Linux / GLX
UnityGLContext::UnityGLContext()
    : _unityDisplay(glXGetCurrentDisplay()), _unityDrawable(glXGetCurrentDrawable()), _unityContext(glXGetCurrentContext())
{
    UNITY_ASSERT("[GL] Init was not done on main thread!" && (int)_unityContext);
}
UnityGLContext::~UnityGLContext()
{
}

WindowGLContext::WindowGLContext(void* aNativeWindow, UnityGLContext* aUnityContext) : _unityGL(aUnityContext)
{
    _windowDrawable = (GLXDrawable)aNativeWindow;
}
void WindowGLContext::MakeCurrent()
{
    glXMakeCurrent(_unityGL->_unityDisplay, _windowDrawable, _unityGL->_unityContext);
}
void WindowGLContext::SwapBuffers()
{
    glXSwapBuffers(_unityGL->_unityDisplay, _windowDrawable);
}
void WindowGLContext::Reset()
{
    glXMakeCurrent(_unityGL->_unityDisplay, _unityGL->_unityDrawable, _unityGL->_unityContext);
}
WindowGLContext::~WindowGLContext()
{
    // Nothing to return
}
#endif // defined(_WIN32)
#endif // HAS_GL
