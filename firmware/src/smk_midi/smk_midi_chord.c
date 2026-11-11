/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2026 RemoteSMK contributors */
#pragma once
#include "smk_midi_types.h"
#include "smk_midi_scales.h"

enum {
    VC_CLOSE,
    VC_OPEN,
    VC_INV1,
    VC_INV2,
    VC_BASS,
    VC_COUNT
};

#define CHORD_MAX 4u

static const char *const SMK_CHORD_NAMES[CH_TYPE_COUNT] = {
    "TRIAD", "7TH", "9TH", "5TH (POWER)", "SUS4", "OCTAVE"
};

static const char *const SMK_VOIC_NAMES[VC_COUNT] = {
    "CLOSE", "OPEN", "INV 1", "INV 2", "+OCT BASS"
};

/* Quantize or snap note to active scale */
static inline uint32_t smk_scale_snap_note(uint32_t note, int32_t root, uint32_t scale_id)
{
    if (!smk_m.scale_on) return note;
    uint32_t mask;
    int32_t guard = 12;
    int32_t n = (int32_t)note;
    if (scale_id < 16u) {
        mask = SCALE_MASK[scale_id];
    } else {
        mask = SCALE_MASK[1];
    }
    if (mask == 0xFFFu) return note;

    while (guard-- && !((mask >> (uint32_t)((n - root + 120) % 12)) & 1u)) {
        n--;
    }
    return (uint32_t)(n < 0 ? 0 : n > 127 ? 127 : n);
}

/* Build a chord given a root note, mode, and voicing - Diatonic to active scale */
static inline uint32_t smk_chord_build(uint32_t in_note, uint8_t *out_notes)
{
    if (!smk_m.chord_on) {
        out_notes[0] = (uint8_t)in_note;
        return 1;
    }

    uint32_t type = smk_m.chord_type;
    uint32_t voic = smk_m.chord_voic;
    int32_t r = (int32_t)in_note;
    int32_t root_pitch = (int32_t)smk_m.root;
    uint32_t scale_id = (uint32_t)smk_m.scale;
    uint32_t count = 0, i;
    int32_t iv[CHORD_MAX];

    /* Diatonic chord based on the active scale (or Major if chromatic) */
    uint32_t mask = (scale_id < 16u) ? SCALE_MASK[scale_id] : SCALE_MASK[1];
    uint32_t deg[12], c = 0, at = 0, guard = 12;
    if (mask == 0xFFFu) mask = SCALE_MASK[1]; /* Chromatic falls back to Major scale degrees */

    while (guard-- && !((mask >> (uint32_t)((r - root_pitch + 120) % 12)) & 1u))
        r--;

    for (i = 0; i < 12u; i++) {
        if ((mask >> i) & 1u) {
            if (i == (uint32_t)((r - root_pitch + 120) % 12)) at = c;
            deg[c++] = i;
        }
    }
    if (!c) deg[c++] = 0;

    switch (type) {
    case CH_TYPE_TRIAD: {
        /* Diatonic triad: 1st, 3rd, 5th */
        for (i = 0; i < 3; i++) {
            uint32_t k = at + 2u * i;
            iv[count++] = (int32_t)deg[k % c] + 12 * (int32_t)(k / c) - (int32_t)deg[at];
        }
        break;
    }
    case CH_TYPE_7TH: {
        /* Diatonic 7th: 1st, 3rd, 5th, 7th */
        for (i = 0; i < 4; i++) {
            uint32_t k = at + 2u * i;
            iv[count++] = (int32_t)deg[k % c] + 12 * (int32_t)(k / c) - (int32_t)deg[at];
        }
        break;
    }
    case CH_TYPE_9TH: {
        /* Diatonic 9th: 1st, 3rd, 5th, 9th (degrees: 1, 3, 5, 9 => k = at, at+2, at+4, at+8) */
        static const uint8_t deg_idx_9th[4] = {0, 2, 4, 8};
        for (i = 0; i < 4; i++) {
            uint32_t k = at + deg_idx_9th[i];
            iv[count++] = (int32_t)deg[k % c] + 12 * (int32_t)(k / c) - (int32_t)deg[at];
        }
        break;
    }
    case CH_TYPE_5TH: {
        /* Diatonic 5th (Power Chord): 1st, 5th */
        static const uint8_t deg_idx_5th[2] = {0, 4};
        for (i = 0; i < 2; i++) {
            uint32_t k = at + deg_idx_5th[i];
            iv[count++] = (int32_t)deg[k % c] + 12 * (int32_t)(k / c) - (int32_t)deg[at];
        }
        break;
    }
    case CH_TYPE_SUS4: {
        /* Diatonic Sus4: 1st, 4th, 5th */
        static const uint8_t deg_idx_sus4[3] = {0, 3, 4};
        for (i = 0; i < 3; i++) {
            uint32_t k = at + deg_idx_sus4[i];
            iv[count++] = (int32_t)deg[k % c] + 12 * (int32_t)(k / c) - (int32_t)deg[at];
        }
        break;
    }
    case CH_TYPE_OCT: {
        /* Diatonic Octave: 1st, 8ve */
        iv[count++] = 0;
        iv[count++] = 12;
        break;
    }
    default: {
        /* Fallback: Triad */
        for (i = 0; i < 3; i++) {
            uint32_t k = at + 2u * i;
            iv[count++] = (int32_t)deg[k % c] + 12 * (int32_t)(k / c) - (int32_t)deg[at];
        }
        break;
    }
    }

    for (i = 0; i < count; i++) {
        int32_t note_val = r + iv[i];
        if (voic == VC_OPEN && i == 1 && count > 2) note_val += 12;
        else if (voic == VC_INV1 && i == 0) note_val += 12;
        else if (voic == VC_INV2 && (i == 0 || i == 1)) note_val += 12;
        if (note_val < 0) note_val = 0;
        if (note_val > 127) note_val = 127;
        out_notes[i] = (uint8_t)note_val;
    }

    /* Voicing: +Octave Bass */
    if (voic == VC_BASS && count < CHORD_MAX) {
        int32_t bass = r - 12;
        if (bass >= 0) {
            for (i = count; i > 0; i--) out_notes[i] = out_notes[i - 1];
            out_notes[0] = (uint8_t)bass;
            count++;
        }
    }

    return count ? count : 1;
}
