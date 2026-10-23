/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "../platform/pc/mono-present.h"

static int fails, checks;
#define CHECK(c) do { checks++; if (!(c)) { fails++; printf("FAIL line %d: %s\n", __LINE__, #c); } } while (0)

static union { max_align_t align; unsigned char bytes[1027*768*4+64]; cs_rect rects[4]; } tu;
#define tmem tu.bytes
static unsigned char tsave[sizeof tmem], exp_[sizeof tmem];
static _Alignas(16) unsigned char smem[21888+32], ssave[sizeof smem];
static union { max_align_t align; unsigned char bytes[4096+32]; cs_rect rects[4];
  struct { unsigned char pad[128]; cs_pc_mono_report report; } object; } ru;
#define rmem ru.bytes
static unsigned char rsave[sizeof rmem];
static union { cs_rect r[4]; cs_pc_mono_report rep; } ualias;

typedef struct { size_t n; cs_rect r[3]; } rset;
static const rset sets[10] = {
  {0, {{0,0,0,0}}},
  {1, {{0,0,512,342}}},
  {1, {{3,5,11,19}}},
  {2, {{0,0,1,1},{511,341,512,342}}},
  {3, {{10,10,20,20},{10,10,20,20},{15,15,25,25}}},
  {1, {{INT32_MIN,INT32_MIN,INT32_MAX,INT32_MAX}}},
  {2, {{-10,-10,8,8},{508,338,550,400}}},
  {1, {{0,0,0,0}}},
  {1, {{600,600,700,700}}},
  {3, {{0,70,512,224},{253,168,260,175},{273,178,280,185}}}
};
static const unsigned dims[3][2] = {{512,342},{640,480},{1024,768}};

static void fill(unsigned char *p, size_t n, unsigned seed) {
  for (size_t i = 0; i < n; i++) p[i] = (unsigned char)((i * 7 + seed * 53 + 13) & 255);
}
static int black(int x, int y) { return ((3*x + 5*y) % 16) < 7; }
static int64_t clip(int64_t v, int64_t hi) { return v < 0 ? 0 : v > hi ? hi : v; }

static void setup(unsigned W, unsigned H, unsigned fmt, cs_fb_target *t, cs_surface *c) {
  size_t pitch = W + 3;
  fill(tmem, sizeof tmem, 1); fill(rmem, sizeof rmem, 2); fill(smem, sizeof smem, 3);
  for (int y = 0; y < 342; y++)
    for (int x = 0; x < 512; x++) {
      unsigned char *b = &smem[16 + y*64 + x/8], m = (unsigned char)(0x80 >> (x % 8));
      if (black(x, y)) *b |= m; else *b &= (unsigned char)~m;
    }
  CHECK(cs_fb_init(t, tmem + 16, pitch*H*4, W, H, (uint32_t)pitch, fmt) == CS_FB_OK);
  CHECK(cs_surface_init(c, smem + 16, 21888, 512, 342, 64, CS_SURFACE_MONO1_MSB) == CS_SURFACE_OK);
}

static void run_case(unsigned W, unsigned H, unsigned fmt, const rset *s) {
  cs_fb_target t; cs_surface c; cs_pc_mono_report rep;
  int64_t cl[3][4]; uint32_t required = 0; size_t maxw = 0, pitch = W + 3;
  int ox = (int)(W - 512) / 2, oy = (int)(H - 342) / 2;
  setup(W, H, fmt, &t, &c);
  memcpy(tsave, tmem, sizeof tmem); memcpy(rsave, rmem, sizeof rmem); memcpy(ssave, smem, sizeof smem);
  memcpy(exp_, tmem, sizeof tmem);
  memset(&rep, 0xAA, sizeof rep);
  for (size_t i = 0; i < s->n; i++) {
    cl[i][0] = clip(s->r[i].left, 512);  cl[i][1] = clip(s->r[i].top, 342);
    cl[i][2] = clip(s->r[i].right, 512); cl[i][3] = clip(s->r[i].bottom, 342);
    if (cl[i][2] > cl[i][0] && cl[i][3] > cl[i][1]) {
      required += (uint32_t)(cl[i][3] - cl[i][1]);
      if ((size_t)(cl[i][2] - cl[i][0]) > maxw) maxw = (size_t)(cl[i][2] - cl[i][0]);
    }
  }
  for (unsigned py = 0; py < H; py++)
    for (unsigned px = 0; px < W; px++) {
      int lx = (int)px - ox, ly = (int)py - oy, in = 0;
      for (size_t i = 0; i < s->n; i++)
        if (lx >= cl[i][0] && lx < cl[i][2] && ly >= cl[i][1] && ly < cl[i][3]) in = 1;
      if (in) {
        unsigned char v = black(lx, ly) ? 0 : 255, *p = exp_ + 16 + (py*pitch + px)*4;
        p[0] = p[1] = p[2] = v; p[3] = 0;
      }
    }
  int rc = cs_pc_mono_present_rects(&t, &c, rmem + 16, 4*W + 8, s->n ? s->r : NULL, s->n, &rep);
  CHECK(rc == CS_MONO_OK);
  CHECK(rep.phase == CS_MONO_COMPLETE);
  CHECK(rep.rows == required && rep.required == required);
  CHECK(rep.cause == 0);
  CHECK(memcmp(tmem, exp_, sizeof tmem) == 0);
  CHECK(memcmp(smem, ssave, sizeof smem) == 0);
  CHECK(memcmp(rmem, rsave, 16) == 0);
  CHECK(memcmp(rmem + 16 + maxw*4, rsave + 16 + maxw*4, sizeof rmem - 16 - maxw*4) == 0);
}

static cs_rect good[4] = {{0,0,5,5},{1,1,6,6},{2,2,7,7},{3,3,8,8}};

static void refuse(int kind, int expect) {
  cs_fb_target t; cs_surface c; cs_pc_mono_report rep, *out = &rep;
  cs_rect local[4]; const cs_rect *rects = local; size_t count = 2, cap = 4*640 + 8;
  unsigned char *row = rmem + 16;
  unsigned char tcopy[sizeof t], ccopy[sizeof c], rcopy[sizeof rep], rectcopy[sizeof local];
  setup(640, 480, CS_FB_RGB_RESERVED, &t, &c);
  memcpy(local, good, sizeof local);
  memset(&rep, 0xAA, sizeof rep);
  switch (kind) {
  case 0: local[1].left = 9; local[1].right = 3; break;
  case 1: count = 4; break;
  case 2: rects = NULL; count = 1; break;
  case 3: c.format = CS_SURFACE_RGBA8; break;
  case 4: t.width = 500; break;
  case 5: cap = 4*640 - 1; break;
  case 6: rects = tu.rects; memcpy((void *)rects, good, sizeof good); break;
  case 7: rects = ru.rects; memcpy((void *)rects, good, sizeof good); break;
  case 8: memcpy(ualias.r, good, sizeof good); rects = ualias.r; out = &ualias.rep; break;
  case 9: out = &ru.object.report; memcpy(out, &rep, sizeof rep); break;
  case 10: row = (unsigned char *)(tmem + 64); break;
  }
  memcpy(tsave, tmem, sizeof tmem); memcpy(rsave, rmem, sizeof rmem); memcpy(ssave, smem, sizeof smem);
  memcpy(tcopy, &t, sizeof t); memcpy(ccopy, &c, sizeof c); memcpy(rcopy, out, sizeof rep);
  if (rects) memcpy(rectcopy, rects, count > 4 ? sizeof rectcopy : count * sizeof(cs_rect));
  int rc = cs_pc_mono_present_rects(&t, &c, row, cap, rects, count, out);
  CHECK(rc == expect);
  CHECK(memcmp(tmem, tsave, sizeof tmem) == 0);
  CHECK(memcmp(rmem, rsave, sizeof rmem) == 0);
  CHECK(memcmp(smem, ssave, sizeof smem) == 0);
  CHECK(memcmp(&t, tcopy, sizeof t) == 0 && memcmp(&c, ccopy, sizeof c) == 0);
  CHECK(memcmp(out, rcopy, sizeof rep) == 0);
  if (rects) CHECK(memcmp(rects, rectcopy, count > 4 ? sizeof rectcopy : count * sizeof(cs_rect)) == 0);
}

int main(void) {
  int cases = 0;
  for (int d = 0; d < 3; d++)
    for (unsigned f = 0; f < 2; f++)
      for (int s = 0; s < 10; s++) {
        run_case(dims[d][0], dims[d][1], f, &sets[s]);
        cases++;
      }
  refuse(0, CS_MONO_ARGUMENT);
  refuse(1, CS_MONO_ARGUMENT);
  refuse(2, CS_MONO_ARGUMENT);
  refuse(3, CS_MONO_SOURCE);
  refuse(4, CS_MONO_TARGET);
  refuse(5, CS_MONO_CAPACITY);
  refuse(6, CS_MONO_ALIAS);
  refuse(7, CS_MONO_ALIAS);
  refuse(8, CS_MONO_ALIAS);
  refuse(9, CS_MONO_ALIAS);
  refuse(10, CS_MONO_ALIAS);
  printf("mono-rects: %d positive cases, 11 refusal cases, %d checks, %d failures\n", cases, checks, fails);
  return fails ? 1 : 0;
}
