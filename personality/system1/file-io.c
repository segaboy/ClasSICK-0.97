/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "file-io.h"

static uint16_t be16(const uint8_t *p)
{ return (uint16_t)(((uint16_t)p[0]<<8)|p[1]); }
static uint32_t be32(const uint8_t *p)
{ return ((uint32_t)be16(p)<<16)|be16(p+2); }
static void put16(uint8_t *p, uint16_t value)
{ p[0]=(uint8_t)(value>>8); p[1]=(uint8_t)value; }
static void put32(uint8_t *p, uint32_t value)
{ put16(p,(uint16_t)(value>>16)); put16(p+2,(uint16_t)value); }

cs_mfsv_result cs_s1_open_named_fork(cs_s1_open_fork *out, const cs_mfsv_volume *volume,
    cs_mfsv_workspace *work, const uint8_t *name, size_t length, cs_mfsv_fork fork,
    uint16_t reference, const cs_block_writer *writer)
{
    cs_mfs_entry entry;
    cs_mfsv_result status;
    size_t i;
    if (!out || !volume || reference==0 || reference>32767 ||
        (writer && (!writer->write || volume->device.blocks!=writer->blocks))) return CS_MFSV_ARGUMENT;
    status=cs_mfsv_find(volume,work,name,length,&entry);
    if (status!=CS_MFSV_OK) return status;
    status=cs_mfsv_fork_validate(volume,&entry,fork);
    if (status!=CS_MFSV_OK) return status;
    for(i=0;i<sizeof entry;++i) ((uint8_t *)&out->entry)[i]=((const uint8_t *)&entry)[i];
    out->volume=volume; out->writer=writer; out->fork=fork; out->reference=reference; out->mark=0;
    return CS_MFSV_OK;
}

static int publish(uint8_t *pb, cs_trap_frame *frame, int status, uint32_t count, uint32_t mark)
{
    put16(pb+16,(uint16_t)status); put32(pb+40,count); put32(pb+46,mark);
    frame->d[0]=(uint32_t)(int32_t)status;
    return 1;
}

int cs_s1_file_io(cs_s1_open_fork *file, cs_mfsv_workspace *work,
    const cs_guest_map *map, cs_trap_frame *frame, uint16_t word, uint8_t *staging, size_t capacity)
{
    cs_guest_view parameter,buffer;
    uint8_t *pb;
    uint32_t start,delta,logical,address,requested,actual=0,pb_address;
    uint16_t mode;
    cs_mfsv_result io;
    size_t i;
    int status=0;
    if (!file || !file->volume || !work || !map || !frame || !staging ||
        (word!=0xa002 && word!=0xa003) || capacity>CS_BLOCK_NATIVE_LIMIT ||
        cs_guest_validate(map)!=CS_GUEST_OK) return 0;
    for(i=0;i<map->count;++i) if(map->regions[i].size>CS_BLOCK_NATIVE_LIMIT) return 0;
    pb_address=frame->a[0];
    if ((pb_address&1u) || cs_guest_window(map,pb_address,50,CS_GUEST_WRITE,&parameter)!=CS_GUEST_OK) return 0;
    pb=parameter.writable;
    mode=be16(pb+44);
    if(mode>3) return 0;
    if(be16(pb+24)!=file->reference) return publish(pb,frame,-51,0,file->mark);
    requested=be32(pb+36);
    if(requested>INT32_MAX) return publish(pb,frame,-50,0,file->mark);
    if(requested>CS_BLOCK_NATIVE_LIMIT) return 0;
    logical=file->fork==CS_MFSV_DATA ? file->entry.data_logical : file->entry.resource_logical;
    if(logical>INT32_MAX || file->mark>logical) return 0;
    start=mode==1 ? 0 : mode==2 ? logical : file->mark;
    delta=mode ? be32(pb+46) : 0;
    if(delta>INT32_MAX) {
        uint32_t magnitude=0u-delta;
        if(magnitude>start) return publish(pb,frame,-40,0,file->mark);
        start-=magnitude;
    } else {
        if(delta>logical-start) return 0;
        start+=delta;
    }
    actual=requested;
    if(actual>logical-start) {
        if(word==0xa003) return 0;
        actual=logical-start; status=-39;
    }
    if(word==0xa003) {
        if(!file->writer) return publish(pb,frame,-61,0,file->mark);
        if(file->volume->info.attributes&0x80u) return publish(pb,frame,-44,0,file->mark);
        if(file->volume->info.attributes&0x8000u) return publish(pb,frame,-46,0,file->mark);
        if(file->entry.flags&1u) return publish(pb,frame,-45,0,file->mark);
    }
    if(actual) {
        address=be32(pb+32);
        if(cs_guest_window(map,address,actual,word==0xa002 ? CS_GUEST_WRITE : CS_GUEST_READ,&buffer)!=CS_GUEST_OK ||
           (address<pb_address+50 && pb_address<address+actual)) return 0;
        if(word==0xa002) {
            if(actual>capacity) return 0;
            io=cs_mfsv_fork_read(file->volume,work,&file->entry,file->fork,start,(size_t)actual,
                buffer.writable,(size_t)actual,staging,capacity);
            if(io!=CS_MFSV_OK) { actual=0; status=-36; }
        } else {
            cs_mfsw_progress progress;
            progress.completed=0;
            io=cs_mfsw_fork_overwrite(file->volume,work,&file->entry,file->fork,file->writer,
                start,(size_t)actual,buffer.bytes,(size_t)actual,&progress);
            if(io!=CS_MFSV_OK) { actual=progress.completed; status=-36; }
        }
    }
    file->mark=start+actual;
    return publish(pb,frame,status,actual,file->mark);
}

int cs_s1_file_step(cs_s1_open_fork *file, cs_mfsv_workspace *work,
    const cs_guest_map *map, cs_trap_frame *frame, uint8_t *staging, size_t capacity)
{
    uint32_t opcode;
    size_t i;
    if(!frame || cs_guest_validate(map)!=CS_GUEST_OK) return 0;
    for(i=0;i<map->count;++i) if(map->regions[i].size>CS_BLOCK_NATIVE_LIMIT) return 0;
    if(cs_guest_read(map,frame->pc,2,CS_GUEST_EXECUTE|CS_GUEST_READ,&opcode)!=CS_GUEST_OK ||
       !cs_s1_file_io(file,work,map,frame,(uint16_t)opcode,staging,capacity)) return 0;
    frame->pc+=2u;
    return 1;
}
