/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../personality/system1/code-stream-load.h"
#include "../core/storage/startup-files.h"
#include <stdio.h>
#include <string.h>

static uint8_t disk[409600];
static cs_startup_workspace startup_work;
static cs_startup_files files;
static cs_cstream_workspace work;
static cs_cstream_manifest manifest;
static unsigned checks;
#define CHECK(x) do { ++checks; if (!(x)) { fprintf(stderr,"load check failed at %d\n",__LINE__); return 1; } } while(0)
static cs_block_io_result block_read(void *ctx,uint32_t block,uint8_t *out)
{
    (void)ctx;
    if(block>=800)return CS_BLOCK_IO_ERROR;
    memcpy(out,disk+block*512u,512);return CS_BLOCK_IO_OK;
}
typedef struct {cs_rfs_source source;unsigned calls, fail_at;} fixture_reader;
static cs_rfs_io read_at(void *ctx,uint32_t at,uint16_t count,uint8_t *out)
{
    fixture_reader *reader=ctx;
    if(++reader->calls==reader->fail_at){memset(out,0xed,count);return CS_RFS_IO_ERROR;}
    return reader->source.read(reader->source.context,at,count,out);
}
static int filled(const uint8_t *memory,size_t size,uint8_t value)
{
    size_t i;for(i=0;i<size;++i)if(memory[i]!=value)return 0;return 1;
}
int main(int argc,char **argv)
{
    FILE *input;cs_block_device device={800,block_read,NULL};
    cs_startup_report sr;cs_cstream_report report;
    cs_cstream_request request={0};cs_rfs_view source;fixture_reader reader={0};
    struct {uint8_t before[8],body[620],after[8];} guarded;
    cs_loaded_segment published={99,88};
    unsigned i;
    CHECK(argc==2);input=fopen(argv[1],"rb");CHECK(input!=NULL);
    CHECK(fread(disk,1,sizeof disk,input)==sizeof disk && fgetc(input)==EOF && fclose(input)==0);
    CHECK(cs_startup_files_open(&device,&startup_work,&files,&sr)==CS_STARTUP_OK);
    source=files.resources[CS_STARTUP_SHELL];reader.source=source.source;
    source.source.context=&reader;source.source.read=read_at;
    request.source=&source;request.maximum_source=409600;request.maximum_entries=8192;
    request.maximum_segments=64;request.context=CS_CSTREAM_ORDINARY;request.routine_base=CS_CSTREAM_AFTER_HEADER;
    CHECK(cs_code_stream_inspect(&request,&work,sizeof work,&manifest,&report)==CS_CSTREAM_OK);
    memset(&work,0xab,sizeof work);
    memset(&guarded,0xa5,sizeof guarded);
    CHECK(cs_code_stream_load(&manifest,0,guarded.body,619,&published)==CS_CSTREAM_PROFILE_LIMIT);
    CHECK(cs_code_stream_load(&manifest,180,guarded.body,620,&published)==CS_CSTREAM_ARGUMENT);
    CHECK(cs_code_stream_load(&manifest,0,(uint8_t *)&manifest,620,&published)==CS_CSTREAM_ARGUMENT);
    CHECK(filled(guarded.body,620,0xa5));
    for(i=1;i<=4;++i){
        reader.calls=0;reader.fail_at=i;memset(guarded.body,0xa5,620);
        CHECK(cs_code_stream_load(&manifest,0,guarded.body,620,&published)==CS_CSTREAM_SOURCE);
        CHECK(reader.calls==i && filled(guarded.body,620,i==1?0xa5:0));
        CHECK(filled(guarded.before,8,0xa5) && filled(guarded.after,8,0xa5));
        CHECK(published.size==99 && published.entry_offset==88);
    }
    reader.calls=0;reader.fail_at=0;
    CHECK(cs_code_stream_load(&manifest,0,guarded.body,620,&published)==CS_CSTREAM_OK);
    CHECK(published.size==620 && published.entry_offset==2 && reader.calls==4);
    CHECK(memcmp(guarded.body,"\116\161\160\045\116\165",6)==0 && filled(guarded.body+6,614,0));
    CHECK(filled(guarded.before,8,0xa5) && filled(guarded.after,8,0xa5));
    printf("Code stream load: %u checks passed\n",checks);return 0;
}
