/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "code-stream.h"
#include "../../core/storage/startup-files.h"
#include "../../personality/system1/code-stream-load.h"
#if defined(CS_EXECUTE_FIXTURE) && defined(CS_C16_REQUIRE_16BIT)
#include "../../core/cpu/m68k.h"
static cs_trap_gateway gateway;
#endif

static cs_startup_workspace startup_work;
static cs_startup_files files;
static cs_cstream_manifest manifest;
static cs_cstream_request request;
static union {
    cs_cstream_workspace inspect;
    uint16_t words[8192]; /* Even-aligned, isolated 16 KiB body destination. */
} ram;
volatile uint32_t cs_code_stream_report[CS_CODE_STREAM_WORDS];

#ifdef CS_EXECUTE_FIXTURE
static int execute_original(uint8_t *body, const cs_loaded_segment *loaded, uint32_t *result)
{
    /* The test builder supplies only original executable media to this build. */
#ifdef CS_C16_REQUIRE_16BIT
    uint8_t stack[4] = {0, 1, 0, 0};
    cs_m68k_memory memory;
    cs_trap_frame frame = {{0},{0},0,0};
    cs_m68k_report report;
    unsigned i;
    memory.code=body;memory.code_base=0x10000;memory.code_size=loaded->size;
    memory.stack=stack;memory.stack_base=0x20000;memory.stack_size=sizeof stack;
    frame.pc=memory.code_base+loaded->entry_offset;frame.a[7]=memory.stack_base;frame.sr=0x2700;
    for (i=0;i<2;++i)
        if(cs_m68k_step(&memory,&gateway,&frame,&report)!=CS_M68K_OK)return 0;
    if(frame.pc!=memory.code_base || frame.a[7]!=memory.stack_base+4u)return 0;
    *result=frame.d[0];return 1;
#else
    typedef uint32_t (*fixture_routine)(void);
    *result=((fixture_routine)(uintptr_t)(body+(size_t)loaded->entry_offset))();
    return 1;
#endif
}
#endif

unsigned cs_code_stream_probe(const cs_block_device *device)
{
    cs_startup_report startup_report;
    cs_cstream_report inspection;
    cs_loaded_segment loaded;
    cs_cstream_result status;
    unsigned i;
    for(i=0;i<CS_CODE_STREAM_WORDS;++i)cs_code_stream_report[i]=0;
    cs_code_stream_report[0]=UINT32_C(0x43534c31);
    cs_code_stream_report[1]=1;
    if(cs_startup_files_open(device,&startup_work,&files,&startup_report)!=CS_STARTUP_OK)
        return 0xd1;
    request.source=&files.resources[CS_STARTUP_SHELL];request.maximum_source=409600;
    request.maximum_entries=8192;request.maximum_segments=64;request.context=CS_CSTREAM_ORDINARY;
    request.routine_base=CS_CSTREAM_AFTER_HEADER;
    cs_code_stream_report[1]=2;
    status=cs_code_stream_inspect(&request,&ram.inspect,sizeof ram.inspect,&manifest,&inspection);
    cs_code_stream_report[2]=(uint32_t)status;
    if(status!=CS_CSTREAM_OK)return 0xd2;
    cs_code_stream_report[1]=3;
    status=cs_code_stream_load(&manifest,0,(uint8_t *)ram.words,sizeof ram.words,&loaded);
    cs_code_stream_report[2]=(uint32_t)status;
    if(status!=CS_CSTREAM_OK)return 0xd3;
    cs_code_stream_report[3]=loaded.size;cs_code_stream_report[4]=loaded.entry_offset;
#ifdef CS_EXECUTE_FIXTURE
    {
        uint32_t answer=0;
        if(!execute_original((uint8_t *)ram.words,&loaded,&answer))return 0xd4;
        cs_code_stream_report[5]=1;cs_code_stream_report[6]=answer;
    }
#endif
    cs_code_stream_report[1]=4;return 0xb8;
}
