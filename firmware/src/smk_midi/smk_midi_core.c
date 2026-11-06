/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2026 RemoteSMK contributors */
#pragma once
#include "smk_midi_types.h"
#include "smk_midi_scales.h"
#include "smk_midi_scale_picker.c"
#include "smk_midi_chord.c"
#include "smk_midi_arp.c"
#include "smk_midi_seq.c"
#include "smk_midi_ctrl.c"
#include "smk_midi_ui.c"

/* Flag indicating host emulated session is suspended in background */
uint8_t smk_session_suspended = 0;

/* Helper to send MIDI 3-byte message over USB & BLE */
extern void sa_midi3(uint32_t a, uint32_t b, uint32_t c);

static inline void smk_midi_send_note_on(uint32_t ch, uint32_t note, uint32_t vel)
{
    sa_midi3(0x90u | (ch & 0x0Fu), note & 0x7Fu, vel & 0x7Fu);
}

static inline void smk_midi_send_note_off(uint32_t ch, uint32_t note)
{
    sa_midi3(0x80u | (ch & 0x0Fu), note & 0x7Fu, 0x40u);
}

static inline void smk_midi_send_cc(uint32_t ch, uint32_t cc, uint32_t val)
{
    sa_midi3(0xB0u | (ch & 0x0Fu), cc & 0x7Fu, val & 0x7Fu);
}

static void smk_set_toast(const char *msg)
{
    uint32_t i;
    for (i = 0; i < sizeof(smk_m.toast) - 1u && msg[i]; i++) {
        smk_m.toast[i] = msg[i];
    }
    smk_m.toast[i] = 0;
    smk_m.toast_ms = 80; /* ~2 seconds */
}

/* Toggle between Synth/Emulator and Dedicated MIDI Controller Mode */
void smk_midi_toggle_mode(void)
{
    smk_m.active = !smk_m.active;
    if (smk_m.active) {
        smk_session_suspended = 1;
        smk_set_toast("MIDI CONTROLLER MODE");
    } else {
        smk_session_suspended = 0;
        smk_midi_send_cc(smk_m.master_ch - 1u, 123, 0); /* All Notes Off */
        smk_arp_clear_held();
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

/* Note name formatter */
static void smk_format_note_name(char *dst, uint32_t note)
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

/* HUD notification formatters */
static void smk_set_hud_knob(uint32_t idx, uint32_t cc, uint8_t val)
{
    char *p = smk_m.hud_msg;
    *p++ = 'K'; *p++ = 'N'; *p++ = 'O'; *p++ = 'B'; *p++ = ' ';
    if (idx + 1u >= 10u) *p++ = (char)('0' + (idx + 1u) / 10u);
    *p++ = (char)('0' + (idx + 1u) % 10u);
    *p++ = ' '; *p++ = '('; *p++ = 'C'; *p++ = 'C'; *p++ = ' ';
    if (cc >= 100u) *p++ = (char)('0' + cc / 100u);
    if (cc >= 10u) *p++ = (char)('0' + (cc % 100u) / 10u);
    *p++ = (char)('0' + cc % 10u);
    *p++ = ')'; *p++ = ':'; *p++ = ' ';
    if (val >= 100u) *p++ = (char)('0' + val / 100u);
    if (val >= 10u) *p++ = (char)('0' + (val % 100u) / 10u);
    *p++ = (char)('0' + val % 10u);
    *p = 0;
    smk_m.hud_val = val;
    smk_m.hud_active = 1;
    smk_m.hud_ms = 60;
}

static void smk_set_hud_fader(uint32_t idx, uint32_t cc, uint8_t val)
{
    char *p = smk_m.hud_msg;
    *p++ = 'F'; *p++ = 'A'; *p++ = 'D'; *p++ = 'E'; *p++ = 'R'; *p++ = ' ';
    if (idx + 1u >= 10u) *p++ = (char)('0' + (idx + 1u) / 10u);
    *p++ = (char)('0' + (idx + 1u) % 10u);
    *p++ = ' '; *p++ = '('; *p++ = 'C'; *p++ = 'C'; *p++ = ' ';
    if (cc >= 100u) *p++ = (char)('0' + cc / 100u);
    if (cc >= 10u) *p++ = (char)('0' + (cc % 100u) / 10u);
    *p++ = (char)('0' + cc % 10u);
    *p++ = ')'; *p++ = ':'; *p++ = ' ';
    if (val >= 100u) *p++ = (char)('0' + val / 100u);
    if (val >= 10u) *p++ = (char)('0' + (val % 100u) / 10u);
    *p++ = (char)('0' + val % 10u);
    *p = 0;
    smk_m.hud_val = val;
    smk_m.hud_active = 1;
    smk_m.hud_ms = 60;
}

static void smk_set_hud_pad(uint32_t idx, uint32_t cc, uint8_t val)
{
    char *p = smk_m.hud_msg;
    *p++ = 'P'; *p++ = 'A'; *p++ = 'D'; *p++ = ' ';
    if (idx + 1u >= 10u) *p++ = (char)('0' + (idx + 1u) / 10u);
    *p++ = (char)('0' + (idx + 1u) % 10u);
    *p++ = ' '; *p++ = '('; *p++ = 'C'; *p++ = 'C'; *p++ = ' ';
    if (cc >= 100u) *p++ = (char)('0' + cc / 100u);
    if (cc >= 10u) *p++ = (char)('0' + (cc % 100u) / 10u);
    *p++ = (char)('0' + cc % 10u);
    *p++ = ')'; *p++ = ':'; *p++ = ' ';
    if (val >= 100u) *p++ = (char)('0' + val / 100u);
    if (val >= 10u) *p++ = (char)('0' + (val % 100u) / 10u);
    *p++ = (char)('0' + val % 10u);
    *p = 0;
    smk_m.hud_val = val;
    smk_m.hud_active = 1;
    smk_m.hud_ms = 60;
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

    /* Scale quantization */
    uint32_t snapped = smk_scale_snap_note(base_note, smk_m.root, (uint32_t)smk_m.scale);

    /* Chord generation */
    count = smk_chord_build(snapped, chord_notes);

    /* Update performance display state */
    smk_m.last_note = (uint8_t)snapped;
    smk_m.last_vel = (uint8_t)vel;
    smk_m.note_held_cnt++;

    if (smk_m.chord_mode == 0) {
        smk_format_note_name(smk_m.last_chord_str, snapped);
        smk_m.last_notes_str[0] = 0;
    } else {
        char *p = smk_m.last_chord_str;
        const char *rname = SMK_ROOT_NAMES[snapped % 12u];
        while (*rname) *p++ = *rname++;
        *p++ = ' ';
        const char *cname = SMK_CHORD_NAMES[smk_m.chord_mode % CH_COUNT];
        while (*cname) *p++ = *cname++;
        *p = 0;

        char *np = smk_m.last_notes_str;
        for (i = 0; i < count && i < 4; i++) {
            if (i > 0) *np++ = ' ';
            char nbuf[8];
            smk_format_note_name(nbuf, chord_notes[i]);
            const char *nb = nbuf;
            while (*nb) *np++ = *nb++;
        }
        *np = 0;
    }

    /* Arp or direct output */
    for (i = 0; i < count; i++) {
        uint32_t n = chord_notes[i];
        if (smk_m.arp_on) {
            smk_arp_add_held(n);
        } else {
            smk_midi_send_note_on(smk_m.master_ch - 1u, n, vel);
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

    if (smk_m.note_held_cnt > 0) smk_m.note_held_cnt--;

    for (i = 0; i < count; i++) {
        uint32_t n = chord_notes[i];
        if (smk_m.arp_on) {
            smk_arp_remove_held(n);
        } else {
            smk_midi_send_note_off(smk_m.master_ch - 1u, n);
        }
    }
}

/* Encoder turn handler */
void smk_midi_handle_encoder(int role, int32_t delta)
{
    if (!delta) return;

    /* Role 0: < and > navigation arrows */
    if (role == 0) {
        if (smk_m.page == SMK_PAGE_LIVE) {
            /* Live mode: < and > adjust master MIDI Channel */
            int32_t ch = (int32_t)smk_m.master_ch + delta;
            while (ch < 1) ch += 16;
            while (ch > 16) ch -= 16;
            smk_m.master_ch = (uint8_t)ch;
            char buf[20];
            buf[0] = 'M'; buf[1] = 'I'; buf[2] = 'D'; buf[3] = 'I'; buf[4] = ' ';
            buf[5] = 'C'; buf[6] = 'H'; buf[7] = ' ';
            buf[8] = (char)('0' + ch / 10);
            buf[9] = (char)('0' + ch % 10);
            buf[10] = 0;
            smk_set_toast(buf);
            return;
        }

        /* In submenus: < and > change the active subfield */
        switch (smk_m.page) {
        case SMK_PAGE_SCL:
            if (smk_m.edit_subfield == 0) { /* ROOT */
                int32_t r = (int32_t)smk_m.root + delta;
                while (r < 0) r += 12;
                smk_m.root = (int16_t)(r % 12);
            } else if (smk_m.edit_subfield == 1) { /* SCALE */
                smk_scale_step(delta);
            } else if (smk_m.edit_subfield == 2) { /* CHORD */
                int32_t c = (int32_t)smk_m.chord_mode + delta;
                while (c < 0) c += CH_COUNT;
                smk_m.chord_mode = (uint8_t)(c % CH_COUNT);
            } else if (smk_m.edit_subfield == 3) { /* VOICING */
                int32_t v = (int32_t)smk_m.chord_voic + delta;
                while (v < 0) v += VC_COUNT;
                smk_m.chord_voic = (uint8_t)(v % VC_COUNT);
            }
            break;

        case SMK_PAGE_CHORD:
            if (smk_m.edit_subfield == 0) { /* CHORD TYPE */
                int32_t c = (int32_t)smk_m.chord_mode + delta;
                while (c < 0) c += CH_COUNT;
                smk_m.chord_mode = (uint8_t)(c % CH_COUNT);
            } else if (smk_m.edit_subfield == 1) { /* VOICING */
                int32_t v = (int32_t)smk_m.chord_voic + delta;
                while (v < 0) v += VC_COUNT;
                smk_m.chord_voic = (uint8_t)(v % VC_COUNT);
            } else if (smk_m.edit_subfield == 2) { /* ROOT */
                int32_t r = (int32_t)smk_m.root + delta;
                while (r < 0) r += 12;
                smk_m.root = (int16_t)(r % 12);
            }
            break;

        case SMK_PAGE_ARP:
            if (smk_m.edit_subfield == 0) { /* MODE */
                int32_t m = (int32_t)smk_m.arp_mode + delta;
                while (m < 0) m += 7;
                smk_m.arp_mode = (uint8_t)(m % 7);
                smk_m.arp_on = (smk_m.arp_mode != 0);
            } else if (smk_m.edit_subfield == 1) { /* RATE */
                int32_t rt = (int32_t)smk_m.arp_rate + delta;
                while (rt < 0) rt += 6;
                smk_m.arp_rate = (uint8_t)(rt % 6);
            } else if (smk_m.edit_subfield == 2) { /* OCTAVES */
                int32_t o = (int32_t)smk_m.arp_oct + delta;
                if (o < 1) o = 1; if (o > 4) o = 4;
                smk_m.arp_oct = (uint8_t)o;
            } else if (smk_m.edit_subfield == 3) { /* GATE */
                int32_t g = (int32_t)smk_m.arp_gate + delta * 5;
                if (g < 10) g = 10; if (g > 100) g = 100;
                smk_m.arp_gate = (uint8_t)g;
            }
            break;

        case SMK_PAGE_SEQ:
            if (smk_m.edit_subfield == 0) { /* PATTERN */
                int32_t pt = (int32_t)smk_m.cur_pat + delta;
                while (pt < 0) pt += SMK_MIDI_NPATS;
                smk_m.cur_pat = (uint8_t)(pt % SMK_MIDI_NPATS);
            } else if (smk_m.edit_subfield == 1) { /* LENGTH */
                int32_t l = (int32_t)smk_m.pat_len + delta;
                if (l < 1) l = 1; if (l > 64) l = 64;
                smk_m.pat_len = (uint8_t)l;
            }
            break;

        case SMK_PAGE_SONG:
            if (smk_m.edit_subfield == 0) { /* ROW */
                int32_t rw = (int32_t)smk_m.cur_song_row + delta;
                while (rw < 0) rw += smk_m.total_song_rows;
                smk_m.cur_song_row = (uint8_t)(rw % smk_m.total_song_rows);
            } else if (smk_m.edit_subfield == 1) { /* PAT */
                uint32_t rw = smk_m.cur_song_row % SMK_MIDI_NROWS;
                int32_t p = (int32_t)smk_song_chain[rw].pattern + delta;
                while (p < 0) p += SMK_MIDI_NPATS;
                smk_song_chain[rw].pattern = (uint8_t)(p % SMK_MIDI_NPATS);
            } else if (smk_m.edit_subfield == 2) { /* RPT */
                uint32_t rw = smk_m.cur_song_row % SMK_MIDI_NROWS;
                int32_t rp = (int32_t)smk_song_chain[rw].repeat + delta;
                if (rp < 1) rp = 1; if (rp > 16) rp = 16;
                smk_song_chain[rw].repeat = (uint8_t)rp;
            }
            break;

        case SMK_PAGE_CC:
            if (smk_m.edit_subfield == 0) { /* CC NUMBER */
                uint32_t idx = smk_m.cc_edit_index;
                if (smk_m.cc_edit_target == 0) {
                    int32_t c = (int32_t)smk_m.knob_cc[idx % SMK_MAX_KNOBS] + delta;
                    if (c < 0) c = 0; if (c > 127) c = 127;
                    smk_m.knob_cc[idx % SMK_MAX_KNOBS] = (uint8_t)c;
                } else if (smk_m.cc_edit_target == 1) {
                    int32_t c = (int32_t)smk_m.fader_cc[idx % SMK_MAX_FADERS] + delta;
                    if (c < 0) c = 0; if (c > 127) c = 127;
                    smk_m.fader_cc[idx % SMK_MAX_FADERS] = (uint8_t)c;
                } else {
                    int32_t c = (int32_t)smk_m.pad_cc[idx % SMK_MAX_PADS] + delta;
                    if (c < 0) c = 0; if (c > 127) c = 127;
                    smk_m.pad_cc[idx % SMK_MAX_PADS] = (uint8_t)c;
                }
            } else if (smk_m.edit_subfield == 1) { /* CHANNEL */
                uint32_t idx = smk_m.cc_edit_index;
                if (smk_m.cc_edit_target == 0) {
                    int32_t ch = (int32_t)smk_m.knob_ch[idx % SMK_MAX_KNOBS] + delta;
                    if (ch < 1) ch = 1; if (ch > 16) ch = 16;
                    smk_m.knob_ch[idx % SMK_MAX_KNOBS] = (uint8_t)ch;
                } else if (smk_m.cc_edit_target == 1) {
                    int32_t ch = (int32_t)smk_m.fader_ch[idx % SMK_MAX_FADERS] + delta;
                    if (ch < 1) ch = 1; if (ch > 16) ch = 16;
                    smk_m.fader_ch[idx % SMK_MAX_FADERS] = (uint8_t)ch;
                } else {
                    int32_t ch = (int32_t)smk_m.pad_ch[idx % SMK_MAX_PADS] + delta;
                    if (ch < 1) ch = 1; if (ch > 16) ch = 16;
                    smk_m.pad_ch[idx % SMK_MAX_PADS] = (uint8_t)ch;
                }
            } else if (smk_m.edit_subfield == 2) { /* TARGET INDEX */
                uint32_t max_idx = (smk_m.cc_edit_target == 0) ? (smk_m.num_knob_banks * 8u) :
                                   (smk_m.cc_edit_target == 1) ? (smk_m.num_fader_banks * 4u) :
                                                                 (smk_m.num_pad_banks * 16u);
                int32_t idx = (int32_t)smk_m.cc_edit_index + delta;
                while (idx < 0) idx += (int32_t)max_idx;
                smk_m.cc_edit_index = (uint8_t)((uint32_t)idx % max_idx);
            }
            break;
        }
        return;
    }

    /* Roles 1..6: Physical Knobs (K1..K4, ALGO, PRESET) */
    int k_idx = -1;
    if (role == 3) k_idx = 0;       /* K1 */
    else if (role == 4) k_idx = 1;  /* K2 */
    else if (role == 5) k_idx = 2;  /* K3 */
    else if (role == 6) k_idx = 3;  /* K4 */
    else if (role == 1) k_idx = 4;  /* ALGO / K5 */
    else if (role == 2) k_idx = 5;  /* PRESET / K6 */

    if (k_idx >= 0) {
        uint32_t full_idx = (smk_m.knob_bank * 8u) + (uint32_t)k_idx;
        if (full_idx < SMK_MAX_KNOBS) {
            int32_t v = (int32_t)smk_m.knob_val[full_idx] + delta * 2;
            if (v < 0) v = 0; if (v > 127) v = 127;
            smk_m.knob_val[full_idx] = (uint8_t)v;
            smk_midi_send_cc(smk_m.knob_ch[full_idx] - 1u, smk_m.knob_cc[full_idx], (uint32_t)v);
            smk_set_hud_knob(full_idx, smk_m.knob_cc[full_idx], (uint8_t)v);

            /* In CC page: auto-select this knob */
            if (smk_m.page == SMK_PAGE_CC) {
                smk_m.cc_edit_target = 0;
                smk_m.cc_edit_index = (uint8_t)full_idx;
            }
        }
    }

    /* Knobs also directly tweak submenus if user turns them */
    if (smk_m.page == SMK_PAGE_SCL || smk_m.page == SMK_PAGE_CHORD) {
        if (role == 3) {
            int32_t r = (int32_t)smk_m.root + delta;
            while (r < 0) r += 12;
            smk_m.root = (int16_t)(r % 12);
            smk_m.edit_subfield = 0;
        } else if (role == 4) {
            smk_scale_step(delta);
            smk_m.edit_subfield = 1;
        } else if (role == 5) {
            int32_t c = (int32_t)smk_m.chord_mode + delta;
            while (c < 0) c += CH_COUNT;
            smk_m.chord_mode = (uint8_t)(c % CH_COUNT);
            smk_m.edit_subfield = 2;
        } else if (role == 6) {
            int32_t v = (int32_t)smk_m.chord_voic + delta;
            while (v < 0) v += VC_COUNT;
            smk_m.chord_voic = (uint8_t)(v % VC_COUNT);
            smk_m.edit_subfield = 3;
        }
    } else if (smk_m.page == SMK_PAGE_ARP) {
        if (role == 3) {
            int32_t m = (int32_t)smk_m.arp_mode + delta;
            while (m < 0) m += 7;
            smk_m.arp_mode = (uint8_t)(m % 7);
            smk_m.arp_on = (smk_m.arp_mode != 0);
            smk_m.edit_subfield = 0;
        } else if (role == 4) {
            int32_t rt = (int32_t)smk_m.arp_rate + delta;
            while (rt < 0) rt += 6;
            smk_m.arp_rate = (uint8_t)(rt % 6);
            smk_m.edit_subfield = 1;
        } else if (role == 5) {
            int32_t o = (int32_t)smk_m.arp_oct + delta;
            if (o < 1) o = 1; if (o > 4) o = 4;
            smk_m.arp_oct = (uint8_t)o;
            smk_m.edit_subfield = 2;
        } else if (role == 6) {
            int32_t g = (int32_t)smk_m.arp_gate + delta * 5;
            if (g < 10) g = 10; if (g > 100) g = 100;
            smk_m.arp_gate = (uint8_t)g;
            smk_m.edit_subfield = 3;
        }
    }
}

/* Fader movement handler: emits CC for active fader bank */
void smk_midi_handle_fader(uint32_t id, uint32_t val)
{
    uint32_t fader_idx = (smk_m.fader_bank * 4u) + (id % 4u);
    if (fader_idx < SMK_MAX_FADERS) {
        uint8_t v = (uint8_t)(val & 0x7Fu);
        smk_m.fader_val[fader_idx] = v;
        smk_midi_send_cc(smk_m.fader_ch[fader_idx] - 1u, smk_m.fader_cc[fader_idx], v);
        smk_set_hud_fader(fader_idx, smk_m.fader_cc[fader_idx], v);
        if (smk_m.page == SMK_PAGE_CC) {
            smk_m.cc_edit_target = 1;
            smk_m.cc_edit_index = (uint8_t)fader_idx;
        }
    }
}

/* Pad event handler: emits CC for active pad bank */
void smk_midi_handle_pad(uint32_t id, uint32_t val)
{
    uint32_t pad_idx = (smk_m.pad_bank * 16u) + (id % 16u);
    if (pad_idx < SMK_MAX_PADS) {
        uint8_t v = (uint8_t)(val & 0x7Fu);
        smk_m.pad_val[pad_idx] = v;
        smk_midi_send_cc(smk_m.pad_ch[pad_idx] - 1u, smk_m.pad_cc[pad_idx], v);
        smk_set_hud_pad(pad_idx, smk_m.pad_cc[pad_idx], v);
        if (smk_m.page == SMK_PAGE_CC) {
            smk_m.cc_edit_target = 2;
            smk_m.cc_edit_index = (uint8_t)pad_idx;
        }
    }
}

/* Hardware bank buttons handler */
void smk_midi_handle_bank_btn(int btn_id)
{
    if (btn_id == 0) { /* KNOB BANK */
        smk_midi_toggle_knob_bank();
        char buf[20];
        buf[0] = 'K'; buf[1] = 'N'; buf[2] = 'O'; buf[3] = 'B'; buf[4] = ' ';
        buf[5] = 'B'; buf[6] = 'A'; buf[7] = 'N'; buf[8] = 'K'; buf[9] = ' ';
        buf[10] = (char)('1' + smk_m.knob_bank);
        buf[11] = 0;
        smk_set_toast(buf);
    } else if (btn_id == 1) { /* FADER BANK */
        smk_midi_toggle_fader_bank();
        char buf[20];
        buf[0] = 'F'; buf[1] = 'A'; buf[2] = 'D'; buf[3] = 'E'; buf[4] = 'R'; buf[5] = ' ';
        buf[6] = 'B'; buf[7] = 'A'; buf[8] = 'N'; buf[9] = 'K'; buf[10] = ' ';
        buf[11] = (char)('1' + smk_m.fader_bank);
        buf[12] = 0;
        smk_set_toast(buf);
    }
}

/* Hardware buttons handler in Dedicated MIDI Mode */
void smk_midi_handle_buttons(uint32_t buttons)
{
    static uint32_t prev_btns = 0;
    uint32_t pressed = buttons & ~prev_btns;
    prev_btns = buttons;

    if (!pressed || !smk_m.active) return;

    /* ARP button: silk "ARP" */
    if (pressed & (1u << 8)) {
        if (smk_m.page == SMK_PAGE_ARP) {
            smk_m.page = SMK_PAGE_LIVE;
            smk_set_toast("LIVE VIEW");
        } else {
            smk_m.page = SMK_PAGE_ARP;
            smk_m.edit_subfield = 0;
            smk_set_toast("ARPEGGIATOR");
        }
        return;
    }

    /* SCALE button: silk "SCALE" (B_ENV, bit 2) */
    if (pressed & (1u << 2)) {
        if (smk_m.page == SMK_PAGE_SCL) {
            smk_m.page = SMK_PAGE_LIVE;
            smk_set_toast("LIVE VIEW");
        } else {
            smk_m.page = SMK_PAGE_SCL;
            smk_m.edit_subfield = 0;
            smk_set_toast("SCALE PICKER");
        }
        return;
    }

    /* CHORD button: silk "CHORD" (B_LFO, bit 3) */
    if (pressed & (1u << 3)) {
        if (smk_m.page == SMK_PAGE_CHORD) {
            smk_m.page = SMK_PAGE_LIVE;
            smk_set_toast("LIVE VIEW");
        } else {
            smk_m.page = SMK_PAGE_CHORD;
            smk_m.edit_subfield = 0;
            smk_set_toast("CHORD ENGINE");
        }
        return;
    }

    /* GLOBE button: silk "GLOBE" (B_GLO, bit 5) -> CC & Channel Mapper */
    if (pressed & (1u << 5)) {
        if (smk_m.page == SMK_PAGE_CC) {
            smk_m.page = SMK_PAGE_LIVE;
            smk_set_toast("LIVE VIEW");
        } else {
            smk_m.page = SMK_PAGE_CC;
            smk_m.edit_subfield = 0;
            smk_set_toast("CC MAPPER");
        }
        return;
    }

    /* SEQ button: silk "SEQ" (B_SEQ, bit 9) */
    if (pressed & (1u << 9)) {
        if (smk_m.page == SMK_PAGE_SEQ) {
            smk_m.page = SMK_PAGE_SONG;
            smk_m.edit_subfield = 0;
            smk_set_toast("SONG MODE");
        } else if (smk_m.page == SMK_PAGE_SONG) {
            smk_m.page = SMK_PAGE_LIVE;
            smk_set_toast("LIVE VIEW");
        } else {
            smk_m.page = SMK_PAGE_SEQ;
            smk_m.edit_subfield = 0;
            smk_set_toast("SEQUENCER");
        }
        return;
    }

    /* Free buttons: PATCH (B_HOME, 6), PARA (B_EDIT, 4), FX (B_FX, 0), NOTE REPEAT (B_SEL, 1), BT (B_SAVE, 7) */
    if (pressed & ((1u << 0) | (1u << 1) | (1u << 4) | (1u << 6) | (1u << 7))) {
        smk_m.page = SMK_PAGE_LIVE;
        smk_set_toast("LIVE VIEW");
        return;
    }

    /* SEQ PLAY (B_PLAY, 10) & SEQ REC (B_REC, 11) */
    if (pressed & (1u << 10)) {
        smk_m.seq_playing = !smk_m.seq_playing;
        smk_set_toast(smk_m.seq_playing ? "SEQ PLAY" : "SEQ STOP");
        return;
    }
    if (pressed & (1u << 11)) {
        smk_m.seq_rec = !smk_m.seq_rec;
        smk_set_toast(smk_m.seq_rec ? "SEQ REC ON" : "SEQ REC OFF");
        return;
    }

    /* Octave Buttons: B_OCTDN (12) and B_OCTUP (13) */
    if (smk_m.page == SMK_PAGE_LIVE) {
        if (pressed & (1u << 12)) {
            if (smk_m.octave > -3) smk_m.octave--;
            char obuf[16];
            obuf[0] = 'O'; obuf[1] = 'C'; obuf[2] = 'T'; obuf[3] = ' ';
            obuf[4] = smk_m.octave < 0 ? '-' : '+';
            obuf[5] = (char)('0' + (smk_m.octave < 0 ? -smk_m.octave : smk_m.octave));
            obuf[6] = 0;
            smk_set_toast(obuf);
        } else if (pressed & (1u << 13)) {
            if (smk_m.octave < 3) smk_m.octave++;
            char obuf[16];
            obuf[0] = 'O'; obuf[1] = 'C'; obuf[2] = 'T'; obuf[3] = ' ';
            obuf[4] = smk_m.octave < 0 ? '-' : '+';
            obuf[5] = (char)('0' + (smk_m.octave < 0 ? -smk_m.octave : smk_m.octave));
            obuf[6] = 0;
            smk_set_toast(obuf);
        }
    } else {
        /* In Submenus: OCT- goes BACK to Live View; OCT+ advances/confirms subfield */
        if (pressed & (1u << 12)) {
            smk_m.page = SMK_PAGE_LIVE;
            smk_set_toast("BACK -> LIVE");
        } else if (pressed & (1u << 13)) {
            uint32_t max_fields = 2;
            if (smk_m.page == SMK_PAGE_SCL || smk_m.page == SMK_PAGE_ARP) max_fields = 4;
            else if (smk_m.page == SMK_PAGE_CHORD || smk_m.page == SMK_PAGE_CC) max_fields = 3;
            smk_m.edit_subfield = (uint8_t)((smk_m.edit_subfield + 1u) % max_fields);
            smk_set_toast("NEXT FIELD");
        }
    }
}

/* Background millisecond tick */
void smk_midi_step_ms(void)
{
    if (smk_m.toast_ms > 0) smk_m.toast_ms--;
    if (smk_m.hud_ms > 0) smk_m.hud_ms--;

    /* Arp timing */
    if (smk_m.active && smk_m.arp_on && smk_arp.nheld > 0) {
        uint32_t now = plat_ms();
        uint32_t period = smk_arp_calc_period();
        if (now - smk_arp.last_step_ms >= period) {
            smk_arp.last_step_ms = now;
            /* Turn off previous note */
            if (smk_arp.current_note) {
                smk_midi_send_note_off(smk_m.master_ch - 1u, smk_arp.current_note);
                smk_arp.current_note = 0;
            }
            /* Calculate next note */
            uint32_t idx = smk_arp.step_idx++;
            uint32_t note_idx = 0;
            switch (smk_m.arp_mode) {
            case 2: /* DOWN */
                note_idx = (smk_arp.nheld - 1u) - (idx % smk_arp.nheld);
                break;
            case 3: /* UPDOWN */ {
                uint32_t cyc = smk_arp.nheld > 1u ? 2u * smk_arp.nheld - 2u : 1u;
                uint32_t k = idx % cyc;
                note_idx = k < smk_arp.nheld ? k : cyc - k;
                break;
            }
            case 4: /* RANDOM */
                note_idx = smk_arp_rng() % smk_arp.nheld;
                break;
            default: /* UP / AS PLAYED */
                note_idx = idx % smk_arp.nheld;
                break;
            }
            uint32_t oct = (idx / smk_arp.nheld) % smk_m.arp_oct;
            uint32_t play_note = (uint32_t)smk_arp.held[note_idx] + oct * 12u;
            if (play_note > 127u) play_note = 127u;
            smk_arp.current_note = (uint8_t)play_note;
            smk_midi_send_note_on(smk_m.master_ch - 1u, play_note, 100);
        }
    }
}
