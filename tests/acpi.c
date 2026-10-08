/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../platform/pc/acpi.h"
#include "acpi-fixture.h"
#include <stdio.h>
static unsigned failures;
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); ++failures; } } while(0)
typedef struct { uint64_t deny_from,deny_to; size_t reads,bytes; } reader_state;
static int reader(void *context,uint64_t physical,unsigned char *out,size_t length)
{
    reader_state *s=context;
    if(physical<FX_PHYS || length>FX_SIZE || physical-FX_PHYS>FX_SIZE-length) return 0;
    if(physical<s->deny_to && s->deny_from<physical+length) return 0;
    memcpy(out,fx+(size_t)(physical-FX_PHYS),length);
    ++s->reads; s->bytes+=length;
    return 1;
}
static cs_acpi_result run(cs_acpi_pm_timer *t)
{
    reader_state s={0,0,0,0};
    return cs_acpi_find_pm_timer(reader,&s,FX_PHYS+FX_RSDP,t);
}
static void reseal(void) { fx_seal(fx+FX_XSDT); fx_seal(fx+FX_FADT); fx_seal_rsdp(); }
static void discovery(void)
{
    cs_acpi_pm_timer t;
    unsigned char *f=fx+FX_FADT;
    fx_build(0,276); memset(&t,0,sizeof t);
    CHECK(run(&t)==CS_ACPI_OK && t.port==0x608 && t.bits==24 && t.source==CS_ACPI_SOURCE_LEGACY && t.fadt==FX_PHYS+FX_FADT);
    fx_build(1u<<8,276);
    CHECK(run(&t)==CS_ACPI_OK && t.bits==32 && t.source==CS_ACPI_SOURCE_LEGACY);
    fx_build(1u<<8,116);
    CHECK(run(&t)==CS_ACPI_OK && t.bits==32 && t.port==0x608);
    /* Usable X_PM_TMR_BLK takes precedence, with either undefined or dword access. */
    for(unsigned access=0;access<4;access+=3) {
        fx_build(0,276); f[208]=1; f[209]=32; f[210]=0; f[211]=(unsigned char)access;
        fx_put(f+212,0xB008,8); reseal();
        CHECK(run(&t)==CS_ACPI_OK && t.port==0xB008 && t.source==CS_ACPI_SOURCE_EXTENDED);
    }
    /* Unusable extended blocks fall back to the legacy port. */
    static const unsigned char bad[][4]={{0,32,0,3},{1,24,0,3},{1,32,8,3},{1,32,0,2}};
    for(unsigned i=0;i<4;++i) {
        fx_build(0,276); memcpy(f+208,bad[i],4); fx_put(f+212,0xB008,8); reseal();
        CHECK(run(&t)==CS_ACPI_OK && t.port==0x608 && t.source==CS_ACPI_SOURCE_LEGACY);
    }
    fx_build(0,276); f[208]=1; f[209]=32; f[211]=3; fx_put(f+212,0xFFFD,8); reseal();
    CHECK(run(&t)==CS_ACPI_OK && t.source==CS_ACPI_SOURCE_LEGACY);
    /* The extended block is ignored when the FADT is shorter than 220 bytes. */
    fx_build(0,219); f[208]=1; f[209]=32; f[211]=3; fx_put(f+212,0xB008,8); reseal();
    CHECK(run(&t)==CS_ACPI_OK && t.port==0x608);
    /* No usable timer, or hardware-reduced ACPI: unsupported, output preserved. */
    memset(&t,0x5A,sizeof t);
    fx_build(1u<<20,276); CHECK(run(&t)==CS_ACPI_UNSUPPORTED);
    fx_build(0,276); fx_put(f+76,0,4); reseal(); CHECK(run(&t)==CS_ACPI_UNSUPPORTED);
    fx_build(0,276); f[91]=3; reseal(); CHECK(run(&t)==CS_ACPI_UNSUPPORTED);
    fx_build(0,276); fx_put(f+76,0xFFFD,4); reseal(); CHECK(run(&t)==CS_ACPI_UNSUPPORTED);
    CHECK(t.port==0x5A5A && t.bits==0x5A && t.source==0x5A);
    fx_build(0,276); fx_put(f+76,0xFFFC,4); reseal();
    CHECK(run(&t)==CS_ACPI_OK && t.port==0xFFFC);
}
static void checks(void)
{
    cs_acpi_pm_timer t;
    unsigned char *r=fx+FX_RSDP,*x=fx+FX_XSDT,*f=fx+FX_FADT;
    reader_state s={0,0,0,0};
    fx_build(0,276);
    CHECK(cs_acpi_find_pm_timer(NULL,&s,FX_PHYS+FX_RSDP,&t)==CS_ACPI_ARGUMENT);
    CHECK(cs_acpi_find_pm_timer(reader,&s,FX_PHYS+FX_RSDP,NULL)==CS_ACPI_ARGUMENT);
    CHECK(cs_acpi_find_pm_timer(reader,&s,0,&t)==CS_ACPI_NOT_FOUND);
    r[7]='X'; CHECK(run(&t)==CS_ACPI_SIGNATURE); fx_build(0,276);
    r[9]^=1; r[32]=0; r[32]=(unsigned char)(0u-fx_sum(r,36)); CHECK(run(&t)==CS_ACPI_CHECKSUM);
    fx_build(0,276); r[33]=7; CHECK(run(&t)==CS_ACPI_CHECKSUM);
    for(unsigned revision=0;revision<2;++revision) {
        fx_build(0,276); r[15]=(unsigned char)revision; fx_seal_rsdp(); CHECK(run(&t)==CS_ACPI_FORMAT);
    }
    fx_build(0,276); fx_put(r+20,35,4); fx_seal_rsdp(); CHECK(run(&t)==CS_ACPI_FORMAT);
    fx_build(0,276); fx_put(r+20,1025,4); fx_seal_rsdp(); CHECK(run(&t)==CS_ACPI_FORMAT);
    fx_build(0,276); fx_put(r+24,0,8); fx_seal_rsdp(); CHECK(run(&t)==CS_ACPI_FORMAT);
    fx_build(0,276); x[0]='R'; reseal(); CHECK(run(&t)==CS_ACPI_SIGNATURE);
    fx_build(0,276); x[20]^=1; CHECK(run(&t)==CS_ACPI_CHECKSUM);
    fx_build(0,276); fx_put(x+4,51,4); reseal(); CHECK(run(&t)==CS_ACPI_FORMAT);
    fx_build(0,276); fx_put(x+4,35,4); x[9]=0; CHECK(run(&t)==CS_ACPI_FORMAT);
    fx_build(0,276); fx_put(x+4,36+8*257,4); memset(x+52,0,8*255); reseal(); CHECK(run(&t)==CS_ACPI_FORMAT);
    fx_build(0,276); fx_put(x+36,0,8); reseal(); CHECK(run(&t)==CS_ACPI_FORMAT);
    fx_build(0,276); fx_put(x+44,FX_PHYS+FX_APIC,8); reseal(); CHECK(run(&t)==CS_ACPI_NOT_FOUND);
    fx_build(0,276); fx_put(x+4,36,4); reseal(); CHECK(run(&t)==CS_ACPI_NOT_FOUND);
    /* Duplicate FADTs are ambiguous, even if both are valid. */
    fx_build(0,276); memcpy(fx+FX_FADT2,f,276); fx_put(x+4,60,4); memset(x+52,0,8);
    fx_put(x+52,FX_PHYS+FX_FADT2,8); reseal(); CHECK(run(&t)==CS_ACPI_DUPLICATE);
    fx_build(0,276); f[100]^=1; CHECK(run(&t)==CS_ACPI_CHECKSUM);
    fx_build(0,115); reseal(); CHECK(run(&t)==CS_ACPI_FORMAT);
    fx_build(0,276); fx_put(f+4,65537,4); CHECK(run(&t)==CS_ACPI_FORMAT);
    /* Exactly 256 XSDT entries is accepted. */
    fx_build(0,276); fx_put(x+4,36+8*256,4);
    memset(fx+0x3000,0,44); fx_header(fx+0x3000,"SSDT",44); fx_seal(fx+0x3000);
    for(unsigned i=0;i<256;++i) fx_put(x+36+8*i,FX_PHYS+(i==255?FX_FADT:0x3000u),8);
    reseal(); CHECK(run(&t)==CS_ACPI_OK && t.port==0x608);
}
static void walk(void)
{
    static const struct { uint64_t from,to; } deny[]={
        {FX_PHYS+FX_RSDP,FX_PHYS+FX_RSDP+1},{FX_PHYS+FX_RSDP+35,FX_PHYS+FX_RSDP+36},
        {FX_PHYS+FX_XSDT,FX_PHYS+FX_XSDT+1},{FX_PHYS+FX_XSDT+51,FX_PHYS+FX_XSDT+52},
        {FX_PHYS+FX_APIC,FX_PHYS+FX_APIC+1},{FX_PHYS+FX_FADT,FX_PHYS+FX_FADT+1},
        {FX_PHYS+FX_FADT+275,FX_PHYS+FX_FADT+276}};
    cs_acpi_pm_timer t;
    fx_build(0,276);
    for(unsigned i=0;i<7;++i) {
        reader_state s={deny[i].from,deny[i].to,0,0};
        CHECK(cs_acpi_find_pm_timer(reader,&s,FX_PHYS+FX_RSDP,&t)==CS_ACPI_ACCESS);
    }
    /* Reads stay within declared lengths: nothing past each table is required. */
    {
        reader_state s={FX_PHYS+FX_FADT+276,FX_PHYS+FX_SIZE,0,0};
        reader_state u={FX_PHYS+FX_XSDT+52,FX_PHYS+FX_APIC,0,0};
        CHECK(cs_acpi_find_pm_timer(reader,&s,FX_PHYS+FX_RSDP,&t)==CS_ACPI_OK);
        CHECK(cs_acpi_find_pm_timer(reader,&u,FX_PHYS+FX_RSDP,&t)==CS_ACPI_OK);
        CHECK(s.bytes<=36u*2+52u*2+16u+36u*3+276u+220u && s.reads<64u);
    }
    /* A table extending beyond readable memory is an access failure. */
    fx_build(0,276); fx_put(fx+FX_FADT+4,FX_SIZE-FX_FADT+4,4);   /* checksum unreachable */
    CHECK(run(&t)==CS_ACPI_ACCESS);
}
int main(int argc,char **argv)
{
    const char *suite=argc>1?argv[1]:"";
    if(strcmp(suite,"discovery")==0) discovery();
    else if(strcmp(suite,"checks")==0) checks();
    else if(strcmp(suite,"walk")==0) walk();
    else { fprintf(stderr,"unknown suite\n"); return 2; }
    if(failures!=0) { fprintf(stderr,"%u failures\n",failures); return 1; }
    printf("ACPI %s: PASS\n",suite);
    return 0;
}
