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
    "MAJOR", "MINOR", "DOM 7TH", "MAJOR 7TH", "MINOR 7TH",
    "DIMINISHED", "AUGMENTED", "SUS2", "SUS4", "9TH",
    "DIATONIC 3RD", "DIATONIC 7TH"
};

static const char *const SMK_VOIC_NAMES[VC_COUNT] = {
    "CLOSE", "OPEN", "INV 1", "INV 2", "+OCT BASS"
};

static const int8_t SMK_CHORD_SHAPES[10][CHORD_MAX] = {
    {0, 4, 7, -1},    /* MAJOR */
    {0, 3, 7, -1},    /* MINOR */
    {0, 4, 7, 10},    /* DOM 7TH */
    {0, 4, 7, 11},    /* MAJOR 7TH */
    {0, 3, 7, 10},    /* MINOR 7TH */
    {0, 3, 6, -1},    /* DIMINISHED */
    {0, 4, 8, -1},    /* AUGMENTED */
    {0, 2, 7, -1},    /* SUS2 */
    {0, 5, 7, -1},    /* SUS4 */
    {0, 4, 7, 14},    /* 9TH */
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

/* Build a chord given a root note, mode, and voicing */
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

    if (type < 10u) {
        for (i = 0; i < CHORD_MAX && SMK_CHORD_SHAPES[type][i] >= 0; i++) {
            iv[count++] = SMK_CHORD_SHAPES[type][i];
        }
    } else {
        /* Diatonic triad or 7th */
        uint32_t mask = (scale_id < 16u) ? SCALE_MASK[scale_id] : SCALE_MASK[1];
        uint32_t deg[12], c = 0, at = 0, guard = 12;
        if (mask == 0xFFFu) mask = SCALE_MASK[1];
        while (guard-- && !((mask >> (uint32_t)((r - root_pitch + 120) % 12)) & 1u))
            r--;
        for (i = 0; i < 12u; i++) {
            if ((mask >> i) & 1u) {
                if (i == (uint32_t)((r - root_pitch + 120) % 12)) at = c;
                deg[c++] = i;
            }
        }
        if (!c) c = 1;
        uint32_t chord_notes_num = (type == CH_TYPE_DIA_7TH ? 4u : 3u);
        for (i = 0; i < chord_notes_num; i++) {
            uint32_t k = at + 2u * i;
            iv[count++] = (int32_t)deg[k % c] + 12 * (int32_t)(k / c) - (int32_t)deg[at];
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
