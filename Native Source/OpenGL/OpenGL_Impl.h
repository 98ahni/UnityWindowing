//  This file is licenced under the GNU Affero General Public License. (See file LICENSE at <https://github.com/98ahni/UnityWindowing?tab=GPL-3.0-1-ov-file>)
//  <Copyright (C) 2026 98ahni> Original file author

#pragma once
#include "IRenderer.h"

#if !defined(HAS_GL)
#if defined(__has_include)
#if __has_include(<gl/GL.h>) || __has_include(<OpenGL/OpenGL.h>) || __has_include(<GL/glx.h>)
#define HAS_GL 1
#else
#define HAS_GL 0
#endif
#elif !defined(HAS_GL)
#error If your compiler doesn't have the __has_include macro, define "HAS_GL" to 1 or 0 in compiler args
#define HAS_GL 0 // to stop undefined symbol errors from cluttering the output
#endif
#endif

#if HAS_GL
typedef unsigned int GLuint;
struct UnityGLContext;
struct WindowGLContext;
class OpenGL_Renderer : public IRenderer
{
public:
	OpenGL_Renderer(IUnityInterfaces* unityInterfaces);
	void GraphicsCreate(Window* aWindow) override;
	void GraphicsNewFrame(Window* aWindow) override;
	void GraphicsRender(Window* aWindow) override;
	void GraphicsDestroy(Window* aWindow) override;
	void GraphicsInvalidate(Window* aWindow) override;

	bool UsesInvertedY() override;

private:
	struct RenderData
	{
		GLuint FrameBuffer;
		WindowGLContext* Context = nullptr;
	};
	UnityGLContext* _unityContext = nullptr;
};
#endif // HAS_GL
