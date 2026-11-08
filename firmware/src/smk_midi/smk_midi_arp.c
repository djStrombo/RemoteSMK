/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2026 RemoteSMK contributors */
#pragma once
#include "smk_midi_types.h"

#define SMK_ARP_MAX_HELD 16u

static const char *const SMK_ARP_MODE_NAMES[6] = {
    "UP", "DOWN", "UP/DOWN", "RANDOM", "AS PLAYED", "REPEAT"
};

static const char *const SMK_ARP_RATE_NAMES[6] = {
    "1/4", "1/8", "1/8T", "1/16", "1/16T", "1/32"
};

/* Fractions of a whole note or ticks per beat */
static const uint16_t SMK_ARP_RATE_MS_DIV[6] = {
    1000, 500, 333, 250, 167, 125 /* at 120 BPM base */
};

typedef struct {
    uint8_t held[SMK_ARP_MAX_HELD];
    uint8_t nheld;
    uint8_t current_note;
    uint32_t step_idx;
    uint32_t last_step_ms;
    uint32_t note_off_ms;
    uint32_t rnd_seed;
} smk_arp_state_t;

static smk_arp_state_t smk_arp;

static inline uint32_t smk_arp_rng(void)
{
    smk_arp.rnd_seed = smk_arp.rnd_seed * 1664525u + 1013904223u;
    return smk_arp.rnd_seed;
}

static inline void smk_arp_add_held(uint32_t note)
{
    uint32_t i;
    for (i = 0; i < smk_arp.nheld; i++) {
        if (smk_arp.held[i] == (uint8_t)note) return;
    }
    if (smk_arp.nheld < SMK_ARP_MAX_HELD) {
        smk_arp.held[smk_arp.nheld++] = (uint8_t)note;
    }
}

static inline void smk_arp_remove_held(uint32_t note)
{
    uint32_t i, k = 0;
    if (smk_m.arp_hold) return; /* Latch */
    for (i = 0; i < smk_arp.nheld; i++) {
        if (smk_arp.held[i] != (uint8_t)note) {
            smk_arp.held[k++] = smk_arp.held[i];
        }
    }
    smk_arp.nheld = (uint8_t)k;
}

static inline void smk_arp_clear_held(void)
{
    smk_arp.nheld = 0;
}

/* Calculate period in milliseconds based on BPM and Rate */
static inline uint32_t smk_arp_calc_period(void)
{
    uint32_t bpm = smk_m.bpm ? smk_m.bpm : 120u;
    uint32_t rate = smk_m.arp_rate < 6u ? smk_m.arp_rate : 3u;
    /* Base ms per quarter note = 60000 / bpm */
    uint32_t quarter_ms = 60000u / bpm;
    switch (rate) {
    case 0: return quarter_ms;             /* 1/4 */
    case 1: return quarter_ms / 2u;        /* 1/8 */
    case 2: return quarter_ms / 3u;        /* 1/8T */
    case 3: return quarter_ms / 4u;        /* 1/16 */
    case 4: return quarter_ms / 6u;        /* 1/16T */
    case 5: return quarter_ms / 8u;        /* 1/32 */
    default: return quarter_ms / 4u;
    }
}
