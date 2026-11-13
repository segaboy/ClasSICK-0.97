/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../core/input/m0110-reply.h"
#include <stdio.h>

static int check(unsigned line, int condition)
{
    if (!condition) fprintf(stderr, "FAIL line %u\n", line);
    return condition;
}
#define CHECK(x) do { if (!check(__LINE__, (x))) return 1; } while (0)

int main(void)
{
    uint32_t b, events = 0, empty = 0, invalid = 0;
    cs_m0110_event e;
    static const uint32_t observed[4] = { 0x01, 0x81, 0x03, 0x83 };
    static const uint32_t positions[4] = { 0, 0, 1, 1 };
    static const uint32_t presses[4] = { 1, 0, 1, 0 };
    for (b = 0; b < 4; ++b) {
        CHECK(cs_m0110_reply(observed[b], &e) == CS_M0110_EVENT);
        CHECK(e.position == positions[b] && e.pressed == presses[b]);
    }
    for (b = 0; b < 256; ++b) {
        int result;
        uint32_t position = (b % 128u) / 2u;
        e.position = 0xabcdef01u; e.pressed = 0x76543210u;
        result = cs_m0110_reply(b, &e);
        if (b == 0x7bu) {
            ++empty; CHECK(result == CS_M0110_EMPTY);
        } else if ((b % 2u) == 1u && position <= 0x39u) {
            ++events; CHECK(result == CS_M0110_EVENT);
            CHECK(e.position == position && e.pressed == (b < 128u ? 1u : 0u));
        } else {
            ++invalid; CHECK(result == CS_M0110_INVALID);
        }
        if (result != CS_M0110_EVENT)
            CHECK(e.position == 0xabcdef01u && e.pressed == 0x76543210u);
    }
    CHECK(events == 116u && empty == 1u && invalid == 139u);
    e.position = 23; e.pressed = 47;
    CHECK(cs_m0110_reply(256, &e) == CS_M0110_ARGUMENT);
    CHECK(e.position == 23 && e.pressed == 47);
    CHECK(cs_m0110_reply(0xffffffffu, &e) == CS_M0110_ARGUMENT);
    CHECK(e.position == 23 && e.pressed == 47);
    CHECK(cs_m0110_reply(1, 0) == CS_M0110_ARGUMENT);
    puts("PASS: all 256 reply bytes, four observed transitions and argument preservation");
    return 0;
}
