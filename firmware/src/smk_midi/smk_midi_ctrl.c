/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2026 RemoteSMK contributors */
#pragma once
#include "smk_midi_types.h"

smk_midi_state_t smk_m;

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
    smk_m.page = SMK_PAGE_GEN;
    smk_m.toast_ms = 0;
    smk_m.toast[0] = 0;

    smk_m.menu_depth = 0;
    smk_m.menu_sel[0] = 0;
    smk_m.menu_sel[1] = 0;
    smk_m.menu_sel[2] = 0;
    smk_m.menu_scroll[0] = 0;
    smk_m.menu_scroll[1] = 0;
    smk_m.menu_scroll[2] = 0;
    smk_m.temp_val = 0;

    /* Scales */
    smk_m.scale_on = 0; /* OFF by default */
    smk_m.root = 0;     /* C */
    smk_m.scale = 1;    /* Major */
    smk_m.scale_family = 0;

    /* Chords */
    smk_m.chord_on = 0; /* OFF by default */
    smk_m.chord_type = CH_TYPE_MAJ;
    smk_m.chord_voic = 0; /* CLOSE */

    /* Arp */
    smk_m.arp_on = 0;   /* OFF by default */
    smk_m.arp_mode = 0; /* UP */
    smk_m.arp_rate = 3; /* 1/16 */
    smk_m.arp_oct = 1;
    smk_m.arp_gate = 2; /* 75% */
    smk_m.arp_hold = 0;
    smk_m.arp_latch = 0;

    /* Note Repeat */
    smk_m.nr_on = 0;
    smk_m.nr_rate = 3;  /* 1/16 */
    smk_m.nr_gate = 2;  /* 75% */
    smk_m.last_nr_tick_ms = 0;

    smk_m.bpm = 120;

    /* Globe Settings */
    smk_m.pad_mode = 0;       /* NOTES */
    smk_m.full_velocity = 0;  /* OFF */
    smk_m.key_vel_curve = 0;  /* NORMAL */
    smk_m.pad_sens = 0;       /* NORMAL */
    smk_m.knob_speed = 0;     /* NORMAL */

    /* MIDI Channels */
    smk_m.master_ch = 1;
    smk_m.ch_keyboard = 1;
    smk_m.ch_knobs = 1;
    smk_m.ch_faders = 1;
    smk_m.ch_pads = 1;
    smk_m.octave = 0;

    /* Banks */
    smk_m.knob_bank = 0;
    smk_m.fader_bank = 0;
    smk_m.pad_bank = 0;
    smk_m.num_knob_banks = 2;
    smk_m.num_fader_banks = 2;
    smk_m.num_pad_banks = 2;

    /* CC Assign */
    smk_m.cc_learn_target = 0;
    smk_m.cc_learn_idx = 0;
    smk_m.cc_learn_val = 0;
    smk_m.cc_learn_seen = 0;

    /* General screen state */
    smk_m.held_pitch_mask = 0;
    smk_m.held_keys_cnt = 0;
    smk_m.last_note_val = 0;
    smk_m.last_note_vel = 0;
    smk_m.last_note_name[0] = 0;
    smk_m.chord_name[0] = 0;

    for (i = 0; i < SMK_MAX_KNOBS; i++) {
        smk_m.knob_cc[i] = i < 16 ? SMK_DEF_KNOB_CC[i] : (uint8_t)(30 + (i - 16));
        smk_m.knob_val[i] = 64;
    }
    for (i = 0; i < SMK_MAX_FADERS; i++) {
        smk_m.fader_cc[i] = i < 8 ? SMK_DEF_FADER_CC[i] : (uint8_t)(80 + (i - 8));
        smk_m.fader_val[i] = 0;
    }
    for (i = 0; i < SMK_MAX_PADS; i++) {
        smk_m.pad_cc[i] = i < 32 ? SMK_DEF_PAD_CC[i] : (uint8_t)(68 + (i - 32));
        smk_m.pad_val[i] = 0;
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
