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

    if (role == 0) { /* SELECT: cycles pages */
        int32_t p = (int32_t)smk_m.page + delta;
        while (p < 0) p += SMK_PAGE_COUNT;
        smk_m.page = (uint8_t)(p % SMK_PAGE_COUNT);
        return;
    }

    /* Page-specific parameter adjustments */
    switch (smk_m.page) {
    case SMK_PAGE_SCL:
    case SMK_PAGE_CHORD:
        if (role == 3) { /* K1: ROOT */
            int32_t r = (int32_t)smk_m.root + delta;
            while (r < 0) r += 12;
            smk_m.root = (int16_t)(r % 12);
        } else if (role == 4) { /* K2: SCALE */
            smk_scale_step(delta);
        } else if (role == 5) { /* K3: CHORD */
            int32_t c = (int32_t)smk_m.chord_mode + delta;
            while (c < 0) c += CH_COUNT;
            smk_m.chord_mode = (uint8_t)(c % CH_COUNT);
        } else if (role == 6) { /* K4: VOICING */
            int32_t v = (int32_t)smk_m.chord_voic + delta;
            while (v < 0) v += VC_COUNT;
            smk_m.chord_voic = (uint8_t)(v % VC_COUNT);
        }
        break;

    case SMK_PAGE_ARP:
        if (role == 3) { /* K1: MODE */
            int32_t m = (int32_t)smk_m.arp_mode + delta;
            while (m < 0) m += 7;
            smk_m.arp_mode = (uint8_t)(m % 7);
            smk_m.arp_on = (smk_m.arp_mode != 0);
        } else if (role == 4) { /* K2: RATE */
            int32_t rt = (int32_t)smk_m.arp_rate + delta;
            while (rt < 0) rt += 6;
            smk_m.arp_rate = (uint8_t)(rt % 6);
        } else if (role == 5) { /* K3: OCTAVES */
            int32_t o = (int32_t)smk_m.arp_oct + delta;
            if (o < 1) o = 1; if (o > 4) o = 4;
            smk_m.arp_oct = (uint8_t)o;
        } else if (role == 6) { /* K4: GATE */
            int32_t g = (int32_t)smk_m.arp_gate + delta * 5;
            if (g < 10) g = 10; if (g > 100) g = 100;
            smk_m.arp_gate = (uint8_t)g;
        }
        break;

    case SMK_PAGE_SEQ:
        if (role == 3) { /* K1: PATTERN */
            int32_t pt = (int32_t)smk_m.cur_pat + delta;
            while (pt < 0) pt += SMK_MIDI_NPATS;
            smk_m.cur_pat = (uint8_t)(pt % SMK_MIDI_NPATS);
        } else if (role == 4) { /* K2: LENGTH */
            int32_t l = (int32_t)smk_m.pat_len + delta;
            if (l < 1) l = 1; if (l > 64) l = 64;
            smk_m.pat_len = (uint8_t)l;
        } else if (role == 5) { /* K3: DIV */
            /* Division */
        } else if (role == 6) { /* K4: SWING */
            /* Swing */
        }
        break;

    case SMK_PAGE_SONG:
        if (role == 3) { /* K1: ROW */
            int32_t rw = (int32_t)smk_m.cur_song_row + delta;
            while (rw < 0) rw += smk_m.total_song_rows;
            smk_m.cur_song_row = (uint8_t)(rw % smk_m.total_song_rows);
        } else if (role == 4) { /* K2: PAT */
            uint32_t rw = smk_m.cur_song_row % SMK_MIDI_NROWS;
            int32_t p = (int32_t)smk_song_chain[rw].pattern + delta;
            while (p < 0) p += SMK_MIDI_NPATS;
            smk_song_chain[rw].pattern = (uint8_t)(p % SMK_MIDI_NPATS);
        } else if (role == 5) { /* K3: RPT */
            uint32_t rw = smk_m.cur_song_row % SMK_MIDI_NROWS;
            int32_t rp = (int32_t)smk_song_chain[rw].repeat + delta;
            if (rp < 1) rp = 1; if (rp > 16) rp = 16;
            smk_song_chain[rw].repeat = (uint8_t)rp;
        } else if (role == 6) { /* K4: PLAY/STOP */
            if (delta > 0) smk_m.song_mode_on = 1;
            else if (delta < 0) smk_m.song_mode_on = 0;
        }
        break;

    case SMK_PAGE_CC:
        if (role == 3) { /* K1: TARGET TYPE */
            int32_t t = (int32_t)smk_m.cc_edit_target + delta;
            while (t < 0) t += 3;
            smk_m.cc_edit_target = (uint8_t)(t % 3);
            smk_m.cc_edit_index = 0;
        } else if (role == 4) { /* K2: TARGET INDEX */
            uint32_t max_idx = (smk_m.cc_edit_target == 0) ? (smk_m.num_knob_banks * 8u) :
                               (smk_m.cc_edit_target == 1) ? (smk_m.num_fader_banks * 4u) :
                                                             (smk_m.num_pad_banks * 16u);
            int32_t idx = (int32_t)smk_m.cc_edit_index + delta;
            while (idx < 0) idx += (int32_t)max_idx;
            smk_m.cc_edit_index = (uint8_t)((uint32_t)idx % max_idx);
        } else if (role == 5) { /* K3: CC NUMBER */
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
        } else if (role == 6) { /* K4: CHANNEL */
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
        }
        break;
    }
}

/* Fader movement handler: emits CC for active fader bank */
void smk_midi_handle_fader(uint32_t id, uint32_t val)
{
    uint32_t fader_idx = (smk_m.fader_bank * 4u) + (id % 4u);
    if (fader_idx < SMK_MAX_FADERS) {
        smk_midi_send_cc(smk_m.fader_ch[fader_idx] - 1u, smk_m.fader_cc[fader_idx], val & 0x7Fu);
    }
}

/* Pad event handler: emits CC for active pad bank */
void smk_midi_handle_pad(uint32_t id, uint32_t val)
{
    uint32_t pad_idx = (smk_m.pad_bank * 16u) + (id % 16u);
    if (pad_idx < SMK_MAX_PADS) {
        smk_midi_send_cc(smk_m.pad_ch[pad_idx] - 1u, smk_m.pad_cc[pad_idx], val & 0x7Fu);
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

    if (pressed & (1u << 0)) { /* B_FX / SCALE on SMK-37 */
        if (smk_m.page == SMK_PAGE_SCL) {
            smk_m.page = SMK_PAGE_CHORD;
            smk_set_toast("CHORD VIEW");
        } else {
            smk_m.page = SMK_PAGE_SCL;
            smk_set_toast("SCALE VIEW");
        }
    } else if (pressed & (1u << 8)) { /* B_ARP on SMK-37 */
        smk_m.page = SMK_PAGE_ARP;
        smk_set_toast("ARPEGGIATOR VIEW");
    } else if (pressed & (1u << 9)) { /* B_SEQ on SMK-37 */
        if (smk_m.page == SMK_PAGE_SEQ) {
            smk_m.page = SMK_PAGE_SONG;
            smk_set_toast("SONG VIEW");
        } else {
            smk_m.page = SMK_PAGE_SEQ;
            smk_set_toast("SEQUENCER VIEW");
        }
    } else if (pressed & ((1u << 1) | (1u << 4) | (1u << 6))) { /* B_SEL, B_EDIT, B_HOME -> CC Mapper */
        smk_m.page = SMK_PAGE_CC;
        smk_set_toast("CC MAPPER VIEW");
    } else if (pressed & (1u << 12)) { /* B_OCTDN (single tap) */
        if (smk_m.octave > -3) smk_m.octave--;
        smk_set_toast("OCTAVE -");
    } else if (pressed & (1u << 13)) { /* B_OCTUP (single tap) */
        if (smk_m.octave < 3) smk_m.octave++;
        smk_set_toast("OCTAVE +");
    } else if (pressed & (1u << 10)) { /* B_PLAY */
        smk_m.seq_playing = !smk_m.seq_playing;
        smk_set_toast(smk_m.seq_playing ? "SEQ PLAY" : "SEQ STOP");
    } else if (pressed & (1u << 11)) { /* B_REC */
        smk_m.seq_rec = !smk_m.seq_rec;
        smk_set_toast(smk_m.seq_rec ? "SEQ REC ON" : "SEQ REC OFF");
    }
}

/* Background millisecond tick */
void smk_midi_step_ms(void)
{
    if (smk_m.toast_ms > 0) smk_m.toast_ms--;

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
