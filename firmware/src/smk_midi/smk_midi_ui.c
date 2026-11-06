/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2026 RemoteSMK contributors */
#pragma once
#include "smk_midi_types.h"
#include "smk_midi_scales.h"

/* Theme colour helpers */
#define CLR_BG      0x0000u
#define CLR_PANEL   0x31EBu  /* Dark blue-grey */
#define CLR_INK     0xFFFFu  /* White */
#define CLR_DIM     0x9492u  /* Light grey */
#define CLR_ACC1    0xFB88u  /* Bright Coral */
#define CLR_ACC2    0x35BFu  /* Bright Cyan */
#define CLR_ACC3    0xFF69u  /* Yellow */
#define CLR_ACC4    0xA788u  /* Bright Lime Green */
#define CLR_LINE    0x2124u  /* Subtle dark border */

static const char *const SMK_ROOT_NAMES[12] = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

static const char *const SMK_PAGE_TITLES[SMK_PAGE_COUNT] = {
    "LIVE", "SCALE", "CHORD", "ARPEGGIATOR", "SEQUENCER", "SONG MODE", "CC MAPPER"
};

/* Forward declarations of sa_gfx primitives */
static void sg_rect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t c);
static void sg_chip(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t c);
static int32_t sg_text(int32_t x, int32_t y, int32_t n, const char *s, uint16_t c);
static int32_t sg_text_w(const char *s, int32_t n);

static inline void smk_ui_draw_header(const char *title)
{
    uint32_t i;
    /* Top header bar */
    sg_rect(0, 0, 240, 24, CLR_PANEL);
    sg_text(8, 6, 1, title, CLR_ACC2);

    /* Page navigation tabs/dots */
    for (i = 0; i < SMK_PAGE_COUNT; i++) {
        uint16_t col = (i == smk_m.page) ? CLR_ACC3 : CLR_DIM;
        sg_rect(160 + (int32_t)i * 10, 10, 6, 6, col);
    }
}

static inline void smk_ui_draw_param_bar(const char *p1, const char *p2, const char *p3, const char *p4, uint8_t active_field)
{
    uint32_t i;
    const char *names[4] = { p1, p2, p3, p4 };
    sg_rect(0, 194, 240, 46, CLR_PANEL);
    sg_rect(0, 194, 240, 1, CLR_ACC2);

    for (i = 0; i < 4; i++) {
        if (!names[i]) continue;
        int32_t x = (int32_t)i * 60;
        if (i == active_field) {
            sg_chip(x + 2, 196, 56, 22, CLR_ACC3);
            sg_text(x + 6, 202, 1, names[i], CLR_BG);
        } else {
            sg_text(x + 6, 202, 1, names[i], CLR_DIM);
        }
        if (i > 0) sg_rect(x, 196, 1, 22, CLR_LINE);
    }

    sg_rect(0, 220, 240, 1, CLR_LINE);
    sg_text(6, 224, 1, "[< >] EDIT   [OCT+] NEXT   [OCT-] LIVE", CLR_ACC2);
}

/* Page 0: Live Performance Controller View */
static void smk_ui_draw_live(void)
{
    char buf[32];
    /* Top Header Bar */
    sg_rect(0, 0, 240, 24, CLR_PANEL);
    sg_text(8, 6, 1, "LIVE CONTROLLER", CLR_ACC2);

    /* Master CH pill */
    buf[0] = 'C'; buf[1] = 'H'; buf[2] = ':'; buf[3] = ' ';
    buf[4] = (char)('0' + smk_m.master_ch / 10u);
    buf[5] = (char)('0' + smk_m.master_ch % 10u);
    buf[6] = 0;
    sg_text(138, 6, 1, buf, CLR_ACC3);

    /* Octave pill */
    buf[0] = 'O'; buf[1] = 'C'; buf[2] = 'T'; buf[3] = ':';
    buf[4] = smk_m.octave < 0 ? '-' : '+';
    buf[5] = (char)('0' + (smk_m.octave < 0 ? -smk_m.octave : smk_m.octave));
    buf[6] = 0;
    sg_text(188, 6, 1, buf, CLR_ACC4);

    /* Central Main Display Box */
    sg_rect(8, 28, 224, 96, CLR_PANEL);
    uint16_t border_col = (smk_m.note_held_cnt > 0) ? CLR_ACC4 : (smk_m.last_note > 0 ? CLR_ACC2 : CLR_LINE);
    sg_rect(8, 28, 224, 1, border_col);
    sg_rect(8, 123, 224, 1, border_col);

    if (smk_m.note_held_cnt > 0) {
        sg_text(16, 34, 1, "[PLAYING]", CLR_ACC4);
    } else if (smk_m.last_note > 0) {
        sg_text(16, 34, 1, "[LAST PLAYED]", CLR_DIM);
    } else {
        sg_text(16, 34, 1, "[READY - ALL CONTROLS ACTIVE]", CLR_DIM);
    }

    if (smk_m.last_note > 0 && smk_m.last_chord_str[0]) {
        /* Large Note or Chord Name */
        sg_text(16, 48, 2, smk_m.last_chord_str, CLR_ACC3);

        /* Notes breakdown if chord */
        if (smk_m.last_notes_str[0]) {
            sg_text(16, 76, 1, smk_m.last_notes_str, CLR_INK);
        } else {
            /* Single note info */
            buf[0] = 'M'; buf[1] = 'I'; buf[2] = 'D'; buf[3] = 'I'; buf[4] = ' ';
            buf[5] = '#';
            buf[6] = (char)('0' + smk_m.last_note / 100u);
            buf[7] = (char)('0' + (smk_m.last_note % 100u) / 10u);
            buf[8] = (char)('0' + smk_m.last_note % 10u);
            buf[9] = 0;
            sg_text(16, 76, 1, buf, CLR_DIM);
        }

        /* Velocity indicator */
        buf[0] = 'V'; buf[1] = 'E'; buf[2] = 'L'; buf[3] = ':'; buf[4] = ' ';
        buf[5] = (char)('0' + smk_m.last_vel / 100u);
        buf[6] = (char)('0' + (smk_m.last_vel % 100u) / 10u);
        buf[7] = (char)('0' + smk_m.last_vel % 10u);
        buf[8] = 0;
        sg_text(16, 98, 1, buf, CLR_DIM);

        /* Velocity bar */
        int32_t bar_w = ((int32_t)smk_m.last_vel * 130) / 127;
        sg_rect(80, 102, 140, 6, CLR_LINE);
        if (bar_w > 0) sg_rect(80, 102, bar_w, 6, CLR_ACC1);
    } else {
        sg_text(24, 64, 2, "PLAY KEYS / PADS", CLR_DIM);
        sg_text(24, 94, 1, "Turn knobs, move faders, wheels", CLR_DIM);
    }

    /* Sub-Strip: Scale / Chord / Arp status (y=128..162) */
    sg_chip(8, 128, 110, 32, CLR_PANEL);
    uint32_t sc_id = (uint32_t)(smk_m.scale >= 0 && (uint32_t)smk_m.scale < SCALE_TOTAL ? smk_m.scale : 0);
    const char *rname = SMK_ROOT_NAMES[smk_m.root % 12];
    buf[0] = rname[0];
    int bp = 1;
    if (rname[1]) buf[bp++] = rname[1];
    buf[bp++] = ' ';
    const char *sct = SCALE_TITLE[sc_id];
    int sp = 0;
    while (sct[sp] && bp < 15) buf[bp++] = sct[sp++];
    buf[bp] = 0;
    sg_text(14, 132, 1, "SCALE:", CLR_DIM);
    sg_text(14, 144, 1, buf, CLR_ACC2);

    sg_chip(122, 128, 110, 32, CLR_PANEL);
    if (smk_m.arp_on) {
        sg_text(128, 132, 1, "ARP ACTIVE:", CLR_DIM);
        sg_text(128, 144, 1, SMK_ARP_MODE_NAMES[smk_m.arp_mode % 7], CLR_ACC4);
    } else if (smk_m.chord_mode > 0) {
        sg_text(128, 132, 1, "CHORD ACTIVE:", CLR_DIM);
        sg_text(128, 144, 1, SMK_CHORD_NAMES[smk_m.chord_mode % CH_COUNT], CLR_ACC1);
    } else {
        sg_text(128, 132, 1, "ENGINE:", CLR_DIM);
        sg_text(128, 144, 1, "DIRECT NOTE", CLR_INK);
    }

    /* Control HUD / Bank Status (y=164..192) */
    if (smk_m.hud_ms > 0 && smk_m.hud_msg[0]) {
        sg_chip(8, 164, 224, 30, CLR_PANEL);
        sg_text(14, 167, 1, smk_m.hud_msg, CLR_ACC3);
        int32_t hw = ((int32_t)smk_m.hud_val * 212) / 127;
        sg_rect(14, 182, 212, 6, CLR_LINE);
        if (hw > 0) sg_rect(14, 182, hw, 6, CLR_ACC2);
    } else {
        /* Bank Status Pills */
        sg_chip(8, 164, 70, 30, CLR_PANEL);
        buf[0] = 'K'; buf[1] = 'B'; buf[2] = ':'; buf[3] = (char)('1' + smk_m.knob_bank); buf[4] = 0;
        sg_text(14, 172, 1, buf, CLR_ACC1);

        sg_chip(85, 164, 70, 30, CLR_PANEL);
        buf[0] = 'F'; buf[1] = 'B'; buf[2] = ':'; buf[3] = (char)('1' + smk_m.fader_bank); buf[4] = 0;
        sg_text(91, 172, 1, buf, CLR_ACC2);

        sg_chip(162, 164, 70, 30, CLR_PANEL);
        buf[0] = 'P'; buf[1] = 'B'; buf[2] = ':'; buf[3] = (char)('1' + smk_m.pad_bank); buf[4] = 0;
        sg_text(168, 172, 1, buf, CLR_ACC3);
    }

    /* Bottom Navigation Bar (y=198..240) */
    sg_rect(0, 198, 240, 42, CLR_PANEL);
    sg_rect(0, 198, 240, 1, CLR_ACC2);
    sg_text(8, 204, 1, "[< >] CH / NAV    [OCT] -/+", CLR_INK);
    sg_text(8, 220, 1, "[ARP/SCL/CHD/GLO] MENUS", CLR_DIM);
}

/* Page 1: Scale Picker View */
static void smk_ui_draw_scl(void)
{
    char buf[32];
    uint32_t sc_id = (uint32_t)(smk_m.scale >= 0 && (uint32_t)smk_m.scale < SCALE_TOTAL ? smk_m.scale : 0);
    smk_ui_draw_header("SCALE PICKER");

    /* Family Badge */
    sg_chip(12, 32, 100, 20, CLR_PANEL);
    sg_text(18, 36, 1, SCALE_FAMILY_TITLE[SCALE_FAMILY[sc_id] % (SCALE_FAMILIES + 2u)], CLR_ACC1);

    /* Root Note Badge - highlight if edit_subfield == 0 */
    sg_chip(120, 32, 108, 20, smk_m.edit_subfield == 0 ? CLR_ACC3 : CLR_PANEL);
    buf[0] = 'R'; buf[1] = 'O'; buf[2] = 'O'; buf[3] = 'T'; buf[4] = ':'; buf[5] = ' ';
    const char *rname = SMK_ROOT_NAMES[smk_m.root % 12];
    buf[6] = rname[0];
    buf[7] = rname[1];
    buf[8] = 0;
    sg_text(126, 36, 1, buf, smk_m.edit_subfield == 0 ? CLR_BG : CLR_ACC3);

    /* Scale Card Box - highlight border if edit_subfield == 1 */
    sg_rect(12, 58, 216, 76, CLR_PANEL);
    uint16_t sc_col = (smk_m.edit_subfield == 1) ? CLR_ACC3 : CLR_ACC2;
    sg_rect(12, 58, 216, 1, sc_col);
    sg_rect(12, 133, 216, 1, sc_col);

    /* Scale title large */
    sg_text(20, 68, 2, SCALE_TITLE[sc_id], CLR_INK);

    /* Scale short code & degrees */
    sg_text(20, 96, 1, N_SCALE[sc_id], CLR_ACC2);
    buf[0] = '(';
    uint32_t deg = SCALE_DEGREES[sc_id];
    buf[1] = (char)('0' + deg / 10u);
    buf[2] = (char)('0' + deg % 10u);
    buf[3] = ' '; buf[4] = 'n'; buf[5] = 'o'; buf[6] = 't'; buf[7] = 'e'; buf[8] = 's'; buf[9] = ')'; buf[10] = 0;
    sg_text(76, 96, 1, buf, CLR_DIM);

    /* Scale ID rank */
    buf[0] = '#';
    buf[1] = (char)('0' + (sc_id + 1u) / 10u);
    buf[2] = (char)('0' + (sc_id + 1u) % 10u);
    buf[3] = '/'; buf[4] = '7'; buf[5] = '0'; buf[6] = 0;
    sg_text(168, 96, 1, buf, CLR_ACC4);

    /* Chord Info Sub-Bar - highlight if edit_subfield == 2 or 3 */
    sg_chip(12, 140, 216, 48, CLR_PANEL);
    uint16_t c_col = (smk_m.edit_subfield == 2) ? CLR_ACC3 : CLR_DIM;
    sg_text(18, 146, 1, "CHORD TRIGGER:", c_col);
    sg_text(112, 146, 1, SMK_CHORD_NAMES[smk_m.chord_mode % CH_COUNT],
            smk_m.edit_subfield == 2 ? CLR_ACC3 : CLR_ACC1);

    uint16_t v_col = (smk_m.edit_subfield == 3) ? CLR_ACC3 : CLR_DIM;
    sg_text(18, 166, 1, "VOICING:", v_col);
    sg_text(80, 166, 1, SMK_VOIC_NAMES[smk_m.chord_voic % VC_COUNT],
            smk_m.edit_subfield == 3 ? CLR_ACC3 : CLR_INK);

    smk_ui_draw_param_bar("ROOT", "SCALE", "CHORD", "VOIC", smk_m.edit_subfield);
}

/* Page 2: Dedicated Chord Engine View */
static void smk_ui_draw_chord(void)
{
    char buf[32];
    smk_ui_draw_header("CHORD ENGINE");

    /* Root note card */
    sg_chip(12, 32, 100, 24, smk_m.edit_subfield == 2 ? CLR_ACC3 : CLR_PANEL);
    buf[0] = 'R'; buf[1] = 'O'; buf[2] = 'O'; buf[3] = 'T'; buf[4] = ':'; buf[5] = ' ';
    const char *rname = SMK_ROOT_NAMES[smk_m.root % 12];
    buf[6] = rname[0];
    buf[7] = rname[1];
    buf[8] = 0;
    sg_text(18, 38, 1, buf, smk_m.edit_subfield == 2 ? CLR_BG : CLR_ACC3);

    /* Scale context */
    sg_chip(118, 32, 110, 24, CLR_PANEL);
    uint32_t sc_id = (uint32_t)(smk_m.scale >= 0 && (uint32_t)smk_m.scale < SCALE_TOTAL ? smk_m.scale : 0);
    sg_text(124, 38, 1, SCALE_TITLE[sc_id], CLR_ACC2);

    /* Chord Mode Banner - highlight if edit_subfield == 0 */
    sg_rect(12, 64, 216, 60, CLR_PANEL);
    uint16_t c_col = (smk_m.edit_subfield == 0) ? CLR_ACC3 : CLR_ACC1;
    sg_rect(12, 64, 216, 1, c_col);
    sg_rect(12, 123, 216, 1, c_col);

    sg_text(20, 72, 1, "CURRENT CHORD TYPE:", smk_m.edit_subfield == 0 ? CLR_ACC3 : CLR_DIM);
    sg_text(20, 90, 2, SMK_CHORD_NAMES[smk_m.chord_mode % CH_COUNT], CLR_INK);

    /* Voicing Card - highlight if edit_subfield == 1 */
    sg_chip(12, 134, 216, 44, CLR_PANEL);
    uint16_t v_col = (smk_m.edit_subfield == 1) ? CLR_ACC3 : CLR_DIM;
    sg_text(20, 142, 1, "VOICING SPREAD:", v_col);
    sg_text(120, 142, 1, SMK_VOIC_NAMES[smk_m.chord_voic % VC_COUNT],
            smk_m.edit_subfield == 1 ? CLR_ACC3 : CLR_ACC4);
    sg_text(20, 158, 1, "Triggered on every played key", CLR_DIM);

    smk_ui_draw_param_bar("TYPE", "VOIC", "ROOT", 0, smk_m.edit_subfield);
}

/* Page 3: Arpeggiator View */
static void smk_ui_draw_arp(void)
{
    char buf[24];
    smk_ui_draw_header("ARPEGGIATOR");

    /* Mode Banner - highlight if edit_subfield == 0 */
    sg_chip(12, 34, 130, 24, smk_m.edit_subfield == 0 ? CLR_ACC3 : CLR_PANEL);
    sg_text(18, 40, 1, "MODE: ", smk_m.edit_subfield == 0 ? CLR_BG : CLR_DIM);
    sg_text(60, 40, 1, SMK_ARP_MODE_NAMES[smk_m.arp_mode % 7],
            smk_m.edit_subfield == 0 ? CLR_BG : CLR_ACC1);

    /* Rate Banner - highlight if edit_subfield == 1 */
    sg_chip(150, 34, 78, 24, smk_m.edit_subfield == 1 ? CLR_ACC3 : CLR_PANEL);
    sg_text(156, 40, 1, "RATE:", smk_m.edit_subfield == 1 ? CLR_BG : CLR_DIM);
    sg_text(194, 40, 1, SMK_ARP_RATE_NAMES[smk_m.arp_rate % 6],
            smk_m.edit_subfield == 1 ? CLR_BG : CLR_ACC3);

    /* Center Graphic Display */
    sg_rect(12, 68, 216, 114, CLR_PANEL);
    sg_rect(12, 68, 216, 1, CLR_ACC1);
    sg_rect(12, 181, 216, 1, CLR_ACC1);

    /* Octaves - highlight if edit_subfield == 2 */
    uint16_t o_col = (smk_m.edit_subfield == 2) ? CLR_ACC3 : CLR_DIM;
    sg_text(20, 82, 1, "OCTAVES:", o_col);
    buf[0] = (char)('0' + smk_m.arp_oct); buf[1] = 0;
    sg_text(100, 82, 1, buf, smk_m.edit_subfield == 2 ? CLR_ACC3 : CLR_INK);

    /* Gate - highlight if edit_subfield == 3 */
    uint16_t g_col = (smk_m.edit_subfield == 3) ? CLR_ACC3 : CLR_DIM;
    sg_text(20, 108, 1, "GATE:", g_col);
    buf[0] = (char)('0' + smk_m.arp_gate / 10u);
    buf[1] = (char)('0' + smk_m.arp_gate % 10u);
    buf[2] = '%'; buf[3] = 0;
    sg_text(100, 108, 1, buf, smk_m.edit_subfield == 3 ? CLR_ACC3 : CLR_INK);

    /* Swing */
    sg_text(20, 134, 1, "SWING:", CLR_DIM);
    buf[0] = (char)('0' + smk_m.arp_swing / 10u);
    buf[1] = (char)('0' + smk_m.arp_swing % 10u);
    buf[2] = '%'; buf[3] = 0;
    sg_text(100, 134, 1, buf, CLR_INK);

    /* Latch indicator */
    sg_chip(130, 130, 86, 26, smk_m.arp_hold ? CLR_ACC4 : CLR_LINE);
    sg_text(138, 137, 1, smk_m.arp_hold ? "LATCH ON" : "LATCH OFF",
            smk_m.arp_hold ? CLR_BG : CLR_DIM);

    smk_ui_draw_param_bar("MODE", "RATE", "OCT", "GATE", smk_m.edit_subfield);
}

/* Page 4: Sequencer View */
static void smk_ui_draw_seq(void)
{
    char buf[24];
    uint32_t s;
    smk_ui_draw_header("SEQUENCER (1 TRK)");

    /* Pattern info bar - highlight if edit_subfield == 0 */
    sg_chip(12, 32, 100, 20, smk_m.edit_subfield == 0 ? CLR_ACC3 : CLR_PANEL);
    buf[0] = 'P'; buf[1] = 'A'; buf[2] = 'T'; buf[3] = ' ';
    buf[4] = (char)('0' + (smk_m.cur_pat + 1u) / 10u);
    buf[5] = (char)('0' + (smk_m.cur_pat + 1u) % 10u);
    buf[6] = 0;
    sg_text(18, 36, 1, buf, smk_m.edit_subfield == 0 ? CLR_BG : CLR_ACC2);

    /* Play/Rec status badge */
    sg_chip(120, 32, 108, 20, smk_m.seq_playing ? CLR_ACC4 : CLR_PANEL);
    sg_text(130, 36, 1, smk_m.seq_rec ? "[REC]" : (smk_m.seq_playing ? "[PLAYING]" : "[STOPPED]"),
            smk_m.seq_playing ? CLR_BG : CLR_DIM);

    /* Matrix Step Grid (4 rows of 16 steps = 64 steps) */
    sg_rect(12, 58, 216, 126, CLR_PANEL);
    uint32_t pat = smk_m.cur_pat % SMK_MIDI_NPATS;
    for (s = 0; s < 64u; s++) {
        int32_t gx = 20 + (int32_t)(s % 16u) * 12 + (int32_t)(s % 16u / 4u) * 3;
        int32_t gy = 70 + (int32_t)(s / 16u) * 26;
        int active = smk_patterns[pat][s].n > 0;
        int is_cur = smk_m.seq_playing && (s == smk_m.cur_step);
        uint16_t c = is_cur ? CLR_ACC3 : (active ? CLR_ACC1 : (s < smk_m.pat_len ? CLR_DIM : CLR_LINE));
        sg_rect(gx, gy, 8, 8, c);
    }

    smk_ui_draw_param_bar("PAT", "LEN", "DIV", "SWING", smk_m.edit_subfield);
}

/* Page 5: Song Mode View */
static void smk_ui_draw_song(void)
{
    char buf[32];
    uint32_t r;
    smk_ui_draw_header("SONG MODE (CHAIN)");

    /* Current row badge */
    sg_chip(12, 32, 110, 20, smk_m.edit_subfield == 0 ? CLR_ACC3 : CLR_PANEL);
    buf[0] = 'R'; buf[1] = 'O'; buf[2] = 'W'; buf[3] = ' ';
    buf[4] = (char)('0' + (smk_m.cur_song_row + 1u) / 10u);
    buf[5] = (char)('0' + (smk_m.cur_song_row + 1u) % 10u);
    buf[6] = '/';
    buf[7] = (char)('0' + smk_m.total_song_rows / 10u);
    buf[8] = (char)('0' + smk_m.total_song_rows % 10u);
    buf[9] = 0;
    sg_text(18, 36, 1, buf, smk_m.edit_subfield == 0 ? CLR_BG : CLR_ACC3);

    /* Chain Rows list preview */
    sg_rect(12, 58, 216, 126, CLR_PANEL);
    for (r = 0; r < 5u && r < smk_m.total_song_rows; r++) {
        uint32_t row_idx = (smk_m.cur_song_row + r) % smk_m.total_song_rows;
        int32_t ry = 64 + (int32_t)r * 22;
        uint16_t col = (r == 0) ? CLR_ACC2 : CLR_INK;
        buf[0] = '>'; buf[1] = ' ';
        buf[2] = (char)('0' + (row_idx + 1u) / 10u);
        buf[3] = (char)('0' + (row_idx + 1u) % 10u);
        buf[4] = ':'; buf[5] = ' '; buf[6] = 'P'; buf[7] = 'A'; buf[8] = 'T'; buf[9] = ' ';
        buf[10] = (char)('0' + (smk_song_chain[row_idx].pattern + 1u) / 10u);
        buf[11] = (char)('0' + (smk_song_chain[row_idx].pattern + 1u) % 10u);
        buf[12] = ' '; buf[13] = 'x';
        buf[14] = (char)('0' + smk_song_chain[row_idx].repeat / 10u);
        buf[15] = (char)('0' + smk_song_chain[row_idx].repeat % 10u);
        buf[16] = 0;
        sg_text(20, ry, 1, buf, col);
    }

    smk_ui_draw_param_bar("ROW", "PAT", "RPT", "PLAY", smk_m.edit_subfield);
}

/* Page 6: CC & Channel Mapper */
static void smk_ui_draw_cc(void)
{
    char buf[32];
    smk_ui_draw_header("MIDI CC MAPPER");

    /* Bank Status Pills */
    sg_chip(12, 30, 66, 20, CLR_PANEL);
    buf[0] = 'K'; buf[1] = 'B'; buf[2] = ':'; buf[3] = (char)('1' + smk_m.knob_bank); buf[4] = 0;
    sg_text(16, 34, 1, buf, CLR_ACC1);

    sg_chip(86, 30, 66, 20, CLR_PANEL);
    buf[0] = 'F'; buf[1] = 'B'; buf[2] = ':'; buf[3] = (char)('1' + smk_m.fader_bank); buf[4] = 0;
    sg_text(90, 34, 1, buf, CLR_ACC2);

    sg_chip(160, 30, 68, 20, CLR_PANEL);
    buf[0] = 'P'; buf[1] = 'B'; buf[2] = ':'; buf[3] = (char)('1' + smk_m.pad_bank); buf[4] = 0;
    sg_text(164, 34, 1, buf, CLR_ACC3);

    /* Inspector Card */
    sg_rect(12, 56, 216, 130, CLR_PANEL);
    sg_rect(12, 56, 216, 1, CLR_ACC4);
    sg_rect(12, 185, 216, 1, CLR_ACC4);

    const char *tgt_names[3] = {"KNOB", "FADER", "PAD"};
    uint32_t ttype = smk_m.cc_edit_target % 3;
    uint32_t tidx = smk_m.cc_edit_index;
    uint32_t cur_cc = (ttype == 0) ? smk_m.knob_cc[tidx % SMK_MAX_KNOBS] :
                      (ttype == 1) ? smk_m.fader_cc[tidx % SMK_MAX_FADERS] :
                                     smk_m.pad_cc[tidx % SMK_MAX_PADS];
    uint32_t cur_ch = (ttype == 0) ? smk_m.knob_ch[tidx % SMK_MAX_KNOBS] :
                      (ttype == 1) ? smk_m.fader_ch[tidx % SMK_MAX_FADERS] :
                                     smk_m.pad_ch[tidx % SMK_MAX_PADS];

    /* Target Name & Index - highlight if edit_subfield == 2 */
    if (smk_m.edit_subfield == 2) {
        sg_chip(20, 64, 140, 24, CLR_ACC3);
        sg_text(26, 68, 1, tgt_names[ttype], CLR_BG);
        buf[0] = '#';
        buf[1] = (char)('0' + (tidx + 1u) / 10u);
        buf[2] = (char)('0' + (tidx + 1u) % 10u);
        buf[3] = 0;
        sg_text(90, 68, 1, buf, CLR_BG);
    } else {
        sg_text(24, 68, 2, tgt_names[ttype], CLR_ACC4);
        buf[0] = '#';
        buf[1] = (char)('0' + (tidx + 1u) / 10u);
        buf[2] = (char)('0' + (tidx + 1u) % 10u);
        buf[3] = 0;
        sg_text(110, 68, 2, buf, CLR_INK);
    }

    /* Assigned CC - highlight if edit_subfield == 0 */
    uint16_t cc_col = (smk_m.edit_subfield == 0) ? CLR_ACC3 : CLR_DIM;
    sg_text(24, 104, 1, "ASSIGNED CC:", cc_col);
    buf[0] = (char)('0' + cur_cc / 100u);
    buf[1] = (char)('0' + (cur_cc % 100u) / 10u);
    buf[2] = (char)('0' + cur_cc % 10u);
    buf[3] = 0;
    if (smk_m.edit_subfield == 0) {
        sg_chip(130, 98, 60, 22, CLR_ACC3);
        sg_text(138, 102, 2, buf, CLR_BG);
    } else {
        sg_text(138, 102, 2, buf, CLR_ACC1);
    }

    /* MIDI Channel - highlight if edit_subfield == 1 */
    uint16_t ch_col = (smk_m.edit_subfield == 1) ? CLR_ACC3 : CLR_DIM;
    sg_text(24, 142, 1, "MIDI CHANNEL:", ch_col);
    buf[0] = (char)('0' + cur_ch / 10u);
    buf[1] = (char)('0' + cur_ch % 10u);
    buf[2] = 0;
    if (smk_m.edit_subfield == 1) {
        sg_chip(130, 136, 60, 22, CLR_ACC3);
        sg_text(138, 140, 2, buf, CLR_BG);
    } else {
        sg_text(138, 140, 2, buf, CLR_ACC3);
    }

    smk_ui_draw_param_bar("CC #", "CHAN", "INDEX", "BANK", smk_m.edit_subfield);
}

/* Master paint callback invoked by sg_step() */
void smk_midi_paint(void)
{
    switch (smk_m.page) {
    case SMK_PAGE_LIVE:  smk_ui_draw_live(); break;
    case SMK_PAGE_SCL:   smk_ui_draw_scl(); break;
    case SMK_PAGE_CHORD: smk_ui_draw_chord(); break;
    case SMK_PAGE_ARP:   smk_ui_draw_arp(); break;
    case SMK_PAGE_SEQ:   smk_ui_draw_seq(); break;
    case SMK_PAGE_SONG:  smk_ui_draw_song(); break;
    case SMK_PAGE_CC:    smk_ui_draw_cc(); break;
    default:             smk_ui_draw_live(); break;
    }

    /* Floating Toast Notification if active */
    if (smk_m.toast_ms > 0 && smk_m.toast[0]) {
        sg_chip(20, 8, 200, 26, CLR_ACC1);
        sg_text(30, 14, 1, smk_m.toast, CLR_BG);
    }
}
