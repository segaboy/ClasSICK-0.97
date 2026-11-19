/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "code-stream.h"
#include "../../core/storage/startup-files.h"
#include "../../personality/system1/code-stream.h"
static cs_startup_workspace startup_work;
static cs_startup_files startup;
static cs_cstream_workspace work;
#ifndef CS_PROBE_ROUTINE_BASE
#define CS_PROBE_ROUTINE_BASE CS_CSTREAM_RESOURCE_START
#endif
static struct {uint32_t before;cs_cstream_manifest value;uint32_t after;} guarded;
volatile uint32_t cs_code_stream_report[CS_CODE_STREAM_WORDS];
static void fill(void *object,size_t bytes,uint8_t value)
{uint8_t *p=object;size_t i;for(i=0;i<bytes;++i)p[i]=value;}
static int filled(const void *object,size_t bytes,uint8_t value)
{const uint8_t *p=object;size_t i;for(i=0;i<bytes;++i)if(p[i]!=value)return 0;return 1;}
unsigned cs_code_stream_probe(const cs_block_device *device)
{
    cs_startup_report start_report;cs_startup_result start_result;
    cs_cstream_request request;cs_cstream_report report;cs_cstream_result result;
    cs_rf_entry code0;cs_rfs_report detail;unsigned i;
    for(i=0;i<CS_CODE_STREAM_WORDS;++i)cs_code_stream_report[i]=0;
    cs_code_stream_report[0]=UINT32_C(0x43535431);
    guarded.before=UINT32_C(0x1234cdef);guarded.after=UINT32_C(0xfedc4321);
    fill(&guarded.value,sizeof guarded.value,0xa5);
    start_result=cs_startup_files_open(device,&startup_work,&startup,&start_report);
    if(start_result!=CS_STARTUP_OK) {
        if(guarded.before!=UINT32_C(0x1234cdef) || guarded.after!=UINT32_C(0xfedc4321) ||
            !filled(&guarded.value,sizeof guarded.value,0xa5))return 0xdf;
        cs_code_stream_report[1]=1;cs_code_stream_report[2]=(uint32_t)start_result;
        cs_code_stream_report[3]=(uint32_t)start_report.stage;
        cs_code_stream_report[4]=(uint32_t)start_report.status;
        cs_code_stream_report[13]=1;cs_code_stream_report[15]=1;return 0xd1;
    }
    fill(&startup_work.candidate,sizeof startup_work.candidate,0x3c);
    fill(&request,sizeof request,0);request.source=&startup.resources[CS_STARTUP_SHELL];
    request.maximum_source=UINT32_C(409600);request.maximum_entries=8192;
    request.maximum_segments=CS_CSTREAM_SEGMENTS;request.context=CS_CSTREAM_ORDINARY;
    request.routine_base=CS_PROBE_ROUTINE_BASE;
    result=cs_code_stream_inspect(&request,&work,sizeof work,&guarded.value,&report);
    cs_code_stream_report[1]=2;cs_code_stream_report[2]=(uint32_t)result;
    cs_code_stream_report[3]=(uint32_t)report.field;
    cs_code_stream_report[4]=report.position_valid?report.position:0;
    cs_code_stream_report[5]=report.entries_checked;cs_code_stream_report[6]=report.segments_seen;
    if(guarded.before!=UINT32_C(0x1234cdef) || guarded.after!=UINT32_C(0xfedc4321))return 0xdf;
    cs_code_stream_report[15]=1;
    if(result!=CS_CSTREAM_OK) {
        if(!filled(&guarded.value,sizeof guarded.value,0xa5))return 0xdf;
        if(result==CS_CSTREAM_ROUTINE_RANGE) {
            cs_code_stream_report[7]=(uint32_t)report.range_reason;
            cs_code_stream_report[8]=report.slot;
        }
        cs_code_stream_report[13]=1;return 0xd2;
    }
    cs_code_stream_report[3]=0;cs_code_stream_report[4]=0;
    cs_code_stream_report[7]=guarded.value.above_a5;cs_code_stream_report[8]=guarded.value.below_a5;
    cs_code_stream_report[9]=guarded.value.table_length;cs_code_stream_report[10]=guarded.value.table_offset;
    cs_code_stream_report[11]=guarded.value.warnings|(guarded.value.reader_warnings<<8);
    fill(&work.candidate,sizeof work.candidate,0x3c);
    if(cs_rfs_find(&guarded.value.source,&work.resource,CS_CSTREAM_TYPE,0,&code0,&detail)!=CS_RFS_OK ||
        code0.data.offset!=guarded.value.code0.entry.data.offset ||
        code0.data.length!=guarded.value.code0.entry.data.length ||
        cs_rfs_read(&guarded.value.source,&guarded.value.code0.entry,CS_RFS_PAYLOAD,0,16,
            work.bytes,sizeof work.bytes,work.staging,sizeof work.staging,&detail)!=CS_RFS_OK)
        return 0xdf;
    cs_code_stream_report[12]=1;
    return 0xb8;
}
