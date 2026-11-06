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
    "SCALE", "CHORD", "ARPEGGIATOR", "SEQUENCER", "SONG MODE", "CC MAPPER"
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
        sg_rect(170 + (int32_t)i * 10, 10, 6, 6, col);
    }
}

static inline void smk_ui_draw_knob_bar(const char *k1, const char *k2, const char *k3, const char *k4)
{
    /* Bottom 4-column parameter bar */
    sg_rect(0, 204, 240, 36, CLR_PANEL);
    sg_rect(0, 203, 240, 1, CLR_ACC2);

    /* 4 columns: 60px each */
    if (k1) { sg_text(6, 208, 1, "K1", CLR_ACC3); sg_text(6, 222, 1, k1, CLR_INK); }
    sg_rect(60, 204, 1, 36, CLR_LINE);
    if (k2) { sg_text(66, 208, 1, "K2", CLR_ACC3); sg_text(66, 222, 1, k2, CLR_INK); }
    sg_rect(120, 204, 1, 36, CLR_LINE);
    if (k3) { sg_text(126, 208, 1, "K3", CLR_ACC3); sg_text(126, 222, 1, k3, CLR_INK); }
    sg_rect(180, 204, 1, 36, CLR_LINE);
    if (k4) { sg_text(186, 208, 1, "K4", CLR_ACC3); sg_text(186, 222, 1, k4, CLR_INK); }
}

static void smk_ui_draw_scl(void)
{
    char buf[32];
    uint32_t sc_id = (uint32_t)(smk_m.scale >= 0 && (uint32_t)smk_m.scale < SCALE_TOTAL ? smk_m.scale : 0);
    smk_ui_draw_header("SCALE (MELODEE)");

    /* Family Badge */
    sg_chip(12, 32, 100, 20, CLR_PANEL);
    sg_text(18, 36, 1, SCALE_FAMILY_TITLE[SCALE_FAMILY[sc_id] % (SCALE_FAMILIES + 2u)], CLR_ACC1);

    /* Root Note Badge */
    sg_chip(120, 32, 60, 20, CLR_PANEL);
    buf[0] = 'R'; buf[1] = ':'; buf[2] = ' ';
    const char *rname = SMK_ROOT_NAMES[smk_m.root % 12];
    buf[3] = rname[0];
    buf[4] = rname[1];
    buf[5] = 0;
    sg_text(126, 36, 1, buf, CLR_ACC3);

    /* Scale Card Box */
    sg_rect(12, 60, 216, 80, CLR_PANEL);
    sg_rect(12, 60, 216, 1, CLR_ACC2);
    sg_rect(12, 139, 216, 1, CLR_ACC2);

    /* Scale title large */
    sg_text(20, 72, 2, SCALE_TITLE[sc_id], CLR_INK);

    /* Scale short code & degrees */
    buf[0] = '['; buf[1] = 0;
    sg_text(20, 104, 1, N_SCALE[sc_id], CLR_ACC2);
    buf[0] = '(';
    uint32_t deg = SCALE_DEGREES[sc_id];
    buf[1] = (char)('0' + deg / 10u);
    buf[2] = (char)('0' + deg % 10u);
    buf[3] = ' '; buf[4] = 'n'; buf[5] = 'o'; buf[6] = 't'; buf[7] = 'e'; buf[8] = 's'; buf[9] = ')'; buf[10] = 0;
    sg_text(80, 104, 1, buf, CLR_DIM);

    /* Scale ID rank */
    buf[0] = '#';
    buf[1] = (char)('0' + (sc_id + 1u) / 10u);
    buf[2] = (char)('0' + (sc_id + 1u) % 10u);
    buf[3] = '/'; buf[4] = '7'; buf[5] = '0'; buf[6] = 0;
    sg_text(168, 104, 1, buf, CLR_ACC4);

    /* Chord Info Sub-Bar */
    sg_chip(12, 148, 216, 44, CLR_PANEL);
    sg_text(18, 154, 1, "CHORD TRIGGER:", CLR_ACC3);
    sg_text(108, 154, 1, SMK_CHORD_NAMES[smk_m.chord_mode % CH_COUNT], CLR_ACC1);
    sg_text(18, 172, 1, "VOICING:", CLR_DIM);
    sg_text(80, 172, 1, SMK_VOIC_NAMES[smk_m.chord_voic % VC_COUNT], CLR_INK);

    smk_ui_draw_knob_bar("ROOT", "SCALE", "CHORD", "VOIC");
}

static void smk_ui_draw_arp(void)
{
    char buf[24];
    smk_ui_draw_header("ARP (MELODEE)");

    /* Mode Banner */
    sg_chip(12, 34, 130, 24, CLR_PANEL);
    sg_text(18, 40, 1, "MODE: ", CLR_DIM);
    sg_text(60, 40, 1, SMK_ARP_MODE_NAMES[smk_m.arp_mode % 7], CLR_ACC1);

    /* Rate Banner */
    sg_chip(150, 34, 78, 24, CLR_PANEL);
    sg_text(156, 40, 1, "RATE:", CLR_DIM);
    sg_text(194, 40, 1, SMK_ARP_RATE_NAMES[smk_m.arp_rate % 6], CLR_ACC3);

    /* Center Graphic Display */
    sg_rect(12, 68, 216, 92, CLR_PANEL);
    sg_rect(12, 68, 216, 1, CLR_ACC1);
    sg_rect(12, 159, 216, 1, CLR_ACC1);

    /* Parameters */
    sg_text(20, 80, 1, "OCTAVES:", CLR_DIM);
    buf[0] = (char)('0' + smk_m.arp_oct); buf[1] = 0;
    sg_text(90, 80, 1, buf, CLR_INK);

    sg_text(20, 102, 1, "GATE:", CLR_DIM);
    buf[0] = (char)('0' + smk_m.arp_gate / 10u);
    buf[1] = (char)('0' + smk_m.arp_gate % 10u);
    buf[2] = '%'; buf[3] = 0;
    sg_text(90, 102, 1, buf, CLR_INK);

    sg_text(20, 124, 1, "SWING:", CLR_DIM);
    buf[0] = (char)('0' + smk_m.arp_swing / 10u);
    buf[1] = (char)('0' + smk_m.arp_swing % 10u);
    buf[2] = '%'; buf[3] = 0;
    sg_text(90, 124, 1, buf, CLR_INK);

    /* Latch indicator */
    sg_chip(140, 120, 76, 22, smk_m.arp_hold ? CLR_ACC4 : CLR_LINE);
    sg_text(148, 125, 1, smk_m.arp_hold ? "LATCH ON" : "LATCH OFF", smk_m.arp_hold ? CLR_BG : CLR_DIM);

    smk_ui_draw_knob_bar("MODE", "RATE", "OCT", "GATE");
}

static void smk_ui_draw_seq(void)
{
    char buf[24];
    uint32_t s;
    smk_ui_draw_header("SEQUENCER (1 TRK)");

    /* Pattern info bar */
    sg_chip(12, 32, 100, 20, CLR_PANEL);
    buf[0] = 'P'; buf[1] = 'A'; buf[2] = 'T'; buf[3] = ' ';
    buf[4] = (char)('0' + (smk_m.cur_pat + 1u) / 10u);
    buf[5] = (char)('0' + (smk_m.cur_pat + 1u) % 10u);
    buf[6] = 0;
    sg_text(18, 36, 1, buf, CLR_ACC2);

    /* Play/Rec status badge */
    sg_chip(120, 32, 108, 20, smk_m.seq_playing ? CLR_ACC4 : CLR_PANEL);
    sg_text(130, 36, 1, smk_m.seq_rec ? "[REC]" : (smk_m.seq_playing ? "[PLAYING]" : "[STOPPED]"),
            smk_m.seq_playing ? CLR_BG : CLR_DIM);

    /* Matrix Step Grid (4 rows of 16 steps = 64 steps) */
    sg_rect(12, 60, 216, 134, CLR_PANEL);
    uint32_t pat = smk_m.cur_pat % SMK_MIDI_NPATS;
    for (s = 0; s < 64u; s++) {
        int32_t gx = 20 + (int32_t)(s % 16u) * 12 + (int32_t)(s % 16u / 4u) * 3;
        int32_t gy = 74 + (int32_t)(s / 16u) * 28;
        int active = smk_patterns[pat][s].n > 0;
        int is_cur = smk_m.seq_playing && (s == smk_m.cur_step);
        uint16_t c = is_cur ? CLR_ACC3 : (active ? CLR_ACC1 : (s < smk_m.pat_len ? CLR_DIM : CLR_LINE));
        sg_rect(gx, gy, 8, 8, c);
    }

    smk_ui_draw_knob_bar("PAT", "LEN", "DIV", "SWING");
}

static void smk_ui_draw_song(void)
{
    char buf[32];
    uint32_t r;
    smk_ui_draw_header("SONG MODE (CHAIN)");

    /* Current row badge */
    sg_chip(12, 32, 110, 20, CLR_PANEL);
    buf[0] = 'R'; buf[1] = 'O'; buf[2] = 'W'; buf[3] = ' ';
    buf[4] = (char)('0' + (smk_m.cur_song_row + 1u) / 10u);
    buf[5] = (char)('0' + (smk_m.cur_song_row + 1u) % 10u);
    buf[6] = '/';
    buf[7] = (char)('0' + smk_m.total_song_rows / 10u);
    buf[8] = (char)('0' + smk_m.total_song_rows % 10u);
    buf[9] = 0;
    sg_text(18, 36, 1, buf, CLR_ACC3);

    /* Chain Rows list preview */
    sg_rect(12, 60, 216, 134, CLR_PANEL);
    for (r = 0; r < 5u && r < smk_m.total_song_rows; r++) {
        uint32_t row_idx = (smk_m.cur_song_row + r) % smk_m.total_song_rows;
        int32_t ry = 68 + (int32_t)r * 24;
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

    smk_ui_draw_knob_bar("ROW", "PAT", "RPT", "PLAY");
}

static void smk_ui_draw_cc(void)
{
    char buf[32];
    smk_ui_draw_header("MIDI CC MAPPER");

    /* Bank Status Pills */
    sg_chip(12, 32, 66, 20, CLR_PANEL);
    buf[0] = 'K'; buf[1] = 'B'; buf[2] = ':'; buf[3] = (char)('1' + smk_m.knob_bank); buf[4] = 0;
    sg_text(16, 36, 1, buf, CLR_ACC1);

    sg_chip(86, 32, 66, 20, CLR_PANEL);
    buf[0] = 'F'; buf[1] = 'B'; buf[2] = ':'; buf[3] = (char)('1' + smk_m.fader_bank); buf[4] = 0;
    sg_text(90, 36, 1, buf, CLR_ACC2);

    sg_chip(160, 32, 68, 20, CLR_PANEL);
    buf[0] = 'P'; buf[1] = 'B'; buf[2] = ':'; buf[3] = (char)('1' + smk_m.pad_bank); buf[4] = 0;
    sg_text(164, 36, 1, buf, CLR_ACC3);

    /* Inspector Card */
    sg_rect(12, 60, 216, 134, CLR_PANEL);
    sg_rect(12, 60, 216, 1, CLR_ACC4);
    sg_rect(12, 193, 216, 1, CLR_ACC4);

    const char *tgt_names[3] = {"KNOB", "FADER", "PAD"};
    uint32_t ttype = smk_m.cc_edit_target % 3;
    uint32_t tidx = smk_m.cc_edit_index;
    uint32_t cur_cc = (ttype == 0) ? smk_m.knob_cc[tidx % SMK_MAX_KNOBS] :
                      (ttype == 1) ? smk_m.fader_cc[tidx % SMK_MAX_FADERS] :
                                     smk_m.pad_cc[tidx % SMK_MAX_PADS];
    uint32_t cur_ch = (ttype == 0) ? smk_m.knob_ch[tidx % SMK_MAX_KNOBS] :
                      (ttype == 1) ? smk_m.fader_ch[tidx % SMK_MAX_FADERS] :
                                     smk_m.pad_ch[tidx % SMK_MAX_PADS];

    sg_text(24, 76, 2, tgt_names[ttype], CLR_ACC4);
    buf[0] = '#';
    buf[1] = (char)('0' + (tidx + 1u) / 10u);
    buf[2] = (char)('0' + (tidx + 1u) % 10u);
    buf[3] = 0;
    sg_text(110, 76, 2, buf, CLR_INK);

    sg_text(24, 116, 1, "ASSIGNED CC:", CLR_DIM);
    buf[0] = (char)('0' + cur_cc / 100u);
    buf[1] = (char)('0' + (cur_cc % 100u) / 10u);
    buf[2] = (char)('0' + cur_cc % 10u);
    buf[3] = 0;
    sg_text(130, 116, 2, buf, CLR_ACC1);

    sg_text(24, 150, 1, "MIDI CHANNEL:", CLR_DIM);
    buf[0] = (char)('0' + cur_ch / 10u);
    buf[1] = (char)('0' + cur_ch % 10u);
    buf[2] = 0;
    sg_text(130, 150, 2, buf, CLR_ACC3);

    smk_ui_draw_knob_bar("TYPE", "IDX", "CC #", "CH");
}

/* Master paint callback invoked by sg_step() */
void smk_midi_paint(void)
{
    switch (smk_m.page) {
    case SMK_PAGE_SCL:   smk_ui_draw_scl(); break;
    case SMK_PAGE_CHORD: smk_ui_draw_scl(); break; /* Scales & Chords integrated */
    case SMK_PAGE_ARP:   smk_ui_draw_arp(); break;
    case SMK_PAGE_SEQ:   smk_ui_draw_seq(); break;
    case SMK_PAGE_SONG:  smk_ui_draw_song(); break;
    case SMK_PAGE_CC:    smk_ui_draw_cc(); break;
    default:             smk_ui_draw_scl(); break;
    }

    /* Floating Toast Notification if active */
    if (smk_m.toast_ms > 0 && smk_m.toast[0]) {
        sg_chip(20, 8, 200, 26, CLR_ACC1);
        sg_text(30, 14, 1, smk_m.toast, CLR_BG);
    }
}
