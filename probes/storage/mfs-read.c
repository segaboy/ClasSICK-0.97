/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "mfs-read.h"
#include "../../core/storage/mfs-read.h"
static void put16(uint8_t *p,uint16_t x){p[0]=(uint8_t)(x>>8);p[1]=(uint8_t)x;}
static void put32(uint8_t *p,uint32_t x){p[0]=(uint8_t)(x>>24);p[1]=(uint8_t)(x>>16);p[2]=(uint8_t)(x>>8);p[3]=(uint8_t)x;}
static void fill(void *object,size_t size,uint8_t value){uint8_t *p=object;size_t i;for(i=0;i<size;++i)p[i]=value;}
static int all(const void *object,size_t size,uint8_t value){const uint8_t *p=object;size_t i;for(i=0;i<size;++i)if(p[i]!=value)return 0;return 1;}
typedef struct {const uint8_t *block;unsigned calls;unsigned fail;} provider;
static cs_block_io_result read_one(void *context,uint32_t lba,uint8_t *block)
{
    provider *p=context;unsigned i,limit=p->fail?17u:512u;
    ++p->calls;if(lba!=2)return CS_BLOCK_IO_UNAVAILABLE;
    for(i=0;i<limit;++i)block[i]=p->block[i];
    return p->fail?CS_BLOCK_IO_ERROR:CS_BLOCK_IO_OK;
}
unsigned cs_mfs_read_probe(void)
{
    uint8_t b[514];cs_mfs_volume volume;cs_mfs_entry entry;
    cs_mfs_workspace work;provider p;cs_block_device device;
    unsigned i;
    fill(b,sizeof b,0x6c);put16(b,0xd2d7);put32(b+2,UINT32_C(0x12345678));
    put32(b+6,UINT32_C(0x89abcdef));put16(b+10,0x8081);put16(b+12,0x1234);
    put16(b+14,4);put16(b+16,1);put16(b+18,4);put32(b+20,1024);
    put32(b+24,UINT32_C(0x76543210));put16(b+28,4);put32(b+30,UINT32_C(0xfedcba98));
    put16(b+34,3);b[36]=3;b[37]='M';b[38]=0;b[39]=0xfe;
    p.block=b;p.calls=0;p.fail=0;device.blocks=32;device.read=read_one;device.context=&p;
    if(cs_mfs_header_read(&device,&work,&volume)!=CS_MFS_OK || p.calls!=1)return 0xc1;
    if(volume.created!=UINT32_C(0x12345678) || volume.backup!=UINT32_C(0x89abcdef) ||
        volume.attributes!=0x8081 || volume.file_count!=0x1234 || volume.alloc_count!=4 ||
        volume.alloc_bytes!=1024 || volume.clump_bytes!=UINT32_C(0x76543210) ||
        volume.next_file!=UINT32_C(0xfedcba98) || volume.name[1]!=0 || volume.name[2]!=0xfe ||
        !all(volume.name+3,24,0))return 0xc2;
    fill(&volume,sizeof volume,0xa5);p.fail=1;p.calls=0;
    if(cs_mfs_header_read(&device,&work,&volume)!=CS_MFS_IO || p.calls!=1 ||
        !all(&volume,sizeof volume,0xa5))return 0xc3;
    p.fail=0;p.calls=0;device.blocks=3;
    if(cs_mfs_header_read(&device,&work,&volume)!=CS_MFS_GEOMETRY || p.calls!=0)return 0xc4;
    put16(b+18,513);put16(b+34,0);put32(b+20,UINT32_C(0xfffffe00));
    if(cs_mfs_header_decode(b,512,UINT32_MAX,&volume)!=CS_MFS_GEOMETRY ||
        !all(&volume,sizeof volume,0xa5))return 0xc5;
    put16(b+18,512);
    if(cs_mfs_header_decode(b,512,UINT32_MAX,&volume)!=CS_MFS_OK)return 0xc6;
    fill(b,sizeof b,0x6c);b[208]=0x80;b[258]=255;fill(&entry,sizeof entry,0xa5);
    if(cs_mfs_entry_decode(b,512,208,&entry)!=CS_MFS_ENTRY_BOUNDS ||
        !all(&entry,sizeof entry,0xa5))return 0xc7;
    b[510]=0x7f;
    if(cs_mfs_entry_decode(b,512,510,&entry)!=CS_MFS_INACTIVE ||
        !all(&entry,sizeof entry,0xa5))return 0xc8;
    fill(b,sizeof b,0x6c);b[460]=0xc1;b[461]=0xa7;b[510]=0;
    put32(b+478,UINT32_C(0x12345678));put16(b+482,0x3456);
    put32(b+484,UINT32_C(0xf1234567));put32(b+488,0);
    if(cs_mfs_entry_decode(b,512,460,&entry)!=CS_MFS_OK || entry.extent!=52 ||
        entry.offset!=460 || entry.flags!=0xc1 || entry.version!=0xa7 ||
        entry.file_id!=UINT32_C(0x12345678) || entry.data_start!=0x3456 ||
        entry.data_logical!=UINT32_C(0xf1234567) || entry.data_physical!=0 ||
        entry.name_length!=0 || !all(entry.name,255,0))return 0xc9;
    fill(b,sizeof b,0x6c);b[206]=0x80;b[256]=255;
    for(i=0;i<255;++i)b[257u+i]=(uint8_t)(i^0xa5u);
    if(cs_mfs_entry_decode(b,512,206,&entry)!=CS_MFS_OK || entry.extent!=306 ||
        entry.name_length!=255)return 0xca;
    for(i=0;i<255;++i)if(entry.name[i]!=(uint8_t)(i^0xa5u))return 0xcb;
    if(b[512]!=0x6c || b[513]!=0x6c)return 0xcc;
    return 0xb5;
}
