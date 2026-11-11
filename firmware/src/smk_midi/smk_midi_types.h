/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2026 RemoteSMK contributors */
#pragma once
#include <stdint.h>
#include <stddef.h>
#include "plat.h"

#define SMK_MIDI_NSTEPS 64u
#define SMK_MIDI_NPATS  16u
#define SMK_MIDI_NROWS  64u

/* Page views in dedicated MIDI Controller mode */
enum {
    SMK_PAGE_GEN = 0,   /* General Live Screen (Keyboard, Notes, Chords) */
    SMK_PAGE_SCALE,     /* Scale Menu (List) */
    SMK_PAGE_CHORD,     /* Chord Menu (List) */
    SMK_PAGE_ARP,       /* Arp Menu (List) */
    SMK_PAGE_NR,        /* Note Repeat Menu (List) */
    SMK_PAGE_GLOBE,     /* Globe / Settings Menu (List) */
    SMK_PAGE_COUNT
};

/* Bank capacities */
#define SMK_MAX_KNOBS   32u
#define SMK_MAX_FADERS  16u
#define SMK_MAX_PADS    64u

/* Chord Types */
enum {
    CH_TYPE_MAJ = 0,
    CH_TYPE_MIN,
    CH_TYPE_7TH,
    CH_TYPE_MAJ7,
    CH_TYPE_MIN7,
    CH_TYPE_DIM,
    CH_TYPE_AUG,
    CH_TYPE_SUS2,
    CH_TYPE_SUS4,
    CH_TYPE_9TH,
    CH_TYPE_DIA_TRIAD,
    CH_TYPE_DIA_7TH,
    CH_TYPE_COUNT
};

/* Live MIDI controller state */
typedef struct {
    /* Mode activation */
    uint8_t active;             /* 1 = Dedicated MIDI Mode, 0 = Emulated / DX7 */
    uint8_t page;               /* SMK_PAGE_* */
    uint8_t toast_ms;           /* Toast display counter */
    char toast[24];             /* Toast text */

    /* Navigation state in menus */
    uint8_t menu_depth;         /* 0 = main list, 1 = submenu / edit */
    int8_t  menu_sel[3];        /* cursor index at depth 0, 1, 2 */
    int8_t  menu_scroll[3];     /* scroll offset */
    int16_t temp_val;           /* temporary editing value */

    /* Scale settings */
    uint8_t scale_on;           /* 0: OFF, 1: ON */
    int16_t root;               /* 0..11: C .. B */
    int16_t scale;              /* 0..69 */
    uint8_t scale_family;       /* Scale family */

    /* Chord settings */
    uint8_t chord_on;           /* 0: OFF, 1: ON */
    uint8_t chord_type;         /* CH_TYPE_* */
    uint8_t chord_voic;         /* 0: CLOSE, 1: OPEN, 2: INV1, 3: INV2, 4: BASS */

    /* Arpeggiator settings */
    uint8_t arp_on;             /* 0: OFF, 1: ON */
    uint8_t arp_mode;           /* 0: UP, 1: DOWN, 2: UPDOWN, 3: RND, 4: AS_PLAYED, 5: REPEAT */
    uint8_t arp_rate;           /* 0: 1/4, 1: 1/8, 2: 1/8T, 3: 1/16, 4: 1/16T, 5: 1/32 */
    uint8_t arp_oct;            /* 1..4 */
    uint8_t arp_gate;           /* 0: 25%, 1: 50%, 2: 75%, 3: 100% */
    uint8_t arp_hold;           /* 0: OFF, 1: ON */
    uint8_t arp_latch;          /* Alias to arp_hold */

    /* Note Repeat settings */
    uint8_t nr_on;              /* 0: OFF, 1: ON */
    uint8_t nr_rate;            /* 0: 1/4, 1: 1/8, 2: 1/8T, 3: 1/16, 4: 1/16T, 5: 1/32 */
    uint8_t nr_gate;            /* 0: 25%, 1: 50%, 2: 75%, 3: 100% */
    uint8_t nr_btn_down;        /* 1 if NOTE REPEAT button held down */
    uint8_t nr_momentary_active;/* 1 if currently repeating momentarily while held */
    uint8_t nr_was_off;         /* 1 if Note Repeat was OFF at press start */
    uint32_t nr_btn_down_ms;    /* timestamp of press down */
    uint32_t last_nr_tick_ms;

    /* Global tempo */
    uint16_t bpm;               /* 40..240 */

    /* Globe / Hardware Settings */
    uint8_t pad_mode;           /* 0: NOTES, 1: CC */
    uint8_t full_velocity;      /* 0: OFF, 1: ON (fixed 127) */
    uint8_t four_banks;         /* 0: 2 banks (default), 1: 4 banks */
    uint8_t key_vel_curve;      /* 0: NORMAL, 1: SOFT, 2: HARD */
    uint8_t pad_sens;           /* 0: NORMAL, 1: LOW, 2: HIGH */
    uint8_t knob_speed;         /* 0: NORMAL, 1: SLOW, 2: FAST */

    /* MIDI Channels */
    uint8_t master_ch;          /* Compat with sa_smk.c */
    uint8_t ch_keyboard;        /* 1..16 */
    uint8_t ch_knobs;           /* 1..16 */
    uint8_t ch_faders;          /* 1..16 */
    uint8_t ch_pads;            /* 1..16 */
    int8_t  octave;             /* -3 .. +3 */

    /* CC Maps */
    uint8_t knob_cc[SMK_MAX_KNOBS];
    uint8_t fader_cc[SMK_MAX_FADERS];
    uint8_t pad_cc[SMK_MAX_PADS];
    uint8_t knob_val[SMK_MAX_KNOBS];
    uint8_t fader_val[SMK_MAX_FADERS];
    uint8_t pad_val[SMK_MAX_PADS];
    uint8_t knob_bank;
    uint8_t fader_bank;
    uint8_t pad_bank;
    uint8_t num_knob_banks;
    uint8_t num_fader_banks;
    uint8_t num_pad_banks;

    /* CC Assign / Learn */
    uint8_t cc_learn_target;    /* 0: Knob, 1: Fader, 2: Pad */
    uint8_t cc_learn_idx;       /* Target index */
    uint8_t cc_learn_val;       /* Temporary CC */
    uint8_t cc_learn_seen;      /* 1 if control moved */

    /* General screen state */
    uint16_t held_pitch_mask;   /* Bits 0..11 for held pitch classes */
    uint8_t  held_keys_cnt;
    uint8_t  last_note_val;
    uint8_t  last_note_vel;
    char     last_note_name[12];
    char     chord_name[24];
} smk_midi_state_t;

extern smk_midi_state_t smk_m;
extern uint8_t smk_session_suspended;

void smk_midi_ctrl_init(void);
void smk_seq_init_patterns(void);
void smk_midi_toggle_mode(void);
int  smk_check_octave_hold(uint32_t buttons);
void smk_midi_handle_key_down(uint32_t key, uint32_t vel);
void smk_midi_handle_key_up(uint32_t key);
void smk_midi_handle_encoder(int role, int32_t delta);
void smk_midi_handle_fader(uint32_t id, uint32_t val);
void smk_midi_handle_pad(uint32_t id, uint32_t val);
void smk_midi_handle_bank_btn(int btn_id, int pressed);
void smk_midi_send_msg(uint32_t st, uint32_t d1, uint32_t d2);
void smk_midi_handle_buttons(uint32_t buttons);
void smk_midi_step_ms(void);
void smk_midi_paint(void);
