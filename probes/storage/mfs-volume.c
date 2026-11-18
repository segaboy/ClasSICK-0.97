/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "mfs-volume.h"
#include "../../core/storage/mfs-volume.h"

/* Sparse original disk: header2/3, records4/5, data allocation2->4 and resource3.
 * No full disk array, executable file or protected media is embedded. */
static cs_mfsv_volume volume;
static cs_mfsv_workspace scratch;
static cs_mfs_entry entry;
static uint8_t output[604],staging[604];
typedef struct {unsigned calls,bad_free,cycle_map;uint32_t fail_lba;} provider;
static provider source;
static void fill(void *object,size_t n,uint8_t value)
{uint8_t *p=object;size_t i;for(i=0;i<n;++i)p[i]=value;}
static int all(const void *object,size_t n,uint8_t value)
{const uint8_t *p=object;size_t i;for(i=0;i<n;++i)if(p[i]!=value)return 0;return 1;}
static void put16(uint8_t *p,uint16_t x){p[0]=(uint8_t)(x>>8);p[1]=(uint8_t)x;}
static void put32(uint8_t *p,uint32_t x)
{p[0]=(uint8_t)(x>>24);p[1]=(uint8_t)(x>>16);p[2]=(uint8_t)(x>>8);p[3]=(uint8_t)x;}
static uint8_t data_byte(uint32_t lba,unsigned offset)
{return (uint8_t)((unsigned)lba*13u+offset*7u+(offset>>8));}
static cs_block_io_result read_one(void *context,uint32_t lba,uint8_t *block)
{
    provider *p=context;unsigned i;++p->calls;
    if(lba>=20)return CS_BLOCK_IO_UNAVAILABLE;
    if(lba==p->fail_lba){for(i=0;i<23u;++i)block[i]=0x66;return CS_BLOCK_IO_ERROR;}
    fill(block,512,0);
    if(lba==2) {
        put16(block,0xd2d7);put16(block+12,2);put16(block+14,4);put16(block+16,2);
        put16(block+18,6);put32(block+20,1024);put32(block+24,2048);
        put16(block+28,8);put32(block+30,3);put16(block+34,(uint16_t)(p->bad_free?4u:3u));
        block[36]=1;block[37]='T';block[64]=0;block[65]=0x40;block[66]=1;
        block[67]=0;block[68]=(uint8_t)(p->cycle_map?0x20u:0x10u);
    } else if(lba==4) {
        block[0]=0x80;put32(block+18,1);put16(block+22,2);put32(block+24,1500);
        put32(block+28,2048);put16(block+32,3);put32(block+34,13);put32(block+38,1024);
        block[50]=3;block[51]='A';block[52]=0;block[53]=0xfe;
    } else if(lba==5) {
        block[0]=0x80;put32(block+18,2);block[50]=1;block[51]='b';
    } else if(lba>=8)for(i=0;i<512u;++i)block[i]=data_byte(lba,i);
    return CS_BLOCK_IO_OK;
}
unsigned cs_mfs_volume_probe(void)
{
    cs_block_device device;cs_mfsv_cursor cursor;unsigned i,calls;uint8_t name[3];
    source.calls=0;source.bad_free=0;source.cycle_map=0;source.fail_lba=UINT32_MAX;
    device.blocks=20;device.read=read_one;device.context=&source;
    if(cs_mfsv_mount(&device,&scratch,&volume)!=CS_MFSV_OK || source.calls!=2u)return 0xd1;
    fill(&cursor,sizeof cursor,0);
    if(cs_mfsv_directory_next(&volume,&scratch,&cursor,&entry)!=CS_MFSV_OK ||
        entry.file_id!=1 || entry.name_length!=3 || entry.name[1]!=0 || entry.name[2]!=0xfe ||
        cursor.block!=0 || cursor.offset!=54 || cursor.seen!=1)return 0xd2;
    if(cs_mfsv_directory_next(&volume,&scratch,&cursor,&entry)!=CS_MFSV_OK ||
        entry.file_id!=2 || cursor.block!=1 || cursor.seen!=2)return 0xd2;
    if(cs_mfsv_directory_next(&volume,&scratch,&cursor,&entry)!=CS_MFSV_END ||
        cursor.block!=2 || cursor.offset!=0)return 0xd2;
    name[0]='A';name[1]=0;name[2]=0xfe;
    if(cs_mfsv_find(&volume,&scratch,name,3,&entry)!=CS_MFSV_OK || entry.file_id!=1)return 0xd2;
    fill(output,sizeof output,0xa5);fill(staging,sizeof staging,0x5a);
    if(cs_mfsv_fork_read(&volume,&scratch,&entry,CS_MFSV_DATA,500,600,
        output+2,600,staging+2,600)!=CS_MFSV_OK)return 0xd3;
    for(i=0;i<600u;++i) {
        uint32_t lba;unsigned at;
        if(i<12u){lba=8;at=i+500u;}
        else if(i<524u){lba=9;at=i-12u;}
        else {lba=12;at=i-524u;}
        if(output[i+2u]!=data_byte(lba,at))return 0xd3;
    }
    if(output[0]!=0xa5 || output[1]!=0xa5 || output[602]!=0xa5 || output[603]!=0xa5 ||
        staging[0]!=0x5a || staging[1]!=0x5a || staging[602]!=0x5a || staging[603]!=0x5a)return 0xd3;
    if(cs_mfsv_fork_read(&volume,&scratch,&entry,CS_MFSV_RESOURCE,2,11,
        output+2,600,staging+2,600)!=CS_MFSV_OK)return 0xd3;
    for(i=0;i<11u;++i)if(output[i+2u]!=data_byte(10,i+2u))return 0xd3;
    calls=source.calls;
    if(cs_mfsv_fork_read(&volume,&scratch,&entry,CS_MFSV_DATA,1500,0,
        output+2,600,staging+2,600)!=CS_MFSV_OK || source.calls!=calls)return 0xd3;
    if(cs_mfsv_fork_read(&volume,&scratch,&entry,CS_MFSV_DATA,1500,1,
        output+2,600,staging+2,600)!=CS_MFSV_RANGE || source.calls!=calls)return 0xd3;
    /* Complete-chain validation must refuse even a short read before reaching
     * the corrupt link. The chosen cycle is2->4->2 with a3-block physical claim. */
    source.cycle_map=1;
    if(cs_mfsv_mount(&device,&scratch,&volume)!=CS_MFSV_OK)return 0xd5;
    entry.data_physical=3072;calls=source.calls;fill(output,sizeof output,0xa5);
    if(cs_mfsv_fork_read(&volume,&scratch,&entry,CS_MFSV_DATA,0,1,
        output+2,600,staging+2,600)!=CS_MFSV_CHAIN || source.calls!=calls ||
        !all(output,sizeof output,0xa5))return 0xd5;
    source.cycle_map=0;
    if(cs_mfsv_mount(&device,&scratch,&volume)!=CS_MFSV_OK)return 0xd5;
    entry.data_physical=2048;
    source.fail_lba=12;calls=source.calls;fill(output,sizeof output,0xa5);
    if(cs_mfsv_fork_read(&volume,&scratch,&entry,CS_MFSV_DATA,500,600,
        output+2,600,staging+2,600)!=CS_MFSV_IO || source.calls!=calls+3u ||
        !all(output,sizeof output,0xa5))return 0xd6;
    source.fail_lba=UINT32_MAX;source.bad_free=1;fill(&volume,sizeof volume,0xa5);
    if(cs_mfsv_mount(&device,&scratch,&volume)!=CS_MFSV_FORMAT ||
        !all(&volume,sizeof volume,0xa5))return 0xd4;
    source.bad_free=0;source.fail_lba=3;fill(&volume,sizeof volume,0xa5);
    if(cs_mfsv_mount(&device,&scratch,&volume)!=CS_MFSV_IO ||
        !all(&volume,sizeof volume,0xa5))return 0xd7;
    return 0xb6;
}
