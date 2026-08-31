//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE at <https://github.com/98ahni/UnityWindowing?tab=GPL-3.0-1-ov-file>)
//  <Copyright (C) 2026 98ahni> Original file author

#pragma once
#include "IRenderer.h"

#if !defined(HAS_METAL)
#if defined(__has_include)
#if __has_include(<Metal/Metal.h>)
#define HAS_METAL 1
#else
#define HAS_METAL 0
#endif
#elif defined(__APPLE__)
#define HAS_METAL 1
#else
#define HAS_METAL 0
#endif
#endif

#if HAS_METAL
class Metal_Renderer : public IRenderer
{
public:
	Metal_Renderer(IUnityInterfaces* unityInterfaces);
	void GraphicsCreate(Window* aWindow) override;
	void GraphicsNewFrame(Window* aWindow) override;
	void GraphicsRender(Window* aWindow) override;
	void GraphicsDestroy(Window* aWindow) override;
	void GraphicsInvalidate(Window* aWindow) override;

	bool UsesInvertedY() override;

private:
	struct RenderData
	{
		// Objective-C object pointers
		void* Layer;         // CAMetalLayer*
		void* CommandQueue;  // id<MTLCommandQueue>
	};
};
#endif // HAS_METAL
