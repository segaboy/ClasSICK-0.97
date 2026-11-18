/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "resource-transport.h"
#include "../../formats/resourcefork/mfs-transport.h"
static cs_mfsv_volume volume;
static cs_mfsv_workspace disk_scratch;
static cs_mfs_entry file;
static cs_rfs_mfs_adapter adapter;
static cs_rfs_source stream;
static cs_rfs_workspace stream_scratch;
static cs_rfs_view view;
static cs_rf_entry resource;
static cs_rfs_report report;
static uint8_t output[304],staging[304];
typedef struct {uint32_t fail_lba;unsigned duplicates;} provider;
static provider source;
static void fill(void *o,size_t n,uint8_t v){uint8_t *p=o;size_t i;for(i=0;i<n;++i)p[i]=v;}
static int all(const void *o,size_t n,uint8_t v){const uint8_t *p=o;size_t i;for(i=0;i<n;++i)if(p[i]!=v)return 0;return 1;}
static void p16(uint8_t *p,uint16_t v){p[0]=(uint8_t)(v>>8);p[1]=(uint8_t)v;}
static void p32(uint8_t *p,uint32_t v){p[0]=(uint8_t)(v>>24);p[1]=(uint8_t)(v>>16);p[2]=(uint8_t)(v>>8);p[3]=(uint8_t)v;}
static uint8_t part32(uint32_t n,unsigned part){return (uint8_t)(n>>(24u-(part<<3)));}
static uint8_t payload(uint32_t at){return (uint8_t)((at<<2)+at+(at>>8));}
/* A 70000-byte original resource fork. Only metadata and a deterministic data
 * function exist in native storage; the map itself begins beyond 64 KiB. */
static uint8_t resource_byte(uint32_t at,unsigned duplicates)
{
    uint32_t p;unsigned names=duplicates?62u:50u;
    if(at<4)return part32(256,(unsigned)at);
    if(at<8)return part32(66256,(unsigned)(at-4));
    if(at<12)return part32(66000,(unsigned)(at-8));
    if(at<16)return part32((uint32_t)(names+6u),(unsigned)(at-12));
    if(at>=256 && at<260)return part32(65540,(unsigned)(at-256));
    if(at>=260 && at<65800)return payload(at-260);
    if(at<66256)return 0;
    p=at-66256;
    if(p==25)return 28;
    if(p==27)return (uint8_t)names;
    if(p>=30 && p<34)return part32(UINT32_C(0x44415441),(unsigned)(p-30));
    if(p==35)return (uint8_t)(duplicates?1u:0u);
    if(p==37)return 10;
    if(p==38 || (duplicates && p==50))return 0xff;
    if(p==39 || (duplicates && p==51))return 0x85;
    if(p==names)return 3;
    if(p==(uint32_t)names+1u)return 'N';
    if(p==(uint32_t)names+3u)return 0xfe;
    return 0;
}
static cs_block_io_result disk_read(void *context,uint32_t lba,uint8_t *block)
{
    provider *p=context;unsigned i;
    if(lba>=152)return CS_BLOCK_IO_UNAVAILABLE;
    if(lba==p->fail_lba){for(i=0;i<19u;++i)block[i]=0x6c;return CS_BLOCK_IO_ERROR;}
    fill(block,512,0);
    if(lba==2) {
        p16(block,0xd2d7);p16(block+12,1);p16(block+14,4);p16(block+16,1);
        p16(block+18,72);p32(block+20,1024);p32(block+24,1024);p16(block+28,8);
        p32(block+30,2);p16(block+34,2);block[36]=1;block[37]='T';
        for(i=0;i<70u;++i) {
            unsigned at=64u+i+(i>>1);uint16_t value=(uint16_t)(i==69u?1u:i+3u);
            if(i==0)value=4;else if(i==1)value=5;else if(i==2)value=3;
            if((i&1u)==0){block[at]=(uint8_t)(value>>4);block[at+1u]|=(uint8_t)(value<<4);}
            else {block[at]|=(uint8_t)(value>>8);block[at+1u]=(uint8_t)value;}
        }
    } else if(lba==4) {
        block[0]=0x80;p32(block+18,1);p16(block+32,2);p32(block+34,70000);
        p32(block+38,71680);block[50]=1;block[51]='R';
    } else if(lba>=8) {
        uint32_t sector=lba-8u,start;
        /* Allocation chain 2,4,3,5,... is genuinely fragmented. */
        if(sector>=2u && sector<4u)sector+=2u;
        else if(sector>=4u && sector<6u)sector-=2u;
        start=sector<<9;
        for(i=0;i<512u;++i)block[i]=resource_byte(start+i,p->duplicates);
    }
    return CS_BLOCK_IO_OK;
}
unsigned cs_resource_transport_probe(void)
{
    cs_block_device device;cs_mfsv_cursor cursor;unsigned i;
    source.fail_lba=UINT32_MAX;source.duplicates=0;
    device.blocks=152;device.read=disk_read;device.context=&source;
    if(cs_mfsv_mount(&device,&disk_scratch,&volume)!=CS_MFSV_OK)return 0xc1;
    fill(&cursor,sizeof cursor,0);
    if(cs_mfsv_directory_next(&volume,&disk_scratch,&cursor,&file)!=CS_MFSV_OK ||
        file.resource_logical!=70000)return 0xc1;
    if(cs_rfs_mfs_bind(&volume,&file,&disk_scratch,&adapter,&stream)!=CS_MFSV_OK ||
        stream.length!=70000)return 0xc2;
    if(cs_rfs_open(&stream,&stream_scratch,&view,&report)!=CS_RFS_OK ||
        view.map.offset!=66256 || view.type_count!=1)return 0xc3;
    if(cs_rfs_find(&view,&stream_scratch,UINT32_C(0x44415441),-123,&resource,&report)!=CS_RFS_OK ||
        resource.data.offset!=260 || resource.data.length!=65540 || resource.name.length!=3)return 0xc3;
    fill(output,sizeof output,0xa5);fill(staging,sizeof staging,0x5a);
    if(cs_rfs_read(&view,&resource,CS_RFS_PAYLOAD,65520,20,output+2,300,staging+2,300,&report)!=CS_RFS_OK)return 0xc4;
    for(i=0;i<20u;++i)if(output[2u+i]!=payload(UINT32_C(65520)+i))return 0xc4;
    if(!all(output+22,sizeof output-22u,0xa5) || output[0]!=0xa5 || output[1]!=0xa5)return 0xc4;
    if(cs_rfs_read(&view,&resource,CS_RFS_RESOURCE_NAME,0,3,output+2,300,staging+2,300,&report)!=CS_RFS_OK ||
        output[2]!='N' || output[3]!=0 || output[4]!=0xfe)return 0xc6;
    source.fail_lba=12;fill(output,sizeof output,0xa5);
    if(cs_rfs_read(&view,&resource,CS_RFS_PAYLOAD,508,300,output+2,300,staging+2,300,&report)!=CS_RFS_IO ||
        !all(output,sizeof output,0xa5))return 0xc5;
    source.fail_lba=UINT32_MAX;
    /* New immutable source configuration/view, not a live-view mutation. */
    source.duplicates=1;
    if(cs_rfs_open(&stream,&stream_scratch,&view,&report)!=CS_RFS_OK)return 0xc7;
    fill(&resource,sizeof resource,0xa5);
    if(cs_rfs_find(&view,&stream_scratch,UINT32_C(0x44415441),-123,&resource,&report)!=CS_RFS_DUPLICATE_ID ||
        !all(&resource,sizeof resource,0xa5))return 0xc7;
    return 0xb7;
}
