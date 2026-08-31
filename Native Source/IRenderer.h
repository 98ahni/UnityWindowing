//  This file is licenced under the GNU Affero General Public License. (See file LICENSE at <https://github.com/98ahni/UnityWindowing?tab=GPL-3.0-1-ov-file>)
//  <Copyright (C) 2026 98ahni> Original file author

#pragma once

struct IUnityInterfaces;
typedef void* UnityTexturePtr;
class Window;
class IRenderer
{
public:
	IRenderer(IUnityInterfaces* unityInterfaces) {}
	virtual void GraphicsCreate(Window* aWindow) = 0;
	virtual void GraphicsNewFrame(Window* aWindow) = 0;
	virtual void GraphicsRender(Window* aWindow) = 0;
	virtual void GraphicsDestroy(Window* aWindow) = 0;
	virtual void GraphicsInvalidate(Window* aWindow) = 0;

	virtual bool UsesInvertedY() = 0;

protected:
	IRenderer() {}
};