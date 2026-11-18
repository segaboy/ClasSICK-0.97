/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Original SPEC-0121 workload. Diagnostics contain aggregate metadata only. */
#include "resource-media.h"
#include "../../formats/resourcefork/mfs-transport.h"
static cs_mfsv_volume volume;
static cs_mfsv_workspace disk_scratch;
static cs_mfs_entry file;
static cs_rfs_mfs_adapter adapter;
static cs_rfs_workspace stream_scratch;
volatile uint32_t cs_resource_media_report[CS_RESOURCE_MEDIA_WORDS];

static unsigned refusal(unsigned outcome,uint32_t ordinal,unsigned domain,
    unsigned status,unsigned field,uint32_t position)
{
    cs_resource_media_report[8]=ordinal;
    cs_resource_media_report[9]=(uint32_t)domain;
    cs_resource_media_report[10]=(uint32_t)status;
    cs_resource_media_report[11]=(uint32_t)field;
    cs_resource_media_report[12]=position;
    return outcome;
}
static void fill(void *object,size_t n,uint8_t value)
{uint8_t *p=object;size_t i;for(i=0;i<n;++i)p[i]=value;}
static int filled(const void *object,size_t n,uint8_t value)
{const uint8_t *p=object;size_t i;for(i=0;i<n;++i)if(p[i]!=value)return 0;return 1;}

unsigned cs_resource_media_probe(const cs_block_device *device)
{
    cs_mfsv_cursor cursor={0,0,0};cs_mfsv_result mfs;
    cs_rfs_source source;cs_rfs_view view;cs_rfs_report report;
    cs_rfs_result result;unsigned i;
    for(i=0;i<CS_RESOURCE_MEDIA_WORDS;++i)cs_resource_media_report[i]=0;
    cs_resource_media_report[0]=UINT32_C(0x52465031);
    cs_resource_media_report[1]=1;
    mfs=cs_mfsv_mount(device,&disk_scratch,&volume);
    if(mfs!=CS_MFSV_OK)return refusal(0xd1,0,1,(unsigned)mfs,0,0);
    cs_resource_media_report[13]=volume.info.file_count;
    for(;;) {
        cs_resource_media_report[1]=2;
        mfs=cs_mfsv_directory_next(&volume,&disk_scratch,&cursor,&file);
        if(mfs==CS_MFSV_END)break;
        if(mfs!=CS_MFSV_OK)return refusal(0xd2,cs_resource_media_report[2]+1u,1,(unsigned)mfs,0,0);
        ++cs_resource_media_report[2];
        if(file.resource_logical==0)continue;
        ++cs_resource_media_report[3];
        cs_resource_media_report[1]=3;
        mfs=cs_rfs_mfs_bind(&volume,&file,&disk_scratch,&adapter,&source);
        if(mfs!=CS_MFSV_OK)return refusal(0xd3,cs_resource_media_report[2],1,(unsigned)mfs,0,0);
        cs_resource_media_report[1]=4;
        fill(&view,sizeof view,0xa5);
        result=cs_rfs_open(&source,&stream_scratch,&view,&report);
        if(result!=CS_RFS_OK) {
            unsigned outcome=filled(&view,sizeof view,0xa5)?0xd4u:0xdfu;
            return refusal(outcome,cs_resource_media_report[2],2,(unsigned)result,
                (unsigned)report.field,report.position);
        }
        if(view.type_count>UINT32_MAX-cs_resource_media_report[5] ||
            report.references>UINT32_MAX-cs_resource_media_report[6])
            return refusal(0xde,cs_resource_media_report[2],3,1,0,0);
        ++cs_resource_media_report[4];
        cs_resource_media_report[5]+=view.type_count;
        cs_resource_media_report[6]+=report.references;
        cs_resource_media_report[7]|=view.warnings;
    }
    cs_resource_media_report[1]=5;
    return 0xb8;
}
