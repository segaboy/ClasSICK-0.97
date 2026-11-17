/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Original shared RAM provider diagnostic, not a physical disk driver. */
#include "block-read.h"
#include "../../core/storage/block-read.h"

typedef struct { uint32_t first; unsigned calls, fail; } provider;
static cs_block_io_result read_one(void *context,uint32_t lba,uint8_t *block)
{
    provider *p=context;
    unsigned i,limit=p->calls+1u==p->fail?23u:512u;
    if(lba!=p->first+(uint32_t)p->calls)return CS_BLOCK_IO_UNAVAILABLE;
    ++p->calls;
    for(i=0;i<limit;++i)block[i]=(uint8_t)((lba&255UL)+(uint32_t)(i&255u));
    return p->calls==p->fail?CS_BLOCK_IO_ERROR:CS_BLOCK_IO_OK;
}
static int all(const uint8_t *bytes,unsigned count,uint8_t value)
{
    unsigned i;for(i=0;i<count;++i)if(bytes[i]!=value)return 0;return 1;
}
unsigned cs_block_read_probe(void)
{
    uint8_t output[1026],staging[1026];
    provider p;
    cs_block_device device;
    unsigned i;
    p.first=UINT32_MAX-2;p.calls=0;p.fail=0;
    device.blocks=UINT32_MAX;device.read=read_one;device.context=&p;
    for(i=0;i<1026;++i){output[i]=0xa5;staging[i]=0x5a;}
    if(cs_block_read(&device,p.first,2,output+1,1024,staging+1,1024)!=CS_BLOCK_OK || p.calls!=2)return 0xe1;
    for(i=0;i<1024;++i) {
        uint8_t expected=(uint8_t)(253u+(i/512u)+(i%256u));
        if(output[i+1]!=expected)return 0xe2;
    }
    if(output[0]!=0xa5 || output[1025]!=0xa5 || staging[0]!=0x5a || staging[1025]!=0x5a)return 0xe3;
    for(i=0;i<1026;++i){output[i]=0xa5;staging[i]=0x5a;}
    p.first=7;p.calls=0;p.fail=2;
    if(cs_block_read(&device,7,2,output+1,1024,staging+1,1024)!=CS_BLOCK_IO ||
        p.calls!=2 || !all(output,1026,0xa5) || staging[513]!=8 || staging[536]!=0x5a)return 0xe4;
    p.first=UINT32_MAX;p.calls=0;p.fail=0;
    if(cs_block_read(&device,UINT32_MAX,1,output+1,512,staging+1,512)!=CS_BLOCK_RANGE || p.calls!=0)return 0xe5;
    if(cs_block_read(&device,UINT32_MAX-1,2,output+1,1024,staging+1,1024)!=CS_BLOCK_RANGE || p.calls!=0)return 0xe6;
    if(cs_block_read(&device,0,64,output+1,1024,staging+1,1024)!=CS_BLOCK_RANGE || p.calls!=0)return 0xe7;
    if(cs_block_read(&device,0,1,output+1,511,staging+1,512)!=CS_BLOCK_CAPACITY || p.calls!=0)return 0xe8;
    if(cs_block_read(&device,0,1,output+1,512,output+1,512)!=CS_BLOCK_ARGUMENT || p.calls!=0)return 0xe9;
    if(!all(output,1026,0xa5) || staging[0]!=0x5a || staging[1025]!=0x5a)return 0xea;
    return 0xb4;
}
