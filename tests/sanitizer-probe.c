/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Intentionally fails. Run only through explicit sanitizer validation. */
#include <limits.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    volatile int index = argc;
    if (argc > 1 && argv[1][0] == 'a') {
        volatile char *bytes = (volatile char *)malloc(1);
        if (bytes == NULL) { return 2; }
        bytes[index] = 1;
        free((void *)bytes);
    } else {
        volatile int largest = INT_MAX;
        return largest + index;
    }
    return 0;
}
