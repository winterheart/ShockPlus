/*

Copyright (C) 2015-2018 Night Dive Studios, LLC.
Copyright (C) 2019 Shockolate Project
Copyright (C) 2021 ShockPlus Project

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

//
// DG 2018: (eventually) SDL versions of the functions previously in kbMac.c, mouse.c and kbcook.c
//

#include "lg.h"
#include "event_ui.h"
#include "kb.h"
#include "kbcook.h"
#include "mouse.h"
#include "rect.h"
#include "sdl_events.h"
#include <stdlib.h>
#include <SDL.h>
#include <OpenGL.h>

extern SDL_Window *window;
extern SDL_Renderer *renderer;

bool fullscreenActive = false;

void dispatch_kb_event(kbs_event *kbe);
void dispatch_ms_event(ss_mouse_event *mse);

void pump_kb_event(SDL_Event *ev);
void pump_ms_button_event(SDL_Event *ev);
void pump_ms_wheel_event(SDL_Event *ev);
void pump_ms_motion_event(SDL_Event *ev);

static void toggleFullScreen() {
    fullscreenActive = !fullscreenActive;
    SDL_SetWindowFullscreen(window, fullscreenActive ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);

    if (!(SDL_GetWindowFlags(window) & SDL_WINDOW_MAXIMIZED))
        SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}

// current state of the keys, based on the SystemShock/Mac Keycodes (sshockKeyStates[keyCode] has the state for that
// key) set at the beginning of each frame in pump_events()
uchar sshockKeyStates[256];

static uchar sdlKeyCodeToSSHOCKkeyCode(SDL_Keycode kc) {
    // apparently System Shock uses the same keycodes as Mac which are luckily documented, see
    // see http://snipplr.com/view/42797/ and https://stackoverflow.com/a/16125341
    // see also GameSrc/movekeys.c for a very short list

    switch (kc) {
    case SDLK_a:
        return 0x00; //  kVK_ANSI_A = 0x00,
    case SDLK_s:
        return 0x01; //  kVK_ANSI_S = 0x01,
    case SDLK_d:
        return 0x02; //  kVK_ANSI_D = 0x02,
    case SDLK_f:
        return 0x03; //  kVK_ANSI_F = 0x03,
    case SDLK_h:
        return 0x04; //  kVK_ANSI_H = 0x04,
    case SDLK_g:
        return 0x05; //  kVK_ANSI_G = 0x05,
    case SDLK_z:
        return 0x06; //  kVK_ANSI_Z = 0x06,
    case SDLK_x:
        return 0x07; //  kVK_ANSI_X = 0x07,
    case SDLK_c:
        return 0x08; //  kVK_ANSI_C = 0x08,
    case SDLK_v:
        return 0x09; //  kVK_ANSI_V = 0x09,
    case SDLK_b:
        return 0x0B; //  kVK_ANSI_B = 0x0B,
    case SDLK_q:
        return 0x0C; //  kVK_ANSI_Q = 0x0C,
    case SDLK_w:
        return 0x0D; //  kVK_ANSI_W = 0x0D,
    case SDLK_e:
        return 0x0E; //  kVK_ANSI_E = 0x0E,
    case SDLK_r:
        return 0x0F; //  kVK_ANSI_R = 0x0F,
    case SDLK_y:
        return 0x10; //  kVK_ANSI_Y = 0x10,
    case SDLK_t:
        return 0x11; //  kVK_ANSI_T = 0x11,
    case SDLK_1:
        return 0x12; //  kVK_ANSI_1 = 0x12,
    case SDLK_2:
        return 0x13; //  kVK_ANSI_2 = 0x13,
    case SDLK_3:
        return 0x14; //  kVK_ANSI_3 = 0x14,
    case SDLK_4:
        return 0x15; //  kVK_ANSI_4 = 0x15,
    case SDLK_6:
        return 0x16; //  kVK_ANSI_6 = 0x16,
    case SDLK_5:
        return 0x17; //  kVK_ANSI_5 = 0x17,
    case SDLK_EQUALS:
        return 0x18; //  kVK_ANSI_Equal = 0x18,
    case SDLK_9:
        return 0x19; //  kVK_ANSI_9 = 0x19,
    case SDLK_7:
        return 0x1A; //  kVK_ANSI_7 = 0x1A,
    case SDLK_MINUS:
        return 0x1B; //  kVK_ANSI_Minus = 0x1B,
    case SDLK_8:
        return 0x1C; //  kVK_ANSI_8 = 0x1C,
    case SDLK_0:
        return 0x1D; //  kVK_ANSI_0 = 0x1D,
    case SDLK_RIGHTBRACKET:
        return 0x1E; //  kVK_ANSI_RightBracket = 0x1E,
    case SDLK_o:
        return 0x1F; //  kVK_ANSI_O = 0x1F,
    case SDLK_u:
        return 0x20; //  kVK_ANSI_U = 0x20,
    case SDLK_LEFTBRACKET:
        return 0x21; //  kVK_ANSI_LeftBracket = 0x21,
    case SDLK_i:
        return 0x22; //  kVK_ANSI_I = 0x22,
    case SDLK_p:
        return 0x23; //  kVK_ANSI_P = 0x23,
    case SDLK_l:
        return 0x25; //  kVK_ANSI_L = 0x25,
    case SDLK_j:
        return 0x26; //  kVK_ANSI_J = 0x26,
    case SDLK_QUOTE:
        return 0x27; //  kVK_ANSI_Quote = 0x27, // TODO: or QUOTEDBL ?
    case SDLK_k:
        return 0x28; //  kVK_ANSI_K = 0x28,
    case SDLK_SEMICOLON:
        return 0x29; //  kVK_ANSI_Semicolon = 0x29,
    case SDLK_BACKSLASH:
        return 0x2A; //  kVK_ANSI_Backslash = 0x2A,
    case SDLK_COMMA:
        return 0x2B; //  kVK_ANSI_Comma = 0x2B,
    case SDLK_SLASH:
        return 0x2C; //  kVK_ANSI_Slash = 0x2C,
    case SDLK_n:
        return 0x2D; //  kVK_ANSI_N = 0x2D,
    case SDLK_m:
        return 0x2E; //  kVK_ANSI_M = 0x2E,
    case SDLK_PERIOD:
        return 0x2F; //  kVK_ANSI_Period = 0x2F,
    case SDLK_BACKQUOTE:
        return 0x32; //  kVK_ANSI_Grave = 0x32, // TODO: really?
    case SDLK_KP_DECIMAL:
        return 0x41; //  kVK_ANSI_KeypadDecimal   = 0x41,
    case SDLK_KP_MULTIPLY:
        return 0x43; //  kVK_ANSI_KeypadMultiply = 0x43,
    case SDLK_KP_PLUS:
        return 0x45; //  kVK_ANSI_KeypadPlus = 0x45,
    case SDLK_KP_CLEAR:
        return 0x47; //  kVK_ANSI_KeypadClear = 0x47,
    case SDLK_KP_DIVIDE:
        return 0x4B; //  kVK_ANSI_KeypadDivide = 0x4B,
    case SDLK_KP_ENTER:
        return 0x4C; //  kVK_ANSI_KeypadEnter   = 0x4C, aka _ENTER2_
    case SDLK_KP_MINUS:
        return 0x4E; //  kVK_ANSI_KeypadMinus   = 0x4E,
    case SDLK_KP_EQUALS:
        return 0x51; //  kVK_ANSI_KeypadEquals = 0x51,
    case SDLK_KP_0:
        return 0x52; //  kVK_ANSI_Keypad0 = 0x52,
    case SDLK_KP_1:
        return 0x53; //  kVK_ANSI_Keypad1 = 0x53, aka _END2_
    case SDLK_KP_2:
        return 0x54; //  kVK_ANSI_Keypad2 = 0x54, aka _DOWN2_
    case SDLK_KP_3:
        return 0x55; //  kVK_ANSI_Keypad3 = 0x55, aka _PGDN2_
    case SDLK_KP_4:
        return 0x56; //  kVK_ANSI_Keypad4 = 0x56, aka _LEFT2_
    case SDLK_KP_5:
        return 0x57; //  kVK_ANSI_Keypad5 = 0x57, aka _PAD5_
    case SDLK_KP_6:
        return 0x58; //  kVK_ANSI_Keypad6 = 0x58, aka _RIGHT2_
    case SDLK_KP_7:
        return 0x59; //  kVK_ANSI_Keypad7 = 0x59, aka _HOME2_
    case SDLK_KP_8:
        return 0x5B; //  kVK_ANSI_Keypad8 = 0x5B, aka _UP2_
    case SDLK_KP_9:
        return 0x5C; //  kVK_ANSI_Keypad9 = 0x5C, aka _PGUP2_

    // keycodes for keys that are independent of keyboard layout
    case SDLK_RETURN:
        return 0x24; //  kVK_Return  = 0x24,
    case SDLK_TAB:
        return 0x30; //  kVK_Tab     = 0x30,
    case SDLK_SPACE:
        return 0x31; //  kVK_Space   = 0x31,
    case SDLK_DELETE:
        return 0x33; //  kVK_Delete  = 0x33,
    case SDLK_BACKSPACE:
        return 0x33; //  kVK_Delete  = 0x33,
    case SDLK_ESCAPE:
        return 0x35; //  kVK_Escape  = 0x35,

        //    returning these is unnecessary and can cause keypresses to be missed
        //    (esp keys with modifiers)
        // case SDLK_LGUI : // fall-through
        // case SDLK_RGUI : return 0x37; //  kVK_Command = 0x37, // FIXME: I think command is the windows/meta key?
        // case SDLK_LSHIFT : return 0x38; //  kVK_Shift   = 0x38,
        // case SDLK_CAPSLOCK : return 0x39; //  kVK_CapsLock= 0x39,
        // case SDLK_LALT : return 0x3A; //  kVK_Option  = 0x3A, Option == Aalt
        // case SDLK_LCTRL : return 0x3B; //  kVK_Control = 0x3B,
        // case SDLK_RSHIFT : return 0x3C; //  kVK_RightShift  = 0x3C,
        // case SDLK_RALT : return 0x3D; //  kVK_RightOption = 0x3D,
        // case SDLK_RCTRL : return 0x3E; //  kVK_RightControl = 0x3E,

    // case SDLK_ : return 0x3F; //  kVK_Function = 0x3F, // TODO: what's this?
    case SDLK_F17:
        return 0x40; //  kVK_F17 = 0x40,
    case SDLK_VOLUMEUP:
        return 0x48; //  kVK_VolumeUp = 0x48,
    case SDLK_VOLUMEDOWN:
        return 0x49; //  kVK_VolumeDown = 0x49,
    case SDLK_MUTE:
        return 0x4A; //  kVK_Mute = 0x4A,
    case SDLK_F18:
        return 0x4F; //  kVK_F18 = 0x4F,
    case SDLK_F19:
        return 0x50; //  kVK_F19 = 0x50,
    case SDLK_F20:
        return 0x5A; //  kVK_F20 = 0x5A,
    case SDLK_F5:
        return 0x60; //  kVK_F5  = 0x60,
    case SDLK_F6:
        return 0x61; //  kVK_F6  = 0x61,
    case SDLK_F7:
        return 0x62; //  kVK_F7  = 0x62,
    case SDLK_F3:
        return 0x63; //  kVK_F3  = 0x63,
    case SDLK_F8:
        return 0x64; //  kVK_F8  = 0x64,
    case SDLK_F9:
        return 0x65; //  kVK_F9  = 0x65,
    case SDLK_F11:
        return 0x67; //  kVK_F11 = 0x67,
    case SDLK_F13:
        return 0x69; //  kVK_F13 = 0x69,
    case SDLK_F16:
        return 0x6A; //  kVK_F16 = 0x6A,
    case SDLK_F14:
        return 0x6B; //  kVK_F14 = 0x6B,
    case SDLK_F10:
        return 0x6D; //  kVK_F10 = 0x6D,
    case SDLK_F12:
        return 0x6F; //  kVK_F12 = 0x6F,
    case SDLK_F15:
        return 0x71; //  kVK_F15 = 0x71,
    case SDLK_HELP:
        return 0x72; //  kVK_Help = 0x72,
    case SDLK_HOME:
        return 0x73; //  kVK_Home = 0x73,
    case SDLK_PAGEUP:
        return 0x74; //  kVK_PageUp = 0x74,
    // case SDLK_ : return 0x75; //  kVK_ForwardDelete = 0x75, // TODO: what's this?
    case SDLK_F4:
        return 0x76; //  kVK_F4 = 0x76,
    case SDLK_END:
        return 0x77; //  kVK_End = 0x77,
    case SDLK_F2:
        return 0x78; //  kVK_F2 = 0x78,
    case SDLK_PAGEDOWN:
        return 0x79; //  kVK_PageDown = 0x79,
    case SDLK_F1:
        return 0x7A; //  kVK_F1 = 0x7A,
    case SDLK_LEFT:
        return 0x7B; //  kVK_LeftArrow  = 0x7B, aka _LEFT_
    case SDLK_RIGHT:
        return 0x7C; //  kVK_RightArrow = 0x7C, aka _RIGHT
    case SDLK_DOWN:
        return 0x7D; //  kVK_DownArrow  = 0x7D, aka _DOWN_
    case SDLK_UP:
        return 0x7E; //  kVK_UpArrow    = 0x7E, aka _UP_
    default:
        return KBC_NONE;
    }
}

int MouseX;
int MouseY;

int MouseChaosX;
int MouseChaosY;

extern bool MouseCaptured;

void SetMouseXY(int mx, int my) {
    int physical_width, physical_height;
    SDL_GetWindowSize(window, &physical_width, &physical_height);

    int w, h;
    SDL_RenderGetLogicalSize(renderer, &w, &h);

    float scale_x = (float)physical_width / w;
    float scale_y = (float)physical_height / h;

    int x, y;
    if (scale_x >= scale_y) {
        x = (physical_width - w * scale_x) / 2;
        y = 0;
    } else {
        x = 0;
        y = (physical_height - h * scale_y) / 2;
    }

    bool inside = (mx >= x && mx < x + w && my >= y && my < y + h);
    bool focus = (SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS); // checking mouse focus isn't what we want here

    if (!inside && focus) {
        if (mx < x)
            mx = x;
        if (mx > x + w - 1)
            mx = x + w - 1;
        if (my < y)
            my = y;
        if (my > y + h - 1)
            my = y + h - 1;
    }

    if (focus) {
        MouseX = mx;
        MouseY = my;
    }

    SDL_ShowCursor((!focus || (!inside && !MouseCaptured)) ? SDL_ENABLE : SDL_DISABLE);
}

extern bool TriggerRelMouseMode;

static SDL_bool saved_rel_mouse = FALSE;

// same codes as returned by sdlKeyCodeToSSHOCKkeyCode()
uchar Ascii2Code[95] = {
    0x31, // space
    0x12, // !
    0x27, // "
    0x14, // #
    0x15, // $
    0x17, // %
    0x1A, // &
    0x27, // '
    0x19, // (
    0x1D, // )
    0x1C, // *
    0x18, // +
    0x2B, // ,
    0x1B, // -
    0x2F, // .
    0x2C, // /
    0x1D, // 0
    0x12, // 1
    0x13, // 2
    0x14, // 3
    0x15, // 4
    0x17, // 5
    0x16, // 6
    0x1A, // 7
    0x1C, // 8
    0x19, // 9
    0x29, // :
    0x29, // ;
    0x2B, // <
    0x18, // =
    0x2F, // >
    0x2C, // ?
    0x13, // @
    0x00, // A
    0x0B, // B
    0x08, // C
    0x02, // D
    0x0E, // E
    0x03, // F
    0x05, // G
    0x04, // H
    0x22, // I
    0x26, // J
    0x28, // K
    0x25, // L
    0x2E, // M
    0x2D, // N
    0x1F, // O
    0x23, // P
    0x0C, // Q
    0x0F, // R
    0x01, // S
    0x11, // T
    0x20, // U
    0x09, // V
    0x0D, // W
    0x07, // X
    0x10, // Y
    0x06, // Z
    0x21, // [
    0x2A, // backslash
    0x1E, // ]
    0x16, // ^
    0x1B, // _
    0x32, // `
    0x00, // a
    0x0B, // b
    0x08, // c
    0x02, // d
    0x0E, // e
    0x03, // f
    0x05, // g
    0x04, // h
    0x22, // i
    0x26, // j
    0x28, // k
    0x25, // l
    0x2E, // m
    0x2D, // n
    0x1F, // o
    0x23, // p
    0x0C, // q
    0x0F, // r
    0x01, // s
    0x11, // t
    0x20, // u
    0x09, // v
    0x0D, // w
    0x07, // x
    0x10, // y
    0x06, // z
    0x21, // {
    0x2A, // |
    0x1E, // }
    0x32  // ~
};

/**
 * Converts key into "cooked" SystemShock codes
 * @param key SDL_Keycode key
 * @return cooked ascii code
 */
uint8_t sdlKeyToASCIICode(SDL_Keycode key) {
    // https://wiki.libsdl.org/SDLKeycodeLookup
    // Keycodes for keys with printable characters are represented by the
    // character byte in parentheses. Keycodes without character representations
    // are determined by their scancode bitwise OR-ed with 1<<30 (0x40000000).
    switch (key) {
    case SDLK_F1:
        return (128 + 0);
    case SDLK_F2:
        return (128 + 1);
    case SDLK_F3:
        return (128 + 2);
    case SDLK_F4:
        return (128 + 3);
    case SDLK_F5:
        return (128 + 4);
    case SDLK_F6:
        return (128 + 5);
    case SDLK_F7:
        return (128 + 6);
    case SDLK_F8:
        return (128 + 7);
    case SDLK_F9:
        return (128 + 8);
    case SDLK_F10:
        return (128 + 9);
    case SDLK_F11:
        return (128 + 10);
    case SDLK_F12:
        return (128 + 11);
    case SDLK_KP_DIVIDE:
        return (128 + 12);
    case SDLK_KP_MULTIPLY:
        return (128 + 13);
    case SDLK_KP_MINUS:
        return (128 + 14);
    case SDLK_KP_PLUS:
        return (128 + 15);
    case SDLK_KP_ENTER:
        return (128 + 16);
    case SDLK_KP_DECIMAL:
        return (128 + 17);
    case SDLK_KP_0:
        return (128 + 18);
    default:
        if (key >= 0x08 && key <= 127) {
            return key;
        } else {
            return 0;
        }
    }
}

void dispatch_kb_event(kbs_event *kbe) {
    LGPoint mousepos;
    uiEvent out;

    mouse_get_xy(&mousepos.x, &mousepos.y);

    if (kbe->code != KBC_NONE) {
        uchar eaten;
        DEBUG("%s: got a keyboard event: %d, %d", __FUNCTION__, kbe->event.key.type, kbe->event.key.keysym.scancode);
        out.pos = mousepos;
        out.type = UI_EVENT_KBD_RAW;
        out.raw_key_data.scancode = kbe->code;
        out.raw_key_data.action = kbe->state;
        out.sdl_data = kbe->event;
        eaten = uiDispatchEvent(&out);
        if (!eaten) {
            ushort cooked;
            uchar result;
            DEBUG("%s: cooking keyboard event: %d, %d", __FUNCTION__, kbe->event.key.type,
                  kbe->event.key.keysym.scancode);
            // Spew(DSRC_UI_Polling,("uiPoll(): cooking keyboard event: <%d,%x>\n",kbe.state,kbe.code));
            errtype err = kb_cook(*kbe, &cooked, &result);
            if (err != OK)
                return; // err;
            if (result) {
                out.subtype = cooked;
                out.type = UI_EVENT_KBD_COOKED;
                uiDispatchEvent(&out);
            }
        }
    }
}

void dispatch_ms_event(ss_mouse_event *mse) {
    LGPoint mousepos;
    uiEvent out;

    mouse_get_xy(&mousepos.x, &mousepos.y);
    out.pos.x = mse->x;
    out.pos.y = mse->y;
    // note that the equality operator here means that motion-only
    // events are MOUSE_MOVE, and others are MOUSE events.
    out.type = (mse->type == MOUSE_MOTION) ? UI_EVENT_MOUSE_MOVE : UI_EVENT_MOUSE;
    out.sdl_data = mse->event;
    out.subtype = mse->type;
    out.mouse_data.tstamp = mse->timestamp;
    out.mouse_data.buttons = mse->buttons;
    out.mouse_data.modifiers = mse->modifiers;
    TRACE("%s: dispatching mouse event: %d", __FUNCTION__, mse->event.type);
    ui_dispatch_mouse_event(&out);
}

void pump_kb_event(SDL_Event *ev) {
    uchar c = sdlKeyCodeToSSHOCKkeyCode(ev->key.keysym.sym);
    if (c != KBC_NONE) {
        kbs_event keyEvent;

        keyEvent.code = c;
        keyEvent.ascii = 0;
        keyEvent.modifiers = 0;
        keyEvent.event = *ev;

        int ch = SDL_GetKeyFromScancode(ev->key.keysym.scancode);
        if (ev->key.keysym.sym >= 0x08 && ev->key.keysym.sym <= 127) {
            keyEvent.code = Ascii2Code[ch - 32];
            if (isprint(ch) && isupper(ch)) {
                ch = tolower(ch);
                keyEvent.modifiers |= KB_MOD_SHIFT;
            }
        }
        keyEvent.ascii = sdlKeyToASCIICode(ev->key.keysym.sym);

        Uint16 mod = ev->key.keysym.mod;

        if (mod & KMOD_SHIFT) {
            keyEvent.event.key.keysym.mod = mod | KMOD_SHIFT;
            keyEvent.modifiers |= KB_MOD_SHIFT;
        }
        if (mod & KMOD_CTRL) {
            keyEvent.event.key.keysym.mod = mod | KMOD_CTRL;
            keyEvent.modifiers |= KB_MOD_CTRL;
        }
        if (mod & KMOD_ALT) {
            keyEvent.event.key.keysym.mod = mod | KMOD_ALT;
            keyEvent.modifiers |= KB_MOD_ALT;
        }
        if (mod & KMOD_GUI) {
            keyEvent.event.key.keysym.mod = mod | KMOD_GUI;
        }

        if (ev->key.state == SDL_PRESSED) {
            keyEvent.state = KBS_DOWN;
            if (ev->key.keysym.sym == SDLK_RETURN && (mod & KMOD_ALT)) {
                toggleFullScreen();
                return;
            }
            sshockKeyStates[c] = keyEvent.modifiers | KB_MOD_PRESSED;
            // add to queue regardless to ev.key.keysym.sym. The Great Event Dispatcher sort 'em all.
            // addKBevent(&keyEvent);
            dispatch_kb_event(&keyEvent);
        } else {
            // key up following text input event case below is handled here
            DEBUG("%s: got a keyboard event: %d, %d, ignoring (KEYUP)", __FUNCTION__, keyEvent.event.key.type,
                  keyEvent.event.key.keysym.scancode);
            sshockKeyStates[c] = 0;
        }
    }

    // hack to allow pressing shift after move key
    // sets all current shock states in array to shifted or non-shifted
    if (ev->key.keysym.sym == SDLK_LSHIFT || ev->key.keysym.sym == SDLK_RSHIFT) {
        for (int i = 0; i < 256; i++)
            if (sshockKeyStates[i]) {
                if (ev->key.state == SDL_PRESSED)
                    sshockKeyStates[i] |= KB_MOD_SHIFT;
                else
                    sshockKeyStates[i] &= ~KB_MOD_SHIFT;
            }
    }
}

void pump_ms_button_event(SDL_Event *ev) {
    bool down = (ev->button.state == SDL_PRESSED);
    ss_mouse_event mouseEvent;
    mouseEvent.event = *ev;
    mouseEvent.type = 0;

    // TODO: the old mac code used to emulate right mouse clicks if space, enter, or return
    //       was pressed at the same time - do the same? (=> could check sshockKeyStates[])

    mouseEvent.buttons = 0;

    switch (ev->button.button) {
    case SDL_BUTTON_LEFT:
        mouseEvent.type = down ? MOUSE_LDOWN : MOUSE_LUP;
        mouseEvent.buttons |= down ? (1 << MOUSE_LBUTTON) : 0;
        break;

    case SDL_BUTTON_RIGHT:
        mouseEvent.type = down ? MOUSE_RDOWN : MOUSE_RUP;
        mouseEvent.buttons |= down ? (1 << MOUSE_RBUTTON) : 0;
        break;

    case SDL_BUTTON_MIDDLE:
        mouseEvent.type = down ? MOUSE_CDOWN : MOUSE_CUP;
        mouseEvent.buttons |= down ? (1 << MOUSE_CBUTTON) : 0;
        break;
    }

    if (mouseEvent.type != 0) {
        bool shifted = ((SDL_GetModState() & KMOD_SHIFT) != 0);

        mouseEvent.x = MouseX;
        mouseEvent.y = MouseY;
        mouseEvent.timestamp = mouse_get_time();
        mouseEvent.modifiers = (shifted ? 1 : 0);

        dispatch_ms_event(&mouseEvent);
    }
}

void pump_ms_wheel_event(SDL_Event *ev) {
    if (ev->wheel.y != 0) {
        ss_mouse_event mouseEvent;
        mouseEvent.event = *ev;
        mouseEvent.type = ev->wheel.y < 0 ? MOUSE_WHEELDN : MOUSE_WHEELUP;
        mouseEvent.x = MouseX;
        mouseEvent.y = MouseY;
        mouseEvent.buttons = 0;
        mouseEvent.timestamp = mouse_get_time();

        dispatch_ms_event(&mouseEvent);
    }
}

void pump_ms_motion_event(SDL_Event *ev) {
    // call this first; it sets MouseX and MouseY
    if (SDL_GetRelativeMouseMode() == SDL_TRUE)
        SetMouseXY(MouseX + ev->motion.xrel, MouseY + ev->motion.yrel);
    else
        SetMouseXY(ev->motion.x, ev->motion.y);

    ss_mouse_event mouseEvent;
    mouseEvent.event = *ev;
    mouseEvent.type = MOUSE_MOTION;
    mouseEvent.x = MouseX;
    mouseEvent.y = MouseY;
    mouseEvent.buttons = 0;
    if (ev->motion.state & SDL_BUTTON_LMASK)
        mouseEvent.buttons |= (1 << MOUSE_LBUTTON);
    if (ev->motion.state & SDL_BUTTON_RMASK)
        mouseEvent.buttons |= (1 << MOUSE_RBUTTON);
    mouseEvent.timestamp = mouse_get_time();

    dispatch_ms_event(&mouseEvent);

    if (TriggerRelMouseMode) {
        TriggerRelMouseMode = FALSE;

        SDL_SetRelativeMouseMode(true);
        // throw away this first relative mouse reading
        int mvelx, mvely;
        get_mouselook_vel(&mvelx, &mvely);
    }
}

void pump_events(void) {
    SDL_Event ev;

    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
        case SDL_QUIT:
            // a bit hacky at this place, but this would allow exiting the game via the window's [x] button
            exit(0); // TODO: I guess there is a better way.

        case SDL_KEYUP:
        case SDL_KEYDOWN: {
            pump_kb_event(&ev);
        } break;

        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP: {
            pump_ms_button_event(&ev);
        } break;

        case SDL_MOUSEMOTION: {
            pump_ms_motion_event(&ev);
        } break;

        case SDL_MOUSEWHEEL:
            pump_ms_wheel_event(&ev);
            break;

        case SDL_WINDOWEVENT:
            switch (ev.window.event) {
            case SDL_WINDOWEVENT_SIZE_CHANGED:
                if (can_use_opengl())
                    opengl_resize(ev.window.data1, ev.window.data2);
                break;

            case SDL_WINDOWEVENT_MOVED:
            case SDL_WINDOWEVENT_RESIZED:
                break;

            case SDL_WINDOWEVENT_FOCUS_GAINED:
                SDL_SetRelativeMouseMode(saved_rel_mouse);
                if (saved_rel_mouse == true) {
                    // throw away this first relative mouse reading
                    int mvelx, mvely;
                    get_mouselook_vel(&mvelx, &mvely);
                }
                SDL_ShowCursor(SDL_DISABLE);
                break;

            case SDL_WINDOWEVENT_FOCUS_LOST:
                saved_rel_mouse = SDL_GetRelativeMouseMode();
                SDL_SetRelativeMouseMode(SDL_FALSE);
                SDL_ShowCursor(SDL_ENABLE);
                break;
            }
            break;
        }
    }
}

//===============================================================
//
// This section is adapted from:
// kbMac.c - All the keyboard handling routines that are specific to the Macintosh.
//
//===============================================================

//---------------------------------------------------------------
//  Startup and keyboard handlers and initialize globals.   Shutdown follows.
//---------------------------------------------------------------
int kb_startup(void *v) {
    memset(sshockKeyStates, 0, sizeof(sshockKeyStates));
    return (0);
}

int kb_shutdown(void) { return (0); }

//---------------------------------------------------------------
//  Flush keyboard events from the event queue.
//---------------------------------------------------------------
void kb_flush(void) {
    SDL_FlushEvents(SDL_KEYDOWN, SDL_KEYUP); // Note: that's a range!
}

//---------------------------
//
// MOUSE STUFF
//
//---------------------------

/**
 * @deprecated does nothing
 * @return
 */
errtype mouse_flush(void) {
    // TODO: anything else?
    return OK;
}

errtype mouse_get_xy(short *x, short *y) {
    *x = MouseX;
    *y = MouseY;

    return OK;
}

void middleize_mouse(void) {
    int w, h;
    SDL_RenderGetLogicalSize(renderer, &w, &h);

    MouseX = latestMouseEvent.x = w / 2;
    MouseY = latestMouseEvent.y = h / 2;
}

void get_mouselook_vel(int *vx, int *vy) {
    if (SDL_ShowCursor(SDL_QUERY) == SDL_ENABLE)
        *vx = *vy = 0;
    else {
        SDL_GetRelativeMouseState(vx, vy);

        *vx += MouseChaosX;
        MouseChaosX = 0;
        *vy += MouseChaosY;
        MouseChaosY = 0;
    }
}

errtype mouse_put_xy(short x, short y) {
    MouseX = x;
    MouseY = y;

    return OK;
}

void set_mouse_chaos(short dx, short dy) {
    MouseChaosX = dx;
    MouseChaosY = dy;
}

/**
 * @deprecated does nothing
 */
void sdl_mouse_init(void) {}
