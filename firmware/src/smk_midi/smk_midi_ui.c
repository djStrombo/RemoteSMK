/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2026 RemoteSMK contributors */
#pragma once
#include "smk_midi_types.h"
#include "smk_midi_scales.h"

#define CLR_BG      0x0000u  /* Pure Black */
#define CLR_INK     0xFFFFu  /* Pure White */
#define CLR_DIM     0x8410u  /* Grey */
#define CLR_BORDER  0x4208u  /* Dark Grey */

static const char *const SMK_ROOT_NAMES[12] = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

static const char *const SMK_RATE_NAMES[6] = {
    "1/4", "1/8", "1/8T", "1/16", "1/16T", "1/32"
};

static const char *const SMK_GATE_NAMES[4] = {
    "25%", "50%", "75%", "100%"
};

static void smk_ui_format_note(char *dst, uint32_t note)
{
    const char *rname = SMK_ROOT_NAMES[note % 12u];
    int oct = ((int)note / 12) - 1;
    while (*rname) *dst++ = *rname++;
    if (oct < 0) {
        *dst++ = '-';
        *dst++ = (char)('0' + (-oct));
    } else {
        *dst++ = (char)('0' + oct);
    }
    *dst = 0;
}

static char *smk_ui_put_str(char *d, const char *s)
{
    while (*s) *d++ = *s++;
    *d = 0;
    return d;
}

static char *smk_ui_put_u(char *d, uint32_t v)
{
    if (v >= 100u) {
        *d++ = (char)('0' + (v / 100u) % 10u);
        *d++ = (char)('0' + (v / 10u) % 10u);
        *d++ = (char)('0' + v % 10u);
    } else if (v >= 10u) {
        *d++ = (char)('0' + (v / 10u) % 10u);
        *d++ = (char)('0' + v % 10u);
    } else {
        *d++ = (char)('0' + v % 10u);
    }
    *d = 0;
    return d;
}

static inline int32_t smk_text_w(const char *s, int32_t n)
{
    if (!s) return 0;
    int32_t l = 0;
    while (s[l]) l++;
    return l * 6 * n;
}

static char *smk_ui_put_u2(char *d, uint32_t v)
{
    *d++ = (char)('0' + (v / 10u) % 10u);
    *d++ = (char)('0' + v % 10u);
    *d = 0;
    return d;
}

static char *smk_ui_put_u3(char *d, uint32_t v)
{
    if (v >= 100u) {
        *d++ = (char)('0' + v / 100u);
        *d++ = (char)('0' + (v % 100u) / 10u);
        *d++ = (char)('0' + v % 10u);
    } else if (v >= 10u) {
        *d++ = ' ';
        *d++ = (char)('0' + v / 10u);
        *d++ = (char)('0' + v % 10u);
    } else {
        *d++ = ' ';
        *d++ = ' ';
        *d++ = (char)('0' + v);
    }
    *d = 0;
    return d;
}

/* Forward declarations of sa_gfx primitives */
static void sg_rect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t c);
static void sg_chip(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t c);
static int32_t sg_text(int32_t x, int32_t y, int32_t n, const char *s, uint16_t c);

/* Helper to draw standard menu header and footer */
static void smk_ui_draw_menu_frame(const char *title, const char *hint)
{
    sg_rect(0, 0, 240, 240, CLR_BG);
    sg_text(10, 8, 1, title, CLR_INK);
    sg_rect(0, 24, 240, 1, CLR_INK);
    sg_rect(0, 214, 240, 1, CLR_INK);
    sg_text(8, 222, 1, hint ? hint : "[< >] CHOOSE  [OCT-] OFF  [OCT+] ON", CLR_INK);
}

/* Helper to draw a single text list row */
static void smk_ui_draw_list_row(int32_t y, const char *prefix, const char *val, int selected)
{
    if (selected) {
        sg_rect(6, y - 2, 228, 20, CLR_INK);
        sg_text(12, y + 2, 1, prefix, CLR_BG);
        if (val) sg_text(130, y + 2, 1, val, CLR_BG);
    } else {
        sg_text(12, y + 2, 1, prefix, CLR_INK);
        if (val) sg_text(130, y + 2, 1, val, CLR_INK);
    }
}

/* Draw a minimalist rotary dial indicator at (x, y), 14x14 */
static void smk_ui_draw_dial(int32_t x, int32_t y, uint8_t val)
{
    sg_rect(x + 3, y, 8, 14, CLR_BORDER);
    sg_rect(x + 1, y + 1, 12, 12, CLR_BORDER);
    sg_rect(x, y + 3, 14, 8, CLR_BORDER);

    sg_rect(x + 3, y + 1, 8, 12, CLR_BG);
    sg_rect(x + 1, y + 3, 12, 8, CLR_BG);

    sg_rect(x + 6, y + 6, 2, 2, CLR_DIM);

    static const int8_t DOT_X[9] = { 2, 1, 1, 3, 6, 9, 11, 11, 10 };
    static const int8_t DOT_Y[9] = { 9, 7, 4, 2, 1, 2,  4,  7,  9 };

    uint32_t idx = ((uint32_t)val * 8u + 63u) / 127u;
    if (idx > 8u) idx = 8u;

    sg_rect(x + DOT_X[idx], y + DOT_Y[idx], 2, 2, CLR_INK);
}

/* Draw a minimalist horizontal fader slider at (x, y), width 48, height 10 */
static void smk_ui_draw_fader_slider(int32_t x, int32_t y, uint8_t val)
{
    sg_rect(x, y + 4, 48, 2, CLR_DIM);
    sg_rect(x, y + 2, 1, 6, CLR_DIM);
    sg_rect(x + 47, y + 2, 1, 6, CLR_DIM);
    sg_rect(x + 23, y + 3, 1, 4, CLR_BORDER);

    int32_t hx = x + ((int32_t)val * 42) / 127;
    sg_rect(hx, y, 6, 10, CLR_INK);
    sg_rect(hx + 2, y + 2, 2, 6, CLR_BG);
}

/* Page 0: Super Minimalist Main Screen */
static void smk_ui_draw_general(void)
{
    sg_rect(0, 0, 240, 240, CLR_BG);

    /* Center hero: Last played chord or note in large font */
    const char *center_text = smk_m.chord_name[0] ? smk_m.chord_name :
                              smk_m.last_note_name[0] ? smk_m.last_note_name : "--";
    uint16_t center_color = (smk_m.chord_name[0] || smk_m.last_note_name[0]) ? CLR_INK : CLR_DIM;

    int32_t scale = 5;
    while (scale > 1 && smk_text_w(center_text, scale) > 220) {
        scale--;
    }
    int32_t text_w = smk_text_w(center_text, scale);
    int32_t text_h = 10 * scale;
    int32_t cx = (240 - text_w) / 2;
    int32_t cy = 105 - (text_h / 2);
    sg_text(cx, cy, scale, center_text, center_color);

    /* Bottom: Transient control feedback (disappears after ~2s) */
    uint32_t now = plat_ms();
    int show_hud = (smk_m.last_ctrl_ms > 0 && (now - smk_m.last_ctrl_ms) < 2000u && smk_m.last_ctrl_type != SMK_HUD_NONE);

    if (show_hud) {
        switch (smk_m.last_ctrl_type) {
        case SMK_HUD_KNOB: {
            uint32_t knum = (uint32_t)smk_m.last_ctrl_bank * 8u + (uint32_t)smk_m.last_ctrl_id + 1u;
            char s_label[8], s_cc[8];
            char *p = smk_ui_put_str(s_label, "K");
            p = smk_ui_put_u(p, knum);
            smk_ui_put_u(s_cc, smk_m.last_ctrl_cc);

            int32_t w1 = smk_text_w(s_label, 1);
            int32_t w2 = smk_text_w(s_cc, 1);
            int32_t dial_w = 14;
            int32_t gap = 8;
            int32_t total_w = w1 + gap + dial_w + gap + w2;
            int32_t sx = (240 - total_w) / 2;
            int32_t sy = 200;

            sg_text(sx, sy + 2, 1, s_label, CLR_INK);
            smk_ui_draw_dial(sx + w1 + gap, sy, smk_m.last_ctrl_val);
            sg_text(sx + w1 + gap + dial_w + gap, sy + 2, 1, s_cc, CLR_INK);
            break;
        }
        case SMK_HUD_FADER: {
            uint32_t fnum = (uint32_t)smk_m.last_ctrl_bank * 4u + (uint32_t)smk_m.last_ctrl_id + 1u;
            char s_label[8], s_cc[8];
            char *p = smk_ui_put_str(s_label, "F");
            p = smk_ui_put_u(p, fnum);
            smk_ui_put_u(s_cc, smk_m.last_ctrl_cc);

            int32_t w1 = smk_text_w(s_label, 1);
            int32_t w2 = smk_text_w(s_cc, 1);
            int32_t fader_w = 48;
            int32_t gap = 8;
            int32_t total_w = w1 + gap + fader_w + gap + w2;
            int32_t sx = (240 - total_w) / 2;
            int32_t sy = 202;

            sg_text(sx, sy, 1, s_label, CLR_INK);
            smk_ui_draw_fader_slider(sx + w1 + gap, sy, smk_m.last_ctrl_val);
            sg_text(sx + w1 + gap + fader_w + gap, sy, 1, s_cc, CLR_INK);
            break;
        }
        case SMK_HUD_PAD: {
            uint32_t pnum = (uint32_t)smk_m.last_ctrl_bank * 16u + (uint32_t)smk_m.last_ctrl_id + 1u;
            if (smk_m.last_ctrl_sub == 0) {
                /* NOTES mode: P<num>  <note>  <vel> */
                char s_pad[32];
                char *p = smk_ui_put_str(s_pad, "P");
                p = smk_ui_put_u(p, pnum);
                *p++ = ' '; *p++ = ' ';
                char nstr[8];
                smk_ui_format_note(nstr, smk_m.last_ctrl_cc);
                p = smk_ui_put_str(p, nstr);
                *p++ = ' '; *p++ = ' ';
                p = smk_ui_put_u(p, smk_m.last_ctrl_val);

                int32_t w = smk_text_w(s_pad, 1);
                sg_text((240 - w) / 2, 202, 1, s_pad, CLR_INK);
            } else {
                /* CC mode: P<num> [dial] <cc> */
                char s_label[8], s_cc[8];
                char *p = smk_ui_put_str(s_label, "P");
                p = smk_ui_put_u(p, pnum);
                smk_ui_put_u(s_cc, smk_m.last_ctrl_cc);

                int32_t w1 = smk_text_w(s_label, 1);
                int32_t w2 = smk_text_w(s_cc, 1);
                int32_t dial_w = 14;
                int32_t gap = 8;
                int32_t total_w = w1 + gap + dial_w + gap + w2;
                int32_t sx = (240 - total_w) / 2;
                int32_t sy = 200;

                sg_text(sx, sy + 2, 1, s_label, CLR_INK);
                smk_ui_draw_dial(sx + w1 + gap, sy, smk_m.last_ctrl_val);
                sg_text(sx + w1 + gap + dial_w + gap, sy + 2, 1, s_cc, CLR_INK);
            }
            break;
        }
        case SMK_HUD_TOAST: {
            int32_t w = smk_text_w(smk_m.last_ctrl_text, 1);
            sg_text((240 - w) / 2, 202, 1, smk_m.last_ctrl_text, CLR_INK);
            break;
        }
        default: break;
        }
    }
}

/* Page 1: Scale Menu */
static void smk_ui_draw_scale(void)
{
    char buf[32];
    if (smk_m.menu_depth == 0) {
        smk_ui_draw_menu_frame("[ SCALE MENU ]", 0);
        smk_ui_draw_list_row(34, "1. ACTIVE:", smk_m.scale_on ? "[ON]" : "[OFF]", smk_m.menu_sel[0] == 0);
        smk_ui_draw_list_row(58, "2. ROOT KEY:", SMK_ROOT_NAMES[smk_m.root % 12], smk_m.menu_sel[0] == 1);
        smk_ui_draw_list_row(82, "3. SCALE:", SCALE_TITLE[smk_m.scale % SCALE_TOTAL], smk_m.menu_sel[0] == 2);

        buf[0] = (char)('0' + smk_m.bpm / 100u);
        buf[1] = (char)('0' + (smk_m.bpm % 100u) / 10u);
        buf[2] = (char)('0' + smk_m.bpm % 10u);
        buf[3] = ' '; buf[4] = 'B'; buf[5] = 'P'; buf[6] = 'M'; buf[7] = 0;
        smk_ui_draw_list_row(106, "4. TEMPO:", buf, smk_m.menu_sel[0] == 3);
    } else {
        /* Submenus */
        if (smk_m.menu_sel[0] == 1) {
            /* ROOT KEY selection list */
            smk_ui_draw_menu_frame("[ SCALE > ROOT KEY ]", "[< >] CHOOSE  [OCT+] OK  [OCT-] CANCEL");
            uint32_t i;
            for (i = 0; i < 7 && (i + (uint32_t)smk_m.menu_scroll[1]) < 12u; i++) {
                uint32_t idx = i + (uint32_t)smk_m.menu_scroll[1];
                smk_ui_draw_list_row(34 + (int32_t)i * 24, SMK_ROOT_NAMES[idx],
                                     idx == (uint32_t)smk_m.root ? "<ACTIVE>" : 0,
                                     idx == (uint32_t)smk_m.menu_sel[1]);
            }
        } else if (smk_m.menu_sel[0] == 2) {
            /* SCALE selection list */
            smk_ui_draw_menu_frame("[ SCALE > SELECT ]", "[< >] CHOOSE  [OCT+] OK  [OCT-] CANCEL");
            uint32_t i;
            for (i = 0; i < 7 && (i + (uint32_t)smk_m.menu_scroll[1]) < SCALE_TOTAL; i++) {
                uint32_t idx = i + (uint32_t)smk_m.menu_scroll[1];
                smk_ui_draw_list_row(34 + (int32_t)i * 24, SCALE_TITLE[idx],
                                     idx == (uint32_t)smk_m.scale ? "<ACTIVE>" : 0,
                                     idx == (uint32_t)smk_m.menu_sel[1]);
            }
        }
    }
}

/* Page 2: Chord Menu */
static void smk_ui_draw_chord(void)
{
    char buf[32];
    if (smk_m.menu_depth == 0) {
        smk_ui_draw_menu_frame("[ CHORD MENU ]", 0);
        smk_ui_draw_list_row(34, "1. ACTIVE:", smk_m.chord_on ? "[ON]" : "[OFF]", smk_m.menu_sel[0] == 0);
        smk_ui_draw_list_row(58, "2. CHORD TYPE:", SMK_CHORD_NAMES[smk_m.chord_type % CH_TYPE_COUNT], smk_m.menu_sel[0] == 1);
        smk_ui_draw_list_row(82, "3. VOICING:", SMK_VOIC_NAMES[smk_m.chord_voic % VC_COUNT], smk_m.menu_sel[0] == 2);

        buf[0] = (char)('0' + smk_m.bpm / 100u);
        buf[1] = (char)('0' + (smk_m.bpm % 100u) / 10u);
        buf[2] = (char)('0' + smk_m.bpm % 10u);
        buf[3] = ' '; buf[4] = 'B'; buf[5] = 'P'; buf[6] = 'M'; buf[7] = 0;
        smk_ui_draw_list_row(106, "4. TEMPO:", buf, smk_m.menu_sel[0] == 3);
    } else {
        if (smk_m.menu_sel[0] == 1) {
            smk_ui_draw_menu_frame("[ CHORD > TYPE ]", "[< >] CHOOSE  [OCT+] OK  [OCT-] CANCEL");
            uint32_t i;
            for (i = 0; i < 7 && (i + (uint32_t)smk_m.menu_scroll[1]) < CH_TYPE_COUNT; i++) {
                uint32_t idx = i + (uint32_t)smk_m.menu_scroll[1];
                smk_ui_draw_list_row(34 + (int32_t)i * 24, SMK_CHORD_NAMES[idx],
                                     idx == (uint32_t)smk_m.chord_type ? "<ACTIVE>" : 0,
                                     idx == (uint32_t)smk_m.menu_sel[1]);
            }
        } else if (smk_m.menu_sel[0] == 2) {
            smk_ui_draw_menu_frame("[ CHORD > VOICING ]", "[< >] CHOOSE  [OCT+] OK  [OCT-] CANCEL");
            uint32_t i;
            for (i = 0; i < VC_COUNT; i++) {
                smk_ui_draw_list_row(34 + (int32_t)i * 24, SMK_VOIC_NAMES[i],
                                     i == (uint32_t)smk_m.chord_voic ? "<ACTIVE>" : 0,
                                     i == (uint32_t)smk_m.menu_sel[1]);
            }
        }
    }
}

/* Page 3: Arpeggiator Menu */
static void smk_ui_draw_arp(void)
{
    char buf[32];
    if (smk_m.menu_depth == 0) {
        smk_ui_draw_menu_frame("[ ARP MENU ]", 0);
        smk_ui_draw_list_row(34, "1. ACTIVE:", smk_m.arp_on ? "[ON]" : "[OFF]", smk_m.menu_sel[0] == 0);
        smk_ui_draw_list_row(58, "2. MODE:", SMK_ARP_MODE_NAMES[smk_m.arp_mode % 6], smk_m.menu_sel[0] == 1);
        smk_ui_draw_list_row(82, "3. RATE:", SMK_RATE_NAMES[smk_m.arp_rate % 6], smk_m.menu_sel[0] == 2);

        buf[0] = (char)('0' + smk_m.arp_oct); buf[1] = 0;
        smk_ui_draw_list_row(106, "4. OCTAVES:", buf, smk_m.menu_sel[0] == 3);
        smk_ui_draw_list_row(130, "5. GATE:", SMK_GATE_NAMES[smk_m.arp_gate % 4], smk_m.menu_sel[0] == 4);
        smk_ui_draw_list_row(154, "6. LATCH:", smk_m.arp_latch ? "[ON]" : "[OFF]", smk_m.menu_sel[0] == 5);

        buf[0] = (char)('0' + smk_m.bpm / 100u);
        buf[1] = (char)('0' + (smk_m.bpm % 100u) / 10u);
        buf[2] = (char)('0' + smk_m.bpm % 10u);
        buf[3] = ' '; buf[4] = 'B'; buf[5] = 'P'; buf[6] = 'M'; buf[7] = 0;
        smk_ui_draw_list_row(178, "7. TEMPO:", buf, smk_m.menu_sel[0] == 6);
    } else {
        if (smk_m.menu_sel[0] == 1) {
            smk_ui_draw_menu_frame("[ ARP > MODE ]", "[< >] CHOOSE  [OCT+] OK  [OCT-] CANCEL");
            uint32_t i;
            for (i = 0; i < 6; i++) {
                smk_ui_draw_list_row(34 + (int32_t)i * 24, SMK_ARP_MODE_NAMES[i],
                                     i == (uint32_t)smk_m.arp_mode ? "<ACTIVE>" : 0,
                                     i == (uint32_t)smk_m.menu_sel[1]);
            }
        } else if (smk_m.menu_sel[0] == 2) {
            smk_ui_draw_menu_frame("[ ARP > RATE ]", "[< >] CHOOSE  [OCT+] OK  [OCT-] CANCEL");
            uint32_t i;
            for (i = 0; i < 6; i++) {
                smk_ui_draw_list_row(34 + (int32_t)i * 24, SMK_RATE_NAMES[i],
                                     i == (uint32_t)smk_m.arp_rate ? "<ACTIVE>" : 0,
                                     i == (uint32_t)smk_m.menu_sel[1]);
            }
        } else if (smk_m.menu_sel[0] == 3) {
            smk_ui_draw_menu_frame("[ ARP > OCTAVES ]", "[< >] CHOOSE  [OCT+] OK  [OCT-] CANCEL");
            uint32_t i;
            for (i = 0; i < 4; i++) {
                buf[0] = (char)('1' + i); buf[1] = ' '; buf[2] = 'O'; buf[3] = 'C'; buf[4] = 'T'; buf[5] = 0;
                smk_ui_draw_list_row(34 + (int32_t)i * 24, buf,
                                     i == (uint32_t)(smk_m.arp_oct - 1) ? "<ACTIVE>" : 0,
                                     i == (uint32_t)smk_m.menu_sel[1]);
            }
        } else if (smk_m.menu_sel[0] == 4) {
            smk_ui_draw_menu_frame("[ ARP > GATE ]", "[< >] CHOOSE  [OCT+] OK  [OCT-] CANCEL");
            uint32_t i;
            for (i = 0; i < 4; i++) {
                smk_ui_draw_list_row(34 + (int32_t)i * 24, SMK_GATE_NAMES[i],
                                     i == (uint32_t)smk_m.arp_gate ? "<ACTIVE>" : 0,
                                     i == (uint32_t)smk_m.menu_sel[1]);
            }
        }
    }
}

/* Page 4: Note Repeat Menu */
static void smk_ui_draw_nr(void)
{
    char buf[32];
    if (smk_m.menu_depth == 0) {
        smk_ui_draw_menu_frame("[ NOTE REPEAT ]", 0);
        smk_ui_draw_list_row(34, "1. ACTIVE:", smk_m.nr_on ? "[ON]" : "[OFF]", smk_m.menu_sel[0] == 0);
        smk_ui_draw_list_row(58, "2. RATE:", SMK_RATE_NAMES[smk_m.nr_rate % 6], smk_m.menu_sel[0] == 1);
        smk_ui_draw_list_row(82, "3. GATE:", SMK_GATE_NAMES[smk_m.nr_gate % 4], smk_m.menu_sel[0] == 2);

        buf[0] = (char)('0' + smk_m.bpm / 100u);
        buf[1] = (char)('0' + (smk_m.bpm % 100u) / 10u);
        buf[2] = (char)('0' + smk_m.bpm % 10u);
        buf[3] = ' '; buf[4] = 'B'; buf[5] = 'P'; buf[6] = 'M'; buf[7] = 0;
        smk_ui_draw_list_row(106, "4. TEMPO:", buf, smk_m.menu_sel[0] == 3);
    } else {
        if (smk_m.menu_sel[0] == 1) {
            smk_ui_draw_menu_frame("[ REPEAT > RATE ]", "[< >] CHOOSE  [OCT+] OK  [OCT-] CANCEL");
            uint32_t i;
            for (i = 0; i < 6; i++) {
                smk_ui_draw_list_row(34 + (int32_t)i * 24, SMK_RATE_NAMES[i],
                                     i == (uint32_t)smk_m.nr_rate ? "<ACTIVE>" : 0,
                                     i == (uint32_t)smk_m.menu_sel[1]);
            }
        } else if (smk_m.menu_sel[0] == 2) {
            smk_ui_draw_menu_frame("[ REPEAT > GATE ]", "[< >] CHOOSE  [OCT+] OK  [OCT-] CANCEL");
            uint32_t i;
            for (i = 0; i < 4; i++) {
                smk_ui_draw_list_row(34 + (int32_t)i * 24, SMK_GATE_NAMES[i],
                                     i == (uint32_t)smk_m.nr_gate ? "<ACTIVE>" : 0,
                                     i == (uint32_t)smk_m.menu_sel[1]);
            }
        }
    }
}

/* Page 5: Globe / Settings Menu */
static void smk_ui_draw_globe(void)
{
    char buf[32];
    static const char *const VEL_CURVES[3] = {"NORMAL", "SOFT", "HARD"};
    static const char *const SENS_NAMES[3] = {"NORMAL", "LOW", "HIGH"};
    static const char *const SPEED_NAMES[3] = {"NORMAL", "SLOW", "FAST"};

    if (smk_m.menu_depth == 0) {
        smk_ui_draw_menu_frame("[ GLOBE SETTINGS ]", 0);
        uint32_t i;
        for (i = 0; i < 7 && (i + (uint32_t)smk_m.menu_scroll[0]) < 9u; i++) {
            uint32_t idx = i + (uint32_t)smk_m.menu_scroll[0];
            int32_t y = 34 + (int32_t)i * 24;
            switch (idx) {
            case 0: smk_ui_draw_list_row(y, "1. CC ASSIGN", ">", idx == (uint32_t)smk_m.menu_sel[0]); break;
            case 1: smk_ui_draw_list_row(y, "2. PAD MODE:", smk_m.pad_mode ? "[CC]" : "[NOTES]", idx == (uint32_t)smk_m.menu_sel[0]); break;
            case 2: smk_ui_draw_list_row(y, "3. FULL VELOCITY:", smk_m.full_velocity ? "[ON]" : "[OFF]", idx == (uint32_t)smk_m.menu_sel[0]); break;
            case 3: smk_ui_draw_list_row(y, "4. 4 BANKS:", smk_m.four_banks ? "[ON]" : "[OFF]", idx == (uint32_t)smk_m.menu_sel[0]); break;
            case 4: smk_ui_draw_list_row(y, "5. KEY VELOCITY:", VEL_CURVES[smk_m.key_vel_curve % 3], idx == (uint32_t)smk_m.menu_sel[0]); break;
            case 5: smk_ui_draw_list_row(y, "6. PAD SENS:", SENS_NAMES[smk_m.pad_sens % 3], idx == (uint32_t)smk_m.menu_sel[0]); break;
            case 6: smk_ui_draw_list_row(y, "7. KNOB SPEED:", SPEED_NAMES[smk_m.knob_speed % 3], idx == (uint32_t)smk_m.menu_sel[0]); break;
            case 7: smk_ui_draw_list_row(y, "8. MIDI CHANNELS", ">", idx == (uint32_t)smk_m.menu_sel[0]); break;
            case 8: {
                buf[0] = (char)('0' + smk_m.bpm / 100u);
                buf[1] = (char)('0' + (smk_m.bpm % 100u) / 10u);
                buf[2] = (char)('0' + smk_m.bpm % 10u);
                buf[3] = ' '; buf[4] = 'B'; buf[5] = 'P'; buf[6] = 'M'; buf[7] = 0;
                smk_ui_draw_list_row(y, "9. TEMPO:", buf, idx == (uint32_t)smk_m.menu_sel[0]);
                break;
            }
            }
        }
    } else {
        if (smk_m.menu_sel[0] == 0) {
            /* CC ASSIGN MODE */
            smk_ui_draw_menu_frame("[ CC ASSIGN ]", "[< >] SET CC  [OCT+] SAVE  [OCT-] CANCEL");
            if (!smk_m.cc_learn_seen) {
                sg_text(16, 60, 1, "Move any Knob, Fader or Pad", CLR_INK);
                sg_text(16, 80, 1, "to select it for CC editing...", CLR_DIM);
            } else {
                const char *tgt_type = smk_m.cc_learn_target == 0 ? "KNOB" :
                                       smk_m.cc_learn_target == 1 ? "FADER" : "PAD";
                sg_text(16, 50, 1, "SELECTED CONTROL:", CLR_DIM);

                buf[0] = '#';
                buf[1] = (char)('0' + (smk_m.cc_learn_idx + 1u) / 10u);
                buf[2] = (char)('0' + (smk_m.cc_learn_idx + 1u) % 10u);
                buf[3] = 0;
                sg_text(16, 70, 2, tgt_type, CLR_INK);
                sg_text(100, 70, 2, buf, CLR_INK);

                sg_text(16, 110, 1, "ASSIGNED CC:", CLR_DIM);
                buf[0] = (char)('0' + smk_m.cc_learn_val / 100u);
                buf[1] = (char)('0' + (smk_m.cc_learn_val % 100u) / 10u);
                buf[2] = (char)('0' + smk_m.cc_learn_val % 10u);
                buf[3] = 0;
                sg_text(16, 130, 2, buf, CLR_INK);

                sg_text(16, 170, 1, "Use < > to change, OCT+ to confirm", CLR_DIM);
            }
        } else if (smk_m.menu_sel[0] == 4) {
            /* KEY VELOCITY */
            smk_ui_draw_menu_frame("[ KEY VELOCITY CURVE ]", "[< >] CHOOSE  [OCT+] OK  [OCT-] CANCEL");
            uint32_t i;
            for (i = 0; i < 3; i++) {
                smk_ui_draw_list_row(34 + (int32_t)i * 24, VEL_CURVES[i],
                                     i == (uint32_t)smk_m.key_vel_curve ? "<ACTIVE>" : 0,
                                     i == (uint32_t)smk_m.menu_sel[1]);
            }
        } else if (smk_m.menu_sel[0] == 5) {
            /* PAD SENSITIVITY */
            smk_ui_draw_menu_frame("[ PAD SENSITIVITY ]", "[< >] CHOOSE  [OCT+] OK  [OCT-] CANCEL");
            uint32_t i;
            for (i = 0; i < 3; i++) {
                smk_ui_draw_list_row(34 + (int32_t)i * 24, SENS_NAMES[i],
                                     i == (uint32_t)smk_m.pad_sens ? "<ACTIVE>" : 0,
                                     i == (uint32_t)smk_m.menu_sel[1]);
            }
        } else if (smk_m.menu_sel[0] == 6) {
            /* KNOB SPEED */
            smk_ui_draw_menu_frame("[ KNOB SPEED ]", "[< >] CHOOSE  [OCT+] OK  [OCT-] CANCEL");
            uint32_t i;
            for (i = 0; i < 3; i++) {
                smk_ui_draw_list_row(34 + (int32_t)i * 24, SPEED_NAMES[i],
                                     i == (uint32_t)smk_m.knob_speed ? "<ACTIVE>" : 0,
                                     i == (uint32_t)smk_m.menu_sel[1]);
            }
        } else if (smk_m.menu_sel[0] == 7) {
            /* MIDI CHANNELS */
            smk_ui_draw_menu_frame("[ MIDI CHANNELS ]", "[< >] CH 01..16  [OCT+] NEXT  [OCT-] BACK");

            buf[0] = (char)('0' + smk_m.ch_keyboard / 10u);
            buf[1] = (char)('0' + smk_m.ch_keyboard % 10u);
            buf[2] = 0;
            smk_ui_draw_list_row(34, "1. KEYBOARD CH:", buf, smk_m.menu_sel[1] == 0);

            buf[0] = (char)('0' + smk_m.ch_knobs / 10u);
            buf[1] = (char)('0' + smk_m.ch_knobs % 10u);
            buf[2] = 0;
            smk_ui_draw_list_row(58, "2. KNOBS CH:", buf, smk_m.menu_sel[1] == 1);

            buf[0] = (char)('0' + smk_m.ch_faders / 10u);
            buf[1] = (char)('0' + smk_m.ch_faders % 10u);
            buf[2] = 0;
            smk_ui_draw_list_row(82, "3. FADERS CH:", buf, smk_m.menu_sel[1] == 2);

            buf[0] = (char)('0' + smk_m.ch_pads / 10u);
            buf[1] = (char)('0' + smk_m.ch_pads % 10u);
            buf[2] = 0;
            smk_ui_draw_list_row(106, "4. PADS CH:", buf, smk_m.menu_sel[1] == 3);
        }
    }
}

/* Master paint callback invoked by sg_step() */
void smk_midi_paint(void)
{
    switch (smk_m.page) {
    case SMK_PAGE_GEN:    smk_ui_draw_general(); break;
    case SMK_PAGE_SCALE:  smk_ui_draw_scale(); break;
    case SMK_PAGE_CHORD:  smk_ui_draw_chord(); break;
    case SMK_PAGE_ARP:    smk_ui_draw_arp(); break;
    case SMK_PAGE_NR:     smk_ui_draw_nr(); break;
    case SMK_PAGE_GLOBE:  smk_ui_draw_globe(); break;
    default:              smk_ui_draw_general(); break;
    }
}
