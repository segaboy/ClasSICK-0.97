/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "presenter.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "presentation check failed at line %d: %s\n", __LINE__, #condition); \
    exit(1); } } while (0)

static void conversion(void)
{
    /* Literal RGBA inputs include alpha zero, partial alpha and distinct rows. */
    uint8_t source[] = {1,2,3,0, 4,5,6,127, 7,8,9,255, 91,92,
                       10,11,12,1, 13,14,15,128, 16,17,18,254, 93,94};
    const uint8_t expected[] = {3,2,1,0, 6,5,4,0, 9,8,7,0, 77,77,77,
                               12,11,10,0, 15,14,13,0, 18,17,16,0, 77,77,77};
    uint8_t output[34], before[sizeof(source)];
    cs_surface surface;
    memcpy(before, source, sizeof(source));
    memset(output, 77, sizeof(output));
    CHECK(cs_surface_init(&surface, source, sizeof(source), 3, 2, 14,
        CS_SURFACE_RGBA8) == CS_SURFACE_OK);
    CHECK(cs_win_pixels(&surface, output + 1, 30, 15) == CS_WIN_OK);
    CHECK(memcmp(output + 1, expected, sizeof(expected)) == 0);
    CHECK(output[0] == 77 && output[31] == 77 && output[33] == 77);
    CHECK(memcmp(source, before, sizeof(source)) == 0);
    {
        uint8_t mono[] = {0x81,0xFF,0x62, 0x7E,0x7F,0x63};
        const uint8_t row0[] = {0,255,255,255,255,255,255,0,0};
        const uint8_t row1[] = {255,0,0,0,0,0,0,255,255};
        uint8_t pixels[83], original[sizeof(mono)];
        memcpy(original, mono, sizeof(mono));
        memset(pixels, 19, sizeof(pixels));
        CHECK(cs_surface_init(&surface, mono, sizeof(mono), 9, 2, 3,
            CS_SURFACE_MONO1_MSB) == CS_SURFACE_OK);
        CHECK(cs_win_pixels(&surface, pixels + 1, 80, 40) == CS_WIN_OK);
        for (size_t y = 0; y < 2; ++y) for (size_t x = 0; x < 9; ++x) {
            uint8_t value = y == 0 ? row0[x] : row1[x];
            size_t offset = 1 + y * 40 + x * 4;
            CHECK(pixels[offset] == value && pixels[offset + 1] == value
                && pixels[offset + 2] == value && pixels[offset + 3] == 0);
        }
        CHECK(pixels[0] == 19 && pixels[81] == 19 && pixels[82] == 19);
        for (size_t y = 0; y < 2; ++y) for (size_t x = 36; x < 40; ++x)
            CHECK(pixels[1 + y * 40 + x] == 19);
        CHECK(memcmp(mono, original, sizeof(mono)) == 0);
    }
    puts("SPEC-0002 conversion: PASS");
}

static void bounds(void)
{
    uint8_t source[8] = {0}, output[32], before[32];
    cs_surface surface;
    cs_rect rect = {11,12,13,14}, original = rect;
    CHECK(cs_surface_init(&surface, source, sizeof(source), 1, 2, 4,
        CS_SURFACE_RGBA8) == CS_SURFACE_OK);
    memset(output, 29, sizeof(output)); memcpy(before, output, sizeof(output));
    CHECK(cs_win_pixels(NULL, output, 32, 4) == CS_WIN_ERR_ARGUMENT);
    CHECK(cs_win_pixels(&surface, NULL, 32, 4) == CS_WIN_ERR_ARGUMENT);
    CHECK(cs_win_pixels(&surface, output, 32, 3) == CS_WIN_ERR_STRIDE);
    CHECK(cs_win_pixels(&surface, output, 7, 4) == CS_WIN_ERR_STORAGE);
    CHECK(cs_win_pixels(&surface, output, SIZE_MAX, SIZE_MAX) == CS_WIN_ERR_OVERFLOW);
    CHECK(cs_win_pixels(&surface, output, SIZE_MAX, (size_t)PTRDIFF_MAX / 2u + 1u)
        == CS_WIN_ERR_OVERFLOW);
    surface.format = (cs_surface_format)99;
    CHECK(cs_win_pixels(&surface, output, 0, 0) == CS_WIN_ERR_SURFACE);
    surface.format = CS_SURFACE_RGBA8; surface.storage_size = 7;
    CHECK(cs_win_pixels(&surface, output, 32, 4) == CS_WIN_ERR_SURFACE);
    surface.storage_size = 8; surface.width = 0;
    CHECK(cs_win_pixels(&surface, output, 32, 4) == CS_WIN_ERR_SURFACE);
    /* Truthful larger object checks the adapter cap without reading its pixels. */
    {
        uint8_t large[CS_WIN_MAX_DIMENSION + 1];
        CHECK(cs_surface_init(&surface, large, sizeof(large), 1,
            CS_WIN_MAX_DIMENSION + 1, 1, CS_SURFACE_MONO1_MSB) == CS_SURFACE_OK);
        CHECK(cs_win_pixels(&surface, output, 32, 4) == CS_WIN_ERR_DIMENSION);
    }
    CHECK(memcmp(output, before, sizeof(output)) == 0);
    CHECK(cs_win_viewport(1, 1, 1, 1, NULL) == CS_WIN_ERR_ARGUMENT);
    CHECK(cs_win_viewport(0, 1, 1, 1, &rect) == CS_WIN_ERR_DIMENSION);
    CHECK(cs_win_viewport(1, 1, INT32_MIN, 1, &rect) == CS_WIN_ERR_DIMENSION);
    CHECK(memcmp(&rect, &original, sizeof(rect)) == 0);
    CHECK(cs_win_viewport(3, 2, 11, 9, &rect) == CS_WIN_OK);
    CHECK(rect.left == 1 && rect.top == 1 && rect.right == 10 && rect.bottom == 7);
    CHECK(cs_win_viewport(9, 7, 2, 2, &rect) == CS_WIN_OK);
    CHECK(rect.left == -3 && rect.top == -2 && rect.right == 6 && rect.bottom == 5);
    CHECK(cs_win_viewport(1, 1, INT32_MAX, INT32_MAX, &rect) == CS_WIN_OK);
    CHECK(rect.left == 0 && rect.top == 0 && rect.right == INT32_MAX
        && rect.bottom == INT32_MAX);
    CHECK(cs_win_viewport(4096, 1, INT32_MAX, 1, &rect) == CS_WIN_OK);
    CHECK(rect.right - rect.left == 4096 && rect.bottom == 1);
    CHECK(cs_win_viewport(3, 2, 0, INT32_MAX, &rect) == CS_WIN_OK);
    CHECK(rect.left == 0 && rect.top == 0 && rect.right == 0 && rect.bottom == 0);
    puts("SPEC-0002 bounds: PASS");
}

static void render_case(const cs_surface *surface, int width, int height,
    int scale, int left, int top, int clipped)
{
    BITMAPINFO info = {0};
    HDC dc = CreateCompatibleDC(NULL);
    void *storage = NULL;
    HBITMAP bitmap, previous;
    uint8_t scratch[128], source_before[64];
    COLORREF old_brush;
    CHECK(dc != NULL && surface->storage_size <= sizeof(source_before));
    memcpy(source_before, surface->storage, surface->storage_size);
    info.bmiHeader.biSize = sizeof(info.bmiHeader);
    info.bmiHeader.biWidth = width; info.bmiHeader.biHeight = -height;
    info.bmiHeader.biPlanes = 1; info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;
    bitmap = CreateDIBSection(dc, &info, DIB_RGB_COLORS, &storage, NULL, 0);
    CHECK(bitmap != NULL && storage != NULL);
    previous = (HBITMAP)SelectObject(dc, bitmap);
    CHECK(previous != NULL && previous != (HBITMAP)HGDI_ERROR);
    memset(storage, 87, (size_t)width * (size_t)height * 4u);
    CHECK(SetStretchBltMode(dc, HALFTONE) != 0);
    old_brush = SetDCBrushColor(dc, RGB(9,8,7));
    CHECK(old_brush != CLR_INVALID);
    if (clipped == 1) CHECK(IntersectClipRect(dc, 1, 1, width - 1, height - 1) != ERROR);
    if (clipped == 2) CHECK(IntersectClipRect(dc, 0, 0, 1, 1) != ERROR);
    if (clipped == 3) {
        RECT clip;
        HRGN empty = CreateRectRgn(0, 0, 0, 0);
        CHECK(empty != NULL);
        CHECK(SelectClipRgn(dc, empty) != ERROR);
        CHECK(GetClipBox(dc, &clip) == NULLREGION);
        CHECK(DeleteObject(empty) != 0);
    }
    CHECK(cs_win_present(dc, surface, scratch, sizeof(scratch), width, height) == CS_WIN_OK);
    CHECK(GetStretchBltMode(dc) == HALFTONE);
    CHECK(GetDCBrushColor(dc) == RGB(9,8,7));
    CHECK(GdiFlush() != 0);
    for (int y = 0; y < height; ++y) for (int x = 0; x < width; ++x) {
        uint8_t expected[3] = {36,28,24};
        uint8_t *pixel = (uint8_t *)storage + ((size_t)y * (size_t)width + (size_t)x) * 4u;
        int inside = x >= left && y >= top && x < left + surface->width * scale
            && y < top + surface->height * scale;
        if ((clipped == 1 && (x == 0 || y == 0 || x == width - 1 || y == height - 1))
                || (clipped == 2 && (x != 0 || y != 0)) || clipped == 3) {
            expected[0] = 87; expected[1] = 87; expected[2] = 87;
        } else if (inside) {
            int sx = (x - left) / scale, sy = (y - top) / scale;
            if (surface->format == CS_SURFACE_RGBA8) {
                /* Hand-written color identities, independent of converted bytes. */
                static const uint8_t colors[6][3] = {
                    {0,0,255}, {0,255,0}, {255,0,0},
                    {255,255,255}, {0,0,0}, {0,255,255}
                };
                memcpy(expected, colors[sy * 3 + sx], 3);
            } else {
                /* Literal 9x2 bit pattern, independent of bit-unpacking helper. */
                static const uint8_t bits[2][9] = {
                    {1,0,0,0,0,0,0,1,1}, {0,1,1,1,1,1,1,0,0}
                };
                uint8_t value = bits[sy][sx] != 0 ? 0 : 255;
                expected[0] = value; expected[1] = value; expected[2] = value;
            }
        }
        CHECK(memcmp(pixel, expected, 3) == 0);
    }
    CHECK(memcmp(surface->storage, source_before, surface->storage_size) == 0);
    CHECK(SelectObject(dc, previous) != NULL);
    CHECK(DeleteObject(bitmap) != 0 && DeleteDC(dc) != 0);
}

static void gdi(void)
{
    uint8_t rgba[] = {255,0,0,0, 0,255,0,64, 0,0,255,255,
                      255,255,255,1, 0,0,0,127, 255,255,0,254};
    uint8_t mono[] = {0x81,0xFF,99, 0x7E,0x7F,98};
    uint8_t scratch[24];
    cs_surface surface;
    HDC dc = CreateCompatibleDC(NULL);
    CHECK(dc != NULL);
    CHECK(cs_surface_init(&surface, rgba, sizeof(rgba), 3, 2, 12,
        CS_SURFACE_RGBA8) == CS_SURFACE_OK);
    CHECK(cs_win_present(NULL, &surface, scratch, sizeof(scratch), 3, 2) == CS_WIN_ERR_ARGUMENT);
    CHECK(cs_win_present(dc, &surface, scratch, sizeof(scratch), -1, 2) == CS_WIN_ERR_ARGUMENT);
    CHECK(cs_win_present(dc, &surface, scratch, 23, 3, 2) == CS_WIN_ERR_STORAGE);
    CHECK(cs_win_present(dc, &surface, scratch, sizeof(scratch), 0, 2) == CS_WIN_OK);
    CHECK(DeleteDC(dc) != 0);
    render_case(&surface, 3, 2, 1, 0, 0, 0);
    render_case(&surface, 11, 9, 3, 1, 1, 0);
    render_case(&surface, 2, 1, 1, 0, 0, 0);
    render_case(&surface, 11, 9, 3, 1, 1, 1);
    render_case(&surface, 11, 9, 3, 1, 1, 2);
    render_case(&surface, 11, 9, 3, 1, 1, 3);
    CHECK(cs_surface_init(&surface, mono, sizeof(mono), 9, 2, 3,
        CS_SURFACE_MONO1_MSB) == CS_SURFACE_OK);
    render_case(&surface, 19, 7, 2, 0, 1, 0);
    render_case(&surface, 4, 2, 1, -2, 0, 0);
    puts("SPEC-0002 GDI pixels, scaling, crop, clipping and DC restoration: PASS");
}

int main(int argc, char **argv)
{
    if (argc != 2) return 2;
    if (strcmp(argv[1], "conversion") == 0) conversion();
    else if (strcmp(argv[1], "bounds") == 0) bounds();
    else if (strcmp(argv[1], "gdi") == 0) gdi();
    else return 2;
    return 0;
}
