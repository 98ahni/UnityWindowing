//  This file is licenced under the GNU Affero General Public License. (See file LICENSE at <https://github.com/98ahni/UnityWindowing?tab=GPL-3.0-1-ov-file>)
//  <Copyright (C) 2026 98ahni> Original file author

using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;
using UnityEngine.Rendering;

namespace Ahni.UnityWindowing
{
    public static class WindowManager
    {
        private static Dictionary<Camera, WindowRenderer> _windowMap = new();

        internal static IntPtr CreateNativeWindow(int aWidth, int aHeight, string aTitle, WindowRenderer aWindow)
        {
            _windowMap[aWindow.Camera] = aWindow;
            return WindowManager_CreateNewWindow(aWidth, aHeight, aTitle);
        }

        [RuntimeInitializeOnLoadMethod]
        private static void Initialize()
        {
#if UNITY_HAS_SRP
            RenderPipelineManager.beginCameraRendering += OnRPCameraNewFrame;
            RenderPipelineManager.endCameraRendering += OnRPCameraRender;
#else
        Camera.onPreRender += OnCameraNewFrame;
        Camera.onPostRender += OnCameraRender;
#endif
            Application.quitting += Shutdown;
        }

        private static void Shutdown()
        {
#if UNITY_HAS_SRP
            RenderPipelineManager.beginCameraRendering -= OnRPCameraNewFrame;
            RenderPipelineManager.endCameraRendering -= OnRPCameraRender;
#else
        Camera.onPreRender -= OnCameraNewFrame;
        Camera.onPostRender -= OnCameraRender;
#endif
            Application.quitting -= Shutdown;
        }

#if UNITY_HAS_SRP
        private static void OnRPCameraNewFrame(ScriptableRenderContext context, Camera camera) => OnCameraNewFrame(camera);
#endif
        private static void OnCameraNewFrame(Camera camera)
        {
            if (_windowMap.ContainsKey(camera))
            {
                _windowMap[camera].NewFrame();
            }
        }

#if UNITY_HAS_SRP
        private static void OnRPCameraRender(ScriptableRenderContext context, Camera camera) => OnCameraRender(camera);
#endif
        private static void OnCameraRender(Camera camera)
        {
            if (_windowMap.ContainsKey(camera))
            {
                _windowMap[camera].Render();
            }
        }

        internal static void DestroyWindow(WindowRenderer aWindow)
        {
            _windowMap.Remove(aWindow.Camera);
            WindowManager_DestroyWindow(aWindow.Handle);
        }

        internal static bool CheckWindow(IntPtr aWindow) => aWindow != IntPtr.Zero && WindowManager_CheckWindow(aWindow);
        public static bool CheckWindow(WindowRenderer aWindow) => aWindow.IsWindowActive;

        internal static bool PlatformUsesInvertedY => WindowManager_PlatformUsesInvertedY();

        [DllImport("UnityWindows.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "WindowManager_SetupNextWindow")]
        internal static extern void SetupNextWindow(in WindowCreationProperties someProperties);
        [DllImport("UnityWindows.dll", CallingConvention = CallingConvention.StdCall)]
        private static extern IntPtr WindowManager_CreateNewWindow(int aWidth, int aHeight, [MarshalAs(UnmanagedType.LPStr)] string aTitle);
        [DllImport("UnityWindows.dll", CallingConvention = CallingConvention.StdCall)]
        private static extern void WindowManager_DestroyWindow(IntPtr aWindow);
        [DllImport("UnityWindows.dll", CallingConvention = CallingConvention.StdCall)]
        private static extern void WindowManager_StartFrame();
        [DllImport("UnityWindows.dll", CallingConvention = CallingConvention.StdCall)]
        private static extern bool WindowManager_CheckWindow(IntPtr aWindow);
        [DllImport("UnityWindows.dll", CallingConvention = CallingConvention.StdCall)]
        private static extern bool WindowManager_PlatformUsesInvertedY();
    }
}
