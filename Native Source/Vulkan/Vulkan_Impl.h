//  This file is licenced under the GNU Affero General Public License. (See file LICENSE at <https://github.com/98ahni/UnityWindowing?tab=GPL-3.0-1-ov-file>)
//  <Copyright (C) 2026 98ahni> Original file author

#pragma once
#include "IRenderer.h"

/*
	In order to use Vulcan, make sure "%(VULCAN_SDK)" is defined in the project path macros
*/

#if !defined(HAS_VK)
#if defined(__has_include)
#if __has_include(<vulkan/vulkan.h>)
#define HAS_VK 1
#else
#define HAS_VK 0
#endif
#elif !defined(HAS_VK)
#error If your compiler doesn't have the __has_include macro, define "HAS_VK" to 1 or 0 in compiler args
#define HAS_VK 0 // to stop undefined symbol errors from cluttering the output
#endif
#endif

#if HAS_VK
#include <vulkan/vulkan.h>

class Vulkan_Renderer : public IRenderer
{
public:
	Vulkan_Renderer(IUnityInterfaces* unityInterfaces);
	void GraphicsCreate(Window* aWindow) override;
	void GraphicsNewFrame(Window* aWindow) override;
	void GraphicsRender(Window* aWindow) override;
	void GraphicsDestroy(Window* aWindow) override;
	void GraphicsInvalidate(Window* aWindow) override;

	bool UsesInvertedY() override;

private:
	static constexpr int NUM_B_BUFFERS = 2;

	struct RenderData
	{
		// Our own surface/swapchain for aWindow's HWND - independent of
		// whatever swapchain Unity manages internally. We own these and
		// must destroy them in GraphicsDestroy/GraphicsInvalidate.
		VkSurfaceKHR Surface;
		VkSwapchainKHR SwapChain;
		VkFormat SwapChainFormat;
		VkExtent2D SwapChainExtent;
		VkImage SwapChainImages[NUM_B_BUFFERS];
		UINT ImageCount;

		VkCommandPool CommandPool;
		VkCommandBuffer CommandBuffer;

		VkSemaphore ImageAvailableSemaphore;
		VkSemaphore RenderFinishedSemaphore;
		VkFence InFlightFence;

		bool ShouldInvalidate;
	};
};
#endif // HAS_VK
