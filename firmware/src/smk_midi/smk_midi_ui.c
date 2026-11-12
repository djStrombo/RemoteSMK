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

/* Page 0: General Screen (1-Octave Keyboard, Notes & Detected Chords) */
static void smk_ui_draw_general(void)
{
    char buf[32];
    sg_rect(0, 0, 240, 240, CLR_BG);

    /* Header */
    sg_text(10, 8, 1, "REMOTE SMK", CLR_INK);

    buf[0] = 'B'; buf[1] = 'P'; buf[2] = 'M'; buf[3] = ':';
    buf[4] = (char)('0' + smk_m.bpm / 100u);
    buf[5] = (char)('0' + (smk_m.bpm % 100u) / 10u);
    buf[6] = (char)('0' + smk_m.bpm % 10u);
    buf[7] = ' '; buf[8] = 'O'; buf[9] = 'C'; buf[10] = 'T'; buf[11] = ':';
    buf[12] = smk_m.octave < 0 ? '-' : '+';
    buf[13] = (char)('0' + (smk_m.octave < 0 ? -smk_m.octave : smk_m.octave));
    buf[14] = 0;
    sg_text(118, 8, 1, buf, CLR_INK);
    sg_rect(0, 24, 240, 1, CLR_INK);

    /* 1-Octave Keyboard graphic:
     * 7 white keys (28px each = 196px total): C, D, E, F, G, A, B
     * Start at x = 22, y = 36, height = 68 */
    static const uint8_t WHITE_PITCH[7] = {0, 2, 4, 5, 7, 9, 11};
    static const int16_t WHITE_X[7] = {22, 50, 78, 106, 134, 162, 190};
    uint32_t i;

    for (i = 0; i < 7; i++) {
        int held = (smk_m.held_pitch_mask >> WHITE_PITCH[i]) & 1u;
        int32_t x = WHITE_X[i];
        if (held) {
            sg_rect(x, 36, 27, 68, CLR_INK);
        } else {
            sg_rect(x, 36, 27, 68, CLR_INK);
            sg_rect(x + 1, 37, 25, 66, CLR_BG);
        }
    }

    /* 5 black keys: C#(1), D#(3), F#(6), G#(8), A#(10)
     * Width: 16px, height: 42px */
    static const uint8_t BLACK_PITCH[5] = {1, 3, 6, 8, 10};
    static const int16_t BLACK_X[5] = {42, 70, 126, 154, 182};

    for (i = 0; i < 5; i++) {
        int held = (smk_m.held_pitch_mask >> BLACK_PITCH[i]) & 1u;
        int32_t x = BLACK_X[i];
        if (held) {
            sg_rect(x, 36, 16, 42, CLR_INK);
            sg_rect(x + 3, 39, 10, 36, CLR_BG);
        } else {
            sg_rect(x, 36, 16, 42, CLR_INK);
            sg_rect(x + 1, 37, 14, 40, CLR_BG);
        }
    }

    /* Split Section: Left = Note / Chord; Right = Live Control HUD / Channels */
    sg_rect(10, 114, 220, 1, CLR_BORDER);
    sg_rect(118, 114, 1, 52, CLR_BORDER);

    /* Left side (x = 12..114) */
    if (smk_m.chord_name[0]) {
        sg_text(12, 118, 1, "CHORD DETECT:", CLR_DIM);
        sg_text(12, 134, 2, smk_m.chord_name, CLR_INK);
        if (smk_m.last_note_name[0]) {
            sg_text(12, 152, 1, smk_m.last_note_name, CLR_DIM);
        }
    } else if (smk_m.last_note_name[0]) {
        sg_text(12, 118, 1, "NOTE PLAYED:", CLR_DIM);
        sg_text(12, 134, 2, smk_m.last_note_name, CLR_INK);
        char vbuf[16];
        snprintf(vbuf, sizeof(vbuf), "VEL: %u", (unsigned)smk_m.last_note_vel);
        sg_text(12, 152, 1, vbuf, CLR_DIM);
    } else {
        sg_text(12, 122, 1, "NOTE / CHORD:", CLR_DIM);
        sg_text(12, 140, 1, "-- READY --", CLR_DIM);
    }

    /* Right side (x = 124..238) */
    uint32_t now = plat_ms();
    int show_hud = (smk_m.last_ctrl_ms > 0 && (now - smk_m.last_ctrl_ms) < 2000u && smk_m.last_ctrl_type != SMK_HUD_NONE);

    if (show_hud) {
        switch (smk_m.last_ctrl_type) {
        case SMK_HUD_KNOB: {
            sg_text(124, 118, 1, "CONTROL MOVED:", CLR_DIM);
            char hbuf[20];
            snprintf(hbuf, sizeof(hbuf), "KNOB K%u  (B%u)", (unsigned)(smk_m.last_ctrl_id + 1), (unsigned)(smk_m.last_ctrl_bank + 1));
            sg_text(124, 134, 2, hbuf, CLR_INK);
            char vbuf[20];
            snprintf(vbuf, sizeof(vbuf), "CC:%-3u VAL:%-3u", (unsigned)smk_m.last_ctrl_cc, (unsigned)smk_m.last_ctrl_val);
            sg_text(124, 152, 1, vbuf, CLR_INK);
            break;
        }
        case SMK_HUD_FADER: {
            sg_text(124, 118, 1, "CONTROL MOVED:", CLR_DIM);
            char hbuf[20];
            snprintf(hbuf, sizeof(hbuf), "FADER F%u (B%u)", (unsigned)(smk_m.last_ctrl_id + 1), (unsigned)(smk_m.last_ctrl_bank + 1));
            sg_text(124, 134, 2, hbuf, CLR_INK);
            char vbuf[20];
            snprintf(vbuf, sizeof(vbuf), "CC:%-3u VAL:%-3u", (unsigned)smk_m.last_ctrl_cc, (unsigned)smk_m.last_ctrl_val);
            sg_text(124, 152, 1, vbuf, CLR_INK);
            break;
        }
        case SMK_HUD_PAD: {
            sg_text(124, 118, 1, "PAD HIT:", CLR_DIM);
            char hbuf[20];
            snprintf(hbuf, sizeof(hbuf), "PAD P%u  (B%u)", (unsigned)(smk_m.last_ctrl_id + 1), (unsigned)(smk_m.last_ctrl_bank + 1));
            sg_text(124, 134, 2, hbuf, CLR_INK);
            char vbuf[24];
            if (smk_m.last_ctrl_sub == 0) {
                char nstr[12];
                smk_ui_format_note(nstr, smk_m.last_ctrl_cc);
                snprintf(vbuf, sizeof(vbuf), "NOTE:%-3s V:%-3u", nstr, (unsigned)smk_m.last_ctrl_val);
            } else {
                snprintf(vbuf, sizeof(vbuf), "CC:%-3u VAL:%-3u", (unsigned)smk_m.last_ctrl_cc, (unsigned)smk_m.last_ctrl_val);
            }
            sg_text(124, 152, 1, vbuf, CLR_INK);
            break;
        }
        case SMK_HUD_TOAST: {
            sg_text(124, 118, 1, "STATUS / ACTION:", CLR_DIM);
            sg_text(124, 136, 1, smk_m.last_ctrl_text, CLR_INK);
            break;
        }
        default: break;
        }
    } else {
        sg_text(124, 118, 1, "MIDI CHANNELS:", CLR_DIM);
        char ch1[24], ch2[24];
        snprintf(ch1, sizeof(ch1), "KBD:%02u   KNB:%02u", (unsigned)smk_m.ch_keyboard, (unsigned)smk_m.ch_knobs);
        snprintf(ch2, sizeof(ch2), "FDR:%02u   PAD:%02u", (unsigned)smk_m.ch_faders, (unsigned)smk_m.ch_pads);
        sg_text(124, 134, 1, ch1, CLR_INK);
        sg_text(124, 150, 1, ch2, CLR_INK);
    }

    /* Status Summary */
    sg_rect(10, 166, 220, 1, CLR_BORDER);

    /* Line 1: Engines */
    char ebuf[40];
    snprintf(ebuf, sizeof(ebuf), "SCL:%s  CHD:%s  ARP:%s  REP:%s",
             smk_m.scale_on ? "ON" : "--",
             smk_m.chord_on ? "ON" : "--",
             smk_m.arp_on ? "ON" : "--",
             smk_m.nr_on ? "ON" : "--");
    sg_text(12, 174, 1, ebuf, CLR_INK);

    /* Line 2: Banks */
    char max_b = smk_m.four_banks ? '4' : '2';
    char bbuf[44];
    snprintf(bbuf, sizeof(bbuf), "KNB:B%u/%c FDR:B%u/%c PAD:B%u/%c [%s]",
             (unsigned)(smk_m.knob_bank + 1), max_b,
             (unsigned)(smk_m.fader_bank + 1), max_b,
             (unsigned)(smk_m.pad_bank + 1), max_b,
             smk_m.pad_mode == 0 ? "NOTES" : "CC");
    sg_text(12, 192, 1, bbuf, CLR_DIM);

    /* Bottom navigation shortcuts */
    sg_rect(0, 214, 240, 1, CLR_INK);
    sg_text(8, 222, 1, "[SCALE]  [CHORD]  [ARP]  [REPEAT]  [GLOBE]", CLR_INK);
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
