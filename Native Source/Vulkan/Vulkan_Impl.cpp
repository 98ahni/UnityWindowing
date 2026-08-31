//  This file is licenced under the GNU Affero General Public License. (See file LICENSE at <https://github.com/98ahni/UnityWindowing?tab=GPL-3.0-1-ov-file>)
//  <Copyright (C) 2026 98ahni> Original file author

#include "pch.h"
#include "Vulkan_Impl.h"
#if HAS_VK
#include <vulkan/vulkan.h>
#include "UnityAPI/IUnityRenderingExtensions.h"
#include "UnityAPI/IUnityGraphicsVulkan.h"
#include <string>
#include <sstream>
#include <algorithm>
#include <vector>
#include "DebugLog.h"
#include "UnityWindow.h"
#include <GLFW/include/GLFW/glfw3.h>
#if defined(_WIN32)
#pragma comment(lib, "vulkan-1.lib")
#elif defined(__APPLE__)
#error No idea how to link the *dylib files or if it's even necessary
#else
#error No idea how to link the *so files or if it's even necessary
#endif

// NOTE: IUnityGraphicsVulkan.h's exact struct/enum names have varied across
// Unity versions. This file assumes the commonly-documented shape:
//   - UnityVulkanInstance Instance() -> { instance, physicalDevice, device,
//     graphicsQueue, queueFamilyIndex, getInstanceProcAddr, getDeviceProcAddr }
//   - UnityVulkanImage AccessTexture(nativeTexturePtr, subResource, layout,
//     pipelineStageFlags, accessFlags, accessMode, outImage) -> VkImage +
//     current layout for a Unity-owned texture.
// If your copy of the header differs, the mismatches should be isolated to
// the two blocks marked "UNITY VULKAN INTEROP" below.

IUnityGraphicsVulkan* g_UnityVulkan;
UnityVulkanInstance g_VulkanInstance;

#define INSTANCE g_VulkanInstance.instance
#define PHYSICAL_DEVICE g_VulkanInstance.physicalDevice
#define DEVICE g_VulkanInstance.device
#define GRAPHICS_QUEUE g_VulkanInstance.graphicsQueue
#define QUEUE_FAMILY_INDEX g_VulkanInstance.queueFamilyIndex

// Helper to throw on failure
#define VK_CHECK(NAME, expr) \
    VkResult NAME = (expr); \
    if ((NAME) != VK_SUCCESS) { \
        Debug::ThrowException((std::stringstream() << "[Vulkan][" #NAME "] result = " << (int)NAME).str().c_str()); \
    }

Vulkan_Renderer::Vulkan_Renderer(IUnityInterfaces* unityInterfaces)
{
    // UNITY VULKAN INTEROP: obtaining the shared instance/device/queue.
    g_UnityVulkan = unityInterfaces->Get<IUnityGraphicsVulkan>();
    g_VulkanInstance = g_UnityVulkan->Instance();
}

static VkSurfaceFormatKHR PickSurfaceFormat(Window* aWindow, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data());

    void* nativeTexture = aWindow->GetUnityTexture(); // Create the texture
    //UnityVulkanImage unityImage = {};                 // Unity always sets the typeless version of its format
    //g_UnityVulkan->AccessTexture(nativeTexture, UnityVulkanWholeImage,
    //    VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
    //    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT,
    //    kUnityVulkanResourceAccess_PipelineBarrier, &unityImage);

    for (const auto& f : formats)
    {
        // Must match the format the Unity source texture was created with
        // for vkCmdCopyImage to succeed.
        //if (f.format == unityImage.format)
        if (f.format == VK_FORMAT_R8G8B8A8_UNORM)
            return f;
    }
    return formats[0];
}

void Vulkan_Renderer::GraphicsCreate(Window* aWindow)
{
    RenderData* rd = (RenderData*)aWindow->GetRenderData();
    bool firstTime = rd == nullptr;
    if (firstTime)
    {
        rd = new RenderData();
        aWindow->SetRenderData(rd);
    }

    GLFWwindow* wnd = aWindow->GetGLFWHandle();
    VK_CHECK(CreateSurface, glfwCreateWindowSurface(INSTANCE, wnd, nullptr, &rd->Surface));

    VkSurfaceCapabilitiesKHR caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PHYSICAL_DEVICE, rd->Surface, &caps);

    int width, height;
    aWindow->GetFrameSize(width, height);

    VkSurfaceFormatKHR surfaceFormat = PickSurfaceFormat(aWindow, PHYSICAL_DEVICE, rd->Surface); // Also creates the texture
    rd->SwapChainFormat = surfaceFormat.format;
    rd->SwapChainExtent = { (uint32_t)width, (uint32_t)height };

    VkSwapchainCreateInfoKHR swapChainInfo = {};
    swapChainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainInfo.surface = rd->Surface;
    swapChainInfo.minImageCount = NUM_B_BUFFERS;
    swapChainInfo.imageFormat = surfaceFormat.format;
    swapChainInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapChainInfo.imageExtent = rd->SwapChainExtent;
    swapChainInfo.imageArrayLayers = 1;
    swapChainInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT; // we only ever CopyImage into it, never render to it directly
    swapChainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapChainInfo.preTransform = caps.currentTransform;
    swapChainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapChainInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR; // vsync'd, matches the DXGI Present(1, 0) used in the D3D versions
    swapChainInfo.clipped = VK_TRUE;
    swapChainInfo.oldSwapchain = VK_NULL_HANDLE;
    VK_CHECK(CreateSwapChain, vkCreateSwapchainKHR(DEVICE, &swapChainInfo, nullptr, &rd->SwapChain));

    uint32_t imageCount = 0;
    vkGetSwapchainImagesKHR(DEVICE, rd->SwapChain, &imageCount, nullptr);
    rd->ImageCount = std::min<UINT>(imageCount, NUM_B_BUFFERS);
    vkGetSwapchainImagesKHR(DEVICE, rd->SwapChain, &imageCount, rd->SwapChainImages);

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = QUEUE_FAMILY_INDEX;
    VK_CHECK(CreateCommandPool, vkCreateCommandPool(DEVICE, &poolInfo, nullptr, &rd->CommandPool));

    VkCommandBufferAllocateInfo cmdAllocInfo = {};
    cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdAllocInfo.commandPool = rd->CommandPool;
    cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdAllocInfo.commandBufferCount = 1;
    VK_CHECK(AllocateCommandBuffer, vkAllocateCommandBuffers(DEVICE, &cmdAllocInfo, &rd->CommandBuffer));

    VkSemaphoreCreateInfo semInfo = {};
    semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VK_CHECK(CreateImageAvailableSem, vkCreateSemaphore(DEVICE, &semInfo, nullptr, &rd->ImageAvailableSemaphore));
    VK_CHECK(CreateRenderFinishedSem, vkCreateSemaphore(DEVICE, &semInfo, nullptr, &rd->RenderFinishedSemaphore));

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // start signaled so the first GraphicsRender doesn't stall on vkWaitForFences
    VK_CHECK(CreateFence, vkCreateFence(DEVICE, &fenceInfo, nullptr, &rd->InFlightFence));

    rd->ShouldInvalidate = false;
}

void Vulkan_Renderer::GraphicsNewFrame(Window* aWindow)
{
    // Nothing to do up front - the swapchain image index is only known
    // after vkAcquireNextImageKHR, which happens in GraphicsRender.
}

void Vulkan_Renderer::GraphicsRender(Window* aWindow)
{
    RenderData* rd = (RenderData*)aWindow->GetRenderData();

    // Wait for the previous frame using this fence/command buffer to finish
    // before reusing them - the Vulkan equivalent of WaitForPreviousFrame
    // in the D3D12 version.
    vkWaitForFences(DEVICE, 1, &rd->InFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(DEVICE, 1, &rd->InFlightFence);

    uint32_t imageIndex = 0;
    VK_CHECK(AcquireImage, vkAcquireNextImageKHR(DEVICE, rd->SwapChain, UINT64_MAX,
        rd->ImageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex));
    VkImage backBuffer = rd->SwapChainImages[imageIndex];

    // UNITY VULKAN INTEROP: get the underlying VkImage for the Unity
    // texture and have Unity transition it to TRANSFER_SRC_OPTIMAL before
    // we touch it. kUnityVulkanResourceAccess_PipelineBarrier tells Unity
    // to record whatever barrier is needed on its own timeline so the
    // image is safe to read by the time this call returns.
    void* nativeTexture = aWindow->GetUnityTexture();
    UnityVulkanImage unityImage = {};
    g_UnityVulkan->AccessTexture(nativeTexture, UnityVulkanWholeImage,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT,
        kUnityVulkanResourceAccess_PipelineBarrier, &unityImage);

    vkResetCommandBuffer(rd->CommandBuffer, 0);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(rd->CommandBuffer, &beginInfo);

    // Transition the swap chain image UNDEFINED -> TRANSFER_DST_OPTIMAL
    VkImageMemoryBarrier toDst = {};
    toDst.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    toDst.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    toDst.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    toDst.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toDst.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toDst.image = backBuffer;
    toDst.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    toDst.srcAccessMask = 0;
    toDst.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vkCmdPipelineBarrier(rd->CommandBuffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        0, 0, nullptr, 0, nullptr, 1, &toDst);

    // Copy Unity texture -> back buffer
    VkImageCopy region = {};
    region.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
    region.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
    region.extent = { rd->SwapChainExtent.width, rd->SwapChainExtent.height, 1 };
    vkCmdCopyImage(rd->CommandBuffer,
        unityImage.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        backBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1, &region);

    // Transition the swap chain image TRANSFER_DST_OPTIMAL -> PRESENT_SRC_KHR
    VkImageMemoryBarrier toPresent = {};
    toPresent.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    toPresent.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    toPresent.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    toPresent.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toPresent.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toPresent.image = backBuffer;
    toPresent.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    toPresent.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    toPresent.dstAccessMask = 0;
    vkCmdPipelineBarrier(rd->CommandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        0, 0, nullptr, 0, nullptr, 1, &toPresent);

    vkEndCommandBuffer(rd->CommandBuffer);

    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &rd->ImageAvailableSemaphore;
    submitInfo.pWaitDstStageMask = &waitStage;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &rd->CommandBuffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &rd->RenderFinishedSemaphore;
    VK_CHECK(QueueSubmit, vkQueueSubmit(GRAPHICS_QUEUE, 1, &submitInfo, rd->InFlightFence));

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &rd->RenderFinishedSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &rd->SwapChain;
    presentInfo.pImageIndices = &imageIndex;
    VK_CHECK(Present, vkQueuePresentKHR(GRAPHICS_QUEUE, &presentInfo));
}

void Vulkan_Renderer::GraphicsDestroy(Window* aWindow)
{
    RenderData* rd = (RenderData*)aWindow->GetRenderData();

    vkDeviceWaitIdle(DEVICE);

    vkDestroyFence(DEVICE, rd->InFlightFence, nullptr);
    vkDestroySemaphore(DEVICE, rd->RenderFinishedSemaphore, nullptr);
    vkDestroySemaphore(DEVICE, rd->ImageAvailableSemaphore, nullptr);
    vkDestroyCommandPool(DEVICE, rd->CommandPool, nullptr); // also frees rd->CommandBuffer
    vkDestroySwapchainKHR(DEVICE, rd->SwapChain, nullptr);
    vkDestroySurfaceKHR(INSTANCE, rd->Surface, nullptr);

    delete rd;
    aWindow->SetRenderData(nullptr);
}

void Vulkan_Renderer::GraphicsInvalidate(Window* aWindow)
{
    RenderData* rd = (RenderData*)aWindow->GetRenderData();

    vkDeviceWaitIdle(DEVICE);

    int width, height;
    aWindow->GetFrameSize(width, height);

    VkSurfaceCapabilitiesKHR caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PHYSICAL_DEVICE, rd->Surface, &caps);

    VkSwapchainKHR oldSwapChain = rd->SwapChain;
    rd->SwapChainExtent = { (uint32_t)width, (uint32_t)height };

    VkSwapchainCreateInfoKHR swapChainInfo = {};
    swapChainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainInfo.surface = rd->Surface;
    swapChainInfo.minImageCount = NUM_B_BUFFERS;
    swapChainInfo.imageFormat = rd->SwapChainFormat;
    swapChainInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    swapChainInfo.imageExtent = rd->SwapChainExtent;
    swapChainInfo.imageArrayLayers = 1;
    swapChainInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    swapChainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapChainInfo.preTransform = caps.currentTransform;
    swapChainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapChainInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    swapChainInfo.clipped = VK_TRUE;
    swapChainInfo.oldSwapchain = oldSwapChain; // lets the driver reuse resources during resize
    VK_CHECK(RecreateSwapChain, vkCreateSwapchainKHR(DEVICE, &swapChainInfo, nullptr, &rd->SwapChain));

    vkDestroySwapchainKHR(DEVICE, oldSwapChain, nullptr);

    uint32_t imageCount = 0;
    vkGetSwapchainImagesKHR(DEVICE, rd->SwapChain, &imageCount, nullptr);
    rd->ImageCount = std::min<UINT>(imageCount, NUM_B_BUFFERS);
    vkGetSwapchainImagesKHR(DEVICE, rd->SwapChain, &imageCount, rd->SwapChainImages);

    aWindow->GetUnityTexture(width, height); // Creates/recreates the Unity texture at the new size
}

bool Vulkan_Renderer::UsesInvertedY()
{
    // Vulkan, like D3D11/D3D12, has NDC Y pointing down and expects
    // textures with the origin at the top-left - same convention as the
    // D3D versions, unlike OpenGL.
    return true;
}
#endif // HAS_VK
