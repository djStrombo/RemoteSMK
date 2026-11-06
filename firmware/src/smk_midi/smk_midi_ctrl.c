/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2026 RemoteSMK contributors */
#pragma once
#include "smk_midi_types.h"

smk_midi_state_t smk_m PLAT_POOL;

/* Default CC assignments */
static const uint8_t SMK_DEF_KNOB_CC[16] = {
    14, 15, 16, 17, 18, 19, 20, 21,   /* Bank 1: Knobs 1-8 */
    22, 23, 24, 25, 26, 27, 28, 29    /* Bank 2: Knobs 9-16 */
};

static const uint8_t SMK_DEF_FADER_CC[8] = {
    70, 71, 72, 73,                   /* Bank 1: Faders 1-4 */
    74, 75, 76, 77                    /* Bank 2: Faders 5-8 */
};

static const uint8_t SMK_DEF_PAD_CC[32] = {
    36, 37, 38, 39, 40, 41, 42, 43,   /* Bank 1: Pads 1-8 */
    44, 45, 46, 47, 48, 49, 50, 51,   /* Bank 1: Pads 9-16 */
    52, 53, 54, 55, 56, 57, 58, 59,   /* Bank 2: Pads 17-24 */
    60, 61, 62, 63, 64, 65, 66, 67    /* Bank 2: Pads 25-32 */
};

void smk_midi_ctrl_init(void)
{
    uint32_t i;
    smk_m.active = 0;
    smk_m.page = SMK_PAGE_SCL;
    smk_m.knob_bank = 0;
    smk_m.fader_bank = 0;
    smk_m.pad_bank = 0;
    smk_m.num_knob_banks = 2;  /* Expandable to 4 */
    smk_m.num_fader_banks = 2; /* Expandable to 4 */
    smk_m.num_pad_banks = 2;   /* Expandable to 4 */
    smk_m.master_ch = 1;
    smk_m.octave = 0;
    smk_m.bpm = 120;
    smk_m.root = 0;     /* C */
    smk_m.scale = 1;    /* Major */
    smk_m.scale_family = 0;
    smk_m.quantize = 0; /* SNAP */
    smk_m.chord_mode = 0; /* OFF */
    smk_m.chord_voic = 0; /* CLOSE */
    smk_m.chord_strum = 0;
    smk_m.arp_on = 0;
    smk_m.arp_mode = 1; /* UP */
    smk_m.arp_rate = 3; /* 1/16 */
    smk_m.arp_oct = 1;
    smk_m.arp_gate = 80;
    smk_m.arp_swing = 0;
    smk_m.arp_hold = 0;
    smk_m.cur_pat = 0;
    smk_m.cur_step = 0;
    smk_m.pat_len = 16;
    smk_m.pat_div = 3; /* 1/16 */
    smk_m.seq_playing = 0;
    smk_m.seq_rec = 0;
    smk_m.song_mode_on = 0;
    smk_m.cur_song_row = 0;
    smk_m.cur_song_repeat = 0;
    smk_m.total_song_rows = 16;
    smk_m.cc_edit_target = 0;
    smk_m.cc_edit_index = 0;

    for (i = 0; i < SMK_MAX_KNOBS; i++) {
        smk_m.knob_cc[i] = i < 16 ? SMK_DEF_KNOB_CC[i] : (uint8_t)(30 + (i - 16));
        smk_m.knob_ch[i] = 1;
    }
    for (i = 0; i < SMK_MAX_FADERS; i++) {
        smk_m.fader_cc[i] = i < 8 ? SMK_DEF_FADER_CC[i] : (uint8_t)(80 + (i - 8));
        smk_m.fader_ch[i] = 1;
    }
    for (i = 0; i < SMK_MAX_PADS; i++) {
        smk_m.pad_cc[i] = i < 32 ? SMK_DEF_PAD_CC[i] : (uint8_t)(68 + (i - 32));
        smk_m.pad_ch[i] = 1;
    }
}

static inline void smk_midi_toggle_knob_bank(void)
{
    smk_m.knob_bank = (smk_m.knob_bank + 1u) % smk_m.num_knob_banks;
}

static inline void smk_midi_toggle_fader_bank(void)
{
    smk_m.fader_bank = (smk_m.fader_bank + 1u) % smk_m.num_fader_banks;
}

static inline void smk_midi_toggle_pad_bank(void)
{
    smk_m.pad_bank = (smk_m.pad_bank + 1u) % smk_m.num_pad_banks;
}
