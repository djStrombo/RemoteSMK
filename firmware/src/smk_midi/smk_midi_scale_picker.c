/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2026 RemoteSMK contributors */
#pragma once
#include "smk_midi_types.h"
#include "smk_midi_scales.h"

static inline int smk_scale_match(uint32_t id)
{
    if (id >= SCALE_TOTAL) return 0;
    if (!smk_m.scale_family) return 1; /* 0: All */
    return (SCALE_FAMILY[id] + 1u == smk_m.scale_family);
}

static inline uint32_t smk_scale_count(void)
{
    uint32_t n = 0, i;
    for (i = 0; i < SCALE_TOTAL; i++)
        n += smk_scale_match(i);
    return n;
}

static inline uint32_t smk_scale_at(uint32_t rank)
{
    uint32_t i;
    for (i = 0; i < SCALE_TOTAL; i++) {
        if (smk_scale_match(i)) {
            if (!rank--) return i;
        }
    }
    return 0;
}

static inline uint32_t smk_scale_rank(void)
{
    uint32_t n = 0, i, sel = (uint32_t)(smk_m.scale >= 0 && (uint32_t)smk_m.scale < SCALE_TOTAL ? smk_m.scale : 0);
    for (i = 0; i < SCALE_TOTAL; i++) {
        if (!smk_scale_match(i)) continue;
        if (i == sel) return n;
        n++;
    }
    return 0;
}

static inline void smk_scale_select(uint32_t id)
{
    if (id < SCALE_TOTAL)
        smk_m.scale = (int16_t)id;
}

static inline void smk_scale_step(int32_t steps)
{
    uint32_t count = smk_scale_count();
    uint32_t rank;
    if (!count) return;
    rank = smk_scale_rank();
    if (steps > 0)
        rank = (rank + (uint32_t)steps) % count;
    else {
        int32_t r = (int32_t)rank + steps;
        while (r < 0) r += (int32_t)count;
        rank = (uint32_t)r % count;
    }
    smk_scale_select(smk_scale_at(rank));
}

static inline void smk_scale_family_step(int32_t steps)
{
    int32_t f = (int32_t)smk_m.scale_family + steps;
    while (f < 0) f += (SCALE_FAMILIES + 1u);
    smk_m.scale_family = (uint8_t)(f % (SCALE_FAMILIES + 1u));
    if (!smk_scale_match((uint32_t)smk_m.scale))
        smk_scale_select(smk_scale_at(0));
}
