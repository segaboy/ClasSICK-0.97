/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../platform/pc/acpi.h"
#include "acpi-fixture.h"
#include <stdio.h>
static unsigned failures;
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); ++failures; } } while(0)
static int read_table(void *context,uint64_t at,unsigned char *out,size_t n)
{
    (void)context;
    if(at<FX_PHYS || n>FX_SIZE || at-FX_PHYS>FX_SIZE-n) return 0;
    memcpy(out,fx+(size_t)(at-FX_PHYS),n); return 1;
}
int main(void)
{
    uint32_t present=7;
    for(unsigned revision=1;revision<=6;++revision) for(unsigned flags=0;flags<256;++flags) {
        fx_build(0,276); fx[FX_FADT+8]=(uint8_t)revision; fx[FX_FADT+109]=(uint8_t)flags; fx_seal(fx+FX_FADT);
        present=7;
        CHECK(cs_acpi_8042(read_table,NULL,FX_PHYS+FX_FADT,&present)==(revision>=3?CS_ACPI_OK:CS_ACPI_UNSUPPORTED));
        CHECK(present==(revision>=3?((flags>>1)&1u):7u));
    }
    CHECK(cs_acpi_8042(NULL,NULL,FX_PHYS+FX_FADT,&present)==CS_ACPI_ARGUMENT);
    CHECK(cs_acpi_8042(read_table,NULL,FX_PHYS+FX_FADT,NULL)==CS_ACPI_ARGUMENT);
    fx_build(0,276); fx[FX_FADT+109]^=2; present=7;
    CHECK(cs_acpi_8042(read_table,NULL,FX_PHYS+FX_FADT,&present)==CS_ACPI_CHECKSUM && present==7);
    fx_build(0,115); CHECK(cs_acpi_8042(read_table,NULL,FX_PHYS+FX_FADT,&present)==CS_ACPI_FORMAT);
    CHECK(cs_acpi_8042(read_table,NULL,UINT64_MAX,&present)==CS_ACPI_ACCESS);
    if(failures) return 1;
    puts("ACPI 8042: PASS"); return 0;
}
