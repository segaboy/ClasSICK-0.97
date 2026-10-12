/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "guest.h"
cs_guest_result cs_guest_validate(const cs_guest_map *m)
{
    if(m==0) return CS_GUEST_ARGUMENT;
    if(m->regions==0 || m->count==0 || m->count>CS_GUEST_MAX_REGIONS) return CS_GUEST_MAP;
    for(size_t i=0;i<m->count;++i) {
        const cs_guest_region *r=&m->regions[i];
        if(r->bytes==0 || r->size==0 || ((r->base|r->size)&1u)!=0 ||
            r->base>=0x1000000u || r->size>0x1000000u-r->base ||
            r->flags==0 || (r->flags&~7u)!=0 || (r->flags&CS_GUEST_READ)==0 ||
            ((r->flags&CS_GUEST_WRITE)!=0 && (r->writable==0 || r->writable!=r->bytes)))
            return CS_GUEST_MAP;
        for(size_t j=0;j<i;++j) {
            const cs_guest_region *p=&m->regions[j];
            if(!(r->base+r->size<=p->base || p->base+p->size<=r->base)) return CS_GUEST_MAP;
        }
    }
    return CS_GUEST_OK;
}
cs_guest_result cs_guest_window(const cs_guest_map *m,uint32_t at,uint32_t bytes,
    uint32_t access,cs_guest_view *out)
{
    if(out==0 || access==0 || (access&~7u)!=0) return CS_GUEST_ARGUMENT;
    cs_guest_result status=cs_guest_validate(m);
    if(status!=CS_GUEST_OK) return status;
    if(bytes==0 || at>=0x1000000u || bytes>0x1000000u-at) return CS_GUEST_ADDRESS;
    for(size_t i=0;i<m->count;++i) {
        const cs_guest_region *r=&m->regions[i];
        if(at>=r->base && bytes<=r->size && at-r->base<=r->size-bytes) {
            if((r->flags&access)!=access) return CS_GUEST_PERMISSION;
            out->bytes=r->bytes+(at-r->base);
            out->writable=(access&CS_GUEST_WRITE)?r->writable+(at-r->base):0;
            return CS_GUEST_OK;
        }
    }
    return CS_GUEST_ADDRESS;
}
static cs_guest_result operand(const cs_guest_map *m,uint32_t at,unsigned width,
    uint32_t access,cs_guest_view *out)
{
    cs_guest_result status=cs_guest_validate(m);
    if(status!=CS_GUEST_OK) return status;
    if(width!=1 && width!=2 && width!=4) return CS_GUEST_WIDTH;
    if(width!=1 && (at&1u)!=0) return CS_GUEST_ADDRESS;
    return cs_guest_window(m,at,width,access,out);
}
cs_guest_result cs_guest_read(const cs_guest_map *m,uint32_t at,unsigned width,
    uint32_t access,uint32_t *out)
{
    if(out==0 || (access&CS_GUEST_READ)==0) return CS_GUEST_ARGUMENT;
    cs_guest_view v;cs_guest_result status=operand(m,at,width,access,&v);
    if(status!=CS_GUEST_OK) return status;
    uint32_t value=0;
    for(unsigned i=0;i<width;++i) value=(value<<8)|v.bytes[i];
    *out=value;return CS_GUEST_OK;
}
cs_guest_result cs_guest_write(const cs_guest_map *m,uint32_t at,unsigned width,uint32_t value)
{
    cs_guest_view v;cs_guest_result status=operand(m,at,width,CS_GUEST_WRITE,&v);
    if(status!=CS_GUEST_OK) return status;
    for(unsigned i=0;i<width;++i) v.writable[i]=(uint8_t)(value>>((width-1u-i)*8u));
    return CS_GUEST_OK;
}
