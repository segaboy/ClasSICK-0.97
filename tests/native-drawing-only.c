/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Direct native workload/service closure only. */
#include "../apps/native-drawing/workload.h"
int main(void)
{
    cs_qd_rect_port port;cs_native_draw_result result;
    return !cs_native_draw(&port,&result) || result.success!=1 || result.value!=97 ||
        result.phase!=6 || result.black_pixels!=2304 || port.paints!=1 || port.erases!=1;
}
