/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "contract.h"

static uint32_t le32(const unsigned char *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1]<<8) | ((uint32_t)p[2]<<16) | ((uint32_t)p[3]<<24);
}
static uint64_t le64(const unsigned char *p)
{
    return (uint64_t)le32(p) | ((uint64_t)le32(p+4)<<32);
}
static int overlap(uint64_t a,uint64_t end_a,uint64_t b,uint64_t end_b)
{
    return a<end_b && b<end_a;
}
cs_uefi_result cs_uefi_check_framebuffer(const cs_uefi_framebuffer *f,
    cs_uefi_framebuffer_layout *out)
{
    uint64_t stride,required;
    if(f==NULL || out==NULL) return CS_UEFI_ARGUMENT;
    if(f->format>1) return CS_UEFI_FORMAT;
    if(f->width==0 || f->height==0 || f->width>8192 || f->height>8192
            || f->pitch<f->width || f->pitch>16384 || f->size==0
            || f->size>UINT64_C(268435456) || f->base==0) return CS_UEFI_LIMIT;
    if(f->base>= (UINT64_C(1)<<47) || f->size>(UINT64_C(1)<<47)-f->base) return CS_UEFI_OVERFLOW;
    stride=(uint64_t)f->pitch*4;
    required=stride*f->height;
    if(required>f->size) return CS_UEFI_LIMIT;
    out->stride=stride; out->required=required;
    return CS_UEFI_OK;
}
cs_uefi_result cs_uefi_check_map(const unsigned char *bytes,size_t length,
    size_t stride,uint32_t version,size_t *out_count)
{
    size_t count;
    if(bytes==NULL || out_count==NULL) return CS_UEFI_ARGUMENT;
    if(version!=1 || stride<40 || stride>256 || stride%8!=0) return CS_UEFI_FORMAT;
    if(length==0 || length>262144 || length%stride!=0) return CS_UEFI_LIMIT;
    count=length/stride;
    if(count>1024) return CS_UEFI_LIMIT;
    for(size_t i=0;i<count;++i) {
        const unsigned char *p=bytes+i*stride;
        uint64_t base=le64(p+8),virt=le64(p+16),pages=le64(p+24),size;
        if(base%4096!=0 || virt%4096!=0 || pages==0) return CS_UEFI_FORMAT;
        if(pages>(UINT64_MAX>>12)) return CS_UEFI_OVERFLOW;
        size=pages<<12;
        if(size>UINT64_MAX-base || size>UINT64_MAX-virt) return CS_UEFI_OVERFLOW;
        for(size_t j=0;j<i;++j) {
            const unsigned char *q=bytes+j*stride;
            uint64_t other=le64(q+8),end=other+(le64(q+24)<<12);
            if(overlap(base,base+size,other,end)) return CS_UEFI_OVERLAP;
        }
    }
    *out_count=count;
    return CS_UEFI_OK;
}
cs_uefi_result cs_uefi_check_owned(const unsigned char *bytes,size_t length,
    size_t stride,uint32_t version,const cs_uefi_owned_span *spans,size_t count,
    const cs_uefi_framebuffer *f)
{
    size_t descriptors;
    cs_uefi_framebuffer_layout layout;
    cs_uefi_result result;
    if(spans==NULL || f==NULL) return CS_UEFI_ARGUMENT;
    if(count==0 || count>8) return CS_UEFI_LIMIT;
    result=cs_uefi_check_framebuffer(f,&layout);
    if(result!=CS_UEFI_OK) return result;
    result=cs_uefi_check_map(bytes,length,stride,version,&descriptors);
    if(result!=CS_UEFI_OK) return result;
    for(size_t i=0;i<count;++i) {
        const cs_uefi_owned_span *s=&spans[i];
        int found=0;
        if(s->kind!=1 && s->kind!=2) return CS_UEFI_FORMAT;
        if(s->base==0 || s->base%4096!=0 || s->size==0 || s->size%4096!=0) return CS_UEFI_FORMAT;
        if(s->base>=(UINT64_C(1)<<47) || s->size>(UINT64_C(1)<<47)-s->base) return CS_UEFI_OVERFLOW;
        if(overlap(s->base,s->base+s->size,f->base,f->base+f->size)) return CS_UEFI_OVERLAP;
        for(size_t j=0;j<i;++j)
            if(overlap(s->base,s->base+s->size,spans[j].base,spans[j].base+spans[j].size)) return CS_UEFI_OVERLAP;
        for(size_t j=0;j<descriptors;++j) {
            const unsigned char *p=bytes+j*stride;
            uint64_t base=le64(p+8),end=base+(le64(p+24)<<12);
            if(le32(p)==s->kind && (le64(p+32)&(UINT64_C(1)<<63))==0
                    && s->base>=base && s->base+s->size<=end) found=1;
        }
        if(!found) return CS_UEFI_OWNERSHIP;
    }
    return CS_UEFI_OK;
}
static int state_valid(const cs_uefi_exit_state *s)
{
    if(s==NULL || s->phase>CS_UEFI_FAILED || s->attempts>3) return 0;
    if(s->phase==CS_UEFI_READY) return s->attempts==0;
    if(s->phase==CS_UEFI_RETRY) return s->attempts>0 && s->attempts<3;
    if(s->phase==CS_UEFI_SNAPSHOT) return s->attempts<3;
    return s->attempts>0;
}
cs_uefi_result cs_uefi_exit_init(cs_uefi_exit_state *s)
{
    if(s==NULL) return CS_UEFI_ARGUMENT;
    s->phase=CS_UEFI_READY; s->attempts=0; s->key=0;
    return CS_UEFI_OK;
}
cs_uefi_result cs_uefi_exit_snapshot(cs_uefi_exit_state *s,uint64_t key)
{
    if(s==NULL) return CS_UEFI_ARGUMENT;
    if(!state_valid(s) || (s->phase!=CS_UEFI_READY && s->phase!=CS_UEFI_RETRY)) return CS_UEFI_STATE;
    s->key=key; s->phase=CS_UEFI_SNAPSHOT;
    return CS_UEFI_OK;
}
cs_uefi_result cs_uefi_exit_observe(cs_uefi_exit_state *s,uint32_t outcome)
{
    if(s==NULL) return CS_UEFI_ARGUMENT;
    if(!state_valid(s) || s->phase!=CS_UEFI_SNAPSHOT) return CS_UEFI_STATE;
    if(outcome>2) return CS_UEFI_FORMAT;
    ++s->attempts;
    if(outcome==0) s->phase=CS_UEFI_EXITED;
    else if(outcome==1 && s->attempts<3) s->phase=CS_UEFI_RETRY;
    else s->phase=CS_UEFI_FAILED;
    return CS_UEFI_OK;
}
int cs_uefi_exit_allowed(const cs_uefi_exit_state *s,uint32_t call)
{
    if(!state_valid(s)) return 0;
    if(call==CS_UEFI_OTHER_BOOT) return s->phase==CS_UEFI_READY;
    if(call==CS_UEFI_GET_MAP) return s->phase==CS_UEFI_READY || s->phase==CS_UEFI_RETRY;
    if(call==CS_UEFI_EXIT_BOOT) return s->phase==CS_UEFI_SNAPSHOT;
    if(call==CS_UEFI_NATIVE_LOOP) return s->phase==CS_UEFI_EXITED;
    return 0;
}
