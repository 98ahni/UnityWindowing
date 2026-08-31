//  This file is licenced under the GNU Affero General Public License. (See file LICENSE at <https://github.com/98ahni/UnityWindowing?tab=GPL-3.0-1-ov-file>)
//  <Copyright (C) 2026 98ahni> Original file author

#include "pch.h"
#include "OpenGL_Impl.h"
#if HAS_GL
#include "GLEW/glew.h"
#include <string>
#include <sstream>
#include "DebugLog.h"
#include "UnityWindow.h"
#include "ScopedGLContext.h"
#include <GLFW/include/GLFW/glfw3.h>
#if defined(_WIN32)
#pragma comment(lib, "opengl32.lib")
#elif defined(__APPLE__)
#error No idea how to link the *dylib files or if it's even necessary
#else
#error No idea how to link the *so files or if it's even necessary
#endif

constexpr int NUM_B_BUFFERS = 2;

// Helper to throw on failure
#define THROW(NAME) Debug::ThrowException((std::stringstream() << "[GL][" #NAME "] (0x" << std::hex << NAME << ") " << glewGetErrorString(NAME)).str().c_str());
#define THROW_IF_FAILED(NAME, expr) GLenum NAME = (expr); if (NAME != GL_NO_ERROR) { THROW(NAME) }
//#define THROW_IF_LAST_FAILED(NAME) for (GLenum NAME = glGetError(); NAME != GL_NO_ERROR; NAME = glGetError()) { THROW(NAME) } // This is supposed to work according to spec but doesn't?
#define THROW_IF_LAST_FAILED(NAME) THROW_IF_FAILED(NAME, glGetError())

OpenGL_Renderer::OpenGL_Renderer(IUnityInterfaces* unityInterfaces)
{
    _unityContext = new UnityGLContext();
    THROW_IF_FAILED(Init, glewInit());  // Plugin is loaded after Unity has initialized its graphics and on the main thread
}

void OpenGL_Renderer::GraphicsCreate(Window* aWindow)
{
    RenderData* rd = (RenderData*)aWindow->GetRenderData();
    bool firstTime = rd == nullptr;

    if (firstTime)
    {
        rd = new RenderData();
        rd->Context = new WindowGLContext(aWindow->GetWindowHandle(), _unityContext);
        aWindow->SetRenderData(rd);
    }

    int width, height;
    aWindow->GetFrameSize(width, height);
    rd->Context->MakeCurrent();
    glGenFramebuffers(1, &rd->FrameBuffer);
    aWindow->GetUnityTexture(width, height); // We just need it to be created
    rd->Context->Reset();

    THROW_IF_LAST_FAILED(CreateTexture);
}

void OpenGL_Renderer::GraphicsNewFrame(Window* aWindow)
{
    aWindow;
}

void OpenGL_Renderer::GraphicsRender(Window* aWindow)
{
    RenderData* rd = (RenderData*)aWindow->GetRenderData();

    int width, height;
    aWindow->GetFrameSize(width, height);
    rd->Context->MakeCurrent();

    GLuint texture = (GLuint)aWindow->GetUnityTexture();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, rd->FrameBuffer);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    THROW_IF_LAST_FAILED(Bind);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    THROW_IF_LAST_FAILED(Blit);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0); // Put the default frame buffer back
    THROW_IF_LAST_FAILED(Reset);

    rd->Context->SwapBuffers();
    rd->Context->Reset();

    THROW_IF_LAST_FAILED(Render);
}

void OpenGL_Renderer::GraphicsDestroy(Window* aWindow)
{
    RenderData* rd = (RenderData*)aWindow->GetRenderData();

    if (rd)
    {
        rd->Context->MakeCurrent();
        // Delete OpenGL Texture
        if (rd->FrameBuffer != 0) {
            glDeleteFramebuffers(1, &rd->FrameBuffer);
        }
        rd->Context->Reset();

        delete rd->Context;
        delete rd;
    }

    aWindow->SetRenderData(nullptr);
    THROW_IF_LAST_FAILED(Destroy);
}

void OpenGL_Renderer::GraphicsInvalidate(Window* aWindow)
{
    RenderData* rd = (RenderData*)aWindow->GetRenderData();
    int width, height;
    aWindow->GetFrameSize(width, height);

    aWindow->GetUnityTexture(width, height);

    THROW_IF_LAST_FAILED(Invalidate);
}

bool OpenGL_Renderer::UsesInvertedY()
{
    return false;
}
#endif // HAS_GL
