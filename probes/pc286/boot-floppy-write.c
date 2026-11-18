/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../../arch/i286/c16.h"
#include "../../platform/pc286/boothush-floppy-write.h"
static cs_pc286_floppy_writer writer;
static cs_pc286_floppy reader;
static cs_block_device device;
static struct {uint16_t before;uint8_t bytes[512];uint16_t after;} source;
static uint8_t received[514],saved[sizeof writer];
static const uint16_t sectors[]={60,74,75,89,90,2399};
static void post(uint8_t value);
#pragma aux post = "out 80h,al" parm [al] modify exact [];
static uint16_t flags_word(void);
#pragma aux flags_word = "pushf" "pop ax" value [ax] modify exact [ax];
static uint16_t ds_word(void);
#pragma aux ds_word = "mov ax,ds" value [ax] modify exact [ax];
static void stop(uint8_t code){post(code);for(;;) { }}
static uint8_t original(uint16_t sector,uint16_t offset)
{return (uint8_t)(sector^(uint16_t)(offset*13u)^(offset>>1)^0x5au);}
static uint8_t written(uint16_t sector,uint16_t offset)
{return (uint8_t)((uint16_t)(sector*3u)^(uint16_t)(offset*29u)^(offset>>3)^0xc7u);}
static void fill(uint8_t *p,uint16_t count,uint8_t value)
{uint16_t i;for(i=0;i<count;++i)p[i]=value;}
static int filled(const uint8_t *p,uint16_t count,uint8_t value)
{uint16_t i;for(i=0;i<count;++i)if(p[i]!=value)return 0;return 1;}
static int same_writer(void)
{uint16_t i;const uint8_t *p=(const uint8_t *)&writer;for(i=0;i<sizeof writer;++i)if(p[i]!=saved[i])return 0;return 1;}
static void snapshot(void)
{uint16_t i;const uint8_t *p=(const uint8_t *)&writer;for(i=0;i<sizeof writer;++i)saved[i]=p[i];}
static void read_sector(uint16_t physical)
{
    fill(received,sizeof received,0x6c);
    if(cs_pc286_floppy_read_one(&reader,(uint16_t)(physical-60u),received+1)!=CS_BLOCK_IO_OK)stop(0xe1);
    if(received[0]!=0x6c || received[513]!=0x6c)stop(0xe6);
}
static int matches(uint16_t physical,int modified)
{
    uint16_t i;for(i=0;i<512;++i)
        if(received[i+1]!=(modified?written(physical,i):original(physical,i)))return 0;
    return 1;
}
static void all_match(int modified)
{uint16_t i;for(i=0;i<sizeof sectors/sizeof sectors[0];++i){read_sector(sectors[i]);if(!matches(sectors[i],modified))stop(0xe2);}}
void start(void)
{
    uint16_t i,j,segment,flags;cs_pc286_write_result result;
    if(ds_word()!=0x1000u)stop(0xe7);
    if(!cs_pc286_floppy_buffer_fits(0x1000u,0xfe00u) || cs_pc286_floppy_buffer_fits(0x1000u,0xfe01u) ||
        !cs_pc286_floppy_buffer_fits(0x1001u,0xfdf0u) || cs_pc286_floppy_buffer_fits(0x1001u,0xfdf1u) ||
        !cs_pc286_floppy_buffer_fits(0x9fc0u,0x200u) || cs_pc286_floppy_buffer_fits(0x9fc0u,0x201u) ||
        cs_pc286_floppy_buffer_fits(0xffffu,0))stop(0xe8);
    fill((uint8_t *)&writer,sizeof writer,0x6c);snapshot();
    if(cs_pc286_floppy_write_bind(NULL,60,2340) || cs_pc286_floppy_write_bind(&writer,60,0) ||
        cs_pc286_floppy_write_bind(&writer,2400,1) || cs_pc286_floppy_write_bind(&writer,2399,2) ||
        cs_pc286_floppy_write_bind(&writer,UINT32_MAX,1) || cs_pc286_floppy_write_bind(&writer,0,UINT32_MAX) ||
        !same_writer())stop(0xe5);
    if(!cs_pc286_floppy_write_bind(&writer,60,2340) || writer.first_sector!=60 || writer.blocks!=2340 ||
        writer.data_segment!=0x1000 || !filled(writer.bounce,512,0x6c) ||
        !cs_pc286_floppy_bind(&reader,60,2340,&device))stop(0xe5);
    source.before=0x1234;source.after=0xabcd;fill(source.bytes,512,0xb3);snapshot();
    if(cs_pc286_floppy_write_one(NULL,0,source.bytes)!=CS_PC286_WRITE_ARGUMENT ||
        cs_pc286_floppy_write_one(&writer,0,NULL)!=CS_PC286_WRITE_ARGUMENT ||
        cs_pc286_floppy_write_one(&writer,0,writer.bounce)!=CS_PC286_WRITE_ARGUMENT ||
        cs_pc286_floppy_write_one(&writer,2340,source.bytes)!=CS_PC286_WRITE_RANGE ||
        cs_pc286_floppy_write_one(&writer,UINT32_MAX,source.bytes)!=CS_PC286_WRITE_RANGE || !same_writer())stop(0xe5);
    segment=writer.data_segment;writer.data_segment=(uint16_t)(segment+1u);
    if(cs_pc286_floppy_write_one(&writer,0,source.bytes)!=CS_PC286_WRITE_STATE)stop(0xe5);
    writer.data_segment=segment;writer.first_sector=2400;
    if(cs_pc286_floppy_write_one(&writer,0,source.bytes)!=CS_PC286_WRITE_STATE)stop(0xe5);
    writer.first_sector=60;writer.blocks=0;
    if(cs_pc286_floppy_write_one(&writer,0,source.bytes)!=CS_PC286_WRITE_STATE)stop(0xe5);
    writer.blocks=2340;
    if(!same_writer() || !filled(source.bytes,512,0xb3) || source.before!=0x1234 || source.after!=0xabcd)stop(0xe6);
    /* A fresh machine sees the persisted marker before making any write. */
    read_sector(60);
    if(matches(60,1)){all_match(1);stop(0xbc);}
    if(!matches(60,0))stop(0xe1);
    for(i=0;i<sizeof sectors/sizeof sectors[0];++i) {
        for(j=0;j<512;++j)source.bytes[j]=written(sectors[i],j);
        flags=flags_word();
        result=cs_pc286_floppy_write_one(&writer,(uint16_t)(sectors[i]-60u),source.bytes);
        if(ds_word()!=0x1000u || (flags_word()&0x0600u)!=(flags&0x0200u))stop(0xe7);
        if(writer.first_sector!=60 || writer.blocks!=2340 || writer.data_segment!=segment ||
            source.before!=0x1234 || source.after!=0xabcd)stop(0xe6);
        for(j=0;j<512;++j)if(source.bytes[j]!=written(sectors[i],j))stop(0xe6);
#ifdef CS_FLOPPY_WRITE_READONLY
        if(result!=CS_PC286_WRITE_PROTECTED)stop(0xe4);
        all_match(0);stop(0xbd);
#else
        if(result!=CS_PC286_WRITE_OK)stop(0xe4);
        read_sector(sectors[i]);
        for(j=0;j<512;++j)if(received[j+1]!=written(sectors[i],j))stop(j==511?0xe3:0xe2);
#endif
    }
    all_match(1);stop(0xbb);
}
