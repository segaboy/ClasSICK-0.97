/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Original read-only media workload. Reports counters/digests, never file bytes. */
#include "mfs-media.h"
#include "../../core/storage/mfs-volume.h"

static cs_mfsv_volume volume;
static cs_mfsv_workspace work;
static cs_mfs_entry entry;
static uint8_t output[256], staging[256];
volatile uint32_t cs_mfs_media_report[12];

static uint32_t fold(uint32_t value, uint8_t byte)
{
    return ((value << 5) | (value >> 27)) ^ byte;
}
unsigned cs_mfs_media_probe(const cs_block_device *device)
{
    cs_mfsv_cursor cursor = {0,0,0};
    cs_mfsv_result result;
    uint32_t files=0,forks=0,bytes=0,digest=0,offset,length;
    unsigned which,i;
    size_t take;
    for(i=0;i<12u;++i)cs_mfs_media_report[i]=0;
    cs_mfs_media_report[0]=UINT32_C(0x4d465356);
    result=cs_mfsv_mount(device,&work,&volume);
    cs_mfs_media_report[1]=1;cs_mfs_media_report[2]=(uint32_t)result;
    if(result!=CS_MFSV_OK)return 0xd1;
    cs_mfs_media_report[3]=volume.info.file_count;
    cs_mfs_media_report[4]=volume.info.alloc_count;
    cs_mfs_media_report[5]=volume.info.free_blocks;
    for(;;) {
        result=cs_mfsv_directory_next(&volume,&work,&cursor,&entry);
        cs_mfs_media_report[2]=(uint32_t)result;
        if(result==CS_MFSV_END)break;
        if(result!=CS_MFSV_OK)return 0xd2;
        ++files;
        for(which=0;which<2u;++which) {
            cs_mfsv_fork fork=which==0u?CS_MFSV_DATA:CS_MFSV_RESOURCE;
            length=which==0u?entry.data_logical:entry.resource_logical;
            result=cs_mfsv_fork_validate(&volume,&entry,fork);
            cs_mfs_media_report[2]=(uint32_t)result;
            if(result!=CS_MFSV_OK)return 0xd3;
            ++forks;offset=0;
            while(offset<length) {
                take=length-offset>sizeof output?sizeof output:(size_t)(length-offset);
                for(i=0;i<sizeof output;++i)output[i]=0xa5;
                result=cs_mfsv_fork_read(&volume,&work,&entry,fork,offset,take,
                    output,sizeof output,staging,sizeof staging);
                cs_mfs_media_report[2]=(uint32_t)result;
                if(result!=CS_MFSV_OK) {
                    for(i=0;i<sizeof output;++i)if(output[i]!=0xa5)return 0xdf;
                    return 0xd4;
                }
                for(i=0;i<take;++i)digest=fold(digest,output[i]);
                for(i=(unsigned)take;i<sizeof output;++i)if(output[i]!=0xa5)return 0xde;
                offset+=(uint32_t)take;bytes+=(uint32_t)take;
            }
        }
        cs_mfs_media_report[6]=files;
        cs_mfs_media_report[7]=forks;
        cs_mfs_media_report[8]=bytes;
        cs_mfs_media_report[9]=digest;
    }
    cs_mfs_media_report[1]=2;
    return 0xb7;
}
