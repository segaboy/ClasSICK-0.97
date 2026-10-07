/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* TEST-0004: independent SPEC-0001 byte vectors and whole-storage oracle. */
#include "surface.h"
#include <stdio.h>
#include <string.h>

#define CHECK(expression) do { \
    if (!(expression)) { \
        fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expression); \
        return 1; \
    } \
} while (0)

static int layout(void)
{
    cs_surface s;
    uint8_t mono[] = {0xcd, 0x55, 0x2a, 0xe1, 0xaa, 0x6d, 0xe2, 0xd7, 0xce};
    const uint8_t filled[] = {0xcd, 0x7f, 0xaa, 0xe1, 0xaa, 0x6d, 0xe2, 0xd7, 0xce};
    const uint8_t cleared[] = {0xcd, 0x00, 0x2a, 0xe1, 0x00, 0x6d, 0xe2, 0xd7, 0xce};
    uint8_t bits[] = {0xe3, 0x00, 0x00, 0x7f, 0xe4};
    const uint8_t bit_vector[] = {0xe3, 0x80, 0x81, 0xff, 0xe4};
    uint8_t rgba[29];
    uint8_t expected[29];
    const uint8_t two_pixels[] = {0x12, 0x34, 0x56, 0x78, 0x12, 0x34, 0x56, 0x78};
    const uint8_t row[] = {0xab, 0xcd, 0xef, 0x00, 0xab, 0xcd, 0xef, 0x00,
                          0xab, 0xcd, 0xef, 0x00};
    CHECK(cs_surface_init(&s, mono + 1, 7, 9, 2, 3, CS_SURFACE_MONO1_MSB) == CS_SURFACE_OK);
    CHECK(mono[1] == 0x55 && mono[2] == 0x2a);
    CHECK(cs_surface_fill(&s, (cs_rect){1, 0, 9, 1}, 1) == CS_SURFACE_OK);
    CHECK(memcmp(mono, filled, sizeof mono) == 0);
    CHECK(cs_surface_clear(&s, 0) == CS_SURFACE_OK);
    CHECK(memcmp(mono, cleared, sizeof mono) == 0);

    CHECK(cs_surface_init(&s, bits + 1, 3, 17, 1, 3, CS_SURFACE_MONO1_MSB) == CS_SURFACE_OK);
    CHECK(cs_surface_fill(&s, (cs_rect){0, 0, 1, 1}, 1) == CS_SURFACE_OK);
    CHECK(cs_surface_fill(&s, (cs_rect){8, 0, 9, 1}, 1) == CS_SURFACE_OK);
    CHECK(cs_surface_fill(&s, (cs_rect){15, 0, 17, 1}, 1) == CS_SURFACE_OK);
    CHECK(memcmp(bits, bit_vector, sizeof bits) == 0);

    memset(rgba, 0x22, sizeof rgba);
    memset(expected, 0x22, sizeof expected);
    CHECK(cs_surface_init(&s, rgba + 1, 27, 3, 2, 13, CS_SURFACE_RGBA8) == CS_SURFACE_OK);
    CHECK(memcmp(rgba, expected, sizeof rgba) == 0);
    memcpy(expected + 1, two_pixels, sizeof two_pixels);
    CHECK(cs_surface_fill(&s, (cs_rect){-1, 0, 2, 1}, UINT32_C(0x12345678)) == CS_SURFACE_OK);
    CHECK(memcmp(rgba, expected, sizeof rgba) == 0);
    memcpy(expected + 1, row, sizeof row);
    memcpy(expected + 14, row, sizeof row);
    CHECK(cs_surface_clear(&s, UINT32_C(0xabcdef00)) == CS_SURFACE_OK);
    CHECK(memcmp(rgba, expected, sizeof rgba) == 0);
    return 0;
}

static int errors(void)
{
    uint8_t buffer[32];
    uint8_t before[32];
    cs_surface s;
    cs_surface saved;
    cs_surface bad;
    const cs_rect empty = {0, 0, 0, 0};
    const cs_rect reversed = {INT32_MAX, 0, INT32_MIN, 1};
    struct construction_case {
        int32_t width;
        int32_t height;
        size_t stride;
        size_t size;
        cs_surface_format format;
        cs_surface_result result;
    };
    const struct construction_case cases[] = {
        {1, 1, 1, 1, (cs_surface_format)0, CS_SURFACE_ERR_FORMAT},
        {0, 1, 1, 1, CS_SURFACE_MONO1_MSB, CS_SURFACE_ERR_DIMENSION},
        {1, 0, 1, 1, CS_SURFACE_MONO1_MSB, CS_SURFACE_ERR_DIMENSION},
        {INT32_MIN, 1, 1, 1, CS_SURFACE_MONO1_MSB, CS_SURFACE_ERR_DIMENSION},
        {1, -1, 1, 1, CS_SURFACE_RGBA8, CS_SURFACE_ERR_DIMENSION},
        {9, 1, 1, 1, CS_SURFACE_MONO1_MSB, CS_SURFACE_ERR_STRIDE},
        {1, 1, 3, 4, CS_SURFACE_RGBA8, CS_SURFACE_ERR_STRIDE},
        {1, 1, 0, 0, CS_SURFACE_MONO1_MSB, CS_SURFACE_ERR_STRIDE},
        {1, 2, 4, 7, CS_SURFACE_MONO1_MSB, CS_SURFACE_ERR_STORAGE},
        {1, 1, 1, 0, CS_SURFACE_MONO1_MSB, CS_SURFACE_ERR_STORAGE},
        {1, 2, SIZE_MAX / 2u + 1u, SIZE_MAX, CS_SURFACE_MONO1_MSB, CS_SURFACE_ERR_OVERFLOW},
        {1, 1, (size_t)PTRDIFF_MAX + 1u, SIZE_MAX, CS_SURFACE_MONO1_MSB, CS_SURFACE_ERR_OVERFLOW},
        {1, INT32_MAX, SIZE_MAX, SIZE_MAX, CS_SURFACE_RGBA8, CS_SURFACE_ERR_OVERFLOW},
        /* Precedence: format before dimensions/stride; dimensions before stride. */
        {0, 0, 0, 0, (cs_surface_format)99, CS_SURFACE_ERR_FORMAT},
        {0, 0, 0, 0, CS_SURFACE_RGBA8, CS_SURFACE_ERR_DIMENSION}
    };
    size_t i;
    CHECK(CS_SURFACE_OK == 0 && CS_SURFACE_ERR_ARGUMENT == 1 && CS_SURFACE_ERR_FORMAT == 2);
    CHECK(CS_SURFACE_ERR_DIMENSION == 3 && CS_SURFACE_ERR_STRIDE == 4);
    CHECK(CS_SURFACE_ERR_OVERFLOW == 5 && CS_SURFACE_ERR_STORAGE == 6);
    CHECK(CS_SURFACE_ERR_RECT == 7 && CS_SURFACE_ERR_COLOR == 8);
    memset(buffer, 0x97, sizeof buffer);
    memcpy(before, buffer, sizeof before);
    memset(&s, 0xa5, sizeof s);
    CHECK(cs_surface_init(&s, buffer, sizeof buffer, 9, 2, 4, CS_SURFACE_MONO1_MSB) == CS_SURFACE_OK);
    memcpy(&saved, &s, sizeof saved);
    CHECK(cs_surface_init(NULL, buffer, 1, 1, 1, 1, CS_SURFACE_MONO1_MSB) == CS_SURFACE_ERR_ARGUMENT);
    CHECK(cs_surface_init(&s, NULL, 0, 0, 0, 0, (cs_surface_format)0) == CS_SURFACE_ERR_ARGUMENT);
    CHECK(memcmp(&s, &saved, sizeof s) == 0);
    for (i = 0; i < sizeof cases / sizeof cases[0]; ++i) {
        CHECK(cs_surface_init(&s, buffer, cases[i].size, cases[i].width,
            cases[i].height, cases[i].stride, cases[i].format) == cases[i].result);
        CHECK(memcmp(&s, &saved, sizeof s) == 0);
        CHECK(memcmp(buffer, before, sizeof buffer) == 0);
    }
    CHECK(cs_surface_init(&s, buffer, 0, INT32_MAX, 1, 0, CS_SURFACE_RGBA8) ==
        ((uintmax_t)INT32_MAX > SIZE_MAX / 4u ? CS_SURFACE_ERR_OVERFLOW : CS_SURFACE_ERR_STRIDE));
    CHECK(memcmp(&s, &saved, sizeof s) == 0);
    CHECK(cs_surface_fill(NULL, empty, 0) == CS_SURFACE_ERR_ARGUMENT);
    CHECK(cs_surface_clear(NULL, 0) == CS_SURFACE_ERR_ARGUMENT);
    CHECK(cs_surface_fill(&s, reversed, 99) == CS_SURFACE_ERR_RECT);
    CHECK(cs_surface_fill(&s, (cs_rect){0, 2, 1, 1}, 1) == CS_SURFACE_ERR_RECT);
    CHECK(cs_surface_fill(&s, empty, 2) == CS_SURFACE_ERR_COLOR);
    CHECK(cs_surface_fill(&s, (cs_rect){-2, -2, -1, -1}, UINT32_MAX) == CS_SURFACE_ERR_COLOR);
    CHECK(cs_surface_clear(&s, 2) == CS_SURFACE_ERR_COLOR);
    CHECK(memcmp(buffer, before, sizeof buffer) == 0);
    CHECK(memcmp(&s, &saved, sizeof s) == 0);

    bad = s;
    bad.storage = NULL;
    CHECK(cs_surface_fill(&bad, reversed, 9) == CS_SURFACE_ERR_ARGUMENT);
    bad = s;
    bad.format = (cs_surface_format)99;
    CHECK(cs_surface_clear(&bad, 0) == CS_SURFACE_ERR_FORMAT);
    bad = s;
    bad.width = -1;
    CHECK(cs_surface_clear(&bad, 0) == CS_SURFACE_ERR_DIMENSION);
    bad = s;
    bad.stride = 1;
    CHECK(cs_surface_fill(&bad, empty, 0) == CS_SURFACE_ERR_STRIDE);
    bad = s;
    bad.stride = SIZE_MAX;
    CHECK(cs_surface_clear(&bad, 0) == CS_SURFACE_ERR_OVERFLOW);
    bad = s;
    bad.storage_size = 7;
    CHECK(cs_surface_fill(&bad, reversed, 2) == CS_SURFACE_ERR_STORAGE);
    CHECK(memcmp(buffer, before, sizeof buffer) == 0);
    /* Exact threshold: all full rows required even if final padding is unused. */
    CHECK(cs_surface_init(&s, buffer + 1, 8, 9, 2, 4, CS_SURFACE_MONO1_MSB) == CS_SURFACE_OK);
    CHECK(s.storage == buffer + 1 && s.storage_size == 8 && s.stride == 4);
    CHECK(s.width == 9 && s.height == 2 && s.format == CS_SURFACE_MONO1_MSB);
    CHECK(cs_surface_clear(&s, 1) == CS_SURFACE_OK);
    CHECK(buffer[0] == 0x97 && buffer[9] == 0x97 && buffer[3] == 0x97 && buffer[4] == 0x97);
    return 0;
}

/* Oracle visits every stored bit/channel, selecting coordinates by comparisons.
 * It neither clips the rectangle nor uses the implementation's pixel loops. */
static void oracle(uint8_t *expected, size_t stride, int32_t width, int32_t height,
    cs_surface_format format, cs_rect rect, uint32_t value)
{
    size_t byte;
    for (byte = 0; byte < stride * (size_t)height; ++byte) {
        int32_t y = (int32_t)(byte / stride);
        size_t column = byte % stride;
        if (format == CS_SURFACE_MONO1_MSB) {
            unsigned bit;
            for (bit = 0; bit < 8u; ++bit) {
                int32_t x = (int32_t)(column * 8u + bit);
                if (x < width && x >= rect.left && x < rect.right &&
                    y >= rect.top && y < rect.bottom) {
                    unsigned mask = 1u << (7u - bit);
                    unsigned old = expected[byte];
                    expected[byte] = (uint8_t)(value == 0u ? old & ~mask : old | mask);
                }
            }
        } else {
            int32_t x = (int32_t)(column / 4u);
            if (x < width && x >= rect.left && x < rect.right &&
                y >= rect.top && y < rect.bottom) {
                const uint32_t divisors[] = {UINT32_C(16777216), UINT32_C(65536),
                                             UINT32_C(256), UINT32_C(1)};
                expected[byte] = (uint8_t)((value / divisors[column % 4u]) % 256u);
            }
        }
    }
}

static int matrix(void)
{
    const int32_t widths[] = {1, 7, 8, 9, 15, 16, 17, 31, 65};
    const int32_t heights[] = {1, 3, 7};
    const cs_rect rectangles[] = {
        {INT32_MIN, INT32_MIN, INT32_MAX, INT32_MAX},
        {-2, -1, 3, 2}, {0, 0, 1, 1}, {1, 1, 16, 4}, {7, 0, 9, 3},
        {0, 0, 0, 7}, {1, 1, 8, 1}, {-7, -7, -1, -1},
        {INT32_MIN, 0, -1, INT32_MAX}, {0, INT32_MIN, INT32_MAX, -1},
        {INT32_MAX, 0, INT32_MAX, 1}, {0, INT32_MAX, 1, INT32_MAX},
        {INT32_MIN, INT32_MIN, INT32_MIN, INT32_MIN},
        {1, 0, INT32_MAX, 2}, {0, 1, 2, INT32_MAX},
        {INT32_MAX, INT32_MIN, INT32_MIN, INT32_MAX}, {0, 4, 2, 3}
    };
    uint8_t actual[2048];
    uint8_t expected[2048];
    cs_surface s;
    size_t w;
    size_t h;
    size_t pad;
    unsigned format;
    size_t count = 0;
    for (format = 1; format <= 2; ++format) {
        for (w = 0; w < sizeof widths / sizeof widths[0]; ++w) {
            for (h = 0; h < sizeof heights / sizeof heights[0]; ++h) {
                for (pad = 0; pad <= 3; ++pad) {
                    size_t row_bytes = format == 1 ? ((size_t)widths[w] + 7u) / 8u : (size_t)widths[w] * 4u;
                    size_t stride = row_bytes + pad;
                    size_t span = stride * (size_t)heights[h];
                    size_t r;
                    CHECK(span + 3u <= sizeof actual);
                    for (r = 0; r < sizeof rectangles / sizeof rectangles[0] + 2u; ++r) {
                        cs_rect rect;
                        unsigned v;
                        if (r < sizeof rectangles / sizeof rectangles[0]) {
                            rect = rectangles[r];
                        } else if (r == sizeof rectangles / sizeof rectangles[0]) {
                            rect = (cs_rect){widths[w] - 1, heights[h] - 1, widths[w], heights[h]};
                        } else {
                            rect = (cs_rect){widths[w], heights[h], INT32_MAX, INT32_MAX};
                        }
                        for (v = 0; v < 2; ++v) {
                            uint32_t value = format == 1 ? v : (v == 0 ? UINT32_C(0x01234567) : UINT32_C(0xfedcba00));
                            size_t b;
                            for (b = 0; b < sizeof actual; ++b) {
                                actual[b] = (uint8_t)((b * 37u + r * 11u) % 256u);
                            }
                            memcpy(expected, actual, sizeof expected);
                            CHECK(cs_surface_init(&s, actual + 1, span + 1u, widths[w], heights[h],
                                stride, (cs_surface_format)format) == CS_SURFACE_OK);
                            if (rect.left > rect.right || rect.top > rect.bottom) {
                                CHECK(cs_surface_fill(&s, rect, value) == CS_SURFACE_ERR_RECT);
                            } else {
                                oracle(expected + 1, stride, widths[w], heights[h],
                                    (cs_surface_format)format, rect, value);
                                CHECK(cs_surface_fill(&s, rect, value) == CS_SURFACE_OK);
                            }
                            CHECK(memcmp(actual, expected, sizeof actual) == 0);
                            ++count;
                        }
                    }
                    /* Clear also covers minimal-length buffers and every padding bit. */
                    memset(actual, 0x5a, sizeof actual);
                    memcpy(expected, actual, sizeof expected);
                    CHECK(cs_surface_init(&s, actual + 1, span, widths[w], heights[h],
                        stride, (cs_surface_format)format) == CS_SURFACE_OK);
                    oracle(expected + 1, stride, widths[w], heights[h], (cs_surface_format)format,
                        (cs_rect){0, 0, widths[w], heights[h]}, 0);
                    CHECK(cs_surface_clear(&s, 0) == CS_SURFACE_OK);
                    CHECK(memcmp(actual, expected, sizeof actual) == 0);
                    ++count;
                }
            }
        }
    }
    printf("TEST-0004 matrix: %zu whole-buffer cases\n", count);
    return 0;
}

static int independent(void)
{
    uint8_t a[] = {0x3c, 0x00, 0x55, 0xc3};
    uint8_t b[] = {0x3c, 0x00, 0x55, 0xc3};
    uint8_t c[10];
    cs_surface first;
    cs_surface second;
    cs_surface color;
    cs_surface first_before;
    memset(c, 0xa5, sizeof c);
    CHECK(cs_surface_init(&first, a + 1, 2, 8, 2, 1, CS_SURFACE_MONO1_MSB) == CS_SURFACE_OK);
    CHECK(cs_surface_init(&second, b + 1, 2, 8, 2, 1, CS_SURFACE_MONO1_MSB) == CS_SURFACE_OK);
    CHECK(cs_surface_init(&color, c + 1, 8, 1, 2, 4, CS_SURFACE_RGBA8) == CS_SURFACE_OK);
    memcpy(&first_before, &first, sizeof first);
    CHECK(cs_surface_clear(&first, 1) == CS_SURFACE_OK);
    CHECK(b[1] == 0x00 && b[2] == 0x55 && c[1] == 0xa5);
    CHECK(cs_surface_fill(&second, (cs_rect){1, 1, 2, 2}, 0) == CS_SURFACE_OK);
    CHECK(b[1] == 0x00 && b[2] == 0x15 && a[1] == 0xff && a[2] == 0xff);
    CHECK(cs_surface_clear(&color, UINT32_C(0x10203040)) == CS_SURFACE_OK);
    CHECK(c[0] == 0xa5 && c[1] == 0x10 && c[4] == 0x40 && c[9] == 0xa5);
    CHECK(a[0] == 0x3c && a[3] == 0xc3 && b[0] == 0x3c && b[3] == 0xc3);
    CHECK(memcmp(&first, &first_before, sizeof first) == 0);
    return 0;
}

int main(int argc, char **argv)
{
    int result;
    if (argc != 2) { return 2; }
    if (strcmp(argv[1], "layout") == 0) { result = layout(); }
    else if (strcmp(argv[1], "errors") == 0) { result = errors(); }
    else if (strcmp(argv[1], "matrix") == 0) { result = matrix(); }
    else if (strcmp(argv[1], "independent") == 0) { result = independent(); }
    else { return 2; }
    if (result == 0) { printf("TEST-0004 %s: PASS\n", argv[1]); }
    return result;
}
