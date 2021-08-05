/*

Copyright (C) 2015-2018 Night Dive Studios, LLC.

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
/*
 * $Source: r:/prj/cit/src/RCS/wrapper.c $
 * $Revision: 1.146 $
 * $Author: dc $
 * $Date: 1994/11/28 06:40:50 $
 */

#include <climits>

#include "Engine/Options.h"

#include "newmfd.h"
#include "wrapper.h"
#include "tools.h"
#include "init.h"
#include "input.h"
#include "invent.h"
#include "invpages.h"
#include "gamescr.h"
#include "mainloop.h"
#include "hkeyfunc.h"
#include "gamewrap.h"
#include "colors.h"
#include "cybstrng.h"
#include "fullscrn.h"
#include "render.h"
#include "gamestrn.h"
#include "gametime.h"
#include "musicai.h"
#include "mfdext.h"
#include "mfdfunc.h"
#include "miscqvar.h"
#include "rendtool.h"
#include "setup.h"
#include "sideicon.h"
#include "sndcall.h"
#include "statics.h"
#include "sfxlist.h"
#include "criterr.h"
#include "gr2ss.h"
#include "player.h"
#include "str.h"
#include "olhext.h"
#include "Xmi.h"

#include "OpenGL.h"

#ifdef AUDIOLOGS
#include "audiolog.h"
#endif

#include "mfdart.h" // for the slider bar

#include "MacTune.h"

#define LOAD_BUTTON 0
#define SAVE_BUTTON 1
#define AUDIO_BUTTON 2
#define INPUT_BUTTON 3
#define OPTIONS_BUTTON 4
#define VIDEO_BUTTON 5
#define RETURN_BUTTON 6
#define QUIT_BUTTON 7
#define AUDIO_OPT_BUTTON 8
#define SCREENMODE_BUTTON 9
#define HEAD_RECENTER_BUTTON 10
#define HEADSET_BUTTON 11

#define MOUSE_DOWN (MOUSE_LDOWN | MOUSE_RDOWN | UI_MOUSE_LDOUBLE)
#define MOUSE_UP (MOUSE_LUP | MOUSE_RUP)
#define MOUSE_LEFT (MOUSE_LDOWN | UI_MOUSE_LDOUBLE)
#define MOUSE_WHEEL (MOUSE_WHEELUP | MOUSE_WHEELDN)

#define STATUS_X 4
#define STATUS_Y 1
#define STATUS_HEIGHT 20
#define STATUS_WIDTH 312

LGCursor option_cursor;
grs_bitmap option_cursor_bmap;

int wrap_id = -1, wrapper_wid, wrap_key_id;
uchar clear_panel = TRUE, wrapper_panel_on = FALSE;
grs_font *opt_font;
uchar olh_temp;
static bool digi_gain = true; // enable sfx volume slider
errtype (*wrapper_cb)(int num_clicked);
errtype (*slot_callback)(int num_clicked);
static uchar cursor_loaded = FALSE;

errtype music_slots();
errtype wrapper_do_save();
errtype wrapper_panel_close(uchar clear_message);
errtype do_savegame_guts(uchar slot);
void quit_verify_pushbutton_handler(uchar butid);
uchar quit_verify_slorker(uchar butid);
void save_verify_pushbutton_handler(uchar butid);
uchar save_verify_slorker(uchar butid);
void free_options_cursor(void);

void input_screen_init(void);
void joystick_screen_init(void);
void sound_screen_init(void);
void soundopt_screen_init(void);
void video_screen_init(void);

uint multi_get_curval(uchar type, void *p);
void multi_set_curval(uchar type, void *p, uint val, void (*deal)(ushort));

void options_screen_init(void);
void wrapper_init(void);
void load_screen_init(void);
void save_screen_init(void);

void draw_button(uchar butid);

#define SLOTNAME_HEIGHT 6
#define PANEL_MARGIN_Y 3
#define WRAPPER_PANEL_HEIGHT (INVENTORY_PANEL_HEIGHT - 2 * PANEL_MARGIN_Y)

#define OPTIONS_FONT RES_tinyTechFont

errtype (*verify_callback)(int num_clicked) = NULL;
char savegame_verify;
char comments[NUM_SAVE_SLOTS + 1][SAVE_COMMENT_LEN];
uchar pause_game_func(ushort keycode, uint32_t context, intptr_t data);
uchar really_quit_key_func(ushort keycode, uint32_t context, intptr_t data);

// separate mouse region for regular-screen and fullscreen.
#define NUM_MOUSEREGION_SCREENS 2
LGRegion options_mouseregion[NUM_MOUSEREGION_SCREENS];
uchar free_mouseregion = 0;

char save_game_name[] = "savgam00.dat";

#define FULL_BACK_X (GAME_MESSAGE_X - INVENTORY_PANEL_X)
#define FULL_BACK_Y (GAME_MESSAGE_Y - INVENTORY_PANEL_Y)

#define BUTTON_COLOR GREEN_BASE + 2
#define BUTTON_SHADOW 7

// SLIDER WIDGETS:
// structure for slider widget.  The slider has a pointer to a uchar,
// ushort, or uint, which it sets to a value in the range [0,maxval].
// It recalculates this value based on interpolation from the actual
// size of the slider.  The function dealfunc (if not NULL) is called
// when the value changes, and is passed the new value.  The value is
// updated continuously if smooth==TRUE; otherwise it is updated upon
// mouse-up.
//
typedef struct {
    uchar color;
    uchar bvalcol;
    uchar sliderpos;
    uchar active;
    Ref descrip;
    uint maxval;
    uchar baseval;
    uchar type;
    uchar smooth;
    void *curval;
    void (*dealfunc)(ushort val);
} opt_slider_state;

/**
 * Pushbutton widgets, the simplest widgets. Calls pushfunc, passing in its
 * own button ID, upon mouse left-click upon it, or on a keyboard event
 * corresponding to keyeq.
 */
typedef struct {
    /// Hotkey binded to button (usually first key of descrip)
    SDL_Scancode keyeq;
    /// Description of button (caption)
    Ref descrip;
    /// Foreground color
    uchar fcolor;
    /// Shadow color
    uchar shadow;
    /// Function callback on mouse or keyboard event
    void (*pushfunc)(uchar butid);
} opt_pushbutton_state;

// MULTI_STATE WIDGETS:
// these are much like pushbuttons, but also have a pointer to a uchar,
// ushort, or uint, which takes on a value in the range [0,num_opts-1].
// The button is labelled both with its description string (descrip) and
// with a string offset from optbase by an amount equal to the current
// value of its associated variable.  Whenever its value changes, it calls
// dealfunc, and message-lines a string offset from feedbackbase by an
// amount equal to its current value.
//
typedef struct {
    SDL_Scancode keyeq;
    uchar type;
    uchar num_opts;
    Ref optbase;
    Ref descrip;
    Ref feedbackbase;
    void *curval;
    void (*dealfunc)(ushort val);
} opt_multi_state;

// TEXT WIDGET
// nothing but a piece of text, folks.  No handler, simple draw func.
//
typedef struct {
    Ref descrip;
    uchar color;
} opt_text_state;

// TEXTLIST WIDGET
// used for editing and selecting (and responding to the editing and
// selecting of) a list of text strings.  You provide a block of text,
// which is assumed to be a 2-D array of chars (you inform the widget
// of the dimension of the subarrays).  The widget may either be edit-
// allowing or not.  If not, then it calls its dealfunc whenever a
// text string is selected (by mouse-clicking on it or by using the
// keyboard to move the highlight to it and hitting ENTER).  If the
// strings are editable, it calls its dealfunc only when you are done
// selecting and editing one.  A mask may be provided of what entries
// on the list are valid candidates for selection, and a string resource
// is given to display in the place of uninitialized selections.  Different
// colors are provided for selectable text, currently selected text,
// and non-selectable text.  Note that the user is responsible for
// providing space for one more line of text than the widget uses, for
// the purposes of saving string information.
//
typedef struct {
    char *text;
    uchar numblocks;
    uchar blocksiz;

    char currstring;
    char index;
    uchar modified;

    uchar editable;
    ushort editmask;
    ushort selectmask;
    ushort initmask;
    Ref invalidstr;

    Ref selectprompt;

    uchar validcol;
    uchar selectcol;
    uchar invalidcol;

    void (*dealfunc)(uchar butid, uchar index);
} opt_textlist_state;

// SLORKER WIDGET
// used to implement default actions in the keyboard interface to options
// screens, slorker widgets respond to no mouse events, but will respond
// to any keyboard events which actually reach them by calling their function
// with their button id as an argument.  Thus, any keypress which is not
// handled by another gadget is taken by the slorker.
//
typedef uchar (*slorker)(uchar butid);

typedef struct {
    LGRect rect;
    union {
        opt_slider_state slider_st;
        opt_pushbutton_state pushbutton_st;
        opt_text_state text_st;
        opt_multi_state multi_st;
        opt_textlist_state textlist_st;
        slorker sl;
    } user;
    ulong evmask;
    void (*drawfunc)(uchar butid);
    uchar (*handler)(uiEvent *ev, uchar butid);
} opt_button;

void verify_screen_init(void (*verify)(uchar butid), slorker slork);
// void verify_screen_init(void (*verify)(uchar butid), void (*slork)(uchar butid));

#define OPT_SLIDER_BAR REF_IMG_BeamSetting

#define MAX_OPTION_BUTTONS 12
#define BR(i) (OButtons[i].rect)

#ifdef STATIC_BUTTON_STORE
opt_button OButtons[MAX_OPTION_BUTTONS];
#else
opt_button *OButtons;
uchar fv;
#endif

#define OPTIONS_COLOR (RED_BROWN_BASE + 4)

// decides on a "standard" width for our widgets based on column count
// of current screen.  Our desire is that uniform widgets of this size
// should have certain margins between them independent of column count.
#define CONSTANT_MARGINS

#ifdef HALF_BUTTON_MARGINS
#define widget_width(t, m) (2 * INVENTORY_PANEL_WIDTH / (3 * (t) + 1))
#define widget_x(c, t, m) ((3 * (t) + 1) * INVENTORY_PANEL_WIDTH / (3 * (t) + 1))
#endif
#ifdef CONSTANT_MARGINS
#define widget_width(t, m) ((INVENTORY_PANEL_WIDTH - ((m) * ((t) + 1))) / (t))
#define widget_x(c, t, m) ((m) * ((c) + 1) + widget_width(t, m) * (c))
#endif

// override get_temp_string() to support hard-coded custom strings without
// providing an actual resource file

#define MIDI_OUT_STR_SIZE 1024
static char MIDI_STR_BUFFER[MIDI_OUT_STR_SIZE];

static char *_get_temp_string(int num) {
    switch (num) {
    case REF_STR_Renderer:
        return "Renderer";
    case REF_STR_Software:
        return "Software";
    case REF_STR_OpenGL:
        return "OpenGL";

    case REF_STR_TextFilt:
        return "Tex Filter";
    case REF_STR_TFUnfil:
        return "Unfiltered";
    case REF_STR_TFBilin:
        return "Bilinear";

    case REF_STR_MousLook:
        return "Mouselook";
    case REF_STR_MousNorm:
        return "Normal";
    case REF_STR_MousInv:
        return "Inverted";

    case REF_STR_Seqer:
        return "Midi Player";
    case REF_STR_ADLMIDI:
        return "ADLMIDI";
    case REF_STR_NativeMI:
        return "Native MIDI";
#ifdef USE_FLUIDSYNTH
    case REF_STR_FluidSyn:
        return "FluidSynth";
#endif

    case REF_STR_MidiOut:
        return "Midi Output";
    }

    if (num >= REF_STR_MidiOutX && num <= (REF_STR_MidiOutX | 0x0fffffff)) {
        const unsigned int midiOutputIndex = (unsigned int)num - REF_STR_MidiOutX;
        MIDI_STR_BUFFER[0] = '\0';
        GetOutputNameXMI(midiOutputIndex, &MIDI_STR_BUFFER[0], MIDI_OUT_STR_SIZE);
        return &MIDI_STR_BUFFER[0];
    }

    return get_temp_string(num);
}

#define get_temp_string _get_temp_string

//#ifdef NOT_YET //

void draw_button(uchar butid) {
    if (OButtons[butid].drawfunc) {
#ifdef SVGA_SUPPORT
        uchar old_over;
        old_over = gr2ss_override;
        gr2ss_override = OVERRIDE_ALL;
#endif
        uiHideMouse(NULL);
        gr_push_canvas(&inv_norm_canvas);
        gr_set_font(opt_font);
        OButtons[butid].drawfunc(butid);
        gr_pop_canvas();
        uiShowMouse(NULL);
#ifdef GR2SS_OVERRIDE
        gr2ss_override = old_over;
#endif
    }
}

void wrapper_draw_background(short ulx, short uly, short lrx, short lry) {
    short cx1, cx2, cy1, cy2;
    short a1, a2, a3, a4;

#ifdef SVGA_SUPPORT
    uchar old_over;
    old_over = gr2ss_override;
    gr2ss_override = OVERRIDE_ALL;
#endif
    // draw background behind the slider.
    STORE_CLIP(cx1, cy1, cx2, cy2);
    ss_safe_set_cliprect(ulx, uly, lrx, lry);
    if (full_game_3d) {
        //      gr_bitmap(&inv_view360_canvas.bm,FULL_BACK_X,FULL_BACK_Y);
        gr_get_cliprect(&a1, &a2, &a3, &a4);
        ss_noscale_bitmap(&inv_view360_canvas.bm, FULL_BACK_X, FULL_BACK_Y);
    } else
        ss_bitmap(&inv_backgnd, 0, 0);
    RESTORE_CLIP(cx1, cy1, cx2, cy2);
#ifdef SVGA_SUPPORT
    gr2ss_override = old_over;
#endif
}

void slider_draw_func(uchar butid) {
    opt_slider_state *st = &(OButtons[butid].user.slider_st);
    short w, h;
    char *title;

#ifdef SVGA_SUPPORT
    uchar old_over;
    old_over = gr2ss_override;
    gr2ss_override = OVERRIDE_ALL;
#endif

    short sw = res_bm_size(OPT_SLIDER_BAR).x;
    gr_set_fcolor(st->color);
    title = get_temp_string(st->descrip);
    gr_string_size(title, &w, &h);

    // draw background behind the slider
    wrapper_draw_background(BR(butid).ul.x - sw / 2, BR(butid).ul.y - h, BR(butid).lr.x + sw / 2, BR(butid).lr.y);
    draw_shadowed_string(title, BR(butid).ul.x, BR(butid).ul.y - h, full_game_3d);

    gr_set_fcolor(st->bvalcol);
    ss_vline(BR(butid).ul.x + st->baseval, BR(butid).ul.y, BR(butid).lr.y - 1);

    gr_set_fcolor(st->color);
    ss_box(BR(butid).ul.x, BR(butid).ul.y, BR(butid).lr.x, BR(butid).lr.y);

    if (!(st->active))
        draw_raw_resource_bm(OPT_SLIDER_BAR, BR(butid).ul.x + st->sliderpos + 1 - sw / 2, BR(butid).ul.y);

#ifdef SVGA_SUPPORT
    gr2ss_override = old_over;
#endif
}

void slider_deal(uchar butid, uchar deal) {
    opt_slider_state *st = &(OButtons[butid].user.slider_st);
    uint val;

    deal = deal || st->smooth;

    val = (st->sliderpos * (st->maxval + 1)) / (BR(butid).lr.x - BR(butid).ul.x - 3);
    if (val > st->maxval)
        val = st->maxval;

    multi_set_curval(st->type, st->curval, val, deal ? st->dealfunc : NULL);
}

//
// every time you find yourself,
// you lose a little bit of me, from within
//

uchar slider_handler(uiEvent *ev, uchar butid) {
    opt_slider_state *st = &(OButtons[butid].user.slider_st);

    switch (ev->type) {
    case UI_EVENT_MOUSE_MOVE:
        if (ev->mouse_data.buttons) {
            st->sliderpos = ev->pos.x - BR(butid).ul.x;
            slider_deal(butid, TRUE);
            draw_button(butid);
        }
        break;
    case UI_EVENT_MOUSE:
        if (ev->mouse_data.action & MOUSE_WHEELUP) {
            st->sliderpos = st->sliderpos <= 5 ? 0 : st->sliderpos - 5;
        } else if (ev->mouse_data.action & MOUSE_WHEELDN) {
            uchar max = BR(butid).lr.x - BR(butid).ul.x - 3;
            st->sliderpos = lg_min(st->sliderpos + 5, max);
        } else {
            st->sliderpos = ev->pos.x - BR(butid).ul.x;
        }
        slider_deal(butid, TRUE);
        draw_button(butid);
        return TRUE;
    default:
        break;
    }
    return FALSE;
}

void slider_init(uchar butid, Ref descrip, uchar type, uchar smooth, void *var, uint maxval, uchar baseval,
                 void (*dealfunc)(ushort val), LGRect *r) {
    opt_slider_state *st = &OButtons[butid].user.slider_st;
    uint val;

    if (maxval) {
        val = ((r->lr.x - r->ul.x - 3) * multi_get_curval(type, var)) / maxval;
    } else {
        // just put it in the middle
        val = (r->lr.x - r->ul.x - 3) / 2;
    }

    st->color = BUTTON_COLOR;
    st->bvalcol = GREEN_YELLOW_BASE + 1;
    st->sliderpos = val;
    st->baseval = baseval;
    st->maxval = maxval;
    st->active = FALSE;
    st->descrip = descrip;
    st->type = type;
    // note that in these settings, we don't care what size of
    // variable we're dealing with, 'cause we secretly know that
    // all pointers are represented the same and we don't
    // have to actually dereference these.
    st->dealfunc = dealfunc;
    st->curval = var;
    st->smooth = smooth;

    OButtons[butid].evmask = UI_EVENT_MOUSE | UI_EVENT_MOUSE_MOVE;
    OButtons[butid].drawfunc = slider_draw_func;
    OButtons[butid].handler = slider_handler;
    OButtons[butid].rect = *r;
}

void pushbutton_draw_func(uchar butid) {
    char *btext;
    short w, h;
    opt_pushbutton_state *st = &OButtons[butid].user.pushbutton_st;

    w = BR(butid).lr.x - BR(butid).ul.x;
    h = BR(butid).lr.y - BR(butid).ul.y;

    btext = get_temp_string(st->descrip);
    gr_string_wrap(btext, BR(butid).lr.x - BR(butid).ul.x - 3);
    text_button(btext, BR(butid).ul.x, BR(butid).ul.y, st->fcolor, st->shadow, -w, -h);
    gr_font_string_unwrap(btext);
}

uchar pushbutton_handler(uiEvent *ev, uchar butid) {
    if (((ev->type == UI_EVENT_MOUSE) && (ev->subtype & MOUSE_DOWN)) ||
        ((ev->sdl_data.type == SDL_KEYDOWN) &&
         (ev->sdl_data.key.keysym.scancode == OButtons[butid].user.pushbutton_st.keyeq))) {
        OButtons[butid].user.pushbutton_st.pushfunc(butid);
        return TRUE;
    }
    return FALSE;
}

/**
 * Initialize button
 * @param butid id of button
 * @param keyeq code of hotkey (usually this is first letter of descrip)
 * @param descrip description (caption) of button
 * @param pushfunc function callback bounded to button
 * @param r rectangle that represents coordinates of button
 */
void pushbutton_init(uchar butid, SDL_Scancode keyeq, Ref descrip, void (*pushfunc)(uchar butid), LGRect *r) {
    opt_pushbutton_state *st = &OButtons[butid].user.pushbutton_st;

    OButtons[butid].rect = *r;
    OButtons[butid].evmask = UI_EVENT_MOUSE | UI_EVENT_KBD_COOKED;
    OButtons[butid].drawfunc = pushbutton_draw_func;
    OButtons[butid].handler = pushbutton_handler;
    st->fcolor = BUTTON_COLOR;
    st->shadow = BUTTON_SHADOW;
    st->keyeq = keyeq;
    st->descrip = descrip;
    st->pushfunc = pushfunc;
}

void dim_pushbutton(uchar butid) {
    opt_pushbutton_state *st = &OButtons[butid].user.pushbutton_st;
    OButtons[butid].evmask = 0;
    st->fcolor += 4;
    st->shadow -= 3;
}

void bright_pushbutton(uchar butid) {
    opt_pushbutton_state *st = &OButtons[butid].user.pushbutton_st;
    OButtons[butid].evmask = 0;
    st->fcolor -= 2;
    st->shadow += 2;
}

// text widget
void text_draw_func(uchar butid) {
    opt_text_state *st = &OButtons[butid].user.text_st;
    char *s = get_temp_string(st->descrip);

    gr_string_wrap(s, BR(butid).lr.x - BR(butid).ul.x);
    gr_set_fcolor(st->color);
    draw_shadowed_string(s, BR(butid).ul.x, BR(butid).ul.y, full_game_3d);
    gr_font_string_unwrap(s);
}

void textwidget_init(uchar butid, uchar color, Ref descrip, LGRect *r) {
    opt_text_state *st = &OButtons[butid].user.text_st;

    OButtons[butid].rect = *r;
    st->descrip = descrip;
    st->color = color;
    OButtons[butid].drawfunc = text_draw_func;
    OButtons[butid].handler = NULL;
    OButtons[butid].evmask = 0;
}

/**
 * Initialize keywidget. A keywidget is just like a pushbutton, but invisible.
 * @param butid id of button
 * @param keyeq code of hotkey (usually this is first letter of descrip)
 * @param pushfunc function callback bounded to button
 */
void keywidget_init(uchar butid, SDL_Scancode keyeq, void (*pushfunc)(uchar butid)) {
    opt_pushbutton_state *st = &OButtons[butid].user.pushbutton_st;

    OButtons[butid].evmask = UI_EVENT_KBD_COOKED;
    OButtons[butid].drawfunc = NULL;
    OButtons[butid].handler = pushbutton_handler;
    st->keyeq = keyeq;
    st->pushfunc = pushfunc;
}

// gets the current "value" of a multi-option widget, whatever size
// thing that may be.
uint multi_get_curval(uchar type, void *p) {
    uint val = 0;

    switch (type) {
    case sizeof(uchar):
        val = *((uchar *)p);
        break;
    case sizeof(ushort):
        val = *((ushort *)p);
        break;
    case sizeof(uint):
        val = *((uint *)p);
        break;
    }
    return val;
}

// sets the current value pointed to by a multi-option widget.
void multi_set_curval(uchar type, void *p, uint val, void (*deal)(ushort)) {
    switch (type) {
    case sizeof(uchar):
        *((uchar *)p) = (uchar)val;
        if (deal)
            ((void (*)(uchar))deal)((uchar)val);
        break;
    case sizeof(ushort):
        *((ushort *)p) = (ushort)val;
        if (deal)
            ((void (*)(ushort))deal)((ushort)val);
        break;
    case sizeof(uint):
        *((uint *)p) = (uint)val;
        if (deal)
            ((void (*)(uint))deal)((uint)val);
        break;
    }
}

void multi_draw_func(uchar butid) {
    char *btext;
    short w, h, x, y;
    uint val = 0;
    opt_multi_state *st = &OButtons[butid].user.multi_st;

    gr_set_fcolor(BUTTON_COLOR);
    ss_rect(BR(butid).ul.x, BR(butid).ul.y, BR(butid).lr.x, BR(butid).lr.y);
    gr_set_fcolor(BUTTON_COLOR + BUTTON_SHADOW);
    ss_rect(BR(butid).ul.x + 1, BR(butid).ul.y + 1, BR(butid).lr.x - 1, BR(butid).lr.y - 1);
    gr_set_fcolor(BUTTON_COLOR);
    x = (BR(butid).lr.x + BR(butid).ul.x) / 2;
    y = (BR(butid).lr.y + BR(butid).ul.y) / 2;
    btext = get_temp_string(st->descrip);
    gr_string_size(btext, &w, &h);
    ss_string(btext, x - w / 2, y - h);
    val = multi_get_curval(st->type, st->curval);
    btext = get_temp_string(st->optbase + val);
    gr_string_size(btext, &w, &h);
    ss_string(btext, x - w / 2, y);
}

uchar multi_handler(uiEvent *ev, uchar butid) {
    uint val = 0, delta = 0;
    opt_multi_state *st = &OButtons[butid].user.multi_st;

    if (ev->type == UI_EVENT_MOUSE) {
        if (ev->subtype & MOUSE_LEFT)
            delta = 1;
        else if (ev->subtype & MOUSE_RDOWN)
            delta = st->num_opts - 1;
    } else if (ev->type == UI_EVENT_KBD_COOKED) {
        short code = ev->cooked_key_data.code;
        if (tolower(code & 0xFF) == st->keyeq) {
            if (isupper(code & 0xFF))
                delta = st->num_opts - 1;
            else
                delta = 1;
        }
    }

    if (delta) {
        val = multi_get_curval(st->type, st->curval);
        val = (val + delta) % (st->num_opts);
        multi_set_curval(st->type, st->curval, val, st->dealfunc);
        draw_button(butid);
        if (st->feedbackbase) {
            string_message_info(st->feedbackbase + val);
        }
        return TRUE;
    }
    return FALSE;
}

void multi_init(uchar butid, uchar key, Ref descrip, Ref optbase, Ref feedbase, uchar type, void *var, uchar num_opts,
                void (*dealfunc)(ushort val), LGRect *r) {
    opt_multi_state *st = &OButtons[butid].user.multi_st;

    OButtons[butid].rect = *r;
    OButtons[butid].drawfunc = multi_draw_func;
    OButtons[butid].handler = multi_handler;
    OButtons[butid].evmask = UI_EVENT_MOUSE | UI_EVENT_KBD_COOKED;
    st->descrip = descrip;
    st->optbase = optbase;
    st->feedbackbase = feedbase;
    st->type = type;
    st->keyeq = static_cast<SDL_Scancode>(key);
    st->num_opts = num_opts;
    // note that in these settings, we don't care what size of
    // variable we're dealing with, 'cause we secretly know that
    // all pointers are represented the same and we don't
    // have to actually dereference these.
    st->dealfunc = dealfunc;
    st->curval = var;
}

uchar keyslork_handler(uiEvent *ev, uchar butid) {
    slorker *slork = &OButtons[butid].user.sl;

    return ((*slork)(butid));
}

void slork_init(uchar butid, slorker slork) {
    memset(&OButtons[butid].rect, 0, sizeof(LGRect));
    OButtons[butid].user.sl = slork;
    OButtons[butid].evmask = UI_EVENT_KBD_COOKED;
    OButtons[butid].drawfunc = NULL;
    OButtons[butid].handler = keyslork_handler;
}

char *textlist_string(opt_textlist_state *st, int ind) { return (st->text + ind * (st->blocksiz)); }

void textlist_draw_line(opt_textlist_state *st, int line, uchar butid) {
    short w, h;
    LGRect scrrect;
    LGRect r;
    char *s;
    uchar col;
#ifdef SVGA_SUPPORT
    uchar old_over;
#endif

    scrrect = BR(butid);
    scrrect.ul.x += INVENTORY_PANEL_X;
    scrrect.ul.y += INVENTORY_PANEL_Y;
    scrrect.lr.x += INVENTORY_PANEL_X;
    scrrect.lr.y += INVENTORY_PANEL_Y;

    if (((1 << line) & (st->initmask)) || (line == st->currstring && st->index >= 0))
        s = textlist_string(st, line);
    else
        s = get_temp_string(st->invalidstr);

    if (line == st->currstring)
        col = st->selectcol;
    else if (st->selectmask & (1 << line))
        col = st->validcol;
    else
        col = st->invalidcol;
    gr_push_canvas(&inv_norm_canvas);
    gr_set_fcolor(col);

    gr_set_font(opt_font);
    gr_string_size(s, &w, &h);
    r.ul.x = BR(butid).ul.x;
    r.ul.y = BR(butid).ul.y + h * line;
    r.lr.x = BR(butid).lr.x;
    r.lr.y = r.ul.y + h;

    uiHideMouse(&scrrect);
#ifdef SVGA_SUPPORT
    old_over = gr2ss_override;
    gr2ss_override = OVERRIDE_ALL;
#endif
    wrapper_draw_background(r.ul.x, r.ul.y, r.lr.x, r.lr.y);
    draw_shadowed_string(s, r.ul.x, r.ul.y, full_game_3d);
#ifdef SVGA_SUPPORT
    gr2ss_override = old_over;
#endif
    uiShowMouse(&scrrect);
    gr_pop_canvas();
}

void textlist_draw_func(uchar butid) {
    int i;
    opt_textlist_state *st = &OButtons[butid].user.textlist_st;

    for (i = 0; i < st->numblocks; i++) {
        textlist_draw_line(st, i, butid);
    }
}

void textlist_cleanup(opt_textlist_state *st) {
    if (st->editable && st->currstring >= 0 && st->index >= 0) {
        strcpy(textlist_string(st, st->currstring), textlist_string(st, st->numblocks));
        st->index = -1;
    }
}

#ifdef WE_USED_THIS
void textlist_edit_line(opt_textlist_state *st, uchar butid, uchar line, uchar end) {
    char *s, *bak;
    char tmp;

    gr_push_canvas(&inv_norm_canvas);
    s = textlist_string(st, line);
    bak = textlist_string(st, st->numblocks);
    tmp = st->currstring;
    st->currstring = line;
    if (tmp >= 0) {
        strcpy(textlist_string(st, tmp), bak);
        textlist_draw_line(st, tmp, butid);
    }
    strcpy(bak, s);
    st->index = end ? strlen(s) : 0;
    s[0] = '\0';
    textlist_draw_line(st, line, butid);
    gr_pop_canvas();
}
#endif

void textlist_select_line(opt_textlist_state *st, uchar butid, uchar line, uchar deal) {
    char tmp;

    gr_push_canvas(&inv_norm_canvas);
    tmp = st->currstring;
    st->currstring = line;
    st->index = -1;
    if (tmp >= 0)
        textlist_draw_line(st, tmp, butid);
    textlist_draw_line(st, line, butid);
    gr_pop_canvas();
    if (deal)
        st->dealfunc(butid, line);
}

uchar textlist_handler(uiEvent *ev, uchar butid) {
    uchar line;
    opt_textlist_state *st = &OButtons[butid].user.textlist_st;

    if ((ev->type == UI_EVENT_MOUSE) && (ev->subtype & MOUSE_DOWN)) {
        short w, h;

        gr_set_font(opt_font);
        gr_char_size('X', &w, &h);

        line = (ev->pos.y - BR(butid).ul.y) / h;

        if (st->editable && (st->editmask & (1 << line))) {
            // this is how you would do this if you wanted right-click to select
            // a line w/ confirm, which would be the right thing to do, instead
            // of confirm without selection, which is what Harvey at Origin wants.
            //
            //          if(st->selectprompt)
            //             textlist_select_line(st,butid,line,FALSE);
            //          textlist_select_line(st,butid,line,(ev->subtype&MOUSE_RDOWN)!=0);
            //
            if (ev->subtype & MOUSE_RDOWN) {
                if (st->currstring >= 0)
                    st->dealfunc(butid, st->currstring);
            } else if (!st->modified) {
                string_message_info(st->selectprompt);
                if (st->selectprompt)
                    textlist_select_line(st, butid, line, FALSE);
            }
        } else if (st->selectmask & (1 << line)) {
            textlist_select_line(st, butid, line, TRUE);
        }
        return TRUE;
    } else if (ev->sdl_data.type == SDL_KEYDOWN) {
        SDL_Keysym key = ev->sdl_data.key.keysym;

        // FIXME It's Unicode, we need to use wchar.h stuff here
        char k = ev->sdl_data.key.keysym.sym;
        // uint keycode = code & ~KB_FLAG_DOWN;
        // uchar special = ((code & KB_FLAG_SPECIAL) != 0);
        char *s;
        char upness = 0;
        char cur = st->currstring;

        // explicitly do not deal with alt-x, but leave
        // it to more capable hands.
        if (key.scancode == SDL_SCANCODE_X && (key.mod & KMOD_ALT))
            return FALSE;
        if (cur >= 0)
            s = textlist_string(st, cur);
        if (st->editable && cur >= 0 && /* !special && */ isprint(k)) {
            if (st->index < 0) {
                strcpy(textlist_string(st, st->numblocks), textlist_string(st, st->currstring));
                st->index = 0;
            }
            if (st->index + 1 < st->blocksiz) {
                s[st->index] = k;
                st->index++;
                s[st->index] = '\0';
                textlist_draw_line(st, cur, butid);
            }
            st->modified = TRUE;
            return TRUE;
        }
        switch (key.scancode) {
        case SDL_SCANCODE_BACKSPACE:
            if (st->editable && cur >= 0) {
                if (st->index < 0) {
                    strcpy(textlist_string(st, st->numblocks), textlist_string(st, st->currstring));
                    st->index = strlen(s);
                }
                if (st->index > 0)
                    st->index--;
                s[st->index] = '\0';
                textlist_draw_line(st, cur, butid);
            }
            break;
        case SDL_SCANCODE_UP:
            upness = st->numblocks - 1;
            break;
        case SDL_SCANCODE_DOWN:
            upness = 1;
            break;
        case SDL_SCANCODE_RETURN:
            if (st->currstring >= 0) {
                st->dealfunc(butid, cur);
                return TRUE;
            }
            break;
        case SDL_SCANCODE_ESCAPE:
            // on ESC, clean up but pass the event through.
            textlist_cleanup(st);
            wrapper_panel_close(TRUE);
            return FALSE;
        default:
            break;
        }
        if (upness != 0) {
            char newstring;
            uchar safety = 0;

            newstring = cur;
            if (newstring < 0)
                newstring = (upness == 1) ? st->numblocks - 1 : 0;
            do {
                newstring = (newstring + upness) % st->numblocks;
                safety++;
            } while (safety < st->numblocks && !((1 << newstring) & st->selectmask));
            if (safety >= st->numblocks)
                newstring = cur;
            if (newstring != cur) {
                textlist_cleanup(st);
                st->currstring = newstring;
                if (cur >= 0 && cur < st->numblocks)
                    textlist_draw_line(st, cur, butid);
                textlist_draw_line(st, newstring, butid);
            }
        }
        return TRUE;
    }
    return TRUE;
}

void textlist_init(uchar butid, char *text, uchar numblocks, uchar blocksiz, uchar editable, ushort editmask,
                   ushort selectmask, ushort initmask, Ref invalidstr, uchar validcol, uchar selectcol,
                   uchar invalidcol, Ref selectprompt, void (*dealfunc)(uchar butid, uchar index), LGRect *r) {
    opt_textlist_state *st = &OButtons[butid].user.textlist_st;

    if (r == NULL) {
        BR(butid).ul.x = 2;
        BR(butid).ul.y = 2;
        BR(butid).lr.x = INVENTORY_PANEL_WIDTH;
        BR(butid).lr.y = INVENTORY_PANEL_HEIGHT;
    } else
        OButtons[butid].rect = *r;
    OButtons[butid].drawfunc = textlist_draw_func;
    OButtons[butid].handler = textlist_handler;
    OButtons[butid].evmask = UI_EVENT_MOUSE | UI_EVENT_KBD_COOKED;
    st->text = text;
    st->numblocks = numblocks;
    st->blocksiz = blocksiz;
    st->editable = editable;
    st->editmask = editmask;
    st->selectmask = selectmask;
    st->initmask = initmask;
    st->invalidstr = invalidstr;
    st->validcol = validcol;
    st->selectcol = selectcol;
    st->invalidcol = invalidcol;
    st->dealfunc = dealfunc;
    st->selectprompt = selectprompt;

    st->currstring = -1;
    st->index = -1;
    st->modified = FALSE;
}

// One, true mouse handler for all options panel mouse events.
// checks all options panel widgets which enclose point of mouse
// event to see if they want to deal with it.
//
uchar opanel_mouse_handler(uiEvent *ev, LGRegion *r, intptr_t user_data) {
    int b;
    uiEvent mev = *ev;

    if (!(ev->type & (UI_EVENT_MOUSE | UI_EVENT_MOUSE_MOVE)))
        return FALSE;
    if (ev->type == UI_EVENT_MOUSE && !(ev->subtype & (MOUSE_DOWN | MOUSE_UP | MOUSE_WHEEL)))
        return FALSE;

    mev.pos.x -= inventory_region->r->ul.x;
    mev.pos.y -= inventory_region->r->ul.y;

    for (b = 0; b < MAX_OPTION_BUTTONS; b++) {
        if (RECT_TEST_PT(&BR(b), mev.pos) && (ev->type & OButtons[b].evmask)) {
            if (OButtons[b].handler && OButtons[b].handler((uiEvent *)(&mev), b))
                return TRUE;
        }
    }
    return TRUE;
}

// One, true keyboard handler for all options mode events.
// checks all options panel widgets to see if they want to deal.
//
uchar opanel_kb_handler(uiEvent *ev, LGRegion *r, intptr_t user_data) {
    if (ev->sdl_data.type == SDL_KEYDOWN) {
        for (int b = 0; b < MAX_OPTION_BUTTONS; b++) {
            if ((ev->type & OButtons[b].evmask) && OButtons[b].handler && OButtons[b].handler(ev, b))
                return TRUE;
        }
        // if no-one else has hooked SDL_SCANCODE_ESCAPE, it defaults to closing the wrapper panel.
        if (ev->sdl_data.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
            wrapper_panel_close(TRUE);
        return TRUE;
    }
    return TRUE;
}

extern uiSlab *uiCurrentSlab;
void clear_obuttons() {
    uiCursorStack *cs;

    uiGetSlabCursorStack(uiCurrentSlab, &cs);
    uiPopCursorEvery(cs, &slider_cursor);
    mouse_unconstrain();
    memset(OButtons, 0, MAX_OPTION_BUTTONS * sizeof(opt_button));
}

void opanel_redraw(uchar back) {
    int but;
    LGRect r = {{INVENTORY_PANEL_X, INVENTORY_PANEL_Y},
                {INVENTORY_PANEL_X + INVENTORY_PANEL_WIDTH, INVENTORY_PANEL_Y + INVENTORY_PANEL_HEIGHT}};
#ifdef SVGA_SUPPORT
    uchar old_over = gr2ss_override;
    gr2ss_override = OVERRIDE_ALL; // Since we are really going straight to screen in our heart of hearts
#endif
    if (!full_game_3d)
        inventory_clear();
    gr_push_canvas(&inv_norm_canvas);
    uiHideMouse(NULL);
    gr_set_font(opt_font);
    if (back) {
        if (full_game_3d)
            ss_noscale_bitmap(&inv_view360_canvas.bm, FULL_BACK_X, FULL_BACK_Y);
        else
            ss_bitmap(&inv_backgnd, 0, 0);
    }

    for (but = 0; but < MAX_OPTION_BUTTONS; but++) {
        if (OButtons[but].drawfunc) {
            OButtons[but].drawfunc(but);
        }
    }
    uiShowMouse(&r);
    gr_pop_canvas();
#ifdef SVGA_SUPPORT
    gr2ss_override = old_over;
#endif
}

// fills in the Rect r with one of the "standard" button rects,
// assuming buttons in three columns, ro rows, high enough for
// a specified number of lines of text.
//
void standard_button_rect(LGRect *r, uchar butid, uchar lines, uchar ro, uchar mar) {
    short w, h;
    char i = butid;

    gr_set_font(opt_font);
    gr_string_size("X", &w, &h);

    h *= lines;

    r->ul.x = widget_x(i % 3, 3, mar);
    r->lr.x = r->ul.x + widget_width(3, mar);
    r->ul.y = INVENTORY_PANEL_HEIGHT * (i / 3 + 1) / (ro + 1) - h / 2;
    if (ro > 2)
        r->ul.y += (3 * ((i / 3) - 1));
    r->lr.y = r->ul.y + h + 2;
}

void standard_slider_rect(LGRect *r, uchar butid, uchar ro, uchar mar) {
    standard_button_rect(r, butid, 2, ro, mar);
    LGPoint size = res_bm_size(OPT_SLIDER_BAR);

    r->ul.x += size.x / 2;
    r->lr.x -= size.x / 2;
    r->ul.y = r->lr.y - size.y;
}

errtype wrapper_panel_close(uchar clear_message) {
    uiCursorStack *cs;
    int i;

    if (!wrapper_panel_on)
        return ERR_NOEFFECT;
    mouse_unconstrain();
    if (clear_message)
        message_info("");
    wrapper_panel_on = FALSE;
    ShockPlus::Options::save();
    inventory_page = inv_last_page;
    if (inventory_page < 0 && inventory_page != INV_3DVIEW_PAGE)
        inventory_page = 0;
    pause_game_func(0, 0, 0);
    uiGetSlabCursorStack(uiCurrentSlab, &cs);
    uiPopCursorEvery(cs, &slider_cursor);
    uiReleaseFocus(inventory_region, UI_EVENT_KBD_COOKED | UI_EVENT_MOUSE);
    uiRemoveRegionHandler(inventory_region, wrap_id);
    uiRemoveRegionHandler(inventory_region, wrap_key_id);
#ifndef STATIC_BUTTON_STORE
    full_visible = fv;
#endif
    inventory_clear();
    inventory_draw();
#ifdef SVGA_SUPPORT
    mfd_clear_all();
#endif
    for (i = 0; i < NUM_MFDS; i++)
        mfd_force_update_single(i);
    ResUnlock(OPTIONS_FONT);
    resume_game_time();
    return (OK);
}

uchar can_save() {
    uchar gp = game_paused;
    if (global_fullmap->cyber) {
        // spoof the game as not being paused so that the message won't go to the
        // phantom message line in full screen mode, where it will stay only for a frame.
        game_paused = FALSE;
        string_message_info(REF_STR_NoCyberSave);
        game_paused = gp;
        return (FALSE);
    }
    if (input_cursor_mode == INPUT_OBJECT_CURSOR) {
        string_message_info(REF_STR_CursorObjSave);
        return (FALSE);
    }
    return (TRUE);
}

//
// THE TOP LEVEL OPTIONS: Initialization, handler
//

void wrapper_pushbutton_func(uchar butid) {
    switch (butid) {
    case LOAD_BUTTON: // Load Game
#ifdef DEMO
        wrapper_panel_close(FALSE);
#else
        load_screen_init();
        string_message_info(REF_STR_LoadSlot);
#endif
        break;
    case SAVE_BUTTON: // Save Game
#ifdef DEMO
        wrapper_panel_close(FALSE);
#else
        if (can_save()) {
            save_screen_init();
            string_message_info(REF_STR_SaveSlot);
        } else
            wrapper_panel_close(FALSE);
#endif
        break;
    case AUDIO_BUTTON: // Audio
        sound_screen_init();
        break;
    case INPUT_BUTTON: // Input
        input_screen_init();
        break;
    case VIDEO_BUTTON: // Input
        video_screen_init();
        break;
#ifdef SVGA_SUPPORT
    case SCREENMODE_BUTTON: // Input
        screenmode_screen_init();
        break;
    case HEAD_RECENTER_BUTTON: // Input
    {
        // extern uchar recenter_headset(ushort keycode, uint32_t context, intptr_t data);
        // recenter_headset(0,0,0);
    } break;
    case HEADSET_BUTTON:
        // headset_screen_init();
        break;
#endif
    case AUDIO_OPT_BUTTON:
        soundopt_screen_init();
        break;
    case OPTIONS_BUTTON: // Options
        options_screen_init();
        break;
    case RETURN_BUTTON: // Return
        wrapper_panel_close(TRUE);
        break;
    case QUIT_BUTTON: // Quit
        verify_screen_init(quit_verify_pushbutton_handler, quit_verify_slorker);
        string_message_info(REF_STR_QuitConfirm);
        break;
    }
    return;
}

void wrapper_init(void) {
    LGRect r;
    int i;
    char *keyequivs;
    // REF_STR_KeyEquivs0 is "lsaiovrq"
    keyequivs = get_temp_string(REF_STR_KeyEquivs0);

    clear_obuttons();
    for (i = 0; i < 8; i++) {
        standard_button_rect(&r, i, 2, 3, 5);
        pushbutton_init(i, SDL_GetScancodeFromKey(keyequivs[i]), REF_STR_WrapperText + i, wrapper_pushbutton_func, &r);
    }
#ifdef DEMO
    dim_pushbutton(LOAD_BUTTON);
    dim_pushbutton(SAVE_BUTTON);
#endif
    opanel_redraw(TRUE);
}

//
// THE VERIFY SCREEN: Initialization, handlers
//

void quit_verify_pushbutton_handler(uchar butid) { really_quit_key_func(0, 0, 0); }

uchar quit_verify_slorker(uchar butid) {
    wrapper_panel_close(TRUE);
    return TRUE;
}

void save_verify_pushbutton_handler(uchar butid) { do_savegame_guts(savegame_verify); }

uchar save_verify_slorker(uchar butid) {
    strcpy(comments[savegame_verify], comments[NUM_SAVE_SLOTS]);
    wrapper_panel_close(TRUE);
    return TRUE;
}

void verify_screen_init(void (*verify)(uchar butid), slorker slork) {
    LGRect r;

    clear_obuttons();

    standard_button_rect(&r, 1, 2, 2, 5);
    pushbutton_init(0, static_cast<SDL_Scancode>(tolower(get_temp_string(REF_STR_VerifyText)[0])), REF_STR_VerifyText,
                    verify, &r);

    standard_button_rect(&r, 4, 2, 2, 5);
    pushbutton_init(1, SDL_GetScancodeFromKey(tolower(get_temp_string(REF_STR_VerifyText + 1)[0])),
                    (REF_STR_VerifyText + 1), (void (*)(uchar))slork, &r);

    slork_init(2, slork);

    opanel_redraw(TRUE);
}

void quit_verify_init(void) { verify_screen_init(quit_verify_pushbutton_handler, quit_verify_slorker); }

//
// THE SOUND OPTIONS SCREEN: Initialization, update funcs

void recompute_music_level(ushort vol) {
    //   curr_vol_lev=long_sqrt(100*vol);
    ShockPlus::Options::musicVolume = QVAR_TO_VOLUME(vol);
    if (vol == 0) {
        music_on = FALSE;
        // stop_music_func(0,0,0);
    } else {
        if (!music_on) {
            music_on = TRUE;
            // start_music_func(0,0,0);
        }
        // mlimbs_change_master_volume(curr_vol_lev);
    }
    MacTuneUpdateVolume();
}

void recompute_digifx_level(ushort vol) {
    ShockPlus::Options::enableSFX = (vol != 0);
    ShockPlus::Options::sfxVolume = QVAR_TO_VOLUME(vol);
    if (ShockPlus::Options::enableSFX) {
#ifdef DEMO
        play_digi_fx(73, 1);
#else
        // play a sample (if not already playing)
        if (!digi_fx_playing(SFX_NEAR_1, NULL))
            play_digi_fx(SFX_NEAR_1, 1);
        // update volume (main loop is not running at this point)
        sound_frame_update();
#endif
    } else {
#ifdef AUDIOLOGS
        audiolog_stop();
#endif
        stop_digi_fx();
    }
}

#ifdef AUDIOLOGS
void recompute_audiolog_level(ushort vol) {
    ShockPlus::Options::voiceVolume = QVAR_TO_VOLUME(vol);
    sound_frame_update();
}
#endif

void digi_toggle_deal(ushort offon) {
    int vol;
    vol = (ShockPlus::Options::enableSFX) ? 100 : 0;
    recompute_digifx_level(vol);
    QUESTVAR_SET(SFX_VOLUME_QVAR, vol);
}

#ifdef AUDIOLOGS
void audiolog_dealfunc(ushort val) {
    if (!val)
        audiolog_stop();
    QUESTVAR_SET(ALOG_OPT_QVAR, ShockPlus::Options::alogPlayback);  // TODO: unuse this
}
#endif

char hack_digi_channels = 1;

void digichan_dealfunc(ushort val) {
    hack_digi_channels = val;
    switch (hack_digi_channels) {
    case 0:
        cur_digi_channels = 2;
        break;
    case 1:
        cur_digi_channels = 4;
        break;
    case 2:
        cur_digi_channels = 8;
        break;
    }
    QUESTVAR_SET(DIGI_CHANNELS_QVAR, hack_digi_channels);
    // snd_set_digital_channels(cur_digi_channels);
}

static void seqer_dealfunc(ushort val) {
    //    INFO("Selected MIDI device %d", val);
    ShockPlus::Options::midiOutput = 0;
    ReloadDecXMI(); // Reload Midi decoder
    soundopt_screen_init();
    (void)val;
}

static void midi_output_dealfunc(ushort val) {
    //    INFO("Selected MIDI output %d", val);
    ReloadDecXMI(); // Reload Midi decoder
    soundopt_screen_init();
    (void)val;
}

#define SLIDER_OFFSET_3 0
void soundopt_screen_init() {
    LGRect r;
    char retkey;
    int i = 0;

    clear_obuttons();

    standard_button_rect(&r, i, 2, 2, 5);
    retkey = tolower(get_temp_string(REF_STR_AilThreeText)[0]);
    multi_init(i, retkey, REF_STR_AilThreeText, REF_STR_DigiChannelState, ID_NULL, sizeof(hack_digi_channels),
               &hack_digi_channels, 3, digichan_dealfunc, &r);
    i++;

    standard_button_rect(&r, i, 2, 2, 5);
    retkey = tolower(get_temp_string(REF_STR_AilThreeText + 1)[0]);
    // multi_init(i, retkey, REF_STR_AilThreeText+1, REF_STR_StereoReverseState, NULL,
    //   sizeof(snd_stereo_reverse), &snd_stereo_reverse, 2, NULL, &r);
    // i++;

#ifdef AUDIOLOGS
    standard_button_rect(&r, i, 2, 2, 5);
    retkey = tolower(get_temp_string(REF_STR_MusicText + 3)[0]);
    multi_init(i, retkey, REF_STR_MusicText + 3, REF_STR_AudiologState, ID_NULL, sizeof(ShockPlus::Options::alogPlayback),
               &ShockPlus::Options::alogPlayback, 3, audiolog_dealfunc, &r);
    i++;
#endif

    standard_button_rect(&r, i, 2, 2, 5);
    multi_init(i, 'p', REF_STR_Seqer, REF_STR_ADLMIDI, ID_NULL, sizeof(ShockPlus::Options::midiBackend),
               &ShockPlus::Options::midiBackend, 3, seqer_dealfunc, &r);
    i++;
    /* standard button is too narrow, so use a slider instead
        const unsigned int numMidiOutputs = GetOutputCountXMI();
        INFO("numMidiOutputs=%d", numMidiOutputs);
        standard_button_rect(&r, i, 2, 2, 5);
        multi_init(i, 'o', REF_STR_MidiOut, REF_STR_MidiOutX, ID_NULL,
                   sizeof(gShockPrefs.soMidiOutput), &gShockPrefs.soMidiOutput, numMidiOutputs, midi_output_dealfunc,
       &r); i++;
    */
    unsigned int midiOutputCount = GetOutputCountXMI();
    if (midiOutputCount > 1) {
        standard_slider_rect(&r, i, 2, 5);
        // this makes it double-wide i guess?
        r.lr.x += (r.lr.x - r.ul.x);
        slider_init(i, REF_STR_MidiOutX + ShockPlus::Options::midiOutput, sizeof(ShockPlus::Options::midiOutput), FALSE,
                    &ShockPlus::Options::midiOutput, midiOutputCount - 1, 0, midi_output_dealfunc, &r);
        i++;
    } else if (midiOutputCount == 1) {
        // just show a text label
        standard_button_rect(&r, i, 1, 2, 10);
        textwidget_init(i, BUTTON_COLOR, REF_STR_MidiOutX, &r);
        i++;
    }

    standard_button_rect(&r, 5, 2, 2, 5);
    retkey = tolower(get_temp_string(REF_STR_MusicText + 2)[0]);
    pushbutton_init(RETURN_BUTTON, SDL_GetScancodeFromKey(retkey), REF_STR_MusicText + 2, wrapper_pushbutton_func, &r);

    // FIXME: Cannot pass a keycode with modifier flags as uchar
    keywidget_init(QUIT_BUTTON, /*KB_FLAG_ALT |*/ SDL_SCANCODE_X, wrapper_pushbutton_func);
    opanel_redraw(TRUE);
}

void sound_screen_init(void) {
    LGRect r;
    uchar sliderbase;
    char retkey;
    char slider_offset = 0;
#ifdef AUDIOLOGS
    slider_offset = 10;
#endif

    clear_obuttons();

    if (music_card) {
        standard_slider_rect(&r, 0, 2, 5);
        // let's double the width of these things, eh?
        r.lr.x += (r.lr.x - r.ul.x);
        r.ul.y -= slider_offset;
        r.lr.y -= slider_offset;
        sliderbase = r.lr.x - r.ul.x - 2;
        slider_init(0, REF_STR_MusicText, sizeof(ushort), TRUE, &player_struct.questvars[MUSIC_VOLUME_QVAR], 100,
                    sliderbase, recompute_music_level, &r);
    } else {
        standard_button_rect(&r, 0, 2, 2, 5);
        r.lr.x += (r.lr.x - r.ul.x);
        r.ul.y -= slider_offset / 2;
        r.lr.y -= slider_offset / 2;
        textwidget_init(0, BUTTON_COLOR, REF_STR_MusicFeedbackText + 2, &r);
    }

    if (digi_gain) {
        standard_slider_rect(&r, 3, 2, 5);
        r.lr.x += (r.lr.x - r.ul.x);
        r.ul.y -= slider_offset;
        r.lr.y -= slider_offset;
        slider_init(1, REF_STR_MusicText + 1, sizeof(ushort), FALSE, &player_struct.questvars[SFX_VOLUME_QVAR], 100,
                    sliderbase, recompute_digifx_level, &r);
    } else {
        standard_button_rect(&r, 3, 2, 2, 5);
        r.ul.y -= slider_offset;
        r.lr.y -= slider_offset;
        multi_init(1, get_temp_string(REF_STR_MusicText + 1)[0], REF_STR_MusicText + 1, REF_STR_OffonText,
                   REF_STR_MusicFeedbackText + 5, sizeof(ShockPlus::Options::enableSFX), &ShockPlus::Options::enableSFX, 2, digi_toggle_deal, &r);
    }

#ifdef AUDIOLOGS
    standard_slider_rect(&r, 6, 2, 5);
    r.lr.x += (r.lr.x - r.ul.x);
    r.ul.y -= slider_offset;
    r.lr.y -= slider_offset;
    slider_init(2, REF_STR_MusicText + 4, sizeof(ushort), FALSE, &player_struct.questvars[ALOG_VOLUME_QVAR], 100,
                sliderbase, recompute_audiolog_level, &r);
#endif

    standard_button_rect(&r, 2, 2, 2, 5);
    retkey = tolower(get_temp_string(REF_STR_AilThreeText + 2)[0]);
    pushbutton_init(AUDIO_OPT_BUTTON, SDL_GetScancodeFromKey(retkey), REF_STR_AilThreeText + 2, wrapper_pushbutton_func,
                    &r);

    standard_button_rect(&r, 5, 2, 2, 5);
    retkey = tolower(get_temp_string(REF_STR_MusicText + 2)[0]);
    pushbutton_init(RETURN_BUTTON, SDL_GetScancodeFromKey(retkey), REF_STR_MusicText + 2, wrapper_pushbutton_func, &r);

    // FIXME: Cannot pass a keycode with modifier flags as uchar
    keywidget_init(QUIT_BUTTON, /*KB_FLAG_ALT |*/ SDL_SCANCODE_X, wrapper_pushbutton_func);

    opanel_redraw(TRUE);
}

//
// THE OPTIONS SCREEN: Initialization, update funcs
//

/*void gamma_dealfunc(ushort gamma_qvar)
{
   fix gamma;

//   gamma=FIX_UNIT-fix_make(0,gamma_qvar);
//   gamma=fix_mul(gamma,gamma)+(FIX_UNIT/2);
   gamma=QVAR_TO_GAMMA(gamma_qvar);
   gr_set_gamma_pal(0,256,gamma);
}*/

#ifdef SVGA_SUPPORT
uchar wrapper_screenmode_hack = FALSE;
void screenmode_change(uchar new_mode) {
    ShockPlus::Options::videoMode = new_mode;
    QUESTVAR_SET(SCREENMODE_QVAR, new_mode);
    change_mode_func(0, 0, _current_loop);
    wrapper_screenmode_hack = TRUE;

    INFO("Changed screen mode to %i", ShockPlus::Options::videoMode);
    wrapper_panel_close(TRUE);
}
#endif

void language_change(uchar lang) {

    ResCloseFile(string_res_file);
    ResCloseFile(mfdart_res_file);

    mfdart_res_file = ResOpenFile(mfdart_files[lang]);
    if (mfdart_res_file < 0)
        critical_error(CRITERR_RES | 2);

    string_res_file = ResOpenFile(language_files[lang]);
    if (string_res_file < 0)
        critical_error(CRITERR_RES | 0);

    QUESTVAR_SET(LANGUAGE_QVAR, lang);

    // in case we got here from interpret_qvars, and thus haven't set this yet
    ShockPlus::Options::language = static_cast<ShockPlus::Options::GeneralLanguage>(lang);

    invent_language_change();
    mfd_language_change();
    side_icon_language_change();
    // free_options_cursor();
    make_options_cursor();
}

void language_dealfunc(ushort lang) {
    language_change(lang);

    render_run();
    opanel_redraw(FALSE);
}

void dclick_dealfunc(ushort dclick_qvar) {
    uiDoubleClickDelay = QVAR_TO_DCLICK(dclick_qvar, 0);
    uiDoubleClickTime = QVAR_TO_DCLICK(dclick_qvar, 1);
}

void joysens_dealfunc(ushort joysens_qvar) { inpJoystickSens = QVAR_TO_JOYSENS(joysens_qvar); }

void center_joy_go(uchar butid) { joystick_screen_init(); }

void center_joy_pushbutton_func(uchar butid) {
    int i;
    string_message_info(REF_STR_CenterJoyPrompt);

    // take over this button, null the other buttons
    // except for RETURN and QUIT;

    for (i = 0; i < MAX_OPTION_BUTTONS; i++) {
        if (i == butid)
            keywidget_init(i, SDL_SCANCODE_RETURN, center_joy_go);
        else if (i != RETURN_BUTTON && i != QUIT_BUTTON)
            OButtons[i].evmask = 0;
    }
}

static void renderer_dealfunc(ushort unused) {
    uiHideMouse(NULL);
    render_run();
    if (full_game_3d) {
        // update stored background bitmap and redraw menu
        ss_get_bitmap(&inv_view360_canvas.bm, GAME_MESSAGE_X, GAME_MESSAGE_Y);
        opanel_redraw(FALSE);
    }
    uiShowMouse(NULL);
    // recalculate menu in case a button needs to be added or removed
    video_screen_init();
    // suppress compiler warning
    (void)unused;
}

void detail_dealfunc(ushort det) {

    change_detail_level(det);
    uiHideMouse(NULL);
    render_run();
    if (full_game_3d)
        opanel_redraw(FALSE);
    uiShowMouse(NULL);
}

void mousehand_dealfunc(ushort lefty) {
    // mouse_set_lefty(lefty);
}

void olh_dealfunc(ushort olh) { toggle_olh_func(0, 0, 0); }

ushort wrap_joy_type = 0;
ushort high_joy_flags;
void joystick_type_func(ushort new_joy_type) {
    // joystick_count = joy_init(high_joy_flags | new_joy_type);
    // config_set_single_value("joystick",CONFIG_INT_TYPE,(config_valtype)(high_joy_flags|new_joy_type));
    joystick_screen_init();
}

void joystick_screen_init(void) {
    LGRect r;
    int i = 0;
    char *keys;
    uchar sliderbase;

    keys = get_temp_string(REF_STR_KeyEquivs6);
    clear_obuttons();

    standard_button_rect(&r, i, 2, 2, 1);
    multi_init(i, keys[i], REF_STR_JoystickType, REF_STR_JoystickTypes, ID_NULL, sizeof(wrap_joy_type), &wrap_joy_type,
               4, joystick_type_func, &r);
    i++;

    standard_button_rect(&r, i, 2, 2, 1);
    pushbutton_init(i, SDL_GetScancodeFromKey(keys[i]), REF_STR_CenterJoy, center_joy_pushbutton_func, &r);
    if (!joystick_count) {
        dim_pushbutton(i);
    }
    i++;

    if (joystick_count) {
        standard_slider_rect(&r, i, 2, 1);
        sliderbase = (r.lr.x - r.ul.x - 2) >> 1;
        slider_init(i, REF_STR_JoystickSens, sizeof(ushort), FALSE, &player_struct.questvars[JOYSENS_QVAR], 256,
                    sliderbase, joysens_dealfunc, &r);
    }
    i++;

    standard_button_rect(&r, 5, 2, 2, 1);
    pushbutton_init(RETURN_BUTTON, SDL_GetScancodeFromKey(keys[i]), REF_STR_OptionsText + 5, wrapper_pushbutton_func,
                    &r);

    // FIXME: Cannot pass a keycode with modifier flags as uchar
    keywidget_init(QUIT_BUTTON, /*KB_FLAG_ALT |*/ SDL_SCANCODE_X, wrapper_pushbutton_func);

    opanel_redraw(TRUE);
}

void joystick_button_func(uchar butid) { joystick_screen_init(); }

void input_screen_init(void) {
    LGRect r;
    char *keys;
    int i = 0;
    uchar sliderbase;

    keys = get_temp_string(REF_STR_KeyEquivs1);
    clear_obuttons();

    standard_button_rect(&r, i, 2, 2, 1);
    r.ul.x -= 1;
    multi_init(i, keys[0], REF_STR_OptionsText + 0, REF_STR_OffonText, REF_STR_PopupCursFeedback,
               sizeof(ShockPlus::Options::showTooltipMessages), &ShockPlus::Options::showTooltipMessages, 2, NULL, &r);
    i++;

    standard_button_rect(&r, i, 2, 2, 1);
    multi_init(i, keys[1], REF_STR_OptionsText + 1, REF_STR_MouseHand, REF_STR_HandFeedback,
               sizeof(player_struct.questvars[MOUSEHAND_QVAR]), &player_struct.questvars[MOUSEHAND_QVAR], 2,
               mousehand_dealfunc, &r);
    i++;

    standard_slider_rect(&r, i, 2, 1);
    r.ul.x -= 1;
    sliderbase = ((r.lr.x - r.ul.x - 3) * (FIX_UNIT / 3)) / USHRT_MAX;
    slider_init(i, REF_STR_DoubleClick, sizeof(ushort), FALSE, &player_struct.questvars[DCLICK_QVAR], USHRT_MAX,
                sliderbase, dclick_dealfunc, &r);
    i++;

    standard_button_rect(&r, i, 2, 2, 1);
    r.ul.x -= 1;
    pushbutton_init(i, SDL_GetScancodeFromKey(keys[2]), REF_STR_Joystick, joystick_button_func, &r);
    i++;

    standard_button_rect(&r, i, 2, 2, 1);
    r.ul.x -= 1;
    multi_init(i, keys[3], REF_STR_MousLook, REF_STR_MousNorm, ID_NULL, sizeof(ShockPlus::Options::invertMouseY),
               &ShockPlus::Options::invertMouseY, 2, NULL, &r);
    i++;

    standard_button_rect(&r, 5, 2, 2, 1);
    pushbutton_init(RETURN_BUTTON, SDL_GetScancodeFromKey(keys[3]), REF_STR_OptionsText + 5, wrapper_pushbutton_func,
                    &r);

    // FIXME: Cannot pass a keycode with modifier flags as uchar
    keywidget_init(QUIT_BUTTON, /*KB_FLAG_ALT |*/ SDL_SCANCODE_X, wrapper_pushbutton_func);

    opanel_redraw(TRUE);
}

// gamma param not used here; see SetSDLPalette() in Shock.c
void gamma_slider_dealfunc(ushort gamma_qvar) {
    gr_set_gamma_pal(0, 256, 0);

    uiHideMouse(NULL);
    render_run();
    if (full_game_3d)
        opanel_redraw(FALSE);
    uiShowMouse(NULL);
}

void video_screen_init(void) {
    LGRect r;
    int i;
    char *keys;
    uchar sliderbase;

    keys = get_temp_string(REF_STR_KeyEquivs3);
    clear_obuttons();
    i = 0;

#ifdef USE_OPENGL
    // renderer
    if (can_use_opengl()) {
        standard_button_rect(&r, i, 2, 2, 2);
        multi_init(i, 'g', REF_STR_Renderer, REF_STR_Software, ID_NULL, sizeof(ShockPlus::Options::enableOpenGL),
                   &ShockPlus::Options::enableOpenGL, 2, renderer_dealfunc, &r);
        i++;
    }
#endif

#ifdef SVGA_SUPPORT
    // video mode
    standard_button_rect(&r, i, 2, 2, 2);
    pushbutton_init(SCREENMODE_BUTTON, SDL_GetScancodeFromKey(keys[0]), REF_STR_VideoText, wrapper_pushbutton_func, &r);
    i++;
#endif

    // detail level
    standard_button_rect(&r, i, 2, 2, 2);
    r.lr.x += 2;
    multi_init(i, keys[1], REF_STR_OptionsText + 4, REF_STR_DetailLvl, REF_STR_DetailLvlFeedback,
               sizeof(ShockPlus::Options::videoDetail), &ShockPlus::Options::videoDetail, 4, detail_dealfunc, &r);
    i++;

    // gamma
    standard_slider_rect(&r, i, 2, 2);
    r.ul.x = r.ul.x + 1;
    sliderbase = ((r.lr.x - r.ul.x - 1) * 29 / 100);
    slider_init(i, REF_STR_OptionsText + 3, sizeof(ShockPlus::Options::gammaCorrection), TRUE,
                &(ShockPlus::Options::gammaCorrection), 100, sliderbase, gamma_slider_dealfunc, &r);
    i++;

#ifdef USE_OPENGL
    // textre filter
    if (can_use_opengl() && ShockPlus::Options::enableOpenGL) {
        standard_button_rect(&r, i, 2, 2, 2);
        multi_init(i, 't', REF_STR_TextFilt, REF_STR_TFUnfil, ID_NULL, sizeof(ShockPlus::Options::videoTextureFilter),
                   &ShockPlus::Options::videoTextureFilter, 2, renderer_dealfunc, &r);
        i++;
    }
#endif

    // return (fixed at position 5)
    standard_button_rect(&r, 5, 2, 2, 2);
    pushbutton_init(RETURN_BUTTON, SDL_GetScancodeFromKey(keys[3]), REF_STR_OptionsText + 5, wrapper_pushbutton_func,
                    &r);

    // FIXME: Cannot pass a keycode with modifier flags as uchar
    keywidget_init(QUIT_BUTTON, /*KB_FLAG_ALT |*/ SDL_SCANCODE_X, wrapper_pushbutton_func);

    opanel_redraw(TRUE);
}

#ifdef SVGA_SUPPORT
void screenmode_screen_init(void) {
    LGRect r;
    int i;
    char *keys;

    if (wrapper_screenmode_hack && !(can_use_opengl() && ShockPlus::Options::enableOpenGL)) {
        uiHideMouse(NULL);
        render_run();
        uiShowMouse(NULL);
        wrapper_screenmode_hack = FALSE;
    }

    keys = get_temp_string(REF_STR_KeyEquivs4);

    clear_obuttons();

    for (i = 0; i < 5; i++) {
        uchar mode_ok = FALSE;
        char j = 0;
        standard_button_rect(&r, i, 2, 2, 2);
        pushbutton_init(i, SDL_GetScancodeFromKey(keys[i]), REF_STR_ScreenModeText + i, screenmode_change, &r);
        while ((grd_info.modes[j] != -1) && !mode_ok) {
            if (grd_info.modes[j] == svga_mode_data[i])
                mode_ok = TRUE;
            j++;
        }
        if (!mode_ok)
            dim_pushbutton(i);
        else if (i == convert_use_mode)
            bright_pushbutton(i);
    }

    standard_button_rect(&r, 5, 2, 2, 2);
    pushbutton_init(RETURN_BUTTON, SDL_GetScancodeFromKey(keys[2]), REF_STR_OptionsText + 5, wrapper_pushbutton_func,
                    &r);

    // FIXME: Cannot pass a keycode with modifier flags as uchar
    keywidget_init(QUIT_BUTTON, /*KB_FLAG_ALT |*/ SDL_SCANCODE_X, wrapper_pushbutton_func);

    opanel_redraw(TRUE);
}
#endif

void options_screen_init(void) {
    LGRect r;
    char *keys;
    int i = 0;

    keys = get_temp_string(REF_STR_KeyEquivs2);
    clear_obuttons();

    // olh_temp=(QUESTBIT_GET(OLH_QBIT)==0);

    olh_temp = ShockPlus::Options::showOnScreenHelp;

    // okay, I admit it, we're going to tweak these "standard" button rects a little bit.

    standard_button_rect(&r, 0, 2, 2, 2);
    r.ul.x -= 2;
    multi_init(i, keys[i], REF_STR_OptionsText + 2, REF_STR_TerseText, REF_STR_TerseFeedback,
               sizeof(ShockPlus::Options::messageFormat), &(ShockPlus::Options::messageFormat), 2, NULL, &r);
    i++;

    i++;

    standard_button_rect(&r, 1, 2, 2, 2);
    multi_init(i, keys[i], REF_STR_OnlineHelp, REF_STR_OffonText, ID_NULL, sizeof(olh_temp), &olh_temp, 2, olh_dealfunc,
               &r);
    i++;

    i++;

    standard_button_rect(&r, 2, 2, 2, 2);
    multi_init(i, keys[i], REF_STR_Language, REF_STR_Languages, ID_NULL, sizeof(ShockPlus::Options::language), &ShockPlus::Options::language, 3,
               language_dealfunc, &r);
    i++;

    standard_button_rect(&r, 5, 2, 2, 2);
    r.lr.x += 2;
    pushbutton_init(RETURN_BUTTON, SDL_GetScancodeFromKey(keys[i]), REF_STR_OptionsText + 5, wrapper_pushbutton_func,
                    &r);

    // FIXME: Cannot pass a keycode with modifier flags as uchar
    keywidget_init(QUIT_BUTTON, /*KB_FLAG_ALT |*/ SDL_SCANCODE_X, wrapper_pushbutton_func);

    opanel_redraw(TRUE);
}

uchar wrapper_options_func(ushort keycode, uint32_t context, intptr_t data) {
    wrapper_start(wrapper_init);
    return (OK);
}

//
// THE LOAD GAME SCREEN: Initialization, update funcs
//

void load_dealfunc(uchar butid, uchar index) {
    begin_wait();
    Poke_SaveName(index);
    // Spew(DSRC_EDITOR_Save,("attempting to load from %s\n",save_game_name));

    if (load_game(save_game_name) != OK) {
        WARN("%s: Load game failed!", __FUNCTION__);
    } else {
        INFO("Game %d loaded!", index);
        // Spew(DSRC_EDITOR_Restore,("Game %d loaded!\n",index));
    }
    end_wait();
    // spoof_mouse_event();
    wrapper_panel_close(TRUE);
}

void load_screen_init(void) {
    clear_obuttons();

    textlist_init(0, *comments, NUM_SAVE_SLOTS, SAVE_COMMENT_LEN, FALSE, 0, valid_save, valid_save, REF_STR_UnusedSave,
                  BUTTON_COLOR, WHITE, BUTTON_COLOR + 2, 0, load_dealfunc, NULL);

    // FIXME: Cannot pass a keycode with modifier flags as uchar
    keywidget_init(QUIT_BUTTON, /*KB_FLAG_ALT |*/ SDL_SCANCODE_X, wrapper_pushbutton_func);

    opanel_redraw(TRUE);
}

//
// THE SAVE GAME SCREEN: Initialization, update funcs
//

void save_dealfunc(uchar butid, uchar index) {
    if (!ObjSysOkay()) {
        string_message_info(REF_STR_ObjSysBad);
        savegame_verify = index;
        verify_screen_init(save_verify_pushbutton_handler, save_verify_slorker);
    } else {
        message_info("");
        do_savegame_guts(index);
    }
}

void save_screen_init(void) {
    clear_obuttons();

    textlist_init(0, *comments, NUM_SAVE_SLOTS, SAVE_COMMENT_LEN, TRUE, 0xFFFF, 0xFFFF, valid_save, REF_STR_UnusedSave,
                  BUTTON_COLOR, WHITE, BUTTON_COLOR + 2, REF_STR_EnterSaveString, save_dealfunc, NULL);

    // FIXME: Cannot pass a keycode with modifier flags as uchar
    keywidget_init(QUIT_BUTTON, /*KB_FLAG_ALT |*/ SDL_SCANCODE_X, wrapper_pushbutton_func);

    opanel_redraw(TRUE);
}

void wrapper_start(void (*init)(void)) {
    if (wrapper_panel_on)
        return;
    inv_last_page = inventory_page;
    if (!game_paused)
        pause_game_func(0, 0, 0);
    if (!full_game_3d)
        message_info("");
    inventory_page = -1;
    wrapper_panel_on = TRUE;
    suspend_game_time();
    opt_font = static_cast<grs_font *>(ResLock(OPTIONS_FONT));
#ifndef STATIC_BUTTON_STORE
    OButtons = (opt_button *)(offscreen_mfd.bm.bits);
    fv = full_visible;
    full_visible = 0;
#endif
    render_run(); // move here to fix ghost mouse cursor
    uiHideMouse(NULL);
    if (full_game_3d) {
#ifdef SVGA_SUPPORT
        uchar old_over = gr2ss_override;
#endif
        gr_push_canvas(grd_screen_canvas);
#ifdef SVGA_SUPPORT
        gr2ss_override = OVERRIDE_ALL;
#endif
        ss_get_bitmap(&inv_view360_canvas.bm, GAME_MESSAGE_X, GAME_MESSAGE_Y);
#ifdef SVGA_SUPPORT
        gr2ss_override = old_over;
#endif
        gr_pop_canvas();
    } else
        inventory_clear();
    uiShowMouse(NULL);
    uiInstallRegionHandler(inventory_region, UI_EVENT_MOUSE | UI_EVENT_MOUSE_MOVE, opanel_mouse_handler, 0, &wrap_id);
    uiInstallRegionHandler(inventory_region, UI_EVENT_KBD_COOKED, opanel_kb_handler, 0, &wrap_key_id);
    uiGrabFocus(inventory_region, UI_EVENT_KBD_COOKED | UI_EVENT_MOUSE);
    region_set_invisible(inventory_region, FALSE);
    reset_input_system();
    init();
}

#define NEEDED_DISKSPACE 630000
errtype check_free_diskspace(int *needed) {
    /*struct diskfree_t freespace;
    _dos_getdiskfree(0, &freespace);
    if (freespace.avail_clusters * freespace.sectors_per_cluster * freespace.bytes_per_sector < NEEDED_DISKSPACE)
    {
       *needed = NEEDED_DISKSPACE - (freespace.avail_clusters * freespace.sectors_per_cluster *
    freespace.bytes_per_sector); return(ERR_NOMEM);
    }
    *needed = 0;*/
    return (OK);
}

errtype do_savegame_guts(uchar slot) {
    errtype retval = OK;

    begin_wait();
    if (!(valid_save & (1 << slot))) {
        int needed;
        //      char buf1[128],buf2[128];
        if (check_free_diskspace(&needed) == ERR_NOMEM) {
            //         lg_sprintf(buf2, get_string(REF_STR_InsufficientDisk, buf1, 128), needed);
            string_message_info(REF_STR_InsufficientDisk);
            retval = ERR_NOMEM;
        }
    }
    if (retval == OK) {
        Poke_SaveName(slot);
        if (save_game(save_game_name, comments[slot]) != OK) {
            ERROR("Save game failed!");
            message_info("Game save failed!");
            //      strcpy(comments[comment_mode], original_comment);
            retval = ERR_NOEFFECT;
            valid_save &= ~(1 << slot);
        } else
            // Spew(DSRC_EDITOR_Save, ("Game %d saved!\n", slot));
            if (retval == OK)
            valid_save |= 1 << slot;
    }
    end_wait();
    // spoof_mouse_event();
    if (retval == OK)
        wrapper_panel_close(TRUE);
    return (retval);
}

//#endif // NOT_YET

uchar wrapper_region_mouse_handler(uiEvent *ev, LGRegion *r, intptr_t data) {
    /*if (global_fullmap->cyber)
    {
       uiSetRegionDefaultCursor(r,NULL);
       return FALSE;
    }
    else*/

    uiSetRegionDefaultCursor(r, &option_cursor);

    if (ev->mouse_data.action & MOUSE_DOWN) {
        wrapper_options_func(0, 0, TRUE);
        return TRUE;
    }
    return FALSE;
}

errtype make_options_cursor(void) {
    char *s;
    short w, h;
    LGPoint hot = {0, 0};
    grs_canvas cursor_canv;
    short orig_w;
    uchar old_over = gr2ss_override;
    gr2ss_override = OVERRIDE_ALL;

    LGPoint size = res_bm_size(REF_IMG_bmOptionCursor);
    orig_w = w = size.x;
    h = size.y;
    ss_point_convert(&w, &h, FALSE);
    gr_init_bitmap(&option_cursor_bmap, svga_options_cursor_bits, BMT_FLAT8, BMF_TRANS, w, h);
    gr_make_canvas(&option_cursor_bmap, &cursor_canv);
    gr_push_canvas(&cursor_canv);
    gr_clear(0);
    s = get_temp_string(REF_STR_ClickForOptions);
    gr_set_font(static_cast<grs_font *>(ResLock(OPTIONS_FONT)));
    gr_string_wrap(s, orig_w - 3);
    gr_string_size(s, &w, &h);
    gr_set_fcolor(0xB8);
    ss_rect(1, 1, w + 2, h + 2);
    gr_set_fcolor(0xD3);
    ss_string(s, 2, 1);
    gr_font_string_unwrap(s);
    uiMakeBitmapCursor(&option_cursor, &option_cursor_bmap, hot);
    gr_pop_canvas();
    ResUnlock(OPTIONS_FONT);
    cursor_loaded = TRUE;
    gr2ss_override = old_over;

    return OK;
}

/*void free_options_cursor(void)
{
#ifndef SVGA_SUPPORT
   if(cursor_loaded)
      Free(option_cursor_bmap.bits);
#endif
}*/

errtype wrapper_create_mouse_region(LGRegion *root) {
    errtype err;
    int id;
    LGRect r = {{0, 0}, {STATUS_X, STATUS_HEIGHT}};
    LGRegion *reg = &(options_mouseregion[free_mouseregion++]);

    err = region_create(root, reg, &r, 2, 0, REG_USER_CONTROLLED | AUTODESTROY_FLAG, NULL, NULL, NULL, NULL);
    if (err != OK)
        return err;
    err = uiInstallRegionHandler(reg, UI_EVENT_MOUSE | UI_EVENT_MOUSE_MOVE, wrapper_region_mouse_handler, 0, &id);
    if (err != OK)
        return err;
    if (!cursor_loaded) {
        err = make_options_cursor();
        if (err != OK)
            return err;
    }
    uiSetRegionDefaultCursor(reg, &option_cursor);
    return OK;
}

//#ifdef NOT_YET //
uchar saveload_hotkey_func(ushort keycode, uint32_t context, intptr_t data) {
#ifdef DEMO
    return (TRUE);
#else
    if ((!data) && (!can_save()))
        return (TRUE);
    wrapper_start(data ? load_screen_init : save_screen_init);
    string_message_info(data ? REF_STR_LoadSlot : REF_STR_SaveSlot);
    return (TRUE);
#endif
}

uchar demo_quit_func(ushort keycode, uint32_t context, intptr_t data) {
    wrapper_start(quit_verify_init);
    string_message_info(REF_STR_QuitConfirm);
    return (TRUE);
}

//#endif // NOT_YET
