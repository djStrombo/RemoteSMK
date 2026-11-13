/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2026 RemoteSMK contributors */
#pragma once
#include "smk_midi_types.h"
#include "smk_midi_scales.h"

#define CLR_BG      0x0000u  /* Pure Black */
#define CLR_INK     0xFFFFu  /* Pure White */
#define CLR_DIM     0x8410u  /* Grey */
#define CLR_BORDER  0x4208u  /* Dark Grey */

static const char *const SMK_ROOT_NAMES[12] = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

static const char *const SMK_RATE_NAMES[6] = {
    "1/4", "1/8", "1/8T", "1/16", "1/16T", "1/32"
};

static const char *const SMK_GATE_NAMES[4] = {
    "25%", "50%", "75%", "100%"
};

static void smk_ui_format_note(char *dst, uint32_t note)
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

static char *smk_ui_put_str(char *d, const char *s)
{
    while (*s) *d++ = *s++;
    *d = 0;
    return d;
}

static char *smk_ui_put_u(char *d, uint32_t v)
{
    if (v >= 100u) {
        *d++ = (char)('0' + (v / 100u) % 10u);
        *d++ = (char)('0' + (v / 10u) % 10u);
        *d++ = (char)('0' + v % 10u);
    } else if (v >= 10u) {
        *d++ = (char)('0' + (v / 10u) % 10u);
        *d++ = (char)('0' + v % 10u);
    } else {
        *d++ = (char)('0' + v % 10u);
    }
    *d = 0;
    return d;
}

static inline int32_t smk_text_w(const char *s, int32_t n)
{
    if (!s) return 0;
    int32_t l = 0;
    while (s[l]) l++;
    return l * 6 * n;
}

static char *smk_ui_put_u2(char *d, uint32_t v)
{
    *d++ = (char)('0' + (v / 10u) % 10u);
    *d++ = (char)('0' + v % 10u);
    *d = 0;
    return d;
}

static char *smk_ui_put_u3(char *d, uint32_t v)
{
    if (v >= 100u) {
        *d++ = (char)('0' + v / 100u);
        *d++ = (char)('0' + (v % 100u) / 10u);
        *d++ = (char)('0' + v % 10u);
    } else if (v >= 10u) {
        *d++ = ' ';
        *d++ = (char)('0' + v / 10u);
        *d++ = (char)('0' + v % 10u);
    } else {
        *d++ = ' ';
        *d++ = ' ';
        *d++ = (char)('0' + v);
    }
    *d = 0;
    return d;
}

/* Forward declarations of sa_gfx primitives */
static void sg_rect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t c);
static void sg_chip(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t c);
static int32_t sg_text(int32_t x, int32_t y, int32_t n, const char *s, uint16_t c);

static const uint8_t SMK_KNOB_RING[21][21] = {
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x37, 0x3A, 0x3D, 0x40, 0x42, 0x45, 0x48, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x30, 0x33, 0x36, 0x39, 0x3C, 0x40, 0x43, 0x46, 0x49, 0x4C, 0x4F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    {0xFF, 0xFF, 0xFF, 0xFF, 0x2C, 0x2F, 0x32, 0x35, 0x38, 0x3C, 0x40, 0x43, 0x47, 0x4A, 0x4D, 0x50, 0x53, 0xFF, 0xFF, 0xFF, 0xFF},
    {0xFF, 0xFF, 0xFF, 0x28, 0x2A, 0x2D, 0x30, 0x33, 0x37, 0xFF, 0xFF, 0xFF, 0x48, 0x4C, 0x4F, 0x52, 0x55, 0x57, 0xFF, 0xFF, 0xFF},
    {0xFF, 0xFF, 0x24, 0x26, 0x28, 0x2A, 0x2D, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x52, 0x55, 0x57, 0x59, 0x5B, 0xFF, 0xFF},
    {0xFF, 0xFF, 0x22, 0x23, 0x25, 0x27, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x58, 0x5A, 0x5C, 0x5D, 0xFF, 0xFF},
    {0xFF, 0x1E, 0x1F, 0x20, 0x22, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x5D, 0x5F, 0x60, 0x61, 0xFF},
    {0xFF, 0x1B, 0x1C, 0x1D, 0x1E, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x61, 0x62, 0x63, 0x64, 0xFF},
    {0xFF, 0x18, 0x19, 0x19, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x66, 0x66, 0x67, 0xFF},
    {0xFF, 0x15, 0x15, 0x15, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x6A, 0x6A, 0x6A, 0xFF},
    {0xFF, 0x12, 0x12, 0x11, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x6E, 0x6D, 0x6D, 0xFF},
    {0xFF, 0x0F, 0x0F, 0x0E, 0x0C, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x73, 0x71, 0x70, 0x70, 0xFF},
    {0xFF, 0x0C, 0x0B, 0x0A, 0x09, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x76, 0x75, 0x74, 0x73, 0xFF},
    {0xFF, 0xFF, 0x09, 0x07, 0x05, 0x03, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7C, 0x7A, 0x78, 0x76, 0xFF, 0xFF},
    {0xFF, 0xFF, 0x06, 0x04, 0x02, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x7D, 0x7B, 0x79, 0xFF, 0xFF},
    {0xFF, 0xFF, 0xFF, 0x02, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x7D, 0xFF, 0xFF, 0xFF},
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
};

static int32_t smk_font_draw(int32_t x, int32_t y, const felucca_font_t *f, int32_t scale, const char *str, uint16_t c)
{
    if (scale <= 0) scale = 1;
    uint32_t r = c >> 11, g = (c >> 5) & 63u, b = c & 31u;
    for (; *str; str++) {
        uint32_t ch = (uint8_t)*str;
        if (ch >= 'a' && ch <= 'z' && f->last < 'a')
            ch -= 32u;
        if (ch < f->first || ch > f->last || (ch >= 127u && ch < 160u))
            ch = '?';
        uint32_t gi = ch - f->first;
        uint32_t w = f->bw[gi];
        uint32_t bpr = (w + 1u) / 2u;
        const uint8_t *gd = f->data + f->off[gi];
        for (uint32_t gy = 0; gy < f->h; gy++) {
            for (uint32_t gx = 0; gx < w; gx++) {
                uint32_t a = gd[gy * bpr + gx / 2u];
                a = (gx & 1u) ? (a & 15u) : (a >> 4);
                if (!a) continue;

                int32_t px = x + (-f->pad + (int32_t)gx) * scale;
                int32_t py = y + (int32_t)gy * scale;

                for (int32_t sy = 0; sy < scale; sy++) {
                    int32_t dpy = py + sy;
                    if (dpy < sg_y0 || dpy >= sg_y0 + SG_BH)
                        continue;
                    uint16_t *line_ptr = sg_px + (dpy - sg_y0) * SG_W;
                    for (int32_t sx = 0; sx < scale; sx++) {
                        int32_t dpx = px + sx;
                        if (dpx < 0 || dpx >= SG_W)
                            continue;
                        if (a == 15u) {
                            line_ptr[dpx] = sg_sw(c);
                        } else {
                            uint16_t bg = sg_sw(line_ptr[dpx]);
                            uint32_t br = bg >> 11, bgg = (bg >> 5) & 63u, bb = bg & 31u;
                            uint32_t nr = (r * a + br * (15u - a)) / 15u;
                            uint32_t ng = (g * a + bgg * (15u - a)) / 15u;
                            uint32_t nb = (b * a + bb * (15u - a)) / 15u;
                            line_ptr[dpx] = sg_sw((uint16_t)((nr << 11) | (ng << 5) | nb));
                        }
                    }
                }
            }
        }
        x += f->adv[gi] * scale;
    }
    return x;
}

static int32_t smk_font_w(const felucca_font_t *f, int32_t scale, const char *str)
{
    if (scale <= 0) scale = 1;
    int32_t w = 0;
    for (; *str; str++) {
        uint32_t ch = (uint8_t)*str;
        if (ch >= 'a' && ch <= 'z' && f->last < 'a')
            ch -= 32u;
        if (ch < f->first || ch > f->last || (ch >= 127u && ch < 160u))
            ch = '?';
        w += f->adv[ch - f->first] * scale;
    }
    return w;
}

/* Helper to draw standard menu header and footer */
static void smk_ui_draw_menu_frame(const char *title, const char *hint)
{
    sg_rect(0, 0, 240, 240, CLR_BG);
    smk_font_draw(10, 6, &FONT_B, 1, title, CLR_INK);
    sg_rect(0, 26, 240, 1, CLR_INK);
    sg_rect(0, 214, 240, 1, CLR_INK);
    smk_font_draw(8, 220, &FONT_T, 1, hint ? hint : "[< >] CHOOSE  [OCT-] OFF  [OCT+] ON", CLR_INK);
}

/* Helper to draw a single text list row */
static void smk_ui_draw_list_row(int32_t y, const char *prefix, const char *val, int selected)
{
    if (selected) {
        sg_rect(6, y - 2, 228, 20, CLR_INK);
        smk_font_draw(12, y + 2, &FONT_B, 1, prefix, CLR_BG);
        if (val) smk_font_draw(130, y + 2, &FONT_B, 1, val, CLR_BG);
    } else {
        smk_font_draw(12, y + 2, &FONT_B, 1, prefix, CLR_INK);
        if (val) smk_font_draw(130, y + 2, &FONT_B, 1, val, CLR_INK);
    }
}

/* Draw a minimalist circular ring without borders at (x, y), 21x21 */
static void smk_ui_draw_knob_ring(int32_t x, int32_t y, uint8_t val)
{
    int32_t dy, dx;
    for (dy = 0; dy < 21; dy++) {
        int32_t py = y + dy;
        if (py < sg_y0 || py >= sg_y0 + SG_BH) continue;
        uint16_t *line_ptr = sg_px + (py - sg_y0) * SG_W;
        for (dx = 0; dx < 21; dx++) {
            uint8_t th = SMK_KNOB_RING[dy][dx];
            if (th == 0xFFu) continue;
            int32_t px = x + dx;
            if (px < 0 || px >= SG_W) continue;
            if (th <= val) {
                line_ptr[px] = sg_sw(CLR_INK);
            } else {
                line_ptr[px] = sg_sw(CLR_BORDER);
            }
        }
    }
}

/* Draw a proportional horizontal fader slider at (x, y), width 60, height 14 */
static void smk_ui_draw_fader_slider(int32_t x, int32_t y, uint8_t val)
{
    /* Fader track (60px wide, 2px high) */
    sg_rect(x, y + 6, 60, 2, CLR_DIM);
    /* Left and right end limit markers */
    sg_rect(x, y + 3, 1, 8, CLR_DIM);
    sg_rect(x + 59, y + 3, 1, 8, CLR_DIM);
    /* Center tick */
    sg_rect(x + 29, y + 4, 1, 6, CLR_BORDER);

    /* Fader handle (8px wide, 14px high) */
    int32_t hx = x + ((int32_t)val * 52) / 127;
    sg_rect(hx, y, 8, 14, CLR_INK);
    /* Handle grip line */
    sg_rect(hx + 3, y + 3, 2, 8, CLR_BG);
}

/* Page 0: Super Minimalist Main Screen with Fine Barlow Typography */
static void smk_ui_draw_general(void)
{
    sg_rect(0, 0, 240, 240, CLR_BG);

    /* Center hero: Note with superscript octave or Chord with descriptor */
    if (smk_m.chord_name[0]) {
        /* Parse Root and Descriptor, e.g. "C" and "MAJOR", "F#" and "7TH" */
        char root[8], desc[24];
        uint32_t i = 0, j = 0;
        while (smk_m.chord_name[i] && smk_m.chord_name[i] != ' ' && j < 7u) {
            root[j++] = smk_m.chord_name[i++];
        }
        root[j] = 0;
        if (smk_m.chord_name[i] == ' ') i++;
        j = 0;
        while (smk_m.chord_name[i] && j < 23u) {
            desc[j++] = smk_m.chord_name[i++];
        }
        desc[j] = 0;

        int32_t w_root = smk_font_w(&FONT_L, 2, root);
        int32_t w_desc = smk_font_w(&FONT_B, 1, desc);
        int32_t gap = 6;
        int32_t total_w = w_root + gap + w_desc;
        int32_t sx = (240 - total_w) / 2;
        int32_t ny = 68;

        smk_font_draw(sx, ny, &FONT_L, 2, root, CLR_INK);
        smk_font_draw(sx + w_root + gap, ny, &FONT_B, 1, desc, CLR_INK);
    } else if (smk_m.last_note_name[0]) {
        /* Parse Root and Octave number, e.g. "C" and "3", "F#" and "4", "A" and "-1" */
        char root[8], oct_str[8];
        uint32_t i = 0, j = 0;
        while (smk_m.last_note_name[i] && (smk_m.last_note_name[i] < '0' || smk_m.last_note_name[i] > '9') && smk_m.last_note_name[i] != '-' && j < 7u) {
            root[j++] = smk_m.last_note_name[i++];
        }
        root[j] = 0;
        j = 0;
        while (smk_m.last_note_name[i] && j < 7u) {
            oct_str[j++] = smk_m.last_note_name[i++];
        }
        oct_str[j] = 0;

        int32_t w_root = smk_font_w(&FONT_L, 2, root);
        int32_t w_oct = smk_font_w(&FONT_M, 1, oct_str);
        int32_t gap = 4;
        int32_t total_w = w_root + gap + w_oct;
        int32_t sx = (240 - total_w) / 2;
        int32_t ny = 68;

        smk_font_draw(sx, ny, &FONT_L, 2, root, CLR_INK);
        smk_font_draw(sx + w_root + gap, ny, &FONT_M, 1, oct_str, CLR_INK);
    } else {
        int32_t w = smk_font_w(&FONT_L, 2, "--");
        smk_font_draw((240 - w) / 2, 68, &FONT_L, 2, "--", CLR_DIM);
    }

    /* Bottom: Transient control feedback (disappears after ~2s) */
    uint32_t now = plat_ms();
    int show_hud = (smk_m.last_ctrl_ms > 0 && (now - smk_m.last_ctrl_ms) < 2000u && smk_m.last_ctrl_type != SMK_HUD_NONE);

    if (show_hud) {
        switch (smk_m.last_ctrl_type) {
        case SMK_HUD_KNOB: {
            uint32_t knum = (uint32_t)smk_m.last_ctrl_bank * 8u + (uint32_t)smk_m.last_ctrl_id + 1u;
            char s_label[8], s_cc[8];
            char *p = smk_ui_put_str(s_label, "K");
            p = smk_ui_put_u(p, knum);
            smk_ui_put_u(s_cc, smk_m.last_ctrl_cc);

            int32_t w1 = smk_font_w(&FONT_B, 1, s_label);
            int32_t w2 = smk_font_w(&FONT_B, 1, s_cc);
            int32_t ring_w = 21;
            int32_t gap = 10;
            int32_t total_w = w1 + gap + ring_w + gap + w2;
            int32_t sx = (240 - total_w) / 2;
            int32_t sy = 192;

            smk_font_draw(sx, sy + 2, &FONT_B, 1, s_label, CLR_INK);
            smk_ui_draw_knob_ring(sx + w1 + gap, sy, smk_m.last_ctrl_val);
            smk_font_draw(sx + w1 + gap + ring_w + gap, sy + 2, &FONT_B, 1, s_cc, CLR_INK);
            break;
        }
        case SMK_HUD_FADER: {
            uint32_t fnum = (uint32_t)smk_m.last_ctrl_bank * 4u + (uint32_t)smk_m.last_ctrl_id + 1u;
            char s_label[8], s_cc[8];
            char *p = smk_ui_put_str(s_label, "F");
            p = smk_ui_put_u(p, fnum);
            smk_ui_put_u(s_cc, smk_m.last_ctrl_cc);

            int32_t w1 = smk_font_w(&FONT_B, 1, s_label);
            int32_t w2 = smk_font_w(&FONT_B, 1, s_cc);
            int32_t fader_w = 60;
            int32_t gap = 10;
            int32_t total_w = w1 + gap + fader_w + gap + w2;
            int32_t sx = (240 - total_w) / 2;
            int32_t sy = 192;

            smk_font_draw(sx, sy + 2, &FONT_B, 1, s_label, CLR_INK);
            smk_ui_draw_fader_slider(sx + w1 + gap, sy + 3, smk_m.last_ctrl_val);
            smk_font_draw(sx + w1 + gap + fader_w + gap, sy + 2, &FONT_B, 1, s_cc, CLR_INK);
            break;
        }
        case SMK_HUD_PAD: {
            uint32_t pnum = (uint32_t)smk_m.last_ctrl_bank * 16u + (uint32_t)smk_m.last_ctrl_id + 1u;
            if (smk_m.last_ctrl_sub == 0) {
                /* NOTES mode: P<num>  Note^Oct  Vel */
                char s_pad[8], s_vel[8], nstr[8], root[8], oct_str[8];
                char *p = smk_ui_put_str(s_pad, "P");
                p = smk_ui_put_u(p, pnum);
                smk_ui_put_u(s_vel, smk_m.last_ctrl_val);
                smk_ui_format_note(nstr, smk_m.last_ctrl_cc);

                uint32_t ni = 0, rj = 0;
                while (nstr[ni] && (nstr[ni] < '0' || nstr[ni] > '9') && nstr[ni] != '-' && rj < 7u) {
                    root[rj++] = nstr[ni++];
                }
                root[rj] = 0;
                rj = 0;
                while (nstr[ni] && rj < 7u) oct_str[rj++] = nstr[ni++];
                oct_str[rj] = 0;

                int32_t w_pad = smk_font_w(&FONT_B, 1, s_pad);
                int32_t w_root = smk_font_w(&FONT_B, 1, root);
                int32_t w_oct = smk_font_w(&FONT_XS, 1, oct_str);
                int32_t w_vel = smk_font_w(&FONT_B, 1, s_vel);
                int32_t gap = 12;
                int32_t total_w = w_pad + gap + w_root + 2 + w_oct + gap + w_vel;
                int32_t sx = (240 - total_w) / 2;
                int32_t sy = 194;

                smk_font_draw(sx, sy, &FONT_B, 1, s_pad, CLR_INK);
                sx += w_pad + gap;
                smk_font_draw(sx, sy, &FONT_B, 1, root, CLR_INK);
                sx += w_root + 2;
                smk_font_draw(sx, sy - 3, &FONT_XS, 1, oct_str, CLR_INK);
                sx += w_oct + gap;
                smk_font_draw(sx, sy, &FONT_B, 1, s_vel, CLR_INK);
            } else {
                /* CC mode */
                char s_label[8], s_cc[8];
                char *p = smk_ui_put_str(s_label, "P");
                p = smk_ui_put_u(p, pnum);
                smk_ui_put_u(s_cc, smk_m.last_ctrl_cc);

                int32_t w1 = smk_font_w(&FONT_B, 1, s_label);
                int32_t w2 = smk_font_w(&FONT_B, 1, s_cc);
                int32_t ring_w = 21;
                int32_t gap = 10;
                int32_t total_w = w1 + gap + ring_w + gap + w2;
                int32_t sx = (240 - total_w) / 2;
                int32_t sy = 192;

                smk_font_draw(sx, sy + 2, &FONT_B, 1, s_label, CLR_INK);
                smk_ui_draw_knob_ring(sx + w1 + gap, sy, smk_m.last_ctrl_val);
                smk_font_draw(sx + w1 + gap + ring_w + gap, sy + 2, &FONT_B, 1, s_cc, CLR_INK);
            }
            break;
        }
        case SMK_HUD_TOAST: {
            int32_t w = smk_font_w(&FONT_B, 1, smk_m.last_ctrl_text);
            smk_font_draw((240 - w) / 2, 194, &FONT_B, 1, smk_m.last_ctrl_text, CLR_INK);
            break;
        }
        default: break;
        }
    }
}

/* Page 1: Scale Menu */
static void smk_ui_draw_scale(void)
{
    char buf[32];
    if (smk_m.menu_depth == 0) {
        smk_ui_draw_menu_frame("[ SCALE MENU ]", 0);
        smk_ui_draw_list_row(34, "1. ACTIVE:", smk_m.scale_on ? "[ON]" : "[OFF]", smk_m.menu_sel[0] == 0);
        smk_ui_draw_list_row(58, "2. ROOT KEY:", SMK_ROOT_NAMES[smk_m.root % 12], smk_m.menu_sel[0] == 1);
        smk_ui_draw_list_row(82, "3. SCALE:", SCALE_TITLE[smk_m.scale % SCALE_TOTAL], smk_m.menu_sel[0] == 2);

        buf[0] = (char)('0' + smk_m.bpm / 100u);
        buf[1] = (char)('0' + (smk_m.bpm % 100u) / 10u);
        buf[2] = (char)('0' + smk_m.bpm % 10u);
        buf[3] = ' '; buf[4] = 'B'; buf[5] = 'P'; buf[6] = 'M'; buf[7] = 0;
        smk_ui_draw_list_row(106, "4. TEMPO:", buf, smk_m.menu_sel[0] == 3);
    } else {
        /* Submenus */
        if (smk_m.menu_sel[0] == 1) {
            /* ROOT KEY selection list */
            smk_ui_draw_menu_frame("[ SCALE > ROOT KEY ]", "[< >] CHOOSE  [OCT+] OK  [OCT-] CANCEL");
            uint32_t i;
            for (i = 0; i < 7 && (i + (uint32_t)smk_m.menu_scroll[1]) < 12u; i++) {
                uint32_t idx = i + (uint32_t)smk_m.menu_scroll[1];
                smk_ui_draw_list_row(34 + (int32_t)i * 24, SMK_ROOT_NAMES[idx],
                                     idx == (uint32_t)smk_m.root ? "<ACTIVE>" : 0,
                                     idx == (uint32_t)smk_m.menu_sel[1]);
            }
        } else if (smk_m.menu_sel[0] == 2) {
            /* SCALE selection list */
            smk_ui_draw_menu_frame("[ SCALE > SELECT ]", "[< >] CHOOSE  [OCT+] OK  [OCT-] CANCEL");
            uint32_t i;
            for (i = 0; i < 7 && (i + (uint32_t)smk_m.menu_scroll[1]) < SCALE_TOTAL; i++) {
                uint32_t idx = i + (uint32_t)smk_m.menu_scroll[1];
                smk_ui_draw_list_row(34 + (int32_t)i * 24, SCALE_TITLE[idx],
                                     idx == (uint32_t)smk_m.scale ? "<ACTIVE>" : 0,
                                     idx == (uint32_t)smk_m.menu_sel[1]);
            }
        }
    }
}

/* Page 2: Chord Menu */
static void smk_ui_draw_chord(void)
{
    char buf[32];
    if (smk_m.menu_depth == 0) {
        smk_ui_draw_menu_frame("[ CHORD MENU ]", 0);
        smk_ui_draw_list_row(34, "1. ACTIVE:", smk_m.chord_on ? "[ON]" : "[OFF]", smk_m.menu_sel[0] == 0);
        smk_ui_draw_list_row(58, "2. CHORD TYPE:", SMK_CHORD_NAMES[smk_m.chord_type % CH_TYPE_COUNT], smk_m.menu_sel[0] == 1);
        smk_ui_draw_list_row(82, "3. VOICING:", SMK_VOIC_NAMES[smk_m.chord_voic % VC_COUNT], smk_m.menu_sel[0] == 2);

        buf[0] = (char)('0' + smk_m.bpm / 100u);
        buf[1] = (char)('0' + (smk_m.bpm % 100u) / 10u);
        buf[2] = (char)('0' + smk_m.bpm % 10u);
        buf[3] = ' '; buf[4] = 'B'; buf[5] = 'P'; buf[6] = 'M'; buf[7] = 0;
        smk_ui_draw_list_row(106, "4. TEMPO:", buf, smk_m.menu_sel[0] == 3);
    } else {
        if (smk_m.menu_sel[0] == 1) {
            smk_ui_draw_menu_frame("[ CHORD > TYPE ]", "[< >] CHOOSE  [OCT+] OK  [OCT-] CANCEL");
            uint32_t i;
            for (i = 0; i < 7 && (i + (uint32_t)smk_m.menu_scroll[1]) < CH_TYPE_COUNT; i++) {
                uint32_t idx = i + (uint32_t)smk_m.menu_scroll[1];
                smk_ui_draw_list_row(34 + (int32_t)i * 24, SMK_CHORD_NAMES[idx],
                                     idx == (uint32_t)smk_m.chord_type ? "<ACTIVE>" : 0,
                                     idx == (uint32_t)smk_m.menu_sel[1]);
            }
        } else if (smk_m.menu_sel[0] == 2) {
            smk_ui_draw_menu_frame("[ CHORD > VOICING ]", "[< >] CHOOSE  [OCT+] OK  [OCT-] CANCEL");
            uint32_t i;
            for (i = 0; i < VC_COUNT; i++) {
                smk_ui_draw_list_row(34 + (int32_t)i * 24, SMK_VOIC_NAMES[i],
                                     i == (uint32_t)smk_m.chord_voic ? "<ACTIVE>" : 0,
                                     i == (uint32_t)smk_m.menu_sel[1]);
            }
        }
    }
}

/* Page 3: Arpeggiator Menu */
static void smk_ui_draw_arp(void)
{
    char buf[32];
    if (smk_m.menu_depth == 0) {
        smk_ui_draw_menu_frame("[ ARP MENU ]", 0);
        smk_ui_draw_list_row(34, "1. ACTIVE:", smk_m.arp_on ? "[ON]" : "[OFF]", smk_m.menu_sel[0] == 0);
        smk_ui_draw_list_row(58, "2. MODE:", SMK_ARP_MODE_NAMES[smk_m.arp_mode % 6], smk_m.menu_sel[0] == 1);
        smk_ui_draw_list_row(82, "3. RATE:", SMK_RATE_NAMES[smk_m.arp_rate % 6], smk_m.menu_sel[0] == 2);

        buf[0] = (char)('0' + smk_m.arp_oct); buf[1] = 0;
        smk_ui_draw_list_row(106, "4. OCTAVES:", buf, smk_m.menu_sel[0] == 3);
        smk_ui_draw_list_row(130, "5. GATE:", SMK_GATE_NAMES[smk_m.arp_gate % 4], smk_m.menu_sel[0] == 4);
        smk_ui_draw_list_row(154, "6. LATCH:", smk_m.arp_latch ? "[ON]" : "[OFF]", smk_m.menu_sel[0] == 5);

        buf[0] = (char)('0' + smk_m.bpm / 100u);
        buf[1] = (char)('0' + (smk_m.bpm % 100u) / 10u);
        buf[2] = (char)('0' + smk_m.bpm % 10u);
        buf[3] = ' '; buf[4] = 'B'; buf[5] = 'P'; buf[6] = 'M'; buf[7] = 0;
        smk_ui_draw_list_row(178, "7. TEMPO:", buf, smk_m.menu_sel[0] == 6);
    } else {
        if (smk_m.menu_sel[0] == 1) {
            smk_ui_draw_menu_frame("[ ARP > MODE ]", "[< >] CHOOSE  [OCT+] OK  [OCT-] CANCEL");
            uint32_t i;
            for (i = 0; i < 6; i++) {
                smk_ui_draw_list_row(34 + (int32_t)i * 24, SMK_ARP_MODE_NAMES[i],
                                     i == (uint32_t)smk_m.arp_mode ? "<ACTIVE>" : 0,
                                     i == (uint32_t)smk_m.menu_sel[1]);
            }
        } else if (smk_m.menu_sel[0] == 2) {
            smk_ui_draw_menu_frame("[ ARP > RATE ]", "[< >] CHOOSE  [OCT+] OK  [OCT-] CANCEL");
            uint32_t i;
            for (i = 0; i < 6; i++) {
                smk_ui_draw_list_row(34 + (int32_t)i * 24, SMK_RATE_NAMES[i],
                                     i == (uint32_t)smk_m.arp_rate ? "<ACTIVE>" : 0,
                                     i == (uint32_t)smk_m.menu_sel[1]);
            }
        } else if (smk_m.menu_sel[0] == 3) {
            smk_ui_draw_menu_frame("[ ARP > OCTAVES ]", "[< >] CHOOSE  [OCT+] OK  [OCT-] CANCEL");
            uint32_t i;
            for (i = 0; i < 4; i++) {
                buf[0] = (char)('1' + i); buf[1] = ' '; buf[2] = 'O'; buf[3] = 'C'; buf[4] = 'T'; buf[5] = 0;
                smk_ui_draw_list_row(34 + (int32_t)i * 24, buf,
                                     i == (uint32_t)(smk_m.arp_oct - 1) ? "<ACTIVE>" : 0,
                                     i == (uint32_t)smk_m.menu_sel[1]);
            }
        } else if (smk_m.menu_sel[0] == 4) {
            smk_ui_draw_menu_frame("[ ARP > GATE ]", "[< >] CHOOSE  [OCT+] OK  [OCT-] CANCEL");
            uint32_t i;
            for (i = 0; i < 4; i++) {
                smk_ui_draw_list_row(34 + (int32_t)i * 24, SMK_GATE_NAMES[i],
                                     i == (uint32_t)smk_m.arp_gate ? "<ACTIVE>" : 0,
                                     i == (uint32_t)smk_m.menu_sel[1]);
            }
        }
    }
}

/* Page 4: Note Repeat Menu */
static void smk_ui_draw_nr(void)
{
    char buf[32];
    if (smk_m.menu_depth == 0) {
        smk_ui_draw_menu_frame("[ NOTE REPEAT ]", 0);
        smk_ui_draw_list_row(34, "1. ACTIVE:", smk_m.nr_on ? "[ON]" : "[OFF]", smk_m.menu_sel[0] == 0);
        smk_ui_draw_list_row(58, "2. RATE:", SMK_RATE_NAMES[smk_m.nr_rate % 6], smk_m.menu_sel[0] == 1);
        smk_ui_draw_list_row(82, "3. GATE:", SMK_GATE_NAMES[smk_m.nr_gate % 4], smk_m.menu_sel[0] == 2);

        buf[0] = (char)('0' + smk_m.bpm / 100u);
        buf[1] = (char)('0' + (smk_m.bpm % 100u) / 10u);
        buf[2] = (char)('0' + smk_m.bpm % 10u);
        buf[3] = ' '; buf[4] = 'B'; buf[5] = 'P'; buf[6] = 'M'; buf[7] = 0;
        smk_ui_draw_list_row(106, "4. TEMPO:", buf, smk_m.menu_sel[0] == 3);
    } else {
        if (smk_m.menu_sel[0] == 1) {
            smk_ui_draw_menu_frame("[ REPEAT > RATE ]", "[< >] CHOOSE  [OCT+] OK  [OCT-] CANCEL");
            uint32_t i;
            for (i = 0; i < 6; i++) {
                smk_ui_draw_list_row(34 + (int32_t)i * 24, SMK_RATE_NAMES[i],
                                     i == (uint32_t)smk_m.nr_rate ? "<ACTIVE>" : 0,
                                     i == (uint32_t)smk_m.menu_sel[1]);
            }
        } else if (smk_m.menu_sel[0] == 2) {
            smk_ui_draw_menu_frame("[ REPEAT > GATE ]", "[< >] CHOOSE  [OCT+] OK  [OCT-] CANCEL");
            uint32_t i;
            for (i = 0; i < 4; i++) {
                smk_ui_draw_list_row(34 + (int32_t)i * 24, SMK_GATE_NAMES[i],
                                     i == (uint32_t)smk_m.nr_gate ? "<ACTIVE>" : 0,
                                     i == (uint32_t)smk_m.menu_sel[1]);
            }
        }
    }
}

/* Page 5: Globe / Settings Menu */
static void smk_ui_draw_globe(void)
{
    char buf[32];
    static const char *const VEL_CURVES[3] = {"NORMAL", "SOFT", "HARD"};
    static const char *const SENS_NAMES[3] = {"NORMAL", "LOW", "HIGH"};
    static const char *const SPEED_NAMES[3] = {"NORMAL", "SLOW", "FAST"};

    if (smk_m.menu_depth == 0) {
        smk_ui_draw_menu_frame("[ GLOBE SETTINGS ]", 0);
        uint32_t i;
        for (i = 0; i < 7 && (i + (uint32_t)smk_m.menu_scroll[0]) < 9u; i++) {
            uint32_t idx = i + (uint32_t)smk_m.menu_scroll[0];
            int32_t y = 34 + (int32_t)i * 24;
            switch (idx) {
            case 0: smk_ui_draw_list_row(y, "1. CC ASSIGN", ">", idx == (uint32_t)smk_m.menu_sel[0]); break;
            case 1: smk_ui_draw_list_row(y, "2. PAD MODE:", smk_m.pad_mode ? "[CC]" : "[NOTES]", idx == (uint32_t)smk_m.menu_sel[0]); break;
            case 2: smk_ui_draw_list_row(y, "3. FULL VELOCITY:", smk_m.full_velocity ? "[ON]" : "[OFF]", idx == (uint32_t)smk_m.menu_sel[0]); break;
            case 3: smk_ui_draw_list_row(y, "4. 4 BANKS:", smk_m.four_banks ? "[ON]" : "[OFF]", idx == (uint32_t)smk_m.menu_sel[0]); break;
            case 4: smk_ui_draw_list_row(y, "5. KEY VELOCITY:", VEL_CURVES[smk_m.key_vel_curve % 3], idx == (uint32_t)smk_m.menu_sel[0]); break;
            case 5: smk_ui_draw_list_row(y, "6. PAD SENS:", SENS_NAMES[smk_m.pad_sens % 3], idx == (uint32_t)smk_m.menu_sel[0]); break;
            case 6: smk_ui_draw_list_row(y, "7. KNOB SPEED:", SPEED_NAMES[smk_m.knob_speed % 3], idx == (uint32_t)smk_m.menu_sel[0]); break;
            case 7: smk_ui_draw_list_row(y, "8. MIDI CHANNELS", ">", idx == (uint32_t)smk_m.menu_sel[0]); break;
            case 8: {
                buf[0] = (char)('0' + smk_m.bpm / 100u);
                buf[1] = (char)('0' + (smk_m.bpm % 100u) / 10u);
                buf[2] = (char)('0' + smk_m.bpm % 10u);
                buf[3] = ' '; buf[4] = 'B'; buf[5] = 'P'; buf[6] = 'M'; buf[7] = 0;
                smk_ui_draw_list_row(y, "9. TEMPO:", buf, idx == (uint32_t)smk_m.menu_sel[0]);
                break;
            }
            }
        }
    } else {
        if (smk_m.menu_sel[0] == 0) {
            /* CC ASSIGN MODE */
            smk_ui_draw_menu_frame("[ CC ASSIGN ]", "[< >] SET CC  [OCT+] SAVE  [OCT-] CANCEL");
            if (!smk_m.cc_learn_seen) {
                smk_font_draw(16, 60, &FONT_B, 1, "Move any Knob, Fader or Pad", CLR_INK);
                smk_font_draw(16, 84, &FONT_S, 1, "to select it for CC editing...", CLR_DIM);
            } else {
                const char *tgt_type = smk_m.cc_learn_target == 0 ? "KNOB" :
                                       smk_m.cc_learn_target == 1 ? "FADER" : "PAD";
                smk_font_draw(16, 50, &FONT_S, 1, "SELECTED CONTROL:", CLR_DIM);

                buf[0] = '#';
                buf[1] = (char)('0' + (smk_m.cc_learn_idx + 1u) / 10u);
                buf[2] = (char)('0' + (smk_m.cc_learn_idx + 1u) % 10u);
                buf[3] = 0;
                smk_font_draw(16, 70, &FONT_M, 1, tgt_type, CLR_INK);
                smk_font_draw(110, 70, &FONT_M, 1, buf, CLR_INK);

                smk_font_draw(16, 110, &FONT_S, 1, "ASSIGNED CC:", CLR_DIM);
                buf[0] = (char)('0' + smk_m.cc_learn_val / 100u);
                buf[1] = (char)('0' + (smk_m.cc_learn_val % 100u) / 10u);
                buf[2] = (char)('0' + smk_m.cc_learn_val % 10u);
                buf[3] = 0;
                smk_font_draw(16, 130, &FONT_M, 1, buf, CLR_INK);

                smk_font_draw(16, 170, &FONT_S, 1, "Use < > to change, OCT+ to confirm", CLR_DIM);
            }
        } else if (smk_m.menu_sel[0] == 4) {
            /* KEY VELOCITY */
            smk_ui_draw_menu_frame("[ KEY VELOCITY CURVE ]", "[< >] CHOOSE  [OCT+] OK  [OCT-] CANCEL");
            uint32_t i;
            for (i = 0; i < 3; i++) {
                smk_ui_draw_list_row(34 + (int32_t)i * 24, VEL_CURVES[i],
                                     i == (uint32_t)smk_m.key_vel_curve ? "<ACTIVE>" : 0,
                                     i == (uint32_t)smk_m.menu_sel[1]);
            }
        } else if (smk_m.menu_sel[0] == 5) {
            /* PAD SENSITIVITY */
            smk_ui_draw_menu_frame("[ PAD SENSITIVITY ]", "[< >] CHOOSE  [OCT+] OK  [OCT-] CANCEL");
            uint32_t i;
            for (i = 0; i < 3; i++) {
                smk_ui_draw_list_row(34 + (int32_t)i * 24, SENS_NAMES[i],
                                     i == (uint32_t)smk_m.pad_sens ? "<ACTIVE>" : 0,
                                     i == (uint32_t)smk_m.menu_sel[1]);
            }
        } else if (smk_m.menu_sel[0] == 6) {
            /* KNOB SPEED */
            smk_ui_draw_menu_frame("[ KNOB SPEED ]", "[< >] CHOOSE  [OCT+] OK  [OCT-] CANCEL");
            uint32_t i;
            for (i = 0; i < 3; i++) {
                smk_ui_draw_list_row(34 + (int32_t)i * 24, SPEED_NAMES[i],
                                     i == (uint32_t)smk_m.knob_speed ? "<ACTIVE>" : 0,
                                     i == (uint32_t)smk_m.menu_sel[1]);
            }
        } else if (smk_m.menu_sel[0] == 7) {
            /* MIDI CHANNELS */
            smk_ui_draw_menu_frame("[ MIDI CHANNELS ]", "[< >] CH 01..16  [OCT+] NEXT  [OCT-] BACK");

            buf[0] = (char)('0' + smk_m.ch_keyboard / 10u);
            buf[1] = (char)('0' + smk_m.ch_keyboard % 10u);
            buf[2] = 0;
            smk_ui_draw_list_row(34, "1. KEYBOARD CH:", buf, smk_m.menu_sel[1] == 0);

            buf[0] = (char)('0' + smk_m.ch_knobs / 10u);
            buf[1] = (char)('0' + smk_m.ch_knobs % 10u);
            buf[2] = 0;
            smk_ui_draw_list_row(58, "2. KNOBS CH:", buf, smk_m.menu_sel[1] == 1);

            buf[0] = (char)('0' + smk_m.ch_faders / 10u);
            buf[1] = (char)('0' + smk_m.ch_faders % 10u);
            buf[2] = 0;
            smk_ui_draw_list_row(82, "3. FADERS CH:", buf, smk_m.menu_sel[1] == 2);

            buf[0] = (char)('0' + smk_m.ch_pads / 10u);
            buf[1] = (char)('0' + smk_m.ch_pads % 10u);
            buf[2] = 0;
            smk_ui_draw_list_row(106, "4. PADS CH:", buf, smk_m.menu_sel[1] == 3);
        }
    }
}

/* Master paint callback invoked by sg_step() */
void smk_midi_paint(void)
{
    switch (smk_m.page) {
    case SMK_PAGE_GEN:    smk_ui_draw_general(); break;
    case SMK_PAGE_SCALE:  smk_ui_draw_scale(); break;
    case SMK_PAGE_CHORD:  smk_ui_draw_chord(); break;
    case SMK_PAGE_ARP:    smk_ui_draw_arp(); break;
    case SMK_PAGE_NR:     smk_ui_draw_nr(); break;
    case SMK_PAGE_GLOBE:  smk_ui_draw_globe(); break;
    default:              smk_ui_draw_general(); break;
    }
}
