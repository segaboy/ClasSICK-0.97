/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../personality/system1/code-stream.h"
#include "../core/storage/startup-files.h"
#include "../probes/storage/code-stream.h"
#include <stdio.h>
#include <string.h>
static uint8_t original[409600],media[409600];
static cs_startup_workspace startup_work;static cs_startup_files startup;
static cs_cstream_workspace work;
static struct {uint8_t before[8];cs_cstream_manifest value;uint8_t after[8];} guarded;
static cs_cstream_manifest snapshot;
static cs_cstream_request request;static unsigned checks;
static unsigned calls,fail_at;static cs_block_io_result failure;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"code stream check failed: %u\n",(unsigned)__LINE__);return 1;}}while(0)
static cs_block_io_result read_one(void *context,uint32_t lba,uint8_t *out)
{
    unsigned call=calls++;(void)context;if(lba>=800)return CS_BLOCK_IO_ERROR;
    memcpy(out,media+(size_t)lba*512u,512);
    if(call==fail_at){memset(out,0x72,512);return failure;}return CS_BLOCK_IO_OK;
}
static cs_block_device device={800,read_one,NULL};
static int uniform(const void *p,size_t n,uint8_t value)
{const uint8_t *b=p;size_t i;for(i=0;i<n;++i)if(b[i]!=value)return 0;return 1;}
static void reset(void)
{memcpy(media,original,sizeof media);calls=0;fail_at=(unsigned)-1;failure=CS_BLOCK_IO_ERROR;memset(&guarded,0xa5,sizeof guarded);}
static int prepare(void)
{
    cs_startup_report report;
    CHECK(cs_startup_files_open(&device,&startup_work,&startup,&report)==CS_STARTUP_OK);
    memset(&request,0,sizeof request);request.source=&startup.resources[1];
    request.maximum_source=409600;request.maximum_entries=8192;request.maximum_segments=64;
    request.context=CS_CSTREAM_ORDINARY;calls=0;return 0;
}
static uint8_t *file_byte(uint32_t at)
{
    static const unsigned first[5]={3,8,4,9,5};
    uint32_t index=at/1024u,block=index<5?first[index]:index==65?7:index+5;
    return media+(size_t)(16u+(block-2u)*2u)*512u+(size_t)(at%1024u);
}
static void put16(uint32_t at,unsigned value)
{*file_byte(at)=(uint8_t)(value>>8);*file_byte(at+1)=(uint8_t)value;}
static void put32(uint32_t at,uint32_t value)
{put16(at,(unsigned)(value>>16));put16(at+2,(unsigned)(value&65535u));}
static int consume(void)
{
    cs_rfs_workspace cache;cs_rf_entry entry;cs_rfs_report report;
    uint8_t bytes[16],staging[16];unsigned i;
    memset(&work.candidate,0x3c,sizeof work.candidate);
    CHECK(guarded.value.source.source.context==startup.resources[1].source.context);
    CHECK(cs_rfs_find(&guarded.value.source,&cache,CS_CSTREAM_TYPE,20,&entry,&report)==CS_RFS_OK);
    CHECK(entry.data.offset==guarded.value.segments[19].resource.entry.data.offset);
    CHECK(cs_rfs_read(&guarded.value.source,&guarded.value.segments[19].resource.entry,
        CS_RFS_PAYLOAD,4,16,bytes,sizeof bytes,staging,sizeof staging,&report)==CS_RFS_OK);
    for(i=0;i<16;++i)CHECK(bytes[i]==(uint8_t)(19u*17u+i*11u));return 0;
}
int main(int argc,char **argv)
{
    FILE *input;cs_cstream_report report,before;unsigned i,j,total;
    const uint32_t code0=65265,segment1=66725,code0_ref=67259;
    CHECK(argc==4);input=fopen(argv[1],"rb");CHECK(input!=NULL);
    CHECK(fread(original,1,sizeof original,input)==sizeof original);CHECK(fgetc(input)==EOF);CHECK(fclose(input)==0);
    reset();CHECK(prepare()==0);
    CHECK(cs_code_stream_inspect(&request,&work,sizeof work,&guarded.value,&report)==CS_CSTREAM_OK);
    total=calls;CHECK(total>20);
    CHECK(guarded.value.entry_count==180 && guarded.value.segment_count==20);
    CHECK(guarded.value.above_a5==1472 && guarded.value.below_a5==256 && guarded.value.table_length==1440 && guarded.value.table_offset==32);
    CHECK(guarded.value.warnings==0 && guarded.value.reader_warnings==64);
    CHECK(guarded.value.resource_start_literal==1 && guarded.value.historical_offset_parity_unverified==1 && guarded.value.execution_authorized==0);
    CHECK(guarded.value.code0.entry.data.offset==code0 && guarded.value.code0.entry.data.length==1456);
    for(i=0;i<20;++i){CHECK(guarded.value.segments[i].first_entry==i*9 && guarded.value.segments[i].entry_count==9);
        CHECK(guarded.value.segments[i].resource.entry.id==(int32_t)i+1);
        CHECK(guarded.value.segments[i].resource.entry.data.offset==segment1+i*24 && guarded.value.segments[i].resource.entry.data.length==20);}
    CHECK(consume()==0);CHECK(uniform(guarded.before,8,0xa5) && uniform(guarded.after,8,0xa5));
    for(i=0;i<total;++i)for(j=0;j<3;++j){
        calls=0;fail_at=i;failure=j==0?CS_BLOCK_IO_ERROR:j==1?CS_BLOCK_IO_UNAVAILABLE:(cs_block_io_result)88;
        memset(&guarded,0xa5,sizeof guarded);
        CHECK(cs_code_stream_inspect(&request,&work,sizeof work,&guarded.value,&report)==CS_CSTREAM_SOURCE);
        CHECK(calls==i+1 && uniform(&guarded,sizeof guarded,0xa5));
    }
    reset();CHECK(prepare()==0);memset(&report,0x47,sizeof report);memcpy(&before,&report,sizeof report);
    CHECK(cs_code_stream_inspect(NULL,&work,sizeof work,&guarded.value,&report)==CS_CSTREAM_ARGUMENT);
    CHECK(cs_code_stream_inspect(&request,NULL,sizeof work,&guarded.value,&report)==CS_CSTREAM_ARGUMENT);
    CHECK(cs_code_stream_inspect(&request,&work,sizeof work-1,&guarded.value,&report)==CS_CSTREAM_ARGUMENT);
    CHECK(cs_code_stream_inspect(&request,&work,32768,&guarded.value,&report)==CS_CSTREAM_ARGUMENT);
    CHECK(cs_code_stream_inspect(&request,&work,sizeof work,(cs_cstream_manifest *)&work,&report)==CS_CSTREAM_ARGUMENT);
    CHECK(memcmp(&report,&before,sizeof report)==0 && calls==0 && uniform(&guarded,sizeof guarded,0xa5));
    for(i=0;i<9;++i){
        cs_cstream_request saved=request;cs_cstream_result expected=CS_CSTREAM_CONTEXT;
        if(i==0)request.context=CS_CSTREAM_UNKNOWN;
        if(i==1)request.context=CS_CSTREAM_INTERRUPT;
        if(i==2)request.active_root=1;
        if(i==3)request.active_callback=1;
        if(i==4)request.active_patch=1;
        if(i==5)request.active_manager=1;
        if(i==6){request.maximum_source=0;expected=CS_CSTREAM_ARGUMENT;}
        if(i==7){request.maximum_entries=0;expected=CS_CSTREAM_ARGUMENT;}
        if(i==8){request.maximum_segments=65;expected=CS_CSTREAM_ARGUMENT;}
        CHECK(cs_code_stream_inspect(&request,&work,sizeof work,&guarded.value,&report)==expected);
        CHECK(uniform(&guarded,sizeof guarded,0xa5));request=saved;
    }
    for(i=0;i<3;++i){cs_cstream_request saved=request;
        if(i==0)request.maximum_source=startup.resources[1].source.length-1;
        if(i==1)request.maximum_entries=179;
        if(i==2)request.maximum_segments=19;
        CHECK(cs_code_stream_inspect(&request,&work,sizeof work,&guarded.value,&report)==CS_CSTREAM_PROFILE_LIMIT);
        CHECK(uniform(&guarded,sizeof guarded,0xa5));request=saved;
    }
    request.maximum_entries=180;request.maximum_segments=20;
    CHECK(cs_code_stream_inspect(&request,&work,sizeof work,&guarded.value,&report)==CS_CSTREAM_OK);
    for(i=0;i<21;++i){
        cs_cstream_result expected=CS_CSTREAM_CODE0_FORMAT;reset();
        switch(i){
        case 0:put32(code0,1473);break;
        case 1:put32(code0+8,0);break;
        case 2:put32(code0+8,1442);break;
        case 3:put32(code0+12,0x8000);break;
        case 4:put32(code0,1470);break;
        case 5:put32(code0+8,UINT32_C(0xfffffff8));break;
        case 6:put32(code0-4,14);break;
        case 7:put16(code0+16+2,0x4e71);expected=CS_CSTREAM_ENTRY_FORM;break;
        case 8:put16(code0+16+6,0xa9f1);expected=CS_CSTREAM_ENTRY_FORM;break;
        case 9:put16(code0+16+4,2);expected=CS_CSTREAM_ENTRY_FORM;break;
        case 10:put16(code0+16+4,0);expected=CS_CSTREAM_ENTRY_FORM;break;
        case 11:put16(segment1,1);expected=CS_CSTREAM_SEGMENT_HEADER;break;
        case 12:put16(segment1+2,0);expected=CS_CSTREAM_SEGMENT_HEADER;break;
        case 13:put16(segment1+2,8);expected=CS_CSTREAM_SEGMENT_HEADER;break;
        case 14:put16(code0+16+9*8+4,1);expected=CS_CSTREAM_SEGMENT_HEADER;break;
        case 15:put16(code0+16,2);expected=CS_CSTREAM_ROUTINE_RANGE;break;
        case 16:put16(code0+16,5);expected=CS_CSTREAM_ROUTINE_RANGE;break;
        case 17:put16(code0+16,20);expected=CS_CSTREAM_ROUTINE_RANGE;break;
        case 18:put16(code0_ref,30000);expected=CS_CSTREAM_NOT_FOUND;break;
        case 19:*file_byte(code0_ref+4)=0x40;expected=CS_CSTREAM_STORAGE;break;
        default:*file_byte(code0_ref+4)=0x80;expected=CS_CSTREAM_REFERENCE;break;
        }
        CHECK(prepare()==0);
        CHECK(cs_code_stream_inspect(&request,&work,sizeof work,&guarded.value,&report)==expected);
        CHECK(uniform(&guarded,sizeof guarded,0xa5));
    }
    reset();put16(code0_ref+12,0);CHECK(prepare()==0);
    CHECK(cs_code_stream_inspect(&request,&work,sizeof work,&guarded.value,&report)==CS_CSTREAM_AMBIGUOUS);
    CHECK(uniform(&guarded,sizeof guarded,0xa5));
    reset();*file_byte(code0_ref+12+5)=0;*file_byte(code0_ref+12+6)=0;*file_byte(code0_ref+12+7)=0;
    CHECK(prepare()==0);CHECK(cs_code_stream_inspect(&request,&work,sizeof work,&guarded.value,&report)==CS_CSTREAM_PROFILE_LIMIT);
    CHECK(uniform(&guarded,sizeof guarded,0xa5));
    reset();CHECK(prepare()==0);CHECK(cs_code_stream_inspect(&request,&work,sizeof work,&guarded.value,&report)==CS_CSTREAM_OK);
    /* The same jump table cannot silently switch bases when one row fails.
     * This generated resource is 20 bytes long, with a four-byte header. */
    for(i=0;i<180;++i)put16(code0+16+i*8,2*(i%8));
    CHECK(cs_code_stream_inspect(&request,&work,sizeof work,&guarded.value,&report)==CS_CSTREAM_ROUTINE_RANGE);
    CHECK(report.range_reason==CS_CSTREAM_RANGE_HEADER && report.slot==0);
    request.routine_base=CS_CSTREAM_AFTER_HEADER;
    CHECK(cs_code_stream_inspect(&request,&work,sizeof work,&guarded.value,&report)==CS_CSTREAM_OK);
    CHECK(guarded.value.routine_base==CS_CSTREAM_AFTER_HEADER && guarded.value.resource_start_literal==0);
    CHECK(guarded.value.historical_offset_parity_unverified==1 && guarded.value.execution_authorized==0);
    memcpy(&snapshot,&guarded.value,sizeof snapshot);
    for(i=0;i<3;++i){
        put16(code0+16+8*179,i==0?1u:i==1?16u:18u);
        CHECK(cs_code_stream_inspect(&request,&work,sizeof work,&guarded.value,&report)==CS_CSTREAM_ROUTINE_RANGE);
        CHECK(report.slot==179 && report.range_reason==(i==0?CS_CSTREAM_RANGE_ODD:i==1?CS_CSTREAM_RANGE_SHORT_TAIL:CS_CSTREAM_RANGE_BEYOND));
        CHECK(memcmp(&snapshot,&guarded.value,sizeof snapshot)==0);
    }
    request.routine_base=(cs_cstream_base)2;
    CHECK(cs_code_stream_inspect(&request,&work,sizeof work,&guarded.value,&report)==CS_CSTREAM_ARGUMENT);
    CHECK(memcmp(&snapshot,&guarded.value,sizeof snapshot)==0);
    reset();CHECK(prepare()==0);CHECK(cs_code_stream_inspect(&request,&work,sizeof work,&guarded.value,&report)==CS_CSTREAM_OK);
    memcpy(&snapshot,&guarded.value,sizeof snapshot);calls=0;fail_at=total-1;
    CHECK(cs_code_stream_inspect(&request,&work,sizeof work,&guarded.value,&report)==CS_CSTREAM_SOURCE);
    CHECK(memcmp(&snapshot,&guarded.value,sizeof snapshot)==0);fail_at=(unsigned)-1;CHECK(consume()==0);
    reset();CHECK(cs_code_stream_probe(&device)==0xb8);CHECK(cs_code_stream_report[5]==180 && cs_code_stream_report[6]==20 && cs_code_stream_report[12]==1);
    /* Independently generated exact native segment capacity and one-above.
     * The exact case also has a65536-byte table, exercising uint32 offsets. */
    for(i=2;i<4;++i){
        input=fopen(argv[i],"rb");CHECK(input!=NULL);
        CHECK(fread(original,1,sizeof original,input)==sizeof original);CHECK(fgetc(input)==EOF);CHECK(fclose(input)==0);
        reset();CHECK(prepare()==0);
        CHECK(cs_code_stream_inspect(&request,&work,sizeof work,&guarded.value,&report)==
            (i==2?CS_CSTREAM_OK:CS_CSTREAM_PROFILE_LIMIT));
        if(i==2)CHECK(guarded.value.entry_count==8192 && guarded.value.segment_count==64 && guarded.value.table_length==65536);
        else CHECK(report.entries_checked==64 && report.segments_seen==64 && uniform(&guarded,sizeof guarded,0xa5));
    }
    printf("Code stream: %u checks passed\n",checks);return 0;
}
