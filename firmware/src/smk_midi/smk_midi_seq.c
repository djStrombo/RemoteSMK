/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2026 RemoteSMK contributors */
#pragma once
#include "smk_midi_types.h"

smk_seq_step_t smk_patterns[SMK_MIDI_NPATS][SMK_MIDI_NSTEPS] PLAT_POOL;
smk_song_row_t smk_song_chain[SMK_MIDI_NROWS] PLAT_POOL;

void smk_seq_init_patterns(void)
{
    uint32_t p, s;
    for (p = 0; p < SMK_MIDI_NPATS; p++) {
        for (s = 0; s < SMK_MIDI_NSTEPS; s++) {
            smk_patterns[p][s].n = 0;
            smk_patterns[p][s].vel = 100;
            smk_patterns[p][s].gate = 128;
            smk_patterns[p][s].prob = 100;
            smk_patterns[p][s].flags = 0;
        }
    }
    for (s = 0; s < SMK_MIDI_NROWS; s++) {
        smk_song_chain[s].pattern = (uint8_t)(s % SMK_MIDI_NPATS);
        smk_song_chain[s].repeat = 1;
        smk_song_chain[s].mute = 0;
    }
    smk_m.total_song_rows = 16;
}

static inline void smk_seq_toggle_play(void)
{
    smk_m.seq_playing = !smk_m.seq_playing;
    if (!smk_m.seq_playing) {
        smk_m.cur_step = 0;
        smk_m.cur_song_row = 0;
        smk_m.cur_song_repeat = 0;
    }
}

static inline void smk_seq_toggle_rec(void)
{
    smk_m.seq_rec = !smk_m.seq_rec;
}

static inline void smk_seq_clear_cur_pattern(void)
{
    uint32_t s;
    uint32_t p = smk_m.cur_pat % SMK_MIDI_NPATS;
    for (s = 0; s < SMK_MIDI_NSTEPS; s++) {
        smk_patterns[p][s].n = 0;
    }
}

/* Calculate step duration in milliseconds based on BPM */
static inline uint32_t smk_seq_step_duration(void)
{
    uint32_t bpm = smk_m.bpm ? smk_m.bpm : 120u;
    /* 1/16 step = quarter note / 4 = (60000 / bpm) / 4 */
    return (60000u / bpm) / 4u;
}
