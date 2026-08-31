//  This file is licenced under the GNU Affero General Public License. (See file LICENSE at <https://github.com/98ahni/UnityWindowing?tab=GPL-3.0-1-ov-file>)
//  <Copyright (C) 2026 98ahni> Original file author

using System.Runtime.InteropServices;
using UnityEngine;

namespace Ahni.UnityWindowing
{
    /// <summary> Settings for creation of new windows. </summary>
    [System.Serializable]
    [StructLayout(LayoutKind.Explicit)]
    public struct WindowCreationProperties
    {
        // The [FieldOffset(X)] corresponds with the alignas(Y) in c++. To get X: add every Y above in the struct incl. the current field and subtract one
        /// <summary> Can the window be resized after creation?
        /// <br/><b>Default:</b> true </summary>
        [FieldOffset(0)] public bool Resizable;
        /// <summary> Whether to show the titlebar and borders or not.
        /// <br/><b>Default:</b> true </summary>
        [FieldOffset(1)] public bool Decorated;
        /// <summary> Should the window be focused on creation?
        /// <br/><b>Default:</b> true </summary>
        [FieldOffset(2)] public bool Focused;
        /// <summary> Is the window supposed to always be in front of all other?
        /// <br/><b>Default:</b> false </summary>
        [FieldOffset(3)] public bool AlwaysOnTop;
        /// <summary> Does the window start out maximized?
        /// <br/><b>Default:</b> false </summary>
        [FieldOffset(4)] public bool Maximized;
        /// <summary> What's the initial position of the window?
        /// <br/><b>Default:</b> Any position (-1, -1) </summary>
        [FieldOffset(8)] internal Vector2Int InitialPosition;

        /// <summary> Settings for creation of new windows. </summary>
        /// <param name="shouldBeResizable">Can the window be resized after creation?</param>
        /// <param name="shouldBeDecorated">Whether to show the titlebar and borders or not.</param>
        /// <param name="shouldBeFocused">Should the window be focused on creation?</param>
        /// <param name="shouldBeAlwaysOnTop">Is the window supposed to always be in front of all other?</param>
        /// <param name="shouldBeMaximized">Does the window start out maximized?</param>
        public WindowCreationProperties(
            bool shouldBeResizable = true,
            bool shouldBeDecorated = true,
            bool shouldBeFocused = true,
            bool shouldBeAlwaysOnTop = false,
            bool shouldBeMaximized = false
        )
        {
            Resizable = shouldBeResizable;
            Decorated = shouldBeDecorated;
            Focused = shouldBeFocused;
            AlwaysOnTop = shouldBeAlwaysOnTop;
            Maximized = shouldBeMaximized;
            InitialPosition = new(-1, -1);
        }
    }
}
