/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2026 RemoteSMK contributors */
#pragma once
#include "smk_midi_types.h"
#include "smk_midi_scales.h"
#include "smk_midi_scale_picker.c"
#include "smk_midi_chord.c"
#include "smk_midi_arp.c"
#include "smk_midi_ctrl.c"
#include "smk_midi_ui.c"

/* Flag indicating host emulated session is suspended in background */
uint8_t smk_session_suspended = 0;

void smk_seq_init_patterns(void)
{
    /* Legacy stub for sa_main.c initialization */
}

/* Helper to send MIDI 3-byte message over USB & BLE and host sessions */
static void rf_trs_in(uint32_t pkt);
extern void sa_midi3(uint32_t a, uint32_t b, uint32_t c);

void smk_midi_send_msg(uint32_t st, uint32_t d1, uint32_t d2)
{
    sa_midi3(st, d1, d2);
    rf_trs_in((st >> 4) | ((st & 0xFFu) << 8) | ((d1 & 0x7Fu) << 16) | ((d2 & 0x7Fu) << 24));
}

static inline void smk_midi_send_note_on(uint32_t ch, uint32_t note, uint32_t vel)
{
    smk_midi_send_msg(0x90u | (ch & 0x0Fu), note & 0x7Fu, vel & 0x7Fu);
}

static inline void smk_midi_send_note_off(uint32_t ch, uint32_t note)
{
    smk_midi_send_msg(0x80u | (ch & 0x0Fu), note & 0x7Fu, 0x40u);
}

static inline void smk_midi_send_cc(uint32_t ch, uint32_t cc, uint32_t val)
{
    smk_midi_send_msg(0xB0u | (ch & 0x0Fu), cc & 0x7Fu, val & 0x7Fu);
}

void smk_record_hud_ctrl(uint8_t type, uint8_t id, uint8_t bank, uint8_t cc, uint8_t val, uint8_t sub)
{
    smk_m.last_ctrl_type = type;
    smk_m.last_ctrl_id = id;
    smk_m.last_ctrl_bank = bank;
    smk_m.last_ctrl_cc = cc;
    smk_m.last_ctrl_val = val;
    smk_m.last_ctrl_sub = sub;
    smk_m.last_ctrl_ms = plat_ms();
}

static void smk_set_toast(const char *msg)
{
    uint32_t i;
    for (i = 0; i < sizeof(smk_m.toast) - 1u && msg[i]; i++) {
        smk_m.toast[i] = msg[i];
    }
    smk_m.toast[i] = 0;
    smk_m.toast_ms = 60; /* ~1.5 seconds */

    smk_m.last_ctrl_type = SMK_HUD_TOAST;
    smk_m.last_ctrl_ms = plat_ms();
    for (i = 0; i < sizeof(smk_m.last_ctrl_text) - 1u && msg[i]; i++) {
        smk_m.last_ctrl_text[i] = msg[i];
    }
    smk_m.last_ctrl_text[i] = 0;
}

/* Toggle between Synth/Emulator and Dedicated MIDI Controller Mode */
void smk_midi_toggle_mode(void)
{
    smk_m.active = !smk_m.active;
    if (smk_m.active) {
        smk_session_suspended = 1;
        smk_m.page = SMK_PAGE_GEN;
        smk_m.menu_depth = 0;
        smk_set_toast("MIDI CONTROLLER MODE");
    } else {
        smk_session_suspended = 0;
        smk_midi_send_cc(smk_m.ch_keyboard - 1u, 123, 0); /* All Notes Off */
        smk_arp_clear_held();
        smk_m.nr_btn_down = 0;
        smk_m.nr_momentary_active = 0;
        smk_m.held_pitch_mask = 0;
        smk_m.held_keys_cnt = 0;
        plat_leds(0, 0);
        smk_set_toast("EMULATOR / SYNTH RESTORED");
    }
}

/* Hold detection for simultaneous OCTAVE- and OCTAVE+ */
int smk_check_octave_hold(uint32_t buttons)
{
    const uint32_t OD = 1u << 12; /* B_OCTDN */
    const uint32_t OU = 1u << 13; /* B_OCTUP */
    static uint32_t both_t0 = 0;
    static uint8_t both_fired = 0;
    uint32_t now = plat_ms();

    if ((buttons & (OD | OU)) == (OD | OU)) {
        if (!both_t0) both_t0 = now;
        if (!both_fired && (now - both_t0 >= 700u)) {
            both_fired = 1;
            smk_midi_toggle_mode();
            return 1;
        }
        return 1; /* Suppress individual octave clicks while both held */
    } else {
        both_t0 = 0;
        both_fired = 0;
    }
    return 0;
}

/* Chord recognition patterns */
static const struct {
    uint16_t mask;
    const char *name;
} KNOWN_CHORDS[] = {
    { 0x0091u, "MAJOR" },
    { 0x0089u, "MINOR" },
    { 0x0491u, "7TH" },
    { 0x0891u, "MAJOR 7TH" },
    { 0x0489u, "MINOR 7TH" },
    { 0x0049u, "DIMINISHED" },
    { 0x0111u, "AUGMENTED" },
    { 0x00A1u, "SUS4" },
    { 0x0085u, "SUS2" },
    { 0x0081u, "POWER (5)" },
    { 0x0249u, "DIM 7TH" },
    { 0x0449u, "HALF-DIM (m7b5)" },
    { 0x0095u, "ADD9" },
    { 0x008Du, "MIN ADD9" },
};
#define NUM_KNOWN_CHORDS (sizeof(KNOWN_CHORDS) / sizeof(KNOWN_CHORDS[0]))

static uint8_t pitch_held_count[12] = {0};

static void smk_detect_chord(void)
{
    uint16_t mask = smk_m.held_pitch_mask & 0x0FFFu;
    if (!mask || smk_m.held_keys_cnt == 0) {
        return;
    }

    if ((mask & (mask - 1u)) == 0) {
        /* Single note */
        smk_m.chord_name[0] = 0;
        return;
    }

    uint32_t r, c;
    for (r = 0; r < 12u; r++) {
        uint16_t rot = (uint16_t)(((mask >> r) | (mask << (12u - r))) & 0x0FFFu);
        for (c = 0; c < NUM_KNOWN_CHORDS; c++) {
            if (rot == KNOWN_CHORDS[c].mask) {
                char *dst = smk_m.chord_name;
                const char *rname = SMK_ROOT_NAMES[r];
                while (*rname) *dst++ = *rname++;
                *dst++ = ' ';
                const char *cname = KNOWN_CHORDS[c].name;
                while (*cname) *dst++ = *cname++;
                *dst = 0;
                return;
            }
        }
    }
    smk_m.chord_name[0] = 0;
}

static void smk_format_note_str(char *dst, uint32_t note)
{
    const char *rname = SMK_ROOT_NAMES[note % 12u];
    int oct = ((int)note / 12) - 1;
    while (*rname) *dst++ = *rname++;
    if (oct < 0) {
        *dst++ = '-';
        *dst++ = (char)('0' + (-oct));
    } else {
        *dst++ = (char)('0' + oct);
    }
    *dst = 0;
}

/* Key press handler */
void smk_midi_handle_key_down(uint32_t key, uint32_t vel)
{
    uint8_t chord_notes[CHORD_MAX];
    uint32_t count, i;
    int note = 48 + (int)key + ((int)smk_m.octave * 12);
    if (note < 0) note = 0;
    if (note > 127) note = 127;
    uint32_t base_note = (uint32_t)note;

    /* Velocity curve */
    uint32_t final_vel = vel;
    if (smk_m.full_velocity) {
        final_vel = 127;
    } else if (smk_m.key_vel_curve == 1) { /* SOFT */
        final_vel = (vel * vel) / 127u;
        if (final_vel < 1) final_vel = 1;
    } else if (smk_m.key_vel_curve == 2) { /* HARD */
        uint32_t inv = 127u - vel;
        final_vel = 127u - (inv * inv) / 127u;
        if (final_vel > 127) final_vel = 127;
    }

    /* Scale quantization (only active if scale_on == 1) */
    uint32_t snapped = smk_scale_snap_note(base_note, smk_m.root, (uint32_t)smk_m.scale);

    /* Chord generation (only active if chord_on == 1) */
    count = smk_chord_build(snapped, chord_notes);

    /* Track held pitch classes for general screen display */
    for (i = 0; i < count; i++) {
        uint32_t pc = chord_notes[i] % 12u;
        pitch_held_count[pc]++;
        smk_m.held_pitch_mask |= (1u << pc);
    }
    smk_m.held_keys_cnt++;
    smk_m.last_note_val = (uint8_t)snapped;
    smk_m.last_note_vel = (uint8_t)final_vel;
    smk_format_note_str(smk_m.last_note_name, snapped);
    smk_detect_chord();

    /* Arp or direct output */
    for (i = 0; i < count; i++) {
        uint32_t n = chord_notes[i];
        if (smk_m.arp_on) {
            smk_arp_add_held(n);
        } else {
            smk_midi_send_note_on(smk_m.ch_keyboard - 1u, n, final_vel);
        }
    }
}

/* Key release handler */
void smk_midi_handle_key_up(uint32_t key)
{
    uint8_t chord_notes[CHORD_MAX];
    uint32_t count, i;
    int note = 48 + (int)key + ((int)smk_m.octave * 12);
    if (note < 0) note = 0;
    if (note > 127) note = 127;
    uint32_t base_note = (uint32_t)note;
    uint32_t snapped = smk_scale_snap_note(base_note, smk_m.root, (uint32_t)smk_m.scale);
    count = smk_chord_build(snapped, chord_notes);

    for (i = 0; i < count; i++) {
        uint32_t pc = chord_notes[i] % 12u;
        if (pitch_held_count[pc] > 0) pitch_held_count[pc]--;
        if (pitch_held_count[pc] == 0) smk_m.held_pitch_mask &= ~(1u << pc);
    }
    if (smk_m.held_keys_cnt > 0) smk_m.held_keys_cnt--;
    smk_detect_chord();

    for (i = 0; i < count; i++) {
        uint32_t n = chord_notes[i];
        if (smk_m.arp_on) {
            smk_arp_remove_held(n);
        } else {
            smk_midi_send_note_off(smk_m.ch_keyboard - 1u, n);
        }
    }
}

/* Helper to adjust cursor with automatic vertical list scrolling */
static void smk_list_navigate(int8_t *sel, int8_t *scroll, int32_t delta, int32_t count)
{
    if (count <= 0) return;
    int32_t s = (int32_t)*sel + delta;
    while (s < 0) s += count;
    s = s % count;
    *sel = (int8_t)s;

    if (*sel < *scroll) {
        *scroll = *sel;
    } else if (*sel >= *scroll + 7) {
        *scroll = (int8_t)(*sel - 6);
    }
}

/* Encoder turn handler: Role 0 (< > arrows) navigates; Knobs send CC */
void smk_midi_handle_encoder(int role, int32_t delta)
{
    if (!delta) return;

    /* Role 0: Navigation with < and > arrow buttons */
    if (role == 0) {
        if (smk_m.page == SMK_PAGE_GEN) {
            /* General page: < and > adjust master octave */
            int32_t oct = (int32_t)smk_m.octave + delta;
            if (oct < -3) oct = -3;
            if (oct > 3) oct = 3;
            smk_m.octave = (int8_t)oct;
            char obuf[16];
            obuf[0] = 'O'; obuf[1] = 'C'; obuf[2] = 'T'; obuf[3] = ' ';
            obuf[4] = smk_m.octave < 0 ? '-' : '+';
            obuf[5] = (char)('0' + (smk_m.octave < 0 ? -smk_m.octave : smk_m.octave));
            obuf[6] = 0;
            smk_set_toast(obuf);
            return;
        }

        /* In Menu pages */
        if (smk_m.menu_depth == 0) {
            int32_t count = 4;
            if (smk_m.page == SMK_PAGE_ARP) count = 7;
            else if (smk_m.page == SMK_PAGE_GLOBE) count = 9;
            smk_list_navigate(&smk_m.menu_sel[0], &smk_m.menu_scroll[0], delta, count);
            return;
        }

        /* In Submenus (menu_depth == 1) */
        if (smk_m.page == SMK_PAGE_SCALE) {
            if (smk_m.menu_sel[0] == 1) {
                smk_list_navigate(&smk_m.menu_sel[1], &smk_m.menu_scroll[1], delta, 12);
            } else if (smk_m.menu_sel[0] == 2) {
                smk_list_navigate(&smk_m.menu_sel[1], &smk_m.menu_scroll[1], delta, SCALE_TOTAL);
            }
        } else if (smk_m.page == SMK_PAGE_CHORD) {
            if (smk_m.menu_sel[0] == 1) {
                smk_list_navigate(&smk_m.menu_sel[1], &smk_m.menu_scroll[1], delta, CH_TYPE_COUNT);
            } else if (smk_m.menu_sel[0] == 2) {
                smk_list_navigate(&smk_m.menu_sel[1], &smk_m.menu_scroll[1], delta, VC_COUNT);
            }
        } else if (smk_m.page == SMK_PAGE_ARP) {
            if (smk_m.menu_sel[0] == 1) {
                smk_list_navigate(&smk_m.menu_sel[1], &smk_m.menu_scroll[1], delta, 6);
            } else if (smk_m.menu_sel[0] == 2) {
                smk_list_navigate(&smk_m.menu_sel[1], &smk_m.menu_scroll[1], delta, 6);
            } else if (smk_m.menu_sel[0] == 3) {
                smk_list_navigate(&smk_m.menu_sel[1], &smk_m.menu_scroll[1], delta, 4);
            } else if (smk_m.menu_sel[0] == 4) {
                smk_list_navigate(&smk_m.menu_sel[1], &smk_m.menu_scroll[1], delta, 4);
            }
        } else if (smk_m.page == SMK_PAGE_NR) {
            if (smk_m.menu_sel[0] == 1) {
                smk_list_navigate(&smk_m.menu_sel[1], &smk_m.menu_scroll[1], delta, 6);
            } else if (smk_m.menu_sel[0] == 2) {
                smk_list_navigate(&smk_m.menu_sel[1], &smk_m.menu_scroll[1], delta, 4);
            }
        } else if (smk_m.page == SMK_PAGE_GLOBE) {
            if (smk_m.menu_sel[0] == 0) {
                /* CC ASSIGN: adjust CC value with arrows */
                int32_t c = (int32_t)smk_m.cc_learn_val + delta;
                if (c < 0) c = 0; if (c > 127) c = 127;
                smk_m.cc_learn_val = (uint8_t)c;
            } else if (smk_m.menu_sel[0] == 4) {
                smk_list_navigate(&smk_m.menu_sel[1], &smk_m.menu_scroll[1], delta, 3);
            } else if (smk_m.menu_sel[0] == 5) {
                smk_list_navigate(&smk_m.menu_sel[1], &smk_m.menu_scroll[1], delta, 3);
            } else if (smk_m.menu_sel[0] == 6) {
                smk_list_navigate(&smk_m.menu_sel[1], &smk_m.menu_scroll[1], delta, 3);
            } else if (smk_m.menu_sel[0] == 7) {
                /* MIDI Channels: < > changes the channel value (1..16) */
                char tbuf[16];
                uint8_t *ch_ptr = (smk_m.menu_sel[1] == 0) ? &smk_m.ch_keyboard :
                                  (smk_m.menu_sel[1] == 1) ? &smk_m.ch_knobs :
                                  (smk_m.menu_sel[1] == 2) ? &smk_m.ch_faders : &smk_m.ch_pads;
                const char *row_name = (smk_m.menu_sel[1] == 0) ? "KBD CH " :
                                       (smk_m.menu_sel[1] == 1) ? "KNOB CH " :
                                       (smk_m.menu_sel[1] == 2) ? "FDR CH " : "PAD CH ";
                int32_t val = (int32_t)*ch_ptr + delta;
                if (val < 1) val = 1;
                if (val > 16) val = 16;
                *ch_ptr = (uint8_t)val;

                uint32_t len = 0;
                while (row_name[len]) { tbuf[len] = row_name[len]; len++; }
                tbuf[len++] = (char)('0' + val / 10);
                tbuf[len++] = (char)('0' + val % 10);
                tbuf[len] = 0;
                smk_set_toast(tbuf);
            }
        }
        return;
    }

    /* Role 0 handled above */
    (void)role;
}

/* Dedicated handler for all 8 physical knobs K1..K8 */
void smk_midi_handle_knob(uint32_t k_idx, int32_t delta)
{
    if (!delta || k_idx >= 8u) return;

    uint32_t full_idx = (smk_m.knob_bank * 8u) + k_idx;
    if (full_idx >= SMK_MAX_KNOBS) return;

    /* If in Globe -> CC Assign, auto-select this knob */
    if (smk_m.page == SMK_PAGE_GLOBE && smk_m.menu_depth == 1 && smk_m.menu_sel[0] == 0) {
        smk_m.cc_learn_target = 0;
        smk_m.cc_learn_idx = (uint8_t)full_idx;
        smk_m.cc_learn_val = smk_m.knob_cc[full_idx];
        smk_m.cc_learn_seen = 1;
    }

    int32_t step = delta * 2;
    if (smk_m.knob_speed == 1) step = delta;
    else if (smk_m.knob_speed == 2) step = delta * 4;

    int32_t v = (int32_t)smk_m.knob_val[full_idx] + step;
    if (v < 0) v = 0; if (v > 127) v = 127;
    smk_m.knob_val[full_idx] = (uint8_t)v;
    smk_midi_send_cc(smk_m.ch_knobs - 1u, smk_m.knob_cc[full_idx], (uint32_t)v);

    smk_record_hud_ctrl(SMK_HUD_KNOB, (uint8_t)k_idx, smk_m.knob_bank, smk_m.knob_cc[full_idx], (uint8_t)v, 0);
}

/* Fader movement handler */
void smk_midi_handle_fader(uint32_t id, uint32_t val)
{
    uint32_t fader_idx = (smk_m.fader_bank * 4u) + (id % 4u);
    if (fader_idx < SMK_MAX_FADERS) {
        uint8_t v = (uint8_t)(val & 0x7Fu);
        smk_m.fader_val[fader_idx] = v;

        /* If in Globe -> CC Assign, auto-select this fader */
        if (smk_m.page == SMK_PAGE_GLOBE && smk_m.menu_depth == 1 && smk_m.menu_sel[0] == 0) {
            smk_m.cc_learn_target = 1;
            smk_m.cc_learn_idx = (uint8_t)fader_idx;
            smk_m.cc_learn_val = smk_m.fader_cc[fader_idx];
            smk_m.cc_learn_seen = 1;
        }

        smk_midi_send_cc(smk_m.ch_faders - 1u, smk_m.fader_cc[fader_idx], v);
        smk_record_hud_ctrl(SMK_HUD_FADER, (uint8_t)(id % 4u), smk_m.fader_bank, smk_m.fader_cc[fader_idx], v, 0);
    }
}

/* Pad event handler */
void smk_midi_handle_pad(uint32_t id, uint32_t val)
{
    uint32_t pad_idx = (smk_m.pad_bank * 16u) + (id % 16u);
    if (pad_idx < SMK_MAX_PADS) {
        uint8_t v = (uint8_t)(val & 0x7Fu);
        smk_m.pad_val[pad_idx] = v;

        /* If in Globe -> CC Assign, auto-select this pad */
        if (smk_m.page == SMK_PAGE_GLOBE && smk_m.menu_depth == 1 && smk_m.menu_sel[0] == 0) {
            smk_m.cc_learn_target = 2;
            smk_m.cc_learn_idx = (uint8_t)pad_idx;
            smk_m.cc_learn_val = smk_m.pad_cc[pad_idx];
            smk_m.cc_learn_seen = 1;
        }

        if (smk_m.pad_mode == 0) {
            /* NOTES mode: play note starting at C2 (36) across 64 notes (4 banks * 16 pads) */
            uint32_t note = 36u + pad_idx;
            uint32_t pvel = (uint32_t)v;
            if (smk_m.full_velocity) {
                pvel = 127u;
            } else if (smk_m.pad_sens == 0) { /* LOW sensitivity */
                pvel = (pvel * pvel) / 127u;
                if (pvel < 1) pvel = 1;
            } else if (smk_m.pad_sens == 2) { /* HIGH sensitivity */
                uint32_t inv = 127u - pvel;
                pvel = 127u - (inv * inv) / 127u;
                if (pvel > 127) pvel = 127;
            }
            if (v) {
                smk_midi_send_note_on(smk_m.ch_pads - 1u, note, pvel);
                smk_record_hud_ctrl(SMK_HUD_PAD, (uint8_t)(id % 16u), smk_m.pad_bank, (uint8_t)note, (uint8_t)pvel, 0);
            } else {
                smk_midi_send_note_off(smk_m.ch_pads - 1u, note);
            }
        } else {
            /* CC mode */
            smk_midi_send_cc(smk_m.ch_pads - 1u, smk_m.pad_cc[pad_idx], v);
            smk_record_hud_ctrl(SMK_HUD_PAD, (uint8_t)(id % 16u), smk_m.pad_bank, smk_m.pad_cc[pad_idx], v, 1);
        }
    }
}

/* Hardware bank buttons state and combo detection */
static uint8_t bank_down[2] = {0, 0};
static uint32_t bank_down_ms[2] = {0, 0};
static uint8_t bank_pending[2] = {0, 0};
static uint8_t bank_combo_fired = 0;

static void smk_show_bank_toast(const char *name, uint32_t bank, uint32_t num_banks)
{
    char buf[20];
    uint32_t p = 0;
    while (*name && p < 10) buf[p++] = *name++;
    buf[p++] = ' ';
    buf[p++] = 'B'; buf[p++] = 'A'; buf[p++] = 'N'; buf[p++] = 'K'; buf[p++] = ' ';
    buf[p++] = (char)('1' + bank);
    buf[p++] = '/';
    buf[p++] = (char)('0' + num_banks);
    buf[p] = 0;
    smk_set_toast(buf);
}

void smk_midi_handle_bank_btn(int btn_id, int pressed)
{
    if (btn_id < 0 || btn_id > 1) return;

    if (pressed) {
        bank_down[btn_id] = 1;
        bank_down_ms[btn_id] = plat_ms();
        bank_pending[btn_id] = 1;

        /* Check simultaneous press of both Knob Bank and Fader Bank */
        if (bank_down[0] && bank_down[1]) {
            bank_combo_fired = 1;
            bank_pending[0] = 0;
            bank_pending[1] = 0;
            smk_midi_toggle_pad_bank();
            smk_show_bank_toast("PAD", smk_m.pad_bank, smk_m.num_pad_banks);
        }
    } else {
        bank_down[btn_id] = 0;
        if (bank_combo_fired) {
            if (!bank_down[0] && !bank_down[1]) {
                bank_combo_fired = 0;
            }
        } else if (bank_pending[btn_id]) {
            /* Released quickly before 60ms delay expired: trigger single bank toggle */
            bank_pending[btn_id] = 0;
            if (btn_id == 0) {
                smk_midi_toggle_knob_bank();
                smk_show_bank_toast("KNOB", smk_m.knob_bank, smk_m.num_knob_banks);
            } else {
                smk_midi_toggle_fader_bank();
                smk_show_bank_toast("FADER", smk_m.fader_bank, smk_m.num_fader_banks);
            }
        }
    }
}

/* Buttons handler for Navigation and Page selection */
void smk_midi_handle_buttons(uint32_t buttons)
{
    static uint32_t prev_btns = 0;
    uint32_t pressed = buttons & ~prev_btns;
    uint32_t released = prev_btns & ~buttons;
    prev_btns = buttons;

    if (!smk_m.active) return;

    /* NOTE REPEAT button (B_SEL, bit 1) press */
    if (pressed & (1u << 1)) {
        smk_m.nr_btn_down = 1;
        smk_m.nr_btn_down_ms = plat_ms();
        smk_m.nr_was_off = (smk_m.nr_on == 0);
        smk_m.nr_momentary_active = 0;
    }

    /* NOTE REPEAT button (B_SEL, bit 1) release */
    if (released & (1u << 1)) {
        if (smk_m.nr_btn_down) {
            smk_m.nr_btn_down = 0;
            if (smk_m.nr_momentary_active) {
                smk_m.nr_on = 0;
                smk_m.nr_momentary_active = 0;
                smk_set_toast("REPEAT OFF");
                if (smk_m.last_note_val) {
                    smk_midi_send_note_off(smk_m.ch_keyboard - 1u, smk_m.last_note_val);
                }
            } else {
                /* Short tap: toggle / open NR page */
                if (smk_m.page == SMK_PAGE_NR) {
                    smk_m.page = SMK_PAGE_GEN;
                    smk_m.menu_depth = 0;
                    smk_set_toast("GENERAL VIEW");
                } else {
                    smk_m.page = SMK_PAGE_NR;
                    smk_m.menu_depth = 0;
                    smk_m.menu_sel[0] = 0;
                    smk_m.menu_scroll[0] = 0;
                }
            }
        }
    }

    if (!pressed) return;

    /* PATCH button: "si se pulsa patch también se vuelve al general." */
    if (pressed & (1u << 6)) {
        smk_m.page = SMK_PAGE_GEN;
        smk_m.menu_depth = 0;
        smk_set_toast("GENERAL VIEW");
        return;
    }

    /* SCALE button (B_ENV, bit 2) */
    if (pressed & (1u << 2)) {
        if (smk_m.page == SMK_PAGE_SCALE) {
            smk_m.page = SMK_PAGE_GEN;
            smk_m.menu_depth = 0;
            smk_set_toast("GENERAL VIEW");
        } else {
            smk_m.page = SMK_PAGE_SCALE;
            smk_m.menu_depth = 0;
            smk_m.menu_sel[0] = 0;
            smk_m.menu_scroll[0] = 0;
        }
        return;
    }

    /* CHORD button (B_LFO, bit 3) */
    if (pressed & (1u << 3)) {
        if (smk_m.page == SMK_PAGE_CHORD) {
            smk_m.page = SMK_PAGE_GEN;
            smk_m.menu_depth = 0;
            smk_set_toast("GENERAL VIEW");
        } else {
            smk_m.page = SMK_PAGE_CHORD;
            smk_m.menu_depth = 0;
            smk_m.menu_sel[0] = 0;
            smk_m.menu_scroll[0] = 0;
        }
        return;
    }

    /* ARP button (B_ARP, bit 8) */
    if (pressed & (1u << 8)) {
        if (smk_m.page == SMK_PAGE_ARP) {
            smk_m.page = SMK_PAGE_GEN;
            smk_m.menu_depth = 0;
            smk_set_toast("GENERAL VIEW");
        } else {
            smk_m.page = SMK_PAGE_ARP;
            smk_m.menu_depth = 0;
            smk_m.menu_sel[0] = 0;
            smk_m.menu_scroll[0] = 0;
        }
        return;
    }

    /* GLOBE button (B_GLO, bit 5) */
    if (pressed & (1u << 5)) {
        if (smk_m.page == SMK_PAGE_GLOBE) {
            smk_m.page = SMK_PAGE_GEN;
            smk_m.menu_depth = 0;
            smk_set_toast("GENERAL VIEW");
        } else {
            smk_m.page = SMK_PAGE_GLOBE;
            smk_m.menu_depth = 0;
            smk_m.menu_sel[0] = 0;
            smk_m.menu_scroll[0] = 0;
        }
        return;
    }

    /* OCT- (B_OCTDN, bit 12) */
    if (pressed & (1u << 12)) {
        if (smk_m.page == SMK_PAGE_GEN) {
            if (smk_m.octave > -3) smk_m.octave--;
            smk_set_toast("OCTAVE -");
            return;
        }

        if (smk_m.menu_depth > 0) {
            smk_m.menu_depth = 0; /* Back to parent list */
            smk_set_toast("CANCELLED");
            return;
        }

        /* In root menu (menu_depth == 0): OCT- turns toggles OFF or decrements TEMPO */
        if (smk_m.page == SMK_PAGE_SCALE) {
            if (smk_m.menu_sel[0] == 0) { smk_m.scale_on = 0; smk_set_toast("SCALE OFF"); }
            else if (smk_m.menu_sel[0] == 3) {
                if (smk_m.bpm > 40) smk_m.bpm--;
            }
        } else if (smk_m.page == SMK_PAGE_CHORD) {
            if (smk_m.menu_sel[0] == 0) { smk_m.chord_on = 0; smk_set_toast("CHORD OFF"); }
            else if (smk_m.menu_sel[0] == 3) {
                if (smk_m.bpm > 40) smk_m.bpm--;
            }
        } else if (smk_m.page == SMK_PAGE_ARP) {
            if (smk_m.menu_sel[0] == 0) { smk_m.arp_on = 0; smk_set_toast("ARP OFF"); }
            else if (smk_m.menu_sel[0] == 5) { smk_m.arp_latch = 0; smk_m.arp_hold = 0; smk_set_toast("LATCH OFF"); }
            else if (smk_m.menu_sel[0] == 6) {
                if (smk_m.bpm > 40) smk_m.bpm--;
            }
        } else if (smk_m.page == SMK_PAGE_NR) {
            if (smk_m.menu_sel[0] == 0) { smk_m.nr_on = 0; smk_set_toast("REPEAT OFF"); }
            else if (smk_m.menu_sel[0] == 3) {
                if (smk_m.bpm > 40) smk_m.bpm--;
            }
        } else if (smk_m.page == SMK_PAGE_GLOBE) {
            if (smk_m.menu_sel[0] == 1) { smk_m.pad_mode = 0; smk_set_toast("PADS: NOTES"); }
            else if (smk_m.menu_sel[0] == 2) { smk_m.full_velocity = 0; smk_set_toast("FULL VEL OFF"); }
            else if (smk_m.menu_sel[0] == 3) {
                smk_m.four_banks = 0;
                smk_m.num_knob_banks = 2;
                smk_m.num_fader_banks = 2;
                smk_m.num_pad_banks = 2;
                smk_m.knob_bank %= 2;
                smk_m.fader_bank %= 2;
                smk_m.pad_bank %= 2;
                smk_set_toast("4 BANKS OFF");
            } else if (smk_m.menu_sel[0] == 8) {
                if (smk_m.bpm > 40) smk_m.bpm--;
            }
        }
        return;
    }

    /* OCT+ (B_OCTUP, bit 13) */
    if (pressed & (1u << 13)) {
        if (smk_m.page == SMK_PAGE_GEN) {
            if (smk_m.octave < 3) smk_m.octave++;
            smk_set_toast("OCTAVE +");
            return;
        }

        if (smk_m.menu_depth == 0) {
            /* Handling top-level menu selections */
            if (smk_m.page == SMK_PAGE_SCALE) {
                if (smk_m.menu_sel[0] == 0) { smk_m.scale_on = 1; smk_set_toast("SCALE ON"); }
                else if (smk_m.menu_sel[0] == 3) {
                    if (smk_m.bpm < 240) smk_m.bpm++;
                } else {
                    smk_m.menu_depth = 1;
                    smk_m.menu_sel[1] = smk_m.menu_sel[0] == 1 ? smk_m.root : (smk_m.menu_sel[0] == 2 ? smk_m.scale : 0);
                    smk_m.menu_scroll[1] = 0;
                }
            } else if (smk_m.page == SMK_PAGE_CHORD) {
                if (smk_m.menu_sel[0] == 0) { smk_m.chord_on = 1; smk_set_toast("CHORD ON"); }
                else if (smk_m.menu_sel[0] == 3) {
                    if (smk_m.bpm < 240) smk_m.bpm++;
                } else {
                    smk_m.menu_depth = 1;
                    smk_m.menu_sel[1] = smk_m.menu_sel[0] == 1 ? smk_m.chord_type : (smk_m.menu_sel[0] == 2 ? smk_m.chord_voic : 0);
                    smk_m.menu_scroll[1] = 0;
                }
            } else if (smk_m.page == SMK_PAGE_ARP) {
                if (smk_m.menu_sel[0] == 0) { smk_m.arp_on = 1; smk_set_toast("ARP ON"); }
                else if (smk_m.menu_sel[0] == 5) { smk_m.arp_latch = 1; smk_m.arp_hold = 1; smk_set_toast("LATCH ON"); }
                else if (smk_m.menu_sel[0] == 6) {
                    if (smk_m.bpm < 240) smk_m.bpm++;
                } else {
                    smk_m.menu_depth = 1;
                    smk_m.menu_sel[1] = 0;
                    smk_m.menu_scroll[1] = 0;
                }
            } else if (smk_m.page == SMK_PAGE_NR) {
                if (smk_m.menu_sel[0] == 0) { smk_m.nr_on = 1; smk_set_toast("REPEAT ON"); }
                else if (smk_m.menu_sel[0] == 3) {
                    if (smk_m.bpm < 240) smk_m.bpm++;
                } else {
                    smk_m.menu_depth = 1;
                    smk_m.menu_sel[1] = 0;
                    smk_m.menu_scroll[1] = 0;
                }
            } else if (smk_m.page == SMK_PAGE_GLOBE) {
                if (smk_m.menu_sel[0] == 1) { smk_m.pad_mode = 1; smk_set_toast("PADS: CC"); }
                else if (smk_m.menu_sel[0] == 2) { smk_m.full_velocity = 1; smk_set_toast("FULL VEL ON"); }
                else if (smk_m.menu_sel[0] == 3) {
                    smk_m.four_banks = 1;
                    smk_m.num_knob_banks = 4;
                    smk_m.num_fader_banks = 4;
                    smk_m.num_pad_banks = 4;
                    smk_set_toast("4 BANKS ON");
                } else if (smk_m.menu_sel[0] == 8) {
                    if (smk_m.bpm < 240) smk_m.bpm++;
                } else {
                    smk_m.menu_depth = 1;
                    smk_m.menu_sel[1] = 0;
                    smk_m.menu_scroll[1] = 0;
                    if (smk_m.menu_sel[0] == 0) { smk_m.cc_learn_seen = 0; smk_m.cc_learn_val = 0; }
                }
            }
        } else {
            /* Confirming in Submenu */
            if (smk_m.page == SMK_PAGE_SCALE) {
                if (smk_m.menu_sel[0] == 1) smk_m.root = smk_m.menu_sel[1];
                else if (smk_m.menu_sel[0] == 2) smk_m.scale = smk_m.menu_sel[1];
            } else if (smk_m.page == SMK_PAGE_CHORD) {
                if (smk_m.menu_sel[0] == 1) smk_m.chord_type = (uint8_t)smk_m.menu_sel[1];
                else if (smk_m.menu_sel[0] == 2) smk_m.chord_voic = (uint8_t)smk_m.menu_sel[1];
            } else if (smk_m.page == SMK_PAGE_ARP) {
                if (smk_m.menu_sel[0] == 1) smk_m.arp_mode = (uint8_t)smk_m.menu_sel[1];
                else if (smk_m.menu_sel[0] == 2) smk_m.arp_rate = (uint8_t)smk_m.menu_sel[1];
                else if (smk_m.menu_sel[0] == 3) smk_m.arp_oct = (uint8_t)(smk_m.menu_sel[1] + 1);
                else if (smk_m.menu_sel[0] == 4) smk_m.arp_gate = (uint8_t)smk_m.menu_sel[1];
            } else if (smk_m.page == SMK_PAGE_NR) {
                if (smk_m.menu_sel[0] == 1) smk_m.nr_rate = (uint8_t)smk_m.menu_sel[1];
                else if (smk_m.menu_sel[0] == 2) smk_m.nr_gate = (uint8_t)smk_m.menu_sel[1];
            } else if (smk_m.page == SMK_PAGE_GLOBE) {
                if (smk_m.menu_sel[0] == 0) {
                    /* Save learned CC */
                    if (smk_m.cc_learn_seen) {
                        if (smk_m.cc_learn_target == 0) smk_m.knob_cc[smk_m.cc_learn_idx] = smk_m.cc_learn_val;
                        else if (smk_m.cc_learn_target == 1) smk_m.fader_cc[smk_m.cc_learn_idx] = smk_m.cc_learn_val;
                        else if (smk_m.cc_learn_target == 2) smk_m.pad_cc[smk_m.cc_learn_idx] = smk_m.cc_learn_val;
                        smk_set_toast("CC SAVED");
                    }
                } else if (smk_m.menu_sel[0] == 4) smk_m.key_vel_curve = (uint8_t)smk_m.menu_sel[1];
                else if (smk_m.menu_sel[0] == 5) smk_m.pad_sens = (uint8_t)smk_m.menu_sel[1];
                else if (smk_m.menu_sel[0] == 6) smk_m.knob_speed = (uint8_t)smk_m.menu_sel[1];
                else if (smk_m.menu_sel[0] == 7) {
                    /* In MIDI Channels, OCT+ advances to next row (KBD -> KNOB -> FDR -> PAD) */
                    smk_m.menu_sel[1] = (smk_m.menu_sel[1] + 1) % 4;
                    return;
                }
            }
            smk_m.menu_depth = 0;
            smk_set_toast("CONFIRMED");
        }
        return;
    }
}

/* Background millisecond tick */
void smk_midi_step_ms(void)
{
    if (smk_m.toast_ms > 0) smk_m.toast_ms--;

    if (!smk_m.active) return;
    uint32_t now = plat_ms();

    /* Check pending single bank button timers (60ms) */
    uint32_t bi;
    for (bi = 0; bi < 2u; bi++) {
        if (bank_pending[bi] && !bank_combo_fired) {
            if (now - bank_down_ms[bi] >= 60u) {
                bank_pending[bi] = 0;
                if (bi == 0) {
                    smk_midi_toggle_knob_bank();
                    smk_show_bank_toast("KNOB", smk_m.knob_bank, smk_m.num_knob_banks);
                } else {
                    smk_midi_toggle_fader_bank();
                    smk_show_bank_toast("FADER", smk_m.fader_bank, smk_m.num_fader_banks);
                }
            }
        }
    }

    /* Note Repeat momentary trigger check (hold >= 200ms from OFF) */
    if (smk_m.nr_btn_down && smk_m.nr_was_off && !smk_m.nr_momentary_active) {
        if (now - smk_m.nr_btn_down_ms >= 200u) {
            smk_m.nr_momentary_active = 1;
            smk_m.nr_on = 1;
            smk_m.last_nr_tick_ms = 0; /* tick immediately */
            smk_set_toast("REPEAT (HOLD)");
        }
    }

    /* Arpeggiator timing */
    if (smk_m.arp_on && smk_arp.nheld > 0) {
        uint32_t period = smk_arp_calc_period();
        if (now - smk_arp.last_step_ms >= period) {
            smk_arp.last_step_ms = now;
            if (smk_arp.current_note) {
                smk_midi_send_note_off(smk_m.ch_keyboard - 1u, smk_arp.current_note);
                smk_arp.current_note = 0;
            }
            uint32_t idx = smk_arp.step_idx++;
            uint32_t note_idx = 0;
            switch (smk_m.arp_mode) {
            case 1: /* DOWN */
                note_idx = (smk_arp.nheld - 1u) - (idx % smk_arp.nheld);
                break;
            case 2: /* UPDOWN */ {
                uint32_t cyc = smk_arp.nheld > 1u ? 2u * smk_arp.nheld - 2u : 1u;
                uint32_t k = idx % cyc;
                note_idx = k < smk_arp.nheld ? k : cyc - k;
                break;
            }
            case 3: /* RANDOM */
                note_idx = smk_arp_rng() % smk_arp.nheld;
                break;
            default: /* UP / AS PLAYED / REPEAT */
                note_idx = idx % smk_arp.nheld;
                break;
            }
            uint32_t oct = (idx / smk_arp.nheld) % smk_m.arp_oct;
            uint32_t play_note = (uint32_t)smk_arp.held[note_idx] + oct * 12u;
            if (play_note > 127u) play_note = 127u;
            smk_arp.current_note = (uint8_t)play_note;
            smk_midi_send_note_on(smk_m.ch_keyboard - 1u, play_note, smk_m.last_note_vel ? smk_m.last_note_vel : 100);
        }
    }

    /* Note Repeat timing */
    if (smk_m.nr_on && !smk_m.arp_on && smk_m.held_keys_cnt > 0) {
        uint32_t bpm = smk_m.bpm ? smk_m.bpm : 120u;
        uint32_t q_ms = 60000u / bpm;
        uint32_t period = q_ms / 4u;
        switch (smk_m.nr_rate) {
        case 0: period = q_ms; break;
        case 1: period = q_ms / 2u; break;
        case 2: period = q_ms / 3u; break;
        case 3: period = q_ms / 4u; break;
        case 4: period = q_ms / 6u; break;
        case 5: period = q_ms / 8u; break;
        }

        if (now - smk_m.last_nr_tick_ms >= period) {
            smk_m.last_nr_tick_ms = now;
            if (smk_m.last_note_val) {
                smk_midi_send_note_off(smk_m.ch_keyboard - 1u, smk_m.last_note_val);
                smk_midi_send_note_on(smk_m.ch_keyboard - 1u, smk_m.last_note_val, smk_m.last_note_vel ? smk_m.last_note_vel : 100);
            }
        }
    }

    /* Button and Key LEDs */
    uint32_t leds = 0;

    /* Current screen indicator LED */
    switch (smk_m.page) {
    case SMK_PAGE_GEN:   leds |= (1u << 6); break; /* B_HOME (PATCH) */
    case SMK_PAGE_SCALE: leds |= (1u << 2); break; /* B_ENV (SCALE) */
    case SMK_PAGE_CHORD: leds |= (1u << 3); break; /* B_LFO (CHORD) */
    case SMK_PAGE_ARP:   leds |= (1u << 8); break; /* B_ARP */
    case SMK_PAGE_NR:    leds |= (1u << 1); break; /* B_SEL (NOTE REPEAT) */
    case SMK_PAGE_GLOBE: leds |= (1u << 5); break; /* B_GLO */
    default: break;
    }

    /* Active engine LEDs (stay lit continuously across all screens) */
    if (smk_m.scale_on) leds |= (1u << 2); /* B_ENV */
    if (smk_m.chord_on) leds |= (1u << 3); /* B_LFO */
    if (smk_m.arp_on)   leds |= (1u << 8); /* B_ARP */
    if (smk_m.nr_on)    leds |= (1u << 1); /* B_SEL */

    /* Gradual Bank button LED brightness according to bank */
    uint32_t phase = (now / 4u) % 4u;
    if (smk_m.four_banks) {
        if (smk_m.knob_bank == 1 && phase == 0) leds |= (1u << 16);
        else if (smk_m.knob_bank == 2 && (phase & 1u) == 0) leds |= (1u << 16);
        else if (smk_m.knob_bank >= 3) leds |= (1u << 16);

        if (smk_m.fader_bank == 1 && phase == 0) leds |= (1u << 17);
        else if (smk_m.fader_bank == 2 && (phase & 1u) == 0) leds |= (1u << 17);
        else if (smk_m.fader_bank >= 3) leds |= (1u << 17);
    } else {
        if (smk_m.knob_bank >= 1) leds |= (1u << 16);
        if (smk_m.fader_bank >= 1) leds |= (1u << 17);
    }

    plat_leds(leds, plat_keys());
}
