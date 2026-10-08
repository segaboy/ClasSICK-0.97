/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "acpi.h"

static uint32_t get32(const unsigned char *p)
{
    return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);
}
static uint64_t get64(const unsigned char *p)
{
    return (uint64_t)get32(p)|((uint64_t)get32(p+4)<<32);
}
static int same(const unsigned char *p,const char *text,unsigned n)
{
    for(unsigned i=0;i<n;++i) if(p[i]!=(unsigned char)text[i]) return 0;
    return 1;
}
/* Sum length bytes in bounded 64-byte chunks; no large stack buffer. */
static cs_acpi_result sum(cs_acpi_reader read,void *context,uint64_t at,uint32_t length,
    unsigned char *total)
{
    unsigned char chunk[64];
    unsigned value=0;
    uint32_t done=0;
    if(at>UINT64_MAX-length) return CS_ACPI_FORMAT;
    while(done<length) {
        uint32_t n=length-done>64u?64u:length-done;
        if(!read(context,at+done,chunk,n)) return CS_ACPI_ACCESS;
        for(uint32_t i=0;i<n;++i) value+=chunk[i];
        done+=n;
    }
    *total=(unsigned char)value;
    return CS_ACPI_OK;
}
static cs_acpi_result table(cs_acpi_reader read,void *context,uint64_t at,const char *signature,
    uint32_t minimum,unsigned char *header,uint32_t *length)
{
    unsigned char total;
    cs_acpi_result result;
    if(at==0) return CS_ACPI_FORMAT;
    if(!read(context,at,header,36)) return CS_ACPI_ACCESS;
    if(!same(header,signature,4)) return CS_ACPI_SIGNATURE;
    *length=get32(header+4);
    if(*length<minimum || *length>CS_ACPI_MAX_TABLE) return CS_ACPI_FORMAT;
    result=sum(read,context,at,*length,&total);
    if(result!=CS_ACPI_OK) return result;
    return total==0?CS_ACPI_OK:CS_ACPI_CHECKSUM;
}

cs_acpi_result cs_acpi_find_pm_timer(cs_acpi_reader read,void *context,uint64_t rsdp,
    cs_acpi_pm_timer *out)
{
    unsigned char block[36],entry[8],fadt[220],total;
    uint32_t length,count,flags,fadt_length=0;
    uint64_t xsdt,found=0;
    cs_acpi_result result;
    cs_acpi_pm_timer timer;
    if(read==NULL || out==NULL) return CS_ACPI_ARGUMENT;
    if(rsdp==0) return CS_ACPI_NOT_FOUND;
    if(!read(context,rsdp,block,36)) return CS_ACPI_ACCESS;
    if(!same(block,"RSD PTR ",8)) return CS_ACPI_SIGNATURE;
    total=0;
    for(unsigned i=0;i<20;++i) total=(unsigned char)(total+block[i]);
    if(total!=0) return CS_ACPI_CHECKSUM;
    length=get32(block+20);
    if(block[15]<2 || length<36 || length>1024) return CS_ACPI_FORMAT;
    result=sum(read,context,rsdp,length,&total);
    if(result!=CS_ACPI_OK) return result;
    if(total!=0) return CS_ACPI_CHECKSUM;
    xsdt=get64(block+24);
    result=table(read,context,xsdt,"XSDT",36,block,&length);
    if(result!=CS_ACPI_OK) return result;
    if((length-36u)%8u!=0) return CS_ACPI_FORMAT;
    count=(length-36u)/8u;
    if(count>CS_ACPI_MAX_ENTRIES) return CS_ACPI_FORMAT;
    for(uint32_t i=0;i<count;++i) {
        uint64_t at;
        if(!read(context,xsdt+36u+8u*(uint64_t)i,entry,8)) return CS_ACPI_ACCESS;
        at=get64(entry);
        if(at==0) return CS_ACPI_FORMAT;
        if(!read(context,at,block,36)) return CS_ACPI_ACCESS;
        if(!same(block,"FACP",4)) continue;
        if(found!=0) return CS_ACPI_DUPLICATE;
        found=at;
    }
    if(found==0) return CS_ACPI_NOT_FOUND;
    result=table(read,context,found,"FACP",116,block,&fadt_length);
    if(result!=CS_ACPI_OK) return result;
    length=fadt_length<220u?fadt_length:220u;
    if(!read(context,found,fadt,length)) return CS_ACPI_ACCESS;
    flags=get32(fadt+112);
    if((flags&(UINT32_C(1)<<20))!=0) return CS_ACPI_UNSUPPORTED;
    timer.fadt=found;
    timer.bits=(uint8_t)((flags&(UINT32_C(1)<<8))!=0?32u:24u);
    timer.source=0;
    timer.port=0;
    /* ACPI 6.6 Table 5.9: a usable nonzero X_PM_TMR_BLK takes precedence. */
    if(fadt_length>=220u) {
        uint64_t address=get64(fadt+212);
        if(address!=0 && fadt[208]==1 && fadt[209]==32 && fadt[210]==0
                && (fadt[211]==0 || fadt[211]==3) && address<=0xFFFCu) {
            timer.port=(uint16_t)address;
            timer.source=CS_ACPI_SOURCE_EXTENDED;
        }
    }
    if(timer.source==0) {
        uint32_t port=get32(fadt+76);
        if(port!=0 && fadt[91]==4 && port<=0xFFFCu) {
            timer.port=(uint16_t)port;
            timer.source=CS_ACPI_SOURCE_LEGACY;
        }
    }
    if(timer.source==0) return CS_ACPI_UNSUPPORTED;
    *out=timer;
    return CS_ACPI_OK;
}
