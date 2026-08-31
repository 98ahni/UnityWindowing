//  This file is licenced under the GNU Affero General Public License. (See file LICENSE at <https://github.com/98ahni/UnityWindowing?tab=GPL-3.0-1-ov-file>)
//  <Copyright (C) 2026 98ahni> Original file author

using UnityEngine;

#if ENABLE_INPUT_SYSTEM
using UnityEngine.InputSystem;
using UnityEngine.InputSystem.Controls;
using UnityEngine.InputSystem.LowLevel;
#endif

namespace Ahni.UnityWindowing
{
    /// <summary> Describes the action of the input in window callbacks. </summary>
    public enum KeyAction { KeyUp, KeyDown, KeyRepeat }
    internal class InputEmulator
    {
        // This could be a singleton and just interact with whatever device is current

#if ENABLE_INPUT_SYSTEM
        private Keyboard _keyboard;
        private Mouse _mouse;
#endif
        private Camera _camera;

        public InputEmulator(string aName, Camera aCamera)
        {
#if ENABLE_INPUT_SYSTEM
            InputSystem.runInBackground = true;
            InputSystem.settings.backgroundBehavior = InputSettings.BackgroundBehavior.IgnoreFocus; // This should probably be set and unset depending on if any window has focus
            _keyboard = InputSystem.AddDevice<Keyboard>(aName);
            _mouse = InputSystem.AddDevice<Mouse>(aName);
#endif
            _camera = aCamera;
        }
        ~InputEmulator()
        {
#if ENABLE_INPUT_SYSTEM
            InputSystem.RemoveDevice(_keyboard);
            InputSystem.RemoveDevice(_mouse);
#endif
        }

        public void RegisterKeyboardEvent(KeyCode aKey, KeyAction anAction, int someMods)
        {
#if ENABLE_INPUT_SYSTEM
            Key key = KeyCodeToKey(aKey);
            if (key != Key.None)
            {
                _keyboard.MakeCurrent();
                using (StateEvent.From(_keyboard, out InputEventPtr eventPtr))
                {
                    _keyboard[key].WriteValueIntoEvent(anAction == KeyAction.KeyUp ? 0f : 1f, eventPtr);
                    InputSystem.QueueEvent(eventPtr);
                    InputSystem.Update();
                }
            }
#endif
#if ENABLE_LEGACY_INPUT_MANAGER || !ENABLE_INPUT_SYSTEM
#endif
        }

        public void RegisterMouseButtonEvent(int aButton, KeyAction anAction, int someMods)
        {
#if ENABLE_INPUT_SYSTEM
            _mouse.MakeCurrent();
            using (StateEvent.From(_mouse, out InputEventPtr eventPtr))
            {
                _mouse.displayIndex.WriteValueIntoEvent(_camera.targetDisplay, eventPtr);
                InputSystem.QueueEvent(eventPtr);
                InputSystem.Update();
            }
            using (StateEvent.From(_mouse, out InputEventPtr eventPtr))
            {
                ButtonControl button = aButton switch
                {
                    0 => _mouse.leftButton,
                    1 => _mouse.rightButton,
                    2 => _mouse.middleButton,
                    3 => _mouse.backButton,
                    4 => _mouse.forwardButton,
                    _ => _mouse.leftButton,
                };
                button.WriteValueIntoEvent(anAction == KeyAction.KeyUp ? 0f : 1f, eventPtr);
                InputSystem.QueueEvent(eventPtr);
                InputSystem.Update();
            }
#endif
#if ENABLE_LEGACY_INPUT_MANAGER || !ENABLE_INPUT_SYSTEM
#endif
        }

        public void RegisterMouseMoveEvent(Vector2 aNewPos)
        {
#if ENABLE_INPUT_SYSTEM
            _mouse.MakeCurrent();
            Vector2 prevPos = _mouse.position.ReadValue();
            using (StateEvent.From(_mouse, out InputEventPtr eventPtr))
            {
                _mouse.displayIndex.WriteValueIntoEvent(_camera.targetDisplay, eventPtr);
                InputSystem.QueueEvent(eventPtr);
                InputSystem.Update();
            }
            using (StateEvent.From(_mouse, out InputEventPtr eventPtr))
            {
                _mouse.position.WriteValueIntoEvent(aNewPos, eventPtr);
                InputSystem.QueueEvent(eventPtr);
                InputSystem.Update();
            }
            using (StateEvent.From(_mouse, out InputEventPtr eventPtr))
            {
                _mouse.delta.WriteValueIntoEvent(aNewPos - prevPos, eventPtr);
                InputSystem.QueueEvent(eventPtr);
                InputSystem.Update();
            }
#endif
#if ENABLE_LEGACY_INPUT_MANAGER || !ENABLE_INPUT_SYSTEM
#endif
        }

        public void RegisterScrollEvent(Vector2 anOffset)
        {
#if ENABLE_INPUT_SYSTEM
            _mouse.MakeCurrent();
            using (StateEvent.From(_mouse, out InputEventPtr eventPtr))
            {
                _mouse.displayIndex.WriteValueIntoEvent(_camera.targetDisplay, eventPtr);
                InputSystem.QueueEvent(eventPtr);
                InputSystem.Update();
            }
            using (StateEvent.From(_mouse, out InputEventPtr eventPtr))
            {
                _mouse.scroll.WriteValueIntoEvent(anOffset, eventPtr);
                InputSystem.QueueEvent(eventPtr);
                InputSystem.Update();
            }
#endif
#if ENABLE_LEGACY_INPUT_MANAGER || !ENABLE_INPUT_SYSTEM
#endif
        }

#if ENABLE_INPUT_SYSTEM
        private Key KeyCodeToKey(KeyCode aKey)
        {
            return aKey switch
            {
                KeyCode.None => Key.None,
                KeyCode.Backspace => Key.Backspace,
                KeyCode.Delete => Key.Delete,
                KeyCode.Tab => Key.Tab,
                // KeyCode.Clear => no equivalent,
                KeyCode.Return => Key.Enter,
                KeyCode.Pause => Key.Pause,
                KeyCode.Escape => Key.Escape,
                KeyCode.Space => Key.Space,
                KeyCode.Keypad0 => Key.Numpad0,
                KeyCode.Keypad1 => Key.Numpad1,
                KeyCode.Keypad2 => Key.Numpad2,
                KeyCode.Keypad3 => Key.Numpad3,
                KeyCode.Keypad4 => Key.Numpad4,
                KeyCode.Keypad5 => Key.Numpad5,
                KeyCode.Keypad6 => Key.Numpad6,
                KeyCode.Keypad7 => Key.Numpad7,
                KeyCode.Keypad8 => Key.Numpad8,
                KeyCode.Keypad9 => Key.Numpad9,
                KeyCode.KeypadPeriod => Key.NumpadPeriod,
                KeyCode.KeypadDivide => Key.NumpadDivide,
                KeyCode.KeypadMultiply => Key.NumpadMultiply,
                KeyCode.KeypadMinus => Key.NumpadMinus,
                KeyCode.KeypadPlus => Key.NumpadPlus,
                KeyCode.KeypadEnter => Key.NumpadEnter,
                KeyCode.KeypadEquals => Key.NumpadEquals,
                KeyCode.UpArrow => Key.UpArrow,
                KeyCode.DownArrow => Key.DownArrow,
                KeyCode.RightArrow => Key.RightArrow,
                KeyCode.LeftArrow => Key.LeftArrow,
                KeyCode.Insert => Key.Insert,
                KeyCode.Home => Key.Home,
                KeyCode.End => Key.End,
                KeyCode.PageUp => Key.PageUp,
                KeyCode.PageDown => Key.PageDown,
                KeyCode.F1 => Key.F1,
                KeyCode.F2 => Key.F2,
                KeyCode.F3 => Key.F3,
                KeyCode.F4 => Key.F4,
                KeyCode.F5 => Key.F5,
                KeyCode.F6 => Key.F6,
                KeyCode.F7 => Key.F7,
                KeyCode.F8 => Key.F8,
                KeyCode.F9 => Key.F9,
                KeyCode.F10 => Key.F10,
                KeyCode.F11 => Key.F11,
                KeyCode.F12 => Key.F12,
                // KeyCode.F13 => no equivalent,
                // KeyCode.F14 => no equivalent,
                // KeyCode.F15 => no equivalent,
                KeyCode.Alpha0 => Key.Digit0,
                KeyCode.Alpha1 => Key.Digit1,
                KeyCode.Alpha2 => Key.Digit2,
                KeyCode.Alpha3 => Key.Digit3,
                KeyCode.Alpha4 => Key.Digit4,
                KeyCode.Alpha5 => Key.Digit5,
                KeyCode.Alpha6 => Key.Digit6,
                KeyCode.Alpha7 => Key.Digit7,
                KeyCode.Alpha8 => Key.Digit8,
                KeyCode.Alpha9 => Key.Digit9,
                // KeyCode.Exclaim => no equivalent,
                // KeyCode.DoubleQuote => no equivalent,
                // KeyCode.Hash => no equivalent,
                // KeyCode.Dollar => no equivalent,
                // KeyCode.Percent => no equivalent,
                // KeyCode.Ampersand => no equivalent,
                KeyCode.Quote => Key.Quote,
                // KeyCode.LeftParen => no equivalent,
                // KeyCode.RightParen => no equivalent,
                // KeyCode.Asterisk => no equivalent,
                // KeyCode.Plus => no equivalent,
                KeyCode.Comma => Key.Comma,
                KeyCode.Minus => Key.Minus,
                KeyCode.Period => Key.Period,
                KeyCode.Slash => Key.Slash,
                // KeyCode.Colon => no equivalent,
                KeyCode.Semicolon => Key.Semicolon,
                // KeyCode.Less => no equivalent,
                KeyCode.Equals => Key.Equals,
                // KeyCode.Greater => no equivalent,
                // KeyCode.Question => no equivalent,
                // KeyCode.At => no equivalent,
                KeyCode.LeftBracket => Key.LeftBracket,
                KeyCode.Backslash => Key.Backslash,
                KeyCode.RightBracket => Key.RightBracket,
                // KeyCode.Caret => no equivalent,
                // KeyCode.Underscore => no equivalent,
                KeyCode.BackQuote => Key.Backquote,
                KeyCode.A => Key.A,
                KeyCode.B => Key.B,
                KeyCode.C => Key.C,
                KeyCode.D => Key.D,
                KeyCode.E => Key.E,
                KeyCode.F => Key.F,
                KeyCode.G => Key.G,
                KeyCode.H => Key.H,
                KeyCode.I => Key.I,
                KeyCode.J => Key.J,
                KeyCode.K => Key.K,
                KeyCode.L => Key.L,
                KeyCode.M => Key.M,
                KeyCode.N => Key.N,
                KeyCode.O => Key.O,
                KeyCode.P => Key.P,
                KeyCode.Q => Key.Q,
                KeyCode.R => Key.R,
                KeyCode.S => Key.S,
                KeyCode.T => Key.T,
                KeyCode.U => Key.U,
                KeyCode.V => Key.V,
                KeyCode.W => Key.W,
                KeyCode.X => Key.X,
                KeyCode.Y => Key.Y,
                KeyCode.Z => Key.Z,
                // KeyCode.LeftCurlyBracket => no equivalent,
                // KeyCode.Pipe => no equivalent,
                // KeyCode.RightCurlyBracket => no equivalent,
                // KeyCode.Tilde => no equivalent,
                KeyCode.Numlock => Key.NumLock,
                KeyCode.CapsLock => Key.CapsLock,
                KeyCode.ScrollLock => Key.ScrollLock,
                KeyCode.RightShift => Key.RightShift,
                KeyCode.LeftShift => Key.LeftShift,
                KeyCode.RightControl => Key.RightCtrl,
                KeyCode.LeftControl => Key.LeftCtrl,
                KeyCode.RightAlt => Key.RightAlt,
                KeyCode.LeftAlt => Key.LeftAlt,
                KeyCode.LeftCommand => Key.LeftCommand,
                KeyCode.LeftWindows => Key.LeftWindows,
                KeyCode.RightCommand => Key.RightCommand,
                KeyCode.RightWindows => Key.RightWindows,
                KeyCode.AltGr => Key.AltGr,
                // KeyCode.Help => no equivalent,
                KeyCode.Print => Key.PrintScreen,
                // KeyCode.SysReq => no equivalent,
                // KeyCode.Break => no equivalent,
                KeyCode.Menu => Key.ContextMenu,
                // KeyCode.WheelUp => removed (mouse),
                // KeyCode.WheelDown => removed (mouse),
                // KeyCode.F16 => no equivalent,
                // KeyCode.F17 => no equivalent,
                // KeyCode.F18 => no equivalent,
                // KeyCode.F19 => no equivalent,
                // KeyCode.F20 => no equivalent,
                // KeyCode.F21 => no equivalent,
                // KeyCode.F22 => no equivalent,
                // KeyCode.F23 => no equivalent,
                // KeyCode.F24 => no equivalent,
                // Mouse0–Mouse6 removed (use UnityEngine.InputSystem.Mouse instead)
                // JoystickButton0–19, Joystick1–8Button0–19 removed (use UnityEngine.InputSystem.Gamepad instead)
                _ => Key.None
            };
        }
#endif

#if ENABLE_INPUT_SYSTEM

#endif
#if ENABLE_LEGACY_INPUT_MANAGER || !ENABLE_INPUT_SYSTEM
#endif
    }
}
