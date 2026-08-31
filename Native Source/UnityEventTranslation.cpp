//  This file is licenced under the GNU Affero General Public License. (See file LICENSE at <https://github.com/98ahni/UnityWindowing?tab=GPL-3.0-1-ov-file>)
//  <Copyright (C) 2026 98ahni> Original file author

#include "UnityEventTranslation.h"
#include <GLFW/include/GLFW/glfw3.h>
#include <DebugLog.h>
#include <sstream>

UnityKeyCode GLFW2UnityKeyCode(UINT aGLFWCode)
{
    switch (aGLFWCode)
    {
        case GLFW_KEY_SPACE: return UnityKeyCode::Space;
        case GLFW_KEY_APOSTROPHE: return UnityKeyCode::Quote;
        case GLFW_KEY_COMMA: return UnityKeyCode::Comma;
        case GLFW_KEY_MINUS: return UnityKeyCode::Minus;
        case GLFW_KEY_PERIOD: return UnityKeyCode::Period;
        case GLFW_KEY_SLASH: return UnityKeyCode::Slash;
        case GLFW_KEY_0: return UnityKeyCode::Alpha0;
        case GLFW_KEY_1: return UnityKeyCode::Alpha1;
        case GLFW_KEY_2: return UnityKeyCode::Alpha2;
        case GLFW_KEY_3: return UnityKeyCode::Alpha3;
        case GLFW_KEY_4: return UnityKeyCode::Alpha4;
        case GLFW_KEY_5: return UnityKeyCode::Alpha5;
        case GLFW_KEY_6: return UnityKeyCode::Alpha6;
        case GLFW_KEY_7: return UnityKeyCode::Alpha7;
        case GLFW_KEY_8: return UnityKeyCode::Alpha8;
        case GLFW_KEY_9: return UnityKeyCode::Alpha9;
        case GLFW_KEY_SEMICOLON: return UnityKeyCode::Semicolon;
        case GLFW_KEY_EQUAL: return UnityKeyCode::Equals;
        case GLFW_KEY_A: return UnityKeyCode::A;
        case GLFW_KEY_B: return UnityKeyCode::B;
        case GLFW_KEY_C: return UnityKeyCode::C;
        case GLFW_KEY_D: return UnityKeyCode::D;
        case GLFW_KEY_E: return UnityKeyCode::E;
        case GLFW_KEY_F: return UnityKeyCode::F;
        case GLFW_KEY_G: return UnityKeyCode::G;
        case GLFW_KEY_H: return UnityKeyCode::H;
        case GLFW_KEY_I: return UnityKeyCode::I;
        case GLFW_KEY_J: return UnityKeyCode::J;
        case GLFW_KEY_K: return UnityKeyCode::K;
        case GLFW_KEY_L: return UnityKeyCode::L;
        case GLFW_KEY_M: return UnityKeyCode::M;
        case GLFW_KEY_N: return UnityKeyCode::N;
        case GLFW_KEY_O: return UnityKeyCode::O;
        case GLFW_KEY_P: return UnityKeyCode::P;
        case GLFW_KEY_Q: return UnityKeyCode::Q;
        case GLFW_KEY_R: return UnityKeyCode::R;
        case GLFW_KEY_S: return UnityKeyCode::S;
        case GLFW_KEY_T: return UnityKeyCode::T;
        case GLFW_KEY_U: return UnityKeyCode::U;
        case GLFW_KEY_V: return UnityKeyCode::V;
        case GLFW_KEY_W: return UnityKeyCode::W;
        case GLFW_KEY_X: return UnityKeyCode::X;
        case GLFW_KEY_Y: return UnityKeyCode::Y;
        case GLFW_KEY_Z: return UnityKeyCode::Z;
        case GLFW_KEY_LEFT_BRACKET: return UnityKeyCode::LeftBracket;
        case GLFW_KEY_BACKSLASH: return UnityKeyCode::Backslash;
        case GLFW_KEY_RIGHT_BRACKET: return UnityKeyCode::RightBracket;
        case GLFW_KEY_GRAVE_ACCENT: return UnityKeyCode::BackQuote;
        case GLFW_KEY_WORLD_1: return UnityKeyCode::Unknown_World1;
        case GLFW_KEY_WORLD_2: return UnityKeyCode::Unknown_World2;
        case GLFW_KEY_ESCAPE: return UnityKeyCode::Escape;
        case GLFW_KEY_ENTER: return UnityKeyCode::Return;
        case GLFW_KEY_TAB: return UnityKeyCode::Tab;
        case GLFW_KEY_BACKSPACE: return UnityKeyCode::Backspace;
        case GLFW_KEY_INSERT: return UnityKeyCode::Insert;
        case GLFW_KEY_DELETE: return UnityKeyCode::Delete;
        case GLFW_KEY_RIGHT: return UnityKeyCode::RightArrow;
        case GLFW_KEY_LEFT: return UnityKeyCode::LeftArrow;
        case GLFW_KEY_DOWN: return UnityKeyCode::DownArrow;
        case GLFW_KEY_UP: return UnityKeyCode::UpArrow;
        case GLFW_KEY_PAGE_UP: return UnityKeyCode::PageUp;
        case GLFW_KEY_PAGE_DOWN: return UnityKeyCode::PageDown;
        case GLFW_KEY_HOME: return UnityKeyCode::Home;
        case GLFW_KEY_END: return UnityKeyCode::End;
        case GLFW_KEY_CAPS_LOCK: return UnityKeyCode::CapsLock;
        case GLFW_KEY_SCROLL_LOCK: return UnityKeyCode::ScrollLock;
        case GLFW_KEY_NUM_LOCK: return UnityKeyCode::Numlock;
        case GLFW_KEY_PRINT_SCREEN: return UnityKeyCode::Print;
        case GLFW_KEY_PAUSE: return UnityKeyCode::Pause;
        case GLFW_KEY_F1: return UnityKeyCode::F1;
        case GLFW_KEY_F2: return UnityKeyCode::F2;
        case GLFW_KEY_F3: return UnityKeyCode::F3;
        case GLFW_KEY_F4: return UnityKeyCode::F4;
        case GLFW_KEY_F5: return UnityKeyCode::F5;
        case GLFW_KEY_F6: return UnityKeyCode::F6;
        case GLFW_KEY_F7: return UnityKeyCode::F7;
        case GLFW_KEY_F8: return UnityKeyCode::F8;
        case GLFW_KEY_F9: return UnityKeyCode::F9;
        case GLFW_KEY_F10: return UnityKeyCode::F10;
        case GLFW_KEY_F11: return UnityKeyCode::F11;
        case GLFW_KEY_F12: return UnityKeyCode::F12;
        case GLFW_KEY_F13: return UnityKeyCode::F13;
        case GLFW_KEY_F14: return UnityKeyCode::F14;
        case GLFW_KEY_F15: return UnityKeyCode::F15;
        case GLFW_KEY_F16: return UnityKeyCode::Unknown_F16;
        case GLFW_KEY_F17: return UnityKeyCode::Unknown_F17;
        case GLFW_KEY_F18: return UnityKeyCode::Unknown_F18;
        case GLFW_KEY_F19: return UnityKeyCode::Unknown_F19;
        case GLFW_KEY_F20: return UnityKeyCode::Unknown_F20;
        case GLFW_KEY_F21: return UnityKeyCode::Unknown_F21;
        case GLFW_KEY_F22: return UnityKeyCode::Unknown_F22;
        case GLFW_KEY_F23: return UnityKeyCode::Unknown_F23;
        case GLFW_KEY_F24: return UnityKeyCode::Unknown_F24;
        case GLFW_KEY_F25: return UnityKeyCode::Unknown_F25;
        case GLFW_KEY_KP_0: return UnityKeyCode::Keypad0;
        case GLFW_KEY_KP_1: return UnityKeyCode::Keypad1;
        case GLFW_KEY_KP_2: return UnityKeyCode::Keypad2;
        case GLFW_KEY_KP_3: return UnityKeyCode::Keypad3;
        case GLFW_KEY_KP_4: return UnityKeyCode::Keypad4;
        case GLFW_KEY_KP_5: return UnityKeyCode::Keypad5;
        case GLFW_KEY_KP_6: return UnityKeyCode::Keypad6;
        case GLFW_KEY_KP_7: return UnityKeyCode::Keypad7;
        case GLFW_KEY_KP_8: return UnityKeyCode::Keypad8;
        case GLFW_KEY_KP_9: return UnityKeyCode::Keypad9;
        case GLFW_KEY_KP_DECIMAL: return UnityKeyCode::KeypadPeriod;
        case GLFW_KEY_KP_DIVIDE: return UnityKeyCode::KeypadDivide;
        case GLFW_KEY_KP_MULTIPLY: return UnityKeyCode::KeypadMultiply;
        case GLFW_KEY_KP_SUBTRACT: return UnityKeyCode::KeypadMinus;
        case GLFW_KEY_KP_ADD: return UnityKeyCode::KeypadPlus;
        case GLFW_KEY_KP_ENTER: return UnityKeyCode::KeypadEnter;
        case GLFW_KEY_KP_EQUAL: return UnityKeyCode::KeypadEquals;
        case GLFW_KEY_LEFT_SHIFT: return UnityKeyCode::LeftShift;
        case GLFW_KEY_LEFT_CONTROL: return UnityKeyCode::LeftControl;
        case GLFW_KEY_LEFT_ALT: return UnityKeyCode::LeftAlt;
        case GLFW_KEY_LEFT_SUPER: return UnityKeyCode::LeftCommand;
        case GLFW_KEY_RIGHT_SHIFT: return UnityKeyCode::RightShift;
        case GLFW_KEY_RIGHT_CONTROL: return UnityKeyCode::RightControl;
        case GLFW_KEY_RIGHT_ALT: return UnityKeyCode::RightAlt;
        case GLFW_KEY_RIGHT_SUPER: return UnityKeyCode::RightCommand;
        case GLFW_KEY_MENU: return UnityKeyCode::Menu;
        default:
            Debug::Log((std::stringstream() << "Unknown GLFW key code: " << aGLFWCode).str().c_str());
    }
    return UnityKeyCode::None;
}

UnityKeyCode NativeScan2UnityKeyCode(UINT aScanCode)
{
    switch (aScanCode)
    {
    default:
        Debug::Log((std::stringstream() << "Unknown key scan code: " << aScanCode).str().c_str());
    }
    return UnityKeyCode::None;
}
