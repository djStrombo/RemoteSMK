/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2026 RemoteSMK contributors */
#pragma once
#include <stdint.h>
#include "plat.h"

#define SMK_MIDI_NSTEPS 64u
#define SMK_MIDI_NPATS  16u
#define SMK_MIDI_NROWS  64u

/* Page views in dedicated MIDI Controller mode */
enum {
    SMK_PAGE_LIVE = 0, /* Live Performance Screen (all controls active, note/chord display) */
    SMK_PAGE_SCL,      /* Scales selection & navigation */
    SMK_PAGE_CHORD,    /* Chord engine (DIA, MAJ, MIN, etc. & Voicings) */
    SMK_PAGE_ARP,      /* Arpeggiator (Mode, Rate, Octave, Gate, Swing) */
    SMK_PAGE_SEQ,      /* 64-step Pattern Sequencer */
    SMK_PAGE_SONG,     /* Song Mode (Pattern chain rows) */
    SMK_PAGE_CC,       /* CC & Channel Mapper for Knobs, Faders, Pads */
    SMK_PAGE_COUNT
};

/* Bank capacities */
#define SMK_MAX_KNOBS   32u
#define SMK_MAX_FADERS  16u
#define SMK_MAX_PADS    64u

/* Sequencer step definition: 8 bytes per step (1024 steps = 8 KiB) */
typedef struct {
    uint8_t note[4];
    uint8_t n:4;       /* Active notes count 0..4 (0 = rest) */
    uint8_t flags:4;   /* Accent, slide */
    uint8_t vel;       /* Velocity 1..127 */
    uint8_t gate;      /* Gate length in 1..255 steps fraction */
    uint8_t prob;      /* Probability 0..100% */
} smk_seq_step_t;

/* Song mode chain row */
typedef struct {
    uint8_t pattern;   /* Pattern index 0..15 */
    uint8_t repeat;    /* Repeat count 1..16 */
    uint8_t mute;      /* 0: active, 1: mute */
    uint8_t rsv;
} smk_song_row_t;

/* Live MIDI controller state */
typedef struct {
    /* Mode and activation */
    uint8_t active;             /* 1 = Dedicated MIDI Mode, 0 = Emulated / DX7 */
    uint8_t page;               /* SMK_PAGE_* */
    uint8_t toast_ms;           /* Toast display counter */
    char toast[24];             /* Toast text */

    /* Active banks (toggled by KNOB BANK, FADER BANK, etc.) */
    uint8_t knob_bank;          /* 0: 1-8, 1: 9-16, 2: 17-24, 3: 25-32 */
    uint8_t fader_bank;         /* 0: 1-4, 1: 5-8, 2: 9-12, 3: 13-16 */
    uint8_t pad_bank;           /* 0: 1-16, 1: 17-32, 2: 33-48, 3: 49-64 */
    uint8_t num_knob_banks;     /* Default 2, expandable to 4 */
    uint8_t num_fader_banks;    /* Default 2, expandable to 4 */
    uint8_t num_pad_banks;      /* Default 2, expandable to 4 */

    /* CC and Channel mappings */
    uint8_t knob_cc[SMK_MAX_KNOBS];
    uint8_t knob_ch[SMK_MAX_KNOBS];
    uint8_t fader_cc[SMK_MAX_FADERS];
    uint8_t fader_ch[SMK_MAX_FADERS];
    uint8_t pad_cc[SMK_MAX_PADS];
    uint8_t pad_ch[SMK_MAX_PADS];
    uint8_t master_ch;
    int8_t  octave;             /* -3 .. +3 */

    /* Musical parameters */
    int16_t root;               /* 0..11: C .. B */
    int16_t scale;              /* 0..69: 70 scales */
    uint8_t scale_family;       /* 0..8 */
    uint8_t quantize;           /* 0: SNAP, 1: WHITE, 2: OFF */

    /* Chord engine */
    uint8_t chord_mode;         /* CH_OFF, CH_DIA3, CH_DIA7, CH_MAJ, etc. */
    uint8_t chord_voic;         /* VC_CLOSE, VC_OPEN, VC_INV1, VC_INV2, VC_BASS */
    uint8_t chord_strum;        /* 0..127 ms */

    /* Arpeggiator */
    uint8_t arp_on;
    uint8_t arp_mode;           /* 0: OFF, 1: UP, 2: DOWN, 3: UPDOWN, 4: RND, 5: AS_PLAYED, 6: REPEAT */
    uint8_t arp_rate;           /* 0: 1/4, 1: 1/8, 2: 1/8T, 3: 1/16, 4: 1/16T, 5: 1/32 */
    uint8_t arp_oct;            /* 1..4 */
    uint8_t arp_gate;           /* 10..100% */
    uint8_t arp_swing;          /* 0..100% */
    uint8_t arp_hold;           /* Latch on/off */

    /* Sequencer */
    uint8_t seq_playing;
    uint8_t seq_rec;
    uint8_t cur_pat;            /* 0..15 */
    uint8_t cur_step;           /* 0..63 */
    uint8_t pat_len;            /* 1..64 (default 16) */
    uint8_t pat_div;            /* 1/16, 1/8, etc. */
    uint16_t bpm;               /* 40..240 */
    uint32_t step_time_ms;
    uint32_t last_tick_ms;

    /* Song Mode */
    uint8_t song_mode_on;
    uint8_t cur_song_row;
    uint8_t cur_song_repeat;
    uint8_t total_song_rows;

    /* CC Mapper Editor Selection */
    uint8_t cc_edit_target;     /* 0: Knob, 1: Fader, 2: Pad */
    uint8_t cc_edit_index;      /* Index within target */

    /* Subfield cursor for < > and OCT+/- navigation */
    uint8_t edit_subfield;      /* 0..3 depending on page */

    /* Live performance display state */
    uint8_t last_note;
    uint8_t last_vel;
    uint8_t note_held_cnt;
    char    last_chord_str[24];
    char    last_notes_str[24];

    /* HUD for control movement */
    char    hud_msg[28];
    uint8_t hud_val;            /* 0..127 for bar render */
    uint8_t hud_active;         /* 1 if hud bar active */
    uint8_t hud_ms;             /* timer ticks */

    /* Live values */
    uint8_t knob_val[SMK_MAX_KNOBS];
    uint8_t fader_val[SMK_MAX_FADERS];
    uint8_t pad_val[SMK_MAX_PADS];
} smk_midi_state_t;

extern smk_midi_state_t smk_m;
extern smk_seq_step_t   smk_patterns[SMK_MIDI_NPATS][SMK_MIDI_NSTEPS];
extern smk_song_row_t   smk_song_chain[SMK_MIDI_NROWS];
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
void smk_midi_handle_bank_btn(int btn_id);
void smk_midi_handle_buttons(uint32_t buttons);
void smk_midi_step_ms(void);
void smk_midi_paint(void);
