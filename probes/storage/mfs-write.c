/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "mfs-write.h"
#include "../../personality/system1/file-io.h"

volatile uint32_t cs_mfs_write_report[16];
static cs_mfsv_volume volume;
static struct {uint32_t before;cs_mfsv_workspace work;uint32_t after;} scratch;
static cs_mfs_entry first,second;
static uint8_t input[1102],output[768],staging[768];
static struct {
    const cs_block_device *reader;
    const cs_block_writer *writer;
    uint32_t reads,writes,fail_read,fail_write;
} transport;
static void fill(uint8_t *p,size_t n,uint8_t value)
{size_t i;for(i=0;i<n;++i)p[i]=value;}
static int filled(const uint8_t *p,size_t n,uint8_t value)
{size_t i;for(i=0;i<n;++i)if(p[i]!=value)return 0;return 1;}
static uint8_t pattern(uint32_t index,unsigned salt)
{return (uint8_t)((unsigned)(index&255u)*17u+(index>>3)+salt);}
static uint8_t old_byte(uint32_t index,unsigned salt)
{return (uint8_t)((unsigned)(index&255u)*29u+(index>>8)+salt);}
static uint8_t expected_byte(unsigned file,cs_mfsv_fork fork,uint32_t index,unsigned style)
{
    uint8_t value=old_byte(index,file==0?(fork==CS_MFSV_DATA?7u:91u):173u);
    if(file==0 && fork==CS_MFSV_DATA) {
        uint32_t end=style==2?1024u:style==3?1536u:1600u;
        if(index>=500u && index<end)value=pattern(index-500u,0x31u);
        if(style==1 && index>=2400u)value=pattern(index-2400u,0x97u);
        if(style==1 && index<512u)value=pattern(index,0xb9u);
        if(style==1 && index>=1000u && index<1096u)value=pattern(index-1000u,0x41u);
    } else if(style==1 && file==0 && index>=3u && index<14u)value=pattern(index-3u,0x53u);
    else if(style==1 && file==1 && index>=500u)value=pattern(index-500u,0x75u);
    return value;
}
static cs_block_io_result read_one(void *context,uint32_t lba,uint8_t *block)
{
    (void)context;++transport.reads;
    if(transport.fail_read && transport.reads==transport.fail_read) {
        block[0]=0x19;return CS_BLOCK_IO_ERROR;
    }
    return transport.reader->read(transport.reader->context,lba,block);
}
static cs_block_io_result write_one(void *context,uint32_t lba,const uint8_t *block)
{
    cs_block_io_result result;(void)context;++transport.writes;
    result=transport.writer->write(transport.writer->context,lba,block);
    if(result==CS_BLOCK_IO_OK && transport.fail_write && transport.writes==transport.fail_write)
        return CS_BLOCK_IO_ERROR; /* Deliberately uncertain acknowledgement after actual commit. */
    return result;
}
static void note(cs_mfsv_result result,const cs_mfsw_progress *progress)
{
    cs_mfs_write_report[3]=(uint32_t)result;cs_mfs_write_report[4]=progress->completed;
    cs_mfs_write_report[5]=progress->lba;cs_mfs_write_report[6]=(uint32_t)progress->phase;
    cs_mfs_write_report[7]=progress->uncertain;cs_mfs_write_report[8]=transport.writes;
    cs_mfs_write_report[9]=transport.reads;
}
static unsigned finish(unsigned marker)
{cs_mfs_write_report[15]=(uint32_t)marker;return marker;}

static void set_long(uint8_t *p,uint32_t value)
{p[0]=(uint8_t)(value>>24);p[1]=(uint8_t)(value>>16);p[2]=(uint8_t)(value>>8);p[3]=(uint8_t)value;}

#ifdef __m68k__
extern uint32_t cs_file_probe_trap(uint32_t pb,uint32_t opcode);
extern volatile uint32_t cs_mac_diag[8];
static cs_s1_open_fork *active_file;
static const cs_guest_map *active_map;
static cs_trap_frame *active_frame;
uint32_t cs_file_probe_service(uint32_t opcode,uint32_t pb)
{
    if(!active_file || pb!=active_frame->a[0] ||
       !cs_s1_file_io(active_file,&scratch.work,active_map,active_frame,(uint16_t)opcode,staging,sizeof staging)) {
        cs_mac_diag[7]=UINT32_C(0xf002);
        for(;;) { }
    }
    return active_frame->d[0];
}
#endif

/* Independent byte-oriented guest parameter block. Controls and buffer share
 * a checked native-backed guest RAM region, with no overlapping live extents. */
static int file_call(cs_s1_open_fork *file,cs_trap_frame *frame,const cs_guest_map *map,
    uint16_t word,unsigned mode,uint32_t offset,unsigned count,int expected,
    unsigned transferred,unsigned mark)
{
    uint8_t expected_tail[4];
    set_long(output+32,frame->a[0]+64u);
    set_long(output+36,(uint32_t)count); set_long(output+46,offset);
    output[44]=0; output[45]=(uint8_t)mode;
#ifdef __m68k__
    active_file=file;active_frame=frame;active_map=map;
    if(cs_file_probe_trap(frame->a[0],word)!=(uint32_t)(int32_t)expected) return 0;
    active_file=NULL;
#else
    frame->pc=word==0xa002 ? 0x1000u : 0x1002u;
    if(!cs_s1_file_step(file,&scratch.work,map,frame,staging,sizeof staging) ||
       frame->pc!=(word==0xa002 ? 0x1002u : 0x1004u)) return 0;
#endif
    if(frame->d[0]!=(uint32_t)(int32_t)expected ||
       output[16]!=(uint8_t)((uint16_t)expected>>8) || output[17]!=(uint8_t)expected) return 0;
    set_long(expected_tail,(uint32_t)transferred);
    for(unsigned i=0;i<4u;++i) if(output[40+i]!=expected_tail[i]) return 0;
    set_long(expected_tail,(uint32_t)mark);
    for(unsigned i=0;i<4u;++i) if(output[46+i]!=expected_tail[i]) return 0;
    return 1;
}

static int file_operations(const cs_block_writer *writer,unsigned mode)
{
    static cs_s1_open_fork file;
    static cs_trap_frame frame;
    cs_guest_region regions[2];
    cs_guest_map map;
    unsigned i;
    regions[0].bytes=output; regions[0].writable=output;
    regions[0].size=sizeof output; regions[0].flags=CS_GUEST_READ|CS_GUEST_WRITE;
    map.regions=regions;
#ifdef __m68k__
    regions[0].base=(uint32_t)(uintptr_t)output;
    map.count=1;
#else
    static const uint8_t instructions[]={0xa0,2,0xa0,3};
    regions[0].base=UINT32_C(0x22000);
    regions[1].bytes=instructions;regions[1].writable=NULL;regions[1].base=0x1000;
    regions[1].size=sizeof instructions;regions[1].flags=CS_GUEST_READ|CS_GUEST_EXECUTE;
    map.count=2;
#endif
    frame.a[0]=regions[0].base;
    fill(output,sizeof output,0);
    if(cs_s1_open_named_fork(&file,&volume,&scratch.work,(const uint8_t *)"Start",5,
       CS_MFSV_DATA,9,mode==0 ? writer : NULL)!=CS_MFSV_OK) return 0;
    output[25]=8;
    if(!file_call(&file,&frame,&map,0xa002,0,0,1,-51,0,0)) return 0;
    output[25]=9;
    if(mode==0) {
        for(i=0;i<96u;++i) output[64+i]=pattern(i,0x41u);
        if(!file_call(&file,&frame,&map,0xa003,1,1000,96,0,96,1096)) return 0;
    } else if(!file_call(&file,&frame,&map,0xa003,1,1000,96,-61,0,0)) return 0;
    /* Read the fragmented region in two sequential calls (at mark ignores offset). */
    fill(output+64,100,0xa5);
    if(!file_call(&file,&frame,&map,0xa002,1,1000,64,0,64,1064)) return 0;
    for(i=0;i<64u;++i) if(output[64+i]!=pattern(i,0x41u)) return 0;
    if(!file_call(&file,&frame,&map,0xa002,0,UINT32_MAX,32,0,32,1096)) return 0;
    for(i=0;i<32u;++i) if(output[64+i]!=pattern(i+64u,0x41u)) return 0;
    /* Negative relative-to-mark and EOF-relative reads, with tail preservation. */
    if(!file_call(&file,&frame,&map,0xa002,3,UINT32_C(0xfffffffe),2,0,2,1096) ||
       output[64]!=pattern(94,0x41u) || output[65]!=pattern(95,0x41u)) return 0;
    fill(output+64,12,0xa5);
    if(!file_call(&file,&frame,&map,0xa002,2,UINT32_C(0xfffffffd),10,-39,3,2437)) return 0;
    for(i=0;i<3u;++i) if(output[64+i]!=expected_byte(0,CS_MFSV_DATA,2434u+i,1)) return 0;
    if(!filled(output+67,9,0xa5)) return 0;
    cs_mfs_write_report[14]=mode==0 ? 0x5036u : 0x5236u;
    return 1;
}
static cs_mfsv_result overwrite(const cs_block_writer *writer,const cs_mfs_entry *entry,
    cs_mfsv_fork fork,uint32_t offset,size_t count,unsigned salt,cs_mfsw_progress *progress)
{
    size_t i;cs_mfsv_result result;
    input[0]=0x96;input[1101]=0x69;
    for(i=0;i<1100u;++i)input[i+1u]=pattern((uint32_t)i,salt);
    result=cs_mfsw_fork_overwrite(&volume,&scratch.work,entry,fork,writer,offset,count,input+1,1100,progress);
    for(i=0;i<1100u;++i)if(input[i+1u]!=pattern((uint32_t)i,salt))cs_mfs_write_report[13]=1;
    if(input[0]!=0x96 || input[1101]!=0x69 || scratch.before!=UINT32_C(0x1234abcd) ||
        scratch.after!=UINT32_C(0xcdef5678))cs_mfs_write_report[13]=1;
    note(result,progress);return result;
}
static int verify(const cs_mfs_entry *entry,unsigned file,cs_mfsv_fork fork,unsigned style)
{
    uint32_t length=fork==CS_MFSV_DATA?entry->data_logical:entry->resource_logical,offset=0;
    while(offset<length) {
        size_t i,take=length-offset>sizeof output?sizeof output:(size_t)(length-offset);
        fill(output,sizeof output,0xa5);
        if(cs_mfsv_fork_read(&volume,&scratch.work,entry,fork,offset,take,
            output,sizeof output,staging,sizeof staging)!=CS_MFSV_OK)return 0;
        for(i=0;i<take;++i) {
            uint32_t digest=cs_mfs_write_report[11];
            if(output[i]!=expected_byte(file,fork,offset+(uint32_t)i,style))return 0;
            cs_mfs_write_report[11]=((digest<<5)|(digest>>27))^output[i];
        }
        if(!filled(output+take,sizeof output-take,0xa5))return 0;
        offset+=(uint32_t)take;cs_mfs_write_report[10]+=(uint32_t)take;
    }
    return 1;
}

unsigned cs_mfs_write_probe(const cs_block_device *device,const cs_block_writer *writer,unsigned mode)
{
    cs_block_device source;cs_block_writer sink,wrong;cs_mfsw_progress progress;
    cs_mfsv_result result;unsigned i,style=1;uint8_t saved;
    for(i=0;i<16u;++i)cs_mfs_write_report[i]=0;
    cs_mfs_write_report[0]=UINT32_C(0x4d465357);cs_mfs_write_report[1]=mode;
    transport.reader=device;transport.writer=writer;
    transport.reads=transport.writes=transport.fail_read=transport.fail_write=0;
    source.blocks=device->blocks;source.read=read_one;source.context=&transport;
    sink.blocks=writer->blocks;sink.write=write_one;sink.context=&transport;
    scratch.before=UINT32_C(0x1234abcd);scratch.after=UINT32_C(0xcdef5678);
    if(cs_mfsv_mount(&source,&scratch.work,&volume)!=CS_MFSV_OK || volume.info.file_count!=2 ||
        cs_mfsv_find(&volume,&scratch.work,(const uint8_t *)"Start",5,&first)!=CS_MFSV_OK ||
        cs_mfsv_find(&volume,&scratch.work,(const uint8_t *)"Desk",4,&second)!=CS_MFSV_OK ||
        first.data_logical!=2437u || first.resource_logical!=17u || second.resource_logical!=513u)
        return finish(0xd0);
    cs_mfs_write_report[2]=1;
    transport.reads=transport.writes=0;
    fill((uint8_t *)&progress,sizeof progress,0xa5);fill(input,sizeof input,0xa5);
    saved=volume.map[2];volume.map[2]=0; /* Last link lies beyond this one-byte request. */
    result=cs_mfsw_fork_overwrite(&volume,&scratch.work,&first,CS_MFSV_DATA,&sink,0,1,input,1,&progress);
    volume.map[2]=saved;
    if(result!=CS_MFSV_CHAIN || transport.reads || transport.writes ||
        !filled((const uint8_t *)&progress,sizeof progress,0xa5))return finish(0xd1);
    wrong.blocks=799;wrong.write=write_one;wrong.context=&transport;
    if(cs_mfsw_fork_overwrite(&volume,&scratch.work,&first,CS_MFSV_DATA,&wrong,0,1,input,1,&progress)!=CS_MFSV_ARGUMENT ||
        cs_mfsw_fork_overwrite(&volume,&scratch.work,&first,CS_MFSV_DATA,&sink,2437,1,input,1,&progress)!=CS_MFSV_RANGE ||
        !filled((const uint8_t *)&progress,sizeof progress,0xa5) || transport.reads || transport.writes)
        return finish(0xd1);
    if(cs_mfsw_fork_overwrite(&volume,&scratch.work,&first,CS_MFSV_DATA,&sink,2437,0,input,1,&progress)!=CS_MFSV_OK ||
        progress.completed || progress.lba || progress.phase!=CS_MFSW_DONE || progress.uncertain ||
        transport.reads || transport.writes)return finish(0xd1);
    fill((uint8_t *)&progress,sizeof progress,0xa5);
    volume.info.attributes=UINT16_C(0x8000);
    result=cs_mfsw_fork_overwrite(&volume,&scratch.work,&first,CS_MFSV_RESOURCE,&sink,0,1,input,1,&progress);
    volume.info.attributes=0; /* Original fixture has no volume locks. */
    if(result!=CS_MFSV_UNAVAILABLE || transport.reads || transport.writes ||
       !filled((const uint8_t *)&progress,sizeof progress,0xa5))return finish(0xd5);
    cs_mfs_write_report[2]=2;
    if(mode==0 || mode==2 || mode==3) {
        transport.fail_write=mode==2?2u:0;transport.fail_read=mode==3?2u:0;
        result=overwrite(&sink,&first,CS_MFSV_DATA,500,1100,0x31,&progress);
        transport.fail_write=transport.fail_read=0;
        if(mode==2) {
            style=2;
            if(result!=CS_MFSV_IO || progress.completed!=12u || progress.lba!=17u ||
                progress.phase!=CS_MFSW_WRITE || progress.uncertain!=1 || transport.writes!=2u)
                return finish(0xd2);
        } else if(mode==3) {
            style=3;
            if(result!=CS_MFSV_IO || progress.completed!=1036u || progress.lba!=23u ||
                progress.phase!=CS_MFSW_READ || progress.uncertain || transport.writes!=3u)
                return finish(0xd2);
        } else {
            if(result!=CS_MFSV_OK || progress.completed!=1100u || transport.writes!=4u || transport.reads!=2u)
                return finish(0xd3);
            if(overwrite(&sink,&first,CS_MFSV_RESOURCE,3,11,0x53,&progress)!=CS_MFSV_OK ||
                overwrite(&sink,&second,CS_MFSV_RESOURCE,500,13,0x75,&progress)!=CS_MFSV_OK ||
                overwrite(&sink,&first,CS_MFSV_DATA,2400,37,0x97,&progress)!=CS_MFSV_OK)return finish(0xd3);
            saved=(uint8_t)transport.reads;
            if(overwrite(&sink,&first,CS_MFSV_DATA,0,512,0xb9,&progress)!=CS_MFSV_OK ||
                transport.reads!=saved || transport.writes!=9u)return finish(0xd3);
        }
        cs_mfs_write_report[12]=mode==0?5u:1u;
    } else style=mode==4?2u:mode==5?3u:1u;
    cs_mfs_write_report[2]=3;
    if(cs_mfs_write_report[13])return finish(0xde);
    if(style==1 && !file_operations(&sink,mode))return finish(0xd4);
    if(!verify(&first,0,CS_MFSV_DATA,style) || !verify(&first,0,CS_MFSV_RESOURCE,style) ||
        !verify(&second,1,CS_MFSV_DATA,style) || !verify(&second,1,CS_MFSV_RESOURCE,style))return finish(0xd4);
    if(cs_mfs_write_report[10]!=2967u)return finish(0xd4);
    cs_mfs_write_report[2]=4;return finish(0xba);
}
