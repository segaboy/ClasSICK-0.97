/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_TEST_ACPI_FIXTURE_H
#define CLASSICK_TEST_ACPI_FIXTURE_H
#include <stdint.h>
#include <string.h>
/* Original synthetic ACPI images written from the ACPI 6.6 field tables
   (SRC-0036): byte offsets only, no firmware or third-party table data. */
enum { FX_PHYS=0x7FF00000u, FX_SIZE=0x10000u, FX_RSDP=0x100u, FX_XSDT=0x200u,
    FX_APIC=0x800u, FX_FADT=0x1000u, FX_FADT2=0x2000u };
static unsigned char fx[FX_SIZE];
static void fx_put(unsigned char *p,uint64_t value,unsigned bytes)
{
    for(unsigned i=0;i<bytes;++i) { p[i]=(unsigned char)value; value>>=8; }
}
static unsigned char fx_sum(const unsigned char *p,size_t n)
{
    unsigned s=0;
    for(size_t i=0;i<n;++i) s+=p[i];
    return (unsigned char)s;
}
static void fx_header(unsigned char *p,const char *sig,uint32_t length)
{
    memcpy(p,sig,4); fx_put(p+4,length,4); p[8]=1; memcpy(p+10,"CLASIK",6);
}
static void fx_seal(unsigned char *p)
{
    uint32_t length=(uint32_t)p[4]|((uint32_t)p[5]<<8)|((uint32_t)p[6]<<16)|((uint32_t)p[7]<<24);
    p[9]=0; p[9]=(unsigned char)(0u-fx_sum(p,length));
}
static void fx_seal_rsdp(void)
{
    unsigned char *r=fx+FX_RSDP;
    r[8]=0; r[8]=(unsigned char)(0u-fx_sum(r,20));
    r[32]=0; r[32]=(unsigned char)(0u-fx_sum(r,36));
}
/* Valid image: RSDP -> XSDT [APIC, FACP]; FADT length 276, legacy port 0x608. */
static void fx_build(uint32_t flags,uint32_t fadt_length)
{
    unsigned char *r=fx+FX_RSDP,*x=fx+FX_XSDT,*a=fx+FX_APIC,*f=fx+FX_FADT;
    memset(fx,0xCC,sizeof fx);
    memset(r,0,36); memcpy(r,"RSD PTR ",8); memcpy(r+9,"CLASIK",6); r[15]=2;
    fx_put(r+20,36,4); fx_put(r+24,FX_PHYS+FX_XSDT,8);
    memset(x,0,52); fx_header(x,"XSDT",52);
    fx_put(x+36,FX_PHYS+FX_APIC,8); fx_put(x+44,FX_PHYS+FX_FADT,8);
    memset(a,0,44); fx_header(a,"APIC",44); fx_seal(a);
    memset(f,0,fadt_length); fx_header(f,"FACP",fadt_length);
    fx_put(f+76,0x608,4); f[91]=4; fx_put(f+112,flags,4);
    fx_seal(x); fx_seal(f); fx_seal_rsdp();
}
#endif
