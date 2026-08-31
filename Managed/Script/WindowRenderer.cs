//  This file is licenced under the GNU Affero General Public License. (See file LICENSE at <https://github.com/98ahni/UnityWindowing?tab=GPL-3.0-1-ov-file>)
//  <Copyright (C) 2026 98ahni> Original file author

using System;
using System.Reflection;
using System.Runtime.InteropServices;
using Unity.Collections;
using Unity.Collections.LowLevel.Unsafe;
using UnityEngine;
using UnityEngine.Experimental.Rendering;

namespace Ahni.UnityWindowing
{
    [RequireComponent(typeof(Camera))]
    public class WindowRenderer : MonoBehaviour
    {
        private GCHandle _pin;
        private bool _skipCloseChecks = false;
        internal IntPtr Handle { get; private set; }
        /// <summary> The Camera rendering to this window. </summary>
        public Camera Camera { get; private set; }
        /// <summary> The render texture of the Camera. </summary>
        public RenderTexture RenderTexture { get; private set; } = null;

        private InputEmulator _inputEmulator = null;

        public delegate void OnMovedCallback(Vector2Int aPosition);
        public delegate void OnResizedCallback(Vector2Int aSize);
        public delegate bool OnCloseCallback(); // If 'false' is returned then don't close
        public delegate void OnRefreshCallback();
        public delegate void OnFocusedCallback(bool anIsFocused);
        public delegate void OnKeyEventCallback(KeyCode aKey, KeyAction anAction, int someMods);
        public delegate void OnCursorEnteredCallback(bool anEntered);
        public delegate void OnCursorMovedCallback(Vector2 aPosition);
        public delegate void OnMouseButtonEventCallback(int aButton, KeyAction anAction, int someMods);
        public delegate void OnScrollEventCallback(Vector2 anOffset);
        public delegate void OnDragDropEventCallback(string[] somePaths);

        [UnmanagedFunctionPointer(CallingConvention.StdCall)] private delegate void UnityOnMoved(int aPosX, int aPosY);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)] private delegate void UnityOnResized(int aSizeX, int aSizeY);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)] private delegate bool UnityOnClose(); // If 'false' is returned then don't close
        [UnmanagedFunctionPointer(CallingConvention.StdCall)] private delegate void UnityOnRefresh();
        [UnmanagedFunctionPointer(CallingConvention.StdCall)] private delegate void UnityOnFocused(bool anIsFocused);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)] private delegate void UnityOnKeyEvent(KeyCode aKey, KeyAction anAction, int someMods);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)] private delegate void UnityOnCursorEntered(bool anEntered);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)] private delegate void UnityOnCursorMoved(double aPosX, double aPosY);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)] private delegate void UnityOnMouseButtonEvent(int aButton, KeyAction anAction, int someMods);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)] private delegate void UnityOnScrollEvent(double anOffsetX, double anOffsetY);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate void UnityOnDragDropEvent(int aCount,
            [MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.LPStr, SizeParamIndex = 0)] string[] somePaths);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)] private delegate IntPtr UnityGetTexture(int aSizeX, int aSizeY);  // The renderer requests the texture

        /// <summary> Called whenever the window was moved.
        /// <code><see cref="void"/> <see cref="OnMovedCallback"/>(<see cref="Vector2Int"/> <see cref="aPosition"/>)</code></summary>
        public event OnMovedCallback OnMoved;
        /// <summary> Called whenever the window was resized.
        /// <code><see cref="void"/> <see cref="OnResizedCallback"/>(<see cref="Vector2Int"/> <see cref="aSize"/>)</code></summary>
        public event OnResizedCallback OnResized;
        /// <summary> Called whenever the window is about to be closed.
        /// <code><see cref="bool"/> <see cref="OnCloseCallback"/>()</code></summary>
        /// <remarks> If any callback returns false; the call to Close() would <b>not</b> destroy the object. </remarks>
        public event OnCloseCallback OnClose;
        /// <summary> Called whenever the OS feels like the window might need an update. 
        /// <code><see cref="void"/> <see cref="OnRefreshCallback"/>()</code></summary>
        public event OnRefreshCallback OnRefresh;
        /// <summary> Called whenever the window was focused or unfocused.
        /// <code><see cref="void"/> <see cref="OnFocusedCallback"/>(<see cref="bool"/> <see cref="anIsFocused"/>)</code></summary>
        public event OnFocusedCallback OnFocused;
        /// <summary> Called whenever a key was pressed, repeted or released when this window is active.
        /// <code><see cref="void"/> <see cref="OnKeyEventCallback"/>(<see cref="KeyCode"/> <see cref="aKey"/>, <see cref="KeyAction"/> <see cref="anAction"/>, <see cref="int"/> <see cref="someMods"/>)</code></summary>
        public event OnKeyEventCallback OnKeyEvent;
        /// <summary> Called whenever the mouse cursor enteres or leaves the window.
        /// <code><see cref="void"/> <see cref="OnCursorEnteredCallback"/>(<see cref="bool"/> <see cref="anEntered"/>)</code></summary>
        public event OnCursorEnteredCallback OnCursorEntered;
        /// <summary> Called whenever the mouse cursor moves within the window.
        /// <code><see cref="void"/> <see cref="OnCursorMovedCallback"/>(<see cref="Vector2"/> <see cref="aPosition"/>)</code></summary>
        public event OnCursorMovedCallback OnCursorMoved;
        /// <summary> Called whenever a button on the mouse was pressed or released when this window is active.
        /// <code><see cref="void"/> <see cref="OnMouseButtonEventCallback"/>(<see cref="int"/> <see cref="aButton"/>, <see cref="KeyAction"/> <see cref="anAction"/>, <see cref="int"/> <see cref="someMods"/>)</code></summary>
        public event OnMouseButtonEventCallback OnMouseButtonEvent;
        /// <summary> Called whenever the scroll wheel moves when the mouse cursor is within the window.
        /// <code><see cref="void"/> <see cref="OnScrollEventCallback"/>(<see cref="Vector2"/> <see cref="anOffset"/>)</code></summary>
        public event OnScrollEventCallback OnScrollEvent;
        /// <summary> Called whenever something was draged into and dropped on the window. 
        /// <code><see cref="void"/> <see cref="OnDragDropEventCallback"/>(<see cref="string"/>[] <see cref="somePaths"/>)</code></summary>
        public event OnDragDropEventCallback OnDragDropEvent;

        private UnityOnMoved _unityOnMoved;
        private UnityOnResized _unityOnResized;
        private UnityOnClose _unityOnClose;
        private UnityOnRefresh _unityOnRefresh;
        private UnityOnFocused _unityOnFocused;
        private UnityOnKeyEvent _unityOnKeyEvent;
        private UnityOnCursorEntered _unityOnCursorEntered;
        private UnityOnCursorMoved _unityOnCursorMoved;
        private UnityOnMouseButtonEvent _unityOnMouseButtonEvent;
        private UnityOnScrollEvent _unityOnScrollEvent;
        private UnityOnDragDropEvent _unityOnDragDropEvent;
        private UnityGetTexture _unityGetTexture;

        /// <summary> A struct containing various settings for the creation of new windows. </summary>
        /// <remarks><b> If these are changed after the window has first been shown, the method <see cref="ApplyProperties"/> has to be called. </b></remarks>
        public WindowCreationProperties CreationProperties = new(shouldBeResizable: true); // A value is set to not use the default constructor

        [Tooltip("The icon used for window decoration.\n<i>Not available on all platforms.</i>")]
        [SerializeField] private Texture2D _icon = null;
        /// <summary> The icon used for window decoration.<br/><i>Not available on all platforms.</i></summary>
        public Texture2D Icon
        {
            get => _icon;
            set
            {
                if (value != null && value.format != TextureFormat.RGBA32)
                {
                    throw new FormatException("Texture format for window icon must be RGBA32! (Was :" + value.format.ToString() + ")");
                }
                _icon = value;
                if (IsWindowActive)
                {
                    SetIcon();
                }
            }
        }

        [Tooltip("The title of the window.")]
        [SerializeField] private string _title = "";
        /// <summary> The title of the window. </summary>
        public string Title
        {
            get
            {
                if (IsWindowActive)
                {
                    _title = Window_GetTitle(Handle);
                }
                return _title;
            }
            set
            {
                _title = value;
                if (IsWindowActive)
                {
                    Window_SetTitle(Handle, value);
                }
            }
        }

        [Tooltip("The position in pixels of the window.")]
        [SerializeField] private Vector2Int _position = new(-1, -1);
        /// <summary> The position in pixels of the window. </summary>
        public Vector2Int Position
        {
            get
            {
                if (IsWindowActive)
                {
                    Window_GetPosition(Handle, out int x, out int y);
                    _position = new Vector2Int(x, y);
                }
                return _position;
            }
            set
            {
                _position = value;
                if (IsWindowActive)
                {
                    Window_SetPosition(Handle, value.x, value.y);
                }
            }
        }

        [Tooltip("The size in pixels of the window.")]
        [SerializeField] private Vector2Int _size = new(320, 280);
        /// <summary> The size in pixels of the window. </summary>
        public Vector2Int Size
        {
            get
            {
                if (IsWindowActive)
                {
                    Window_GetSize(Handle, out int x, out int y);
                    _size = new(x, y);
                }
                return _size;
            }
            set
            {
                _size = value;
                if (IsWindowActive)
                {
                    Window_SetSize(Handle, value.x, value.y);
                }
            }
        }

        [Tooltip("The minimum size the user can make the window.")]
        [SerializeField] private Vector2Int _minSize = new(-1, -1);
        /// <summary> The minimum size the user can make the window. </summary>
        public Vector2Int MinSize
        {
            get => _minSize;
            set
            {
                _minSize = value;
                if (IsWindowActive)
                {
                    Window_SetSizeLimits(Handle, _minSize.x, _minSize.y, _maxSize.x, _maxSize.y);
                }
            }
        }
        [Tooltip("The maximum size the user can make the window.")]
        [SerializeField] private Vector2Int _maxSize = new(-1, -1);
        /// <summary> The maximum size the user can make the window. </summary>
        public Vector2Int MaxSize
        {
            get => _maxSize;
            set
            {
                _maxSize = value;
                if (IsWindowActive)
                {
                    Window_SetSizeLimits(Handle, _minSize.x, _minSize.y, _maxSize.x, _maxSize.y);
                }
            }
        }

        /// <summary> Checks if the window can take commands. </summary>
        public bool IsWindowActive => WindowManager.CheckWindow(Handle);

        private void OnValidate()
        {
            if (IsWindowActive)
            {
                Position = _position;
                Size = _size;
            }
        }

        private void Start()
        {
            _skipCloseChecks = false;
            if (IsWindowActive) { return; }
            _pin = GCHandle.Alloc(this, GCHandleType.Pinned);
            Camera = GetComponent<Camera>();
            if (_inputEmulator == null)
            {
                _inputEmulator = new(_title, Camera);
                OnKeyEvent += _inputEmulator.RegisterKeyboardEvent;
                OnMouseButtonEvent += _inputEmulator.RegisterMouseButtonEvent;
                OnCursorMoved += _inputEmulator.RegisterMouseMoveEvent;
                OnScrollEvent += _inputEmulator.RegisterScrollEvent;
            }
            CreationProperties.InitialPosition = _position;
            WindowManager.SetupNextWindow(CreationProperties);
            Handle = WindowManager.CreateNativeWindow(_size.x, _size.y, _title, this);
            Camera.forceIntoRenderTexture = true;
            SetCallbacks();
            Window_Initialize(Handle);
            Window_GetPosition(Handle, out int x, out int y);
            _position = new Vector2Int(x, y);
            //SetIcon();
        }

        private void OnEnable()
        {
            if (!IsWindowActive)
            {
                Start();
            }
            Window_Show(Handle);
        }

        private void OnDisable()
        {
            Window_Hide(Handle);
        }

        private void OnDestroy()
        {
            if (!_skipCloseChecks)
            {
                RunCloseEvent(); // The window wasn't closed normally but the the object is being destroyed which can't be aborted so ¯\_('~')_/¯
            }
            WindowManager.DestroyWindow(this);
            Handle = IntPtr.Zero;
            _pin.Free();
        }

        /// <summary> Shorthand for activating the GameObject. </summary>
        public void Show() => gameObject.SetActive(true);
        /// <summary> Shorthand for deactivating the GameObject. </summary>
        public void Hide() => gameObject.SetActive(false);
        /// <summary> Applies properties changed during the windows lifetime. <br/><b>This will make the window flash!</b></summary>
        public void ApplyProperties()
        {
            CreationProperties.InitialPosition = _position;
            WindowManager.SetupNextWindow(CreationProperties);
            Window_Recreate(Handle, _size.x, _size.y, _title);
        }
        /// <summary> Force the window to have a certain aspect ratio. </summary>
        /// <param name="aDividend">For 16:9; this should be 16.</param>
        /// <param name="aDivisor">For 16:9; this should be 9.</param>
        public void SetAspectRatio(int aDividend, int aDivisor)
        {
            Window_SetAspectRatio(Handle, aDividend, aDivisor);
        }
        /// <summary> Minimize the window to taskbar/dock. </summary>
        public void Minimize()
        {
            Window_Minimize(Handle);
        }
        /// <summary> Restores the window from being minimized or maximized. </summary>
        public void RestoreSize()
        {
            Window_RestoreSize(Handle);
        }
        /// <summary> Make the window fill the screen it's on. Not the same as fullscreen. </summary>
        public void Maximize()
        {
            Window_Maximize(Handle);
        }
        /// <summary> Alert the user that the window wants attention.</summary>
        /// <remarks>On Windows; the taskbar icon will turn red.<br/>On macOS; the icon in dock will jump.</remarks>
        public void Alert()
        {
            Window_Alert(Handle);
        }
        /// <summary> Puts the window in the foreground and makes it active. </summary>
        public void Focus()
        {
            Window_Focus(Handle);
        }

        internal void NewFrame()
        {
            if (WindowManager.PlatformUsesInvertedY)
            {
                Camera.projectionMatrix = Camera.projectionMatrix * Matrix4x4.Scale(new Vector3(1, -1, 1)); // Only for platforms where y = 0 is bottom
            }
            GL.invertCulling = true;
            Window_NewFrame(Handle);
            //GL.IssuePluginEvent(Marshal.GetFunctionPointerForDelegate((Action<int>)(ev => Window_NewFrame(Handle))), 0x8080);
        }

        internal void Render()
        {
            Window_Render(Handle);
            //GL.IssuePluginEvent(Marshal.GetFunctionPointerForDelegate((Action<int>)(ev => Window_Render(Handle))), 0x8081);
            GL.invertCulling = false;
            Camera.ResetWorldToCameraMatrix();
            Camera.ResetProjectionMatrix();
        }

        private void SetIcon()
        {
            if (_icon != null)
            {
                NativeArray<Color32> data = _icon.GetPixelData<Color32>(0);
                Window_SetIcon(Handle, _icon.width, _icon.height, UnsafeUtility.As<NativeArray<Color32>, IntPtr>(ref data)); // Super ultra unsafe!
            }
            else
            {
                Window_SetIcon(Handle, 0, 0, IntPtr.Zero);
            }
        }

        private void SetCallbacks()
        {
            _unityOnMoved = Moved;
            _unityOnResized = Resized;
            _unityOnClose = Close;
            _unityOnRefresh = Refresh;
            _unityOnFocused = Focused;
            _unityOnKeyEvent = KeyEvent;
            _unityOnCursorEntered = CursorEntered;
            _unityOnCursorMoved = CursorMoved;
            _unityOnMouseButtonEvent = MouseButtonEvent;
            _unityOnScrollEvent = ScrollEvent;
            _unityOnDragDropEvent = DragDropEvent;
            _unityGetTexture = GetTexture;

            Window_SetCallbacks(
                Handle,
                Marshal.GetFunctionPointerForDelegate(_unityOnMoved),
                Marshal.GetFunctionPointerForDelegate(_unityOnResized),
                Marshal.GetFunctionPointerForDelegate(_unityOnClose),
                Marshal.GetFunctionPointerForDelegate(_unityOnRefresh),
                Marshal.GetFunctionPointerForDelegate(_unityOnFocused),
                Marshal.GetFunctionPointerForDelegate(_unityOnKeyEvent),
                Marshal.GetFunctionPointerForDelegate(_unityOnCursorEntered),
                Marshal.GetFunctionPointerForDelegate(_unityOnCursorMoved),
                Marshal.GetFunctionPointerForDelegate(_unityOnMouseButtonEvent),
                Marshal.GetFunctionPointerForDelegate(_unityOnScrollEvent),
                Marshal.GetFunctionPointerForDelegate(_unityOnDragDropEvent),
                Marshal.GetFunctionPointerForDelegate(_unityGetTexture)
            );
        }
        private void Moved(int aPosX, int aPosY)
        {
            _position = new(aPosX, aPosY);
            OnMoved?.Invoke(_position);
        }
        private void Resized(int aSizeX, int aSizeY)
        {
            _size = new(aSizeX, aSizeY);
            OnResized?.Invoke(_size);
        }
        /// <summary> Destroys the GameObject if the window should close. </summary>
        /// <returns> If 'false' is returned then don't close. </returns>
        public bool Close()
        {
            bool shouldClose = RunCloseEvent();
            if (shouldClose)
            {
                _skipCloseChecks = true;
                Destroy(gameObject);
            }
            return shouldClose;
        }
        private bool RunCloseEvent()
        {
            bool shouldClose = true;
            if (OnClose != null && OnClose.GetInvocationList().Length != 0)
            {
                foreach (OnCloseCallback closeEv in OnClose.GetInvocationList())
                {
                    if (!closeEv()) { shouldClose = false; }
                }
            }
            return shouldClose;
        }
        private void Refresh()
        {
            OnRefresh?.Invoke();
        }
        private void Focused(bool anIsFocused)
        {
            OnFocused?.Invoke(anIsFocused);
        }
        private void KeyEvent(KeyCode aKey, KeyAction anAction, int someMods)
        {
            OnKeyEvent?.Invoke(aKey, anAction, someMods);
        }
        private void CursorEntered(bool anEntered)
        {
            OnCursorEntered?.Invoke(anEntered);
        }
        private void CursorMoved(double aPosX, double aPosY)
        {
            OnCursorMoved?.Invoke(new((float)aPosX, _size.y - (float)aPosY));
        }
        private void MouseButtonEvent(int aButton, KeyAction anAction, int someMods)
        {
            OnMouseButtonEvent?.Invoke(aButton, anAction, someMods);
        }
        private void ScrollEvent(double anOffsetX, double anOffsetY)
        {
            OnScrollEvent?.Invoke(new((float)anOffsetX, (float)anOffsetY));
        }
        private void DragDropEvent(int aCount, string[] somePaths)
        {
            OnDragDropEvent?.Invoke(somePaths);
        }
        private IntPtr GetTexture(int aSizeX, int aSizeY)
        {
            if (!RenderTexture || RenderTexture.width != aSizeX || RenderTexture.height != aSizeY)
            {
                if (RenderTexture)
                {
                    RenderTexture.Release();
                    RenderTexture = null;
                }
                RenderTextureDescriptor desc = new (aSizeX, aSizeY, GraphicsFormat.R8G8B8A8_SRGB, 32, 1);
                SetOrClearRenderTextureCreationFlag(ref desc, true, RenderTextureCreationFlags.AllowVerticalFlip);
                RenderTexture = new(desc);
                //RenderTexture = new(aSizeX, aSizeY, 32, GraphicsFormat.R8G8B8A8_SRGB, 1);
                Camera.targetTexture = RenderTexture;
            }
            return RenderTexture.GetNativeTexturePtr();
        }

        private static readonly MethodInfo RenderTextureDescriptor_SetOrClearRenderTextureCreationFlag
        = typeof(RenderTextureDescriptor).GetMethod("SetOrClearRenderTextureCreationFlag", BindingFlags.Instance | BindingFlags.NonPublic);
        private static void SetOrClearRenderTextureCreationFlag(ref RenderTextureDescriptor aDesc, bool aValue, RenderTextureCreationFlags aFlag)
        {
            object desc = aDesc;
            RenderTextureDescriptor_SetOrClearRenderTextureCreationFlag.Invoke(desc, new object[] { aValue, aFlag });
            aDesc = (RenderTextureDescriptor)desc;
        }

        [DllImport("UnityWindows.dll", CallingConvention = CallingConvention.StdCall)]
        private static extern void Window_SetCallbacks(
            IntPtr aWindow,
            [MarshalAs(UnmanagedType.FunctionPtr)] IntPtr aUnityOnMoved,
            [MarshalAs(UnmanagedType.FunctionPtr)] IntPtr aUnityOnResized,
            [MarshalAs(UnmanagedType.FunctionPtr)] IntPtr aUnityOnClose,
            [MarshalAs(UnmanagedType.FunctionPtr)] IntPtr aUnityOnRefresh,
            [MarshalAs(UnmanagedType.FunctionPtr)] IntPtr aUnityOnFocused,
            [MarshalAs(UnmanagedType.FunctionPtr)] IntPtr aUnityOnKeyEvent,
            [MarshalAs(UnmanagedType.FunctionPtr)] IntPtr aUnityOnCursorEntered,
            [MarshalAs(UnmanagedType.FunctionPtr)] IntPtr aUnityOnCursorMoved,
            [MarshalAs(UnmanagedType.FunctionPtr)] IntPtr aUnityOnMouseButtonEvent,
            [MarshalAs(UnmanagedType.FunctionPtr)] IntPtr aUnityOnScrollEvent,
            [MarshalAs(UnmanagedType.FunctionPtr)] IntPtr aUnityOnDragDropEvent,
            [MarshalAs(UnmanagedType.FunctionPtr)] IntPtr aUnityOnInvalidate
        );
        [DllImport("UnityWindows.dll", CallingConvention = CallingConvention.StdCall)]
        private static extern void Window_Initialize(IntPtr aWindow);
        [DllImport("UnityWindows.dll", CallingConvention = CallingConvention.StdCall)]
        private static extern void Window_Recreate(IntPtr aWindow, int aSizeX, int aSizeY, [MarshalAs(UnmanagedType.LPStr)] string aTitle);
        [DllImport("UnityWindows.dll", CallingConvention = CallingConvention.StdCall)]
        private static extern void Window_NewFrame(IntPtr aWindow);
        [DllImport("UnityWindows.dll", CallingConvention = CallingConvention.StdCall)]
        private static extern void Window_Render(IntPtr aWindow);
        [DllImport("UnityWindows.dll", CallingConvention = CallingConvention.StdCall)]
        private static extern void Window_GetPosition(IntPtr aWindow, out int aPosX, out int aPosY);
        [DllImport("UnityWindows.dll", CallingConvention = CallingConvention.StdCall)]
        private static extern void Window_GetSize(IntPtr aWindow, out int aSizeX, out int aSizeY);
        [DllImport("UnityWindows.dll", CallingConvention = CallingConvention.StdCall)]
        private static extern void Window_GetFrameSize(IntPtr aWindow, out int aSizeX, out int aSizeY);
        [DllImport("UnityWindows.dll", CallingConvention = CallingConvention.StdCall)]
        private static extern void Window_SetPosition(IntPtr aWindow, int aPosX, int aPosY);
        [DllImport("UnityWindows.dll", CallingConvention = CallingConvention.StdCall)]
        private static extern void Window_SetSize(IntPtr aWindow, int aSizeX, int aSizeY);
        [DllImport("UnityWindows.dll", CallingConvention = CallingConvention.StdCall)]
        [return: MarshalAs(UnmanagedType.LPStr)]
        private static extern string Window_GetTitle(IntPtr aWindow);
        [DllImport("UnityWindows.dll", CallingConvention = CallingConvention.StdCall)]
        private static extern void Window_SetTitle(IntPtr aWindow, [MarshalAs(UnmanagedType.LPStr)] string aTitle);
        [DllImport("UnityWindows.dll", CallingConvention = CallingConvention.StdCall)]
        private static extern void Window_SetSizeLimits(IntPtr aWindow, int aMinWidth, int aMinHeight, int aMaxWidth, int aMaxHeight);
        [DllImport("UnityWindows.dll", CallingConvention = CallingConvention.StdCall)]
        //private static extern void Window_SetIcon(IntPtr aWindow, int aWidth, int aHeight, IntPtr someData);
        private static extern void Window_SetIcon(IntPtr aWindow, int aWidth, int aHeight, IntPtr someData);
        [DllImport("UnityWindows.dll", CallingConvention = CallingConvention.StdCall)]
        private static extern void Window_SetAspectRatio(IntPtr aWindow, int aDividend, int aDivisor);
        [DllImport("UnityWindows.dll", CallingConvention = CallingConvention.StdCall)]
        private static extern void Window_Minimize(IntPtr aWindow);
        [DllImport("UnityWindows.dll", CallingConvention = CallingConvention.StdCall)]
        private static extern void Window_RestoreSize(IntPtr aWindow);
        [DllImport("UnityWindows.dll", CallingConvention = CallingConvention.StdCall)]
        private static extern void Window_Maximize(IntPtr aWindow);
        [DllImport("UnityWindows.dll", CallingConvention = CallingConvention.StdCall)]
        private static extern void Window_Alert(IntPtr aWindow);
        [DllImport("UnityWindows.dll", CallingConvention = CallingConvention.StdCall)]
        private static extern void Window_Show(IntPtr aWindow);
        [DllImport("UnityWindows.dll", CallingConvention = CallingConvention.StdCall)]
        private static extern void Window_Focus(IntPtr aWindow);
        [DllImport("UnityWindows.dll", CallingConvention = CallingConvention.StdCall)]
        private static extern void Window_Hide(IntPtr aWindow);
    }
}
