/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Literal oracle for the proposed checked access contract. Every expected
 * result, value and changed byte below was written by hand from the frozen
 * order in pc286-span-access-contract.md, before running the implementation.
 * Author-written: this is not an independent reviewer's oracle. */
#include <stdio.h>
#include <string.h>
#include "../probes/pc286/span-access.h"
#include "../probes/pc286/span-build.h"

static int failures;
#define CHECK(c) do { if (!(c)) { failures++; printf("FAIL case %u line %d: %s\n", ci, __LINE__, #c); } } while (0)

/* A 0x1000 RW: bank 9 (3) + bank 4 (5) | B 0x1008 RO: bank 2 (4) adjacent |
 * C 0xFFFFFC RWX: bank 7 (4) | D 0x20000 RW: bank 5 (32770 = 32767 + 3). */
static const cs_bm_bank MB[5] = {{9, 3}, {4, 5}, {2, 4}, {7, 4}, {5, 32770}};
static const cs_bm_region MR[4] = {{0x1000, 8, 3, 0, 2}, {0x1008, 4, 1, 2, 1},
                                   {0xFFFFFC, 4, 7, 3, 1}, {0x20000, 32770, 3, 4, 1}};
static const cs_bm_map MAP = {MR, MB, 4, 5};
static const cs_bm_map NOREGION = {MR, MB, 0, 5};
static const cs_bc_bank CAT[5] = {{9, 0x2000, 0}, {4, 0x3000, 0}, {2, 0x4000, 0},
                                  {7, 0x5000, 0}, {5, 0x10000, 0}};
static const cs_bc_bank DUP[5] = {{9, 0x2000, 0}, {9, 0x3000, 0}, {2, 0x4000, 0},
                                  {7, 0x5000, 0}, {5, 0x10000, 0}};
static uint8_t arena[CS_BK_REAL_TOP], snap[CS_BK_REAL_TOP];
static cs_bk_chunk ch[8];
static cs_bs_span sp[8], spw[8];
static unsigned ci;
static uint8_t sep[3];

enum { RD, WR };
enum { E_OK, E_NULL, E_NOSPANS, E_MANYSPANS, E_EPOCH0, E_STALE, E_DUPCAT, E_NOREGION,
       E_LATENULL, E_MISSING, E_SWAP, E_MERGED, E_WRONGID, E_NULLOUT, E_SPLIT32768,
       E_EXTRA, E_SEPARATE };

typedef struct { uint32_t addr; uint8_t byte; } poke;
typedef struct {
    int op, env; uint32_t at; unsigned w; uint32_t acc, val; int want; uint32_t got;
    poke after[4];
} kase;

static const kase K[] = {
    /* reads that succeed, including bank and chunk crossings */
    {RD, E_OK, 0x1000, 4, 1, 0, CS_BX_OK, 0x11223344UL, {{0}}},
    {RD, E_OK, 0x1002, 2, 1, 0, CS_BX_OK, 0x3344UL, {{0}}},
    {RD, E_OK, 0x1004, 4, 3, 0, CS_BX_OK, 0x55667788UL, {{0}}},
    {RD, E_OK, 0x1001, 1, 1, 0, CS_BX_OK, 0x22UL, {{0}}},
    {RD, E_OK, 0x1008, 4, 1, 0, CS_BX_OK, 0xA1A2A3A4UL, {{0}}},
    {RD, E_OK, 0xFFFFFC, 4, 7, 0, CS_BX_OK, 0xB1B2B3B4UL, {{0}}},
    {RD, E_OK, 0x27FFE, 4, 3, 0, CS_BX_OK, 0xC2C3C4C5UL, {{0}}},
    /* width, alignment, window, permission, argument ladder */
    {RD, E_OK, 0x1008, 2, 5, 0, CS_BX_PERMISSION, 0, {{0}}},
    {RD, E_OK, 0x1001, 2, 1, 0, CS_BX_ADDRESS, 0, {{0}}},
    {RD, E_OK, 0x1001, 3, 1, 0, CS_BX_WIDTH, 0, {{0}}},
    {RD, E_OK, 0x1000, 0, 1, 0, CS_BX_WIDTH, 0, {{0}}},
    {RD, E_OK, 0x1000, 5, 1, 0, CS_BX_WIDTH, 0, {{0}}},
    {RD, E_OK, 0x1006, 4, 1, 0, CS_BX_ADDRESS, 0, {{0}}},   /* A into adjacent B */
    {RD, E_OK, 0xFFFFFE, 4, 1, 0, CS_BX_ADDRESS, 0, {{0}}}, /* past 2^24 */
    {RD, E_OK, 0x1000000, 1, 1, 0, CS_BX_ADDRESS, 0, {{0}}},
    {RD, E_OK, 0x0, 1, 1, 0, CS_BX_ADDRESS, 0, {{0}}},      /* unmapped */
    {RD, E_OK, 0x1001, 2, 9, 0, CS_BX_ADDRESS, 0, {{0}}},   /* alignment before bits */
    {RD, E_OK, 0x1000, 1, 9, 0, CS_BX_ARGUMENT, 0, {{0}}},  /* bits outside 7 */
    {RD, E_OK, 0x1000, 1, 2, 0, CS_BX_ARGUMENT, 0, {{0}}},  /* no READ */
    {RD, E_NULLOUT, 0x1000, 1, 1, 0, CS_BX_ARGUMENT, 0, {{0}}},
    /* writes: exact bytes, chunk split, permission, alignment, width */
    {WR, E_OK, 0x1002, 4, 0, 0xDEADBEEFUL, CS_BX_OK, 0,
     {{0x2002, 0xDE}, {0x3000, 0xAD}, {0x3001, 0xBE}, {0x3002, 0xEF}}},
    {WR, E_OK, 0x27FFE, 2, 0, 0x0102UL, CS_BX_OK, 0, {{0x17FFE, 0x01}, {0x17FFF, 0x02}}},
    {WR, E_OK, 0xFFFFFF, 1, 0, 0x5AUL, CS_BX_OK, 0, {{0x5003, 0x5A}}},
    {WR, E_OK, 0x1008, 1, 0, 0x00UL, CS_BX_PERMISSION, 0, {{0}}},
    {WR, E_OK, 0x1003, 2, 0, 0x00UL, CS_BX_ADDRESS, 0, {{0}}},
    {WR, E_OK, 0x1000, 3, 0, 0x00UL, CS_BX_WIDTH, 0, {{0}}},
    /* environment and late-invalid binding; precedence pairs */
    {RD, E_NULL, 0x1000, 1, 1, 0, CS_BX_ARGUMENT, 0, {{0}}},
    {WR, E_NOSPANS, 0x1000, 1, 0, 0, CS_BX_ARGUMENT, 0, {{0}}},
    {RD, E_MANYSPANS, 0x1000, 1, 1, 0, CS_BX_ARGUMENT, 0, {{0}}},
    {WR, E_EPOCH0, 0x1000, 1, 0, 0, CS_BX_ARGUMENT, 0, {{0}}},
    {RD, E_STALE, 0x1001, 3, 1, 0, CS_BX_STALE, 0, {{0}}},     /* STALE before WIDTH */
    {WR, E_DUPCAT, 0x1001, 2, 0, 0, CS_BX_BACKING, 0, {{0}}},  /* BACKING before ADDRESS */
    {RD, E_NOREGION, 0x1000, 3, 1, 0, CS_BX_MAP, 0, {{0}}},    /* MAP before WIDTH */
    {RD, E_NOREGION, 0x1000, 1, 2, 0, CS_BX_ARGUMENT, 0, {{0}}},/* no READ before MAP */
    {WR, E_LATENULL, 0x1000, 4, 0, 1, CS_BX_BINDING, 0, {{0}}},/* last span bad */
    {RD, E_LATENULL, 0x1000, 1, 2, 0, CS_BX_ARGUMENT, 0, {{0}}},/* ARGUMENT before BINDING */
    {RD, E_MISSING, 0x1000, 1, 1, 0, CS_BX_BINDING, 0, {{0}}},
    {WR, E_SWAP, 0x1000, 1, 0, 1, CS_BX_BINDING, 0, {{0}}},
    {RD, E_MERGED, 0x1000, 1, 1, 0, CS_BX_BINDING, 0, {{0}}},
    {WR, E_WRONGID, 0x1000, 1, 0, 1, CS_BX_BINDING, 0, {{0}}},
    {WR, E_LATENULL, 0x1008, 1, 0, 1, CS_BX_PERMISSION, 0, {{0}}}, /* window before BINDING */
    {RD, E_LATENULL, 0x1008, 2, 5, 0, CS_BX_PERMISSION, 0, {{0}}}, /* read: same order */
    {RD, E_LATENULL, 0x1001, 2, 1, 0, CS_BX_ADDRESS, 0, {{0}}},
    {RD, E_SPLIT32768, 0x1000, 1, 1, 0, CS_BX_BINDING, 0, {{0}}}, /* 32768 + 2 split */
    {RD, E_EXTRA, 0x1000, 1, 1, 0, CS_BX_BINDING, 0, {{0}}},      /* trailing extra span */
    /* bank 5's second chunk is a separate object: bytes come from it */
    {RD, E_SEPARATE, 0x27FFE, 4, 1, 0, CS_BX_OK, 0xC2D1D2D3UL, {{0}}},
};

int main(void)
{
    cs_bx_env base, e;
    uint16_t n = 0;
    static const uint8_t setup[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
                                    0xA1, 0xA2, 0xA3, 0xA4, 0xB1, 0xB2, 0xB3, 0xB4,
                                    0xC1, 0xC2, 0xC3, 0xC4, 0xC5};
    static const uint32_t where[] = {0x2000, 0x2001, 0x2002, 0x3000, 0x3001, 0x3002, 0x3003,
                                     0x3004, 0x4000, 0x4001, 0x4002, 0x4003, 0x5000, 0x5001,
                                     0x5002, 0x5003, 0x17FFD, 0x17FFE, 0x17FFF, 0x18000, 0x18001};
    unsigned s;

    if (cs_bk_plan(&MAP, CAT, 5, 0x1000, 0x2F000, ch, 8, &n) != CS_BK_OK || n != 6u ||
        cs_bk_bind(ch, n, arena, sp, 8) != CS_BK_OK) {
        printf("FAIL fixture\n");
        return 1;
    }
    base.map = &MAP; base.catalog = CAT; base.catalog_count = 5;
    base.ram_first = 0x1000; base.ram_bytes = 0x2F000;
    base.spans = sp; base.span_count = n; base.current_epoch = base.expected_epoch = 7;

    for (ci = 0; ci < sizeof K / sizeof K[0]; ++ci) {
        const kase *k = &K[ci];
        uint32_t out = 0x13579BDFUL;
        int got;
        const cs_bx_env *pe = &e;
        unsigned p;

        memset(arena, 0, sizeof arena);
        for (s = 0; s < sizeof setup; ++s) arena[where[s]] = setup[s];
        e = base;
        memcpy(spw, sp, sizeof sp);
        e.spans = spw;
        switch (k->env) {
        case E_NULL: pe = NULL; break;
        case E_NOSPANS: e.span_count = 0; break;
        case E_MANYSPANS: e.span_count = 193; break;
        case E_EPOCH0: e.current_epoch = 0; break;
        case E_STALE: e.expected_epoch = 6; break;
        case E_DUPCAT: e.catalog = DUP; break;
        case E_NOREGION: e.map = &NOREGION; break;
        case E_LATENULL: spw[5].data = NULL; break;
        case E_MISSING: e.span_count = 5; break;
        case E_SWAP: spw[0] = sp[1]; spw[1] = sp[0]; break;
        case E_MERGED: spw[4].bytes = 32770; spw[5] = spw[4]; e.span_count = 5; break;
        case E_WRONGID: spw[2].bank = 3; break;
        case E_SPLIT32768: spw[4].bytes = 32768; spw[5].offset = 32768; spw[5].bytes = 2; break;
        case E_EXTRA: spw[6] = sp[5]; e.span_count = 7; break;
        case E_SEPARATE: sep[0] = 0xD1; sep[1] = 0xD2; sep[2] = 0xD3; spw[5].data = sep; break;
        default: break;
        }
        memcpy(snap, arena, sizeof arena);
        if (k->op == RD)
            got = (int)cs_bx_read(pe, k->at, k->w, k->acc, k->env == E_NULLOUT ? NULL : &out);
        else
            got = (int)cs_bx_write(pe, k->at, k->w, k->val);
        CHECK(got == k->want);
        if (k->op == RD) CHECK(out == (got == CS_BX_OK ? k->got : 0x13579BDFUL));
        for (p = 0; p < 4u && k->after[p].addr != 0UL; ++p) {
            CHECK(arena[k->after[p].addr] == k->after[p].byte);
            snap[k->after[p].addr] = k->after[p].byte;
        }
        CHECK(memcmp(snap, arena, sizeof arena) == 0); /* nothing else moved */
    }
    if (failures) {
        printf("%d failure(s)\n", failures);
        return 1;
    }
    printf("PASS pc286 span-access oracle: %u literal cases\n", ci);
    return 0;
}
