/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "manager.h"
#define MM_MAGIC UINT32_C(0x4D4D3031)
#define PTR 1u
#define HANDLE 2u
static const uint32_t globals[]={0x114,0x118,0x130,0x2A6,0x2AA,0x31A};
typedef struct { uint32_t at[CS_MM_BLOCKS],cell[CS_MM_BLOCKS];unsigned order[CS_MM_BLOCKS],count; } plan;
typedef struct { cs_guest_view zones[2],global[6];uint32_t current,limit; } views;
static const cs_mm_zone *zone(const cs_mm_config *c,unsigned n)
{return n==0?&c->system:&c->application;}
static uint32_t read32(const uint8_t *p)
{return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|((uint32_t)p[2]<<8)|p[3];}
static void write32(uint8_t *p,uint32_t v)
{p[0]=(uint8_t)(v>>24);p[1]=(uint8_t)(v>>16);p[2]=(uint8_t)(v>>8);p[3]=(uint8_t)v;}
static int intersects(uint32_t a,uint32_t as,uint32_t b,uint32_t bs)
{return a<b+bs && b<a+as;}
static uint32_t extent(uint32_t size)
{return size==0?2u:(size+1u)&~1u;}
static cs_mm_result configuration(const cs_guest_map *map,const cs_mm_config *c,views *v)
{
    if(cs_guest_validate(map)!=CS_GUEST_OK) return CS_MM_MEMORY;
    for(unsigned i=0;i<2;++i) {
        const cs_mm_zone *z=zone(c,i);
        if(z->begin<0x400 || z->begin>=z->end || z->end>0x1000000 ||
            ((z->begin|z->end|z->identity)&1u)!=0 || z->end-z->begin<CS_MM_CELL_BYTES+2u ||
            z->identity==0 || z->identity>0xFFFFFEu) return CS_MM_STATE;
        if(cs_guest_window(map,z->begin,z->end-z->begin,CS_GUEST_READ|CS_GUEST_WRITE,&v->zones[i])!=CS_GUEST_OK)
            return CS_MM_MEMORY;
    }
    if(c->system.identity==c->application.identity ||
        intersects(c->system.begin,c->system.end-c->system.begin,c->application.begin,c->application.end-c->application.begin))
        return CS_MM_STATE;
    for(unsigned i=0;i<2;++i) for(unsigned j=0;j<2;++j) {
        const cs_mm_zone *z=zone(c,j);uint32_t identity=zone(c,i)->identity;
        if(identity>=z->begin && identity<z->end) return CS_MM_STATE;
    }
    for(unsigned i=0;i<6;++i)
        if(cs_guest_window(map,globals[i],4,CS_GUEST_READ|CS_GUEST_WRITE,&v->global[i])!=CS_GUEST_OK) return CS_MM_MEMORY;
    return CS_MM_OK;
}
cs_mm_result cs_mm_init(cs_mm_manager *out,const cs_guest_map *map,const cs_mm_config *c)
{
    if(out==0 || map==0 || c==0) return CS_MM_ARGUMENT;
    views v;cs_mm_result result=configuration(map,c,&v);
    if(result!=CS_MM_OK) return result;
    out->map=map;out->config.system.identity=c->system.identity;
    out->config.system.begin=c->system.begin;out->config.system.end=c->system.end;
    out->config.application.identity=c->application.identity;
    out->config.application.begin=c->application.begin;out->config.application.end=c->application.end;
    for(unsigned i=0;i<CS_MM_BLOCKS;++i) {
        out->blocks[i].kind=0;out->blocks[i].zone=0;out->blocks[i].address=0;
        out->blocks[i].size=0;out->blocks[i].extent=0;
    }
    for(unsigned i=0;i<2;++i) for(unsigned j=0;j<CS_MM_CELL_BYTES;++j) v.zones[i].writable[j]=0;
    write32(v.global[0].writable,c->application.end);write32(v.global[1].writable,c->application.identity);
    write32(v.global[2].writable,c->application.end);write32(v.global[3].writable,c->system.identity);
    write32(v.global[4].writable,c->application.identity);write32(v.global[5].writable,0xFFFFFF);
    out->magic=MM_MAGIC;return CS_MM_OK;
}
static cs_mm_result state(cs_mm_manager *m,views *v,plan *p)
{
    if(m->magic!=MM_MAGIC) return CS_MM_STATE;
    cs_mm_result result=configuration(m->map,&m->config,v);
    if(result!=CS_MM_OK) return result;
    if(read32(v->global[0].bytes)!=m->config.application.end ||
        read32(v->global[3].bytes)!=m->config.system.identity ||
        read32(v->global[4].bytes)!=m->config.application.identity || read32(v->global[5].bytes)!=0xFFFFFF)
        return CS_MM_STATE;
    v->current=read32(v->global[1].bytes);v->limit=read32(v->global[2].bytes);
    if((v->current!=m->config.system.identity && v->current!=m->config.application.identity) ||
        (v->limit&1u)!=0 || v->limit<m->config.application.begin+CS_MM_CELL_BYTES || v->limit>m->config.application.end)
        return CS_MM_STATE;
    p->count=0;
    for(unsigned i=0;i<CS_MM_BLOCKS;++i) {
        const cs_mm_block *b=&m->blocks[i];p->at[i]=b->address;p->cell[i]=0;
        if(b->kind==0) {
            if(b->zone || b->address || b->size || b->extent) return CS_MM_STATE;
            continue;
        }
        if((b->kind!=PTR && b->kind!=HANDLE) || b->zone>=2 || b->size>=0x80000000u) return CS_MM_STATE;
        const cs_mm_zone *z=zone(&m->config,b->zone);
        if((b->address&1u)!=0 || b->address<z->begin+CS_MM_CELL_BYTES || b->extent!=extent(b->size) ||
            b->extent>z->end-z->begin || b->address>z->end-b->extent) return CS_MM_STATE;
        for(unsigned j=0;j<i;++j) {
            const cs_mm_block *a=&m->blocks[j];
            if(a->kind && a->zone==b->zone && intersects(a->address,a->extent,b->address,b->extent)) return CS_MM_STATE;
        }
        if(b->kind==HANDLE) {
            p->cell[i]=read32(v->zones[b->zone].bytes+i*4u);
            uint32_t address=p->cell[i]&0xFFFFFFu;
            if(address!=0 && address!=b->address) return CS_MM_STATE;
        }
    }
    return CS_MM_OK;
}
static void compact(const cs_mm_manager *m,plan *p,unsigned zn,unsigned fixed)
{
    p->count=0;
    for(unsigned i=0;i<CS_MM_BLOCKS;++i) if(m->blocks[i].kind && m->blocks[i].zone==zn) {
        unsigned n=p->count++;
        while(n>0 && m->blocks[p->order[n-1]].address>m->blocks[i].address) {
            p->order[n]=p->order[n-1];--n;
        }
        p->order[n]=i;
    }
    uint32_t cursor=zone(&m->config,zn)->begin+CS_MM_CELL_BYTES;
    for(unsigned n=0;n<p->count;++n) {
        unsigned i=p->order[n];const cs_mm_block *b=&m->blocks[i];
        if(b->kind==HANDLE && i!=fixed && (p->cell[i]&CS_MM_LOCK)==0 && (p->cell[i]&0xFFFFFFu)!=0)
            p->at[i]=cursor;
        cursor=p->at[i]+b->extent;
    }
}
static int fits(const cs_mm_manager *m,const plan *p,unsigned zn,uint32_t at,uint32_t bytes,
    uint32_t limit,unsigned ignored)
{
    const cs_mm_zone *z=zone(&m->config,zn);
    if(at<z->begin+CS_MM_CELL_BYTES || bytes>limit-(z->begin+CS_MM_CELL_BYTES) || at>limit-bytes) return 0;
    for(unsigned i=0;i<CS_MM_BLOCKS;++i) {
        const cs_mm_block *b=&m->blocks[i];
        if(i!=ignored && b->kind && b->zone==zn && intersects(at,bytes,p->at[i],b->extent)) return 0;
    }
    return 1;
}
static int gap(const cs_mm_manager *m,const plan *p,unsigned zn,uint32_t bytes,uint32_t limit,
    uint32_t reserve,uint32_t reserved,uint32_t *out)
{
    uint32_t begin=zone(&m->config,zn)->begin+CS_MM_CELL_BYTES;
    if(bytes>limit-begin) return 0;
    uint32_t at=limit-bytes;
    for(unsigned attempt=0;attempt<=CS_MM_BLOCKS+1u;++attempt) {
        uint32_t hit=0x1000000u;
        for(unsigned i=0;i<CS_MM_BLOCKS;++i) {
            const cs_mm_block *b=&m->blocks[i];
            if(b->kind && b->zone==zn && intersects(at,bytes,p->at[i],b->extent) && p->at[i]<hit) hit=p->at[i];
        }
        if(reserved && intersects(at,bytes,reserve,reserved) && reserve<hit) hit=reserve;
        if(hit==0x1000000u) {*out=at;return 1;}
        if(hit<begin || bytes>hit-begin) return 0;
        at=hit-bytes;
    }
    return 0;
}
static void copy_bytes(uint8_t *to,const uint8_t *from,uint32_t bytes,int backwards)
{
    if(backwards) {for(uint32_t i=bytes;i>0;--i) to[i-1]=from[i-1];}
    else {for(uint32_t i=0;i<bytes;++i) to[i]=from[i];}
}
static uint32_t commit_moves(cs_mm_manager *m,const views *v,const plan *p)
{
    uint32_t count=0;
    for(unsigned n=0;n<p->count;++n) {
        unsigned i=p->order[n];cs_mm_block *b=&m->blocks[i];
        if(p->at[i]==b->address) continue;
        const cs_mm_zone *z=zone(&m->config,b->zone);uint8_t *bytes=v->zones[b->zone].writable;
        copy_bytes(bytes+(p->at[i]-z->begin),bytes+(b->address-z->begin),b->size,0);
        b->address=p->at[i];write32(bytes+i*4u,(p->cell[i]&0xFF000000u)|b->address);++count;
    }
    return count;
}
static cs_mm_result reply(cs_mm_reply *out,int code,uint32_t value,uint32_t moved)
{out->code=(int16_t)code;out->value=value;out->moved=moved;return CS_MM_OK;}
static unsigned object(const cs_mm_manager *m,uint32_t address,unsigned kind)
{
    for(unsigned i=0;i<CS_MM_BLOCKS;++i) {
        const cs_mm_block *b=&m->blocks[i];
        uint32_t identity=kind==HANDLE?zone(&m->config,b->zone)->begin+i*4u:b->address;
        if(b->kind==kind && identity==address) return i;
    }
    return CS_MM_BLOCKS;
}
cs_mm_result cs_mm_execute(cs_mm_manager *m,const cs_mm_request *q,cs_mm_reply *out)
{
    if(m==0 || q==0 || out==0) return CS_MM_ARGUMENT;
    if(q->interrupt_context) return CS_MM_INTERRUPT;
    if((unsigned)q->operation>CS_MM_BLOCK_MOVE) return CS_MM_OPERATION;
    int allocate=q->operation==CS_MM_NEW_PTR || q->operation==CS_MM_NEW_HANDLE;
    if(q->flags>3 || (!allocate && q->flags)) return CS_MM_FLAGS;
    views v;plan p;cs_mm_result result=state(m,&v,&p);
    if(result!=CS_MM_OK) return result;
    if(q->operation==CS_MM_BLOCK_MOVE) {
        if(q->size>=0x80000000u) return reply(out,CS_MM_WRONG_ZONE,0,0);
        if(q->size==0) return reply(out,0,0,0);
        cs_guest_view from,to;
        if(cs_guest_window(m->map,q->address,q->size,CS_GUEST_READ,&from)!=CS_GUEST_OK ||
            cs_guest_window(m->map,q->other,q->size,CS_GUEST_WRITE,&to)!=CS_GUEST_OK) return CS_MM_MEMORY;
        copy_bytes(to.writable,from.bytes,q->size,q->other>q->address && q->other-q->address<q->size);
        return reply(out,0,0,0);
    }
    if(allocate) {
        unsigned slot=0;while(slot<CS_MM_BLOCKS && m->blocks[slot].kind) ++slot;
        unsigned zn=(q->flags&CS_MM_SYSTEM) || v.current==m->config.system.identity?0u:1u;
        const cs_mm_zone *z=zone(&m->config,zn);uint32_t limit=zn==0?z->end:v.limit;
        if(slot==CS_MM_BLOCKS || q->size>=0x80000000u || q->size>limit-z->begin-CS_MM_CELL_BYTES)
            return reply(out,CS_MM_FULL,0,0);
        uint32_t bytes=extent(q->size),at=0;
        if(!gap(m,&p,zn,bytes,limit,0,0,&at)) {
            compact(m,&p,zn,CS_MM_BLOCKS);
            if(!gap(m,&p,zn,bytes,limit,0,0,&at)) return reply(out,CS_MM_FULL,0,0);
        }
        uint32_t moved=commit_moves(m,&v,&p);cs_mm_block *b=&m->blocks[slot];
        b->kind=q->operation==CS_MM_NEW_HANDLE?HANDLE:PTR;b->zone=zn;b->address=at;b->size=q->size;b->extent=bytes;
        if(q->flags&CS_MM_CLEAR) for(uint32_t i=0;i<q->size;++i) v.zones[zn].writable[at-z->begin+i]=0;
        uint32_t identity=at;
        if(b->kind==HANDLE) {write32(v.zones[zn].writable+slot*4u,at);identity=z->begin+slot*4u;}
        return reply(out,0,identity,moved);
    }
    unsigned kind=(q->operation==CS_MM_DISPOS_PTR || q->operation==CS_MM_GET_PTR_SIZE || q->operation==CS_MM_SET_PTR_SIZE)?PTR:HANDLE;
    unsigned slot=object(m,q->address,kind);
    int dispose=q->operation==CS_MM_DISPOS_PTR || q->operation==CS_MM_DISPOS_HANDLE;
    if(slot==CS_MM_BLOCKS) return reply(out,kind==HANDLE && q->address==0 && !dispose?CS_MM_NIL_HANDLE:CS_MM_WRONG_ZONE,0,0);
    cs_mm_block *b=&m->blocks[slot];const cs_mm_zone *z=zone(&m->config,b->zone);
    if(dispose) {
        if(kind==HANDLE) write32(v.zones[b->zone].writable+slot*4u,0);
        b->kind=0;b->zone=0;b->address=0;b->size=0;b->extent=0;return reply(out,0,0,0);
    }
    if(kind==HANDLE && (p.cell[slot]&0xFFFFFFu)==0) return reply(out,CS_MM_NIL_HANDLE,0,0);
    if(q->operation==CS_MM_GET_PTR_SIZE || q->operation==CS_MM_GET_HANDLE_SIZE) return reply(out,0,b->size,0);
    if(q->operation==CS_MM_HLOCK || q->operation==CS_MM_HUNLOCK) {
        uint32_t cell=q->operation==CS_MM_HLOCK?p.cell[slot]|CS_MM_LOCK:p.cell[slot]&~CS_MM_LOCK;
        write32(v.zones[b->zone].writable+slot*4u,cell);return reply(out,0,0,0);
    }
    uint32_t limit=b->zone==0?z->end:v.limit;
    if(q->size>=0x80000000u || q->size>z->end-z->begin-CS_MM_CELL_BYTES) return reply(out,CS_MM_FULL,0,0);
    uint32_t bytes=extent(q->size),at=b->address;
    if(bytes>b->extent && !fits(m,&p,b->zone,at,bytes,limit,slot)) {
        compact(m,&p,b->zone,slot);
        if(!fits(m,&p,b->zone,at,bytes,limit,slot)) {
            if(kind==PTR || (p.cell[slot]&CS_MM_LOCK)!=0 ||
                !gap(m,&p,b->zone,bytes,limit,0,0,&at)) return reply(out,CS_MM_FULL,0,0);
        }
    }
    uint32_t moved=commit_moves(m,&v,&p);
    if(at!=b->address) {
        uint8_t *data=v.zones[b->zone].writable;
        copy_bytes(data+(at-z->begin),data+(b->address-z->begin),b->size<q->size?b->size:q->size,at>b->address);
        b->address=at;write32(data+slot*4u,(p.cell[slot]&0xFF000000u)|at);++moved;
    }
    b->size=q->size;b->extent=bytes;return reply(out,0,0,moved);
}
