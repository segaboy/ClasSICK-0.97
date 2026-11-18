/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "startup-files.h"
static cs_startup_workspace workspace;
static struct {uint32_t before;cs_startup_files files;uint32_t after;} guarded;
static cs_rfs_workspace resource_scratch;
volatile uint32_t cs_startup_files_report[CS_STARTUP_REPORT_WORDS];
static void fill(void *object,size_t count,uint8_t value)
{uint8_t *p=object;size_t i;for(i=0;i<count;++i)p[i]=value;}
static int filled(const void *object,size_t count,uint8_t value)
{const uint8_t *p=object;size_t i;for(i=0;i<count;++i)if(p[i]!=value)return 0;return 1;}
unsigned cs_startup_files_probe(const cs_block_device *device)
{
    cs_startup_report report;cs_startup_result result;unsigned i;
    cs_rfs_view reopened;cs_rfs_report detail;
    for(i=0;i<CS_STARTUP_REPORT_WORDS;++i)cs_startup_files_report[i]=0;
    cs_startup_files_report[0]=UINT32_C(0x53544631);
    guarded.before=UINT32_C(0x1234cdef);guarded.after=UINT32_C(0xfedc4321);
    fill(&guarded.files,sizeof guarded.files,0xa5);fill(&report,sizeof report,0);
    result=cs_startup_files_open(device,&workspace,&guarded.files,&report);
    cs_startup_files_report[1]=(uint32_t)result;
    cs_startup_files_report[2]=(uint32_t)report.stage;
    cs_startup_files_report[3]=(uint32_t)report.file;
    cs_startup_files_report[4]=(uint32_t)report.domain;
    cs_startup_files_report[5]=(uint32_t)report.status;
    cs_startup_files_report[6]=(uint32_t)report.field;
    cs_startup_files_report[7]=report.position;
    cs_startup_files_report[8]=report.opened;cs_startup_files_report[9]=report.types;
    cs_startup_files_report[10]=report.references;cs_startup_files_report[11]=report.warnings;
    if(guarded.before!=UINT32_C(0x1234cdef) || guarded.after!=UINT32_C(0xfedc4321))return 0xdf;
    cs_startup_files_report[15]=1;
    if(result!=CS_STARTUP_OK) {
        if(!filled(&guarded.files,sizeof guarded.files,0xa5))return 0xdf;
        cs_startup_files_report[13]=1;return 0xd1;
    }
    /* Destroy all temporary session controls before consuming either source. */
    fill(&workspace.candidate,sizeof workspace.candidate,0x3c);
    for(i=0;i<CS_STARTUP_FILES;++i) {
        const cs_rfs_view *published=&guarded.files.resources[i];
        if(cs_rfs_open(&published->source,&resource_scratch,&reopened,&detail)!=CS_RFS_OK ||
            reopened.type_count!=published->type_count || reopened.warnings!=published->warnings ||
            reopened.map.offset!=published->map.offset || reopened.map.length!=published->map.length)
            return 0xdf;
        ++cs_startup_files_report[12];
    }
    return 0xb8;
}
