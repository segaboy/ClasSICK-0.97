/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "presenter.h"

cs_win_result cs_win_present(HDC dc, const cs_surface *source, uint8_t *scratch,
    size_t scratch_size, int32_t client_width, int32_t client_height)
{
    cs_win_result result;
    cs_rect viewport;
    BITMAPINFO info = {0};
    RECT client = {0, 0, client_width, client_height};
    int saved, rows;
    if (dc == NULL || source == NULL || scratch == NULL
            || client_width < 0 || client_height < 0) return CS_WIN_ERR_ARGUMENT;
    result = cs_win_pixels(source, scratch, scratch_size, (size_t)source->width * 4u);
    if (result != CS_WIN_OK) return result;
    result = cs_win_viewport(source->width, source->height,
        client_width, client_height, &viewport);
    if (result != CS_WIN_OK) return result;
    if (client_width == 0 || client_height == 0) return CS_WIN_OK;
    saved = SaveDC(dc);
    if (saved == 0) return CS_WIN_ERR_HOST;
    result = CS_WIN_ERR_HOST;
    if (!RectVisible(dc, &client)) {
        result = CS_WIN_OK;
        goto restore;
    }
    if (SetDCBrushColor(dc, RGB(24, 28, 36)) == CLR_INVALID) goto restore;
    if (FillRect(dc, &client, (HBRUSH)GetStockObject(DC_BRUSH)) == 0) goto restore;
    {
        RECT image = {viewport.left, viewport.top, viewport.right, viewport.bottom};
        if (!RectVisible(dc, &image)) {
            /* An empty/letterbox-only update region needs no image scanlines. */
            result = CS_WIN_OK;
            goto restore;
        }
    }
    if (SetStretchBltMode(dc, COLORONCOLOR) == 0) goto restore;
    info.bmiHeader.biSize = sizeof(info.bmiHeader);
    info.bmiHeader.biWidth = source->width;
    info.bmiHeader.biHeight = -source->height;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;
    rows = StretchDIBits(dc, viewport.left, viewport.top,
        viewport.right - viewport.left, viewport.bottom - viewport.top,
        0, 0, source->width, source->height, scratch, &info, DIB_RGB_COLORS, SRCCOPY);
    if (rows != 0 && (DWORD)rows != GDI_ERROR) result = CS_WIN_OK;
restore:
    if (RestoreDC(dc, saved) == 0) result = CS_WIN_ERR_HOST;
    return result;
}
