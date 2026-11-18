/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../core/storage/startup-files.h"
#include "../probes/storage/startup-files.h"
#include <stdio.h>
#include <string.h>
static uint8_t original[409600],media[409600];
static cs_startup_workspace work;
static struct {uint8_t before[8];cs_startup_files files;uint8_t after[8];} guarded;
static cs_startup_files snapshot;
static unsigned checks;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"startup check failed: %u\n",(unsigned)__LINE__);return 1;}}while(0)
typedef struct {unsigned calls,fail_at;cs_block_io_result failure;} provider;
static provider io;
static cs_block_io_result read_one(void *context,uint32_t lba,uint8_t *out)
{
    provider *p=context;unsigned call=p->calls++;
    if(lba>=800)return CS_BLOCK_IO_ERROR;
    memcpy(out,media+(size_t)lba*512u,512);
    if(call==p->fail_at){memset(out,0x72,512);return p->failure;}
    return CS_BLOCK_IO_OK;
}
static cs_block_device device={800,read_one,&io};
static int uniform(const void *object,size_t count,uint8_t value)
{const uint8_t *p=object;size_t i;for(i=0;i<count;++i)if(p[i]!=value)return 0;return 1;}
static void reset(void)
{memcpy(media,original,sizeof media);io.calls=0;io.fail_at=(unsigned)-1;io.failure=CS_BLOCK_IO_ERROR;memset(&guarded,0xa5,sizeof guarded);}
static int sources(void)
{
    cs_rfs_workspace cache;cs_rfs_view reopened;cs_rfs_report report;
    cs_rf_entry entry;uint8_t output[900],staging[900];unsigned i,j;
    memset(&work.candidate,0x3c,sizeof work.candidate);
    for(i=0;i<2;++i){
        const cs_rfs_view *view=&guarded.files.resources[i];
        CHECK(guarded.files.adapters[i].volume==&guarded.files.volume);
        CHECK(guarded.files.adapters[i].entry==&guarded.files.files[i]);
        CHECK(view->source.context==&guarded.files.adapters[i]);
        CHECK(cs_rfs_open(&view->source,&cache,&reopened,&report)==CS_RFS_OK);
        CHECK(reopened.type_count==(i?4u:2u));
        CHECK(cs_rfs_find(view,&cache,i?UINT32_C(0x4c4f4e47):UINT32_C(0x4f574e31),
                         i?1:32767,&entry,&report)==CS_RFS_OK);
        CHECK(entry.data.length==(i?256u:900u));
        memset(output,0xa5,sizeof output);
        CHECK(cs_rfs_read(view,&entry,CS_RFS_PAYLOAD,0,(size_t)entry.data.length,
              output,sizeof output,staging,sizeof staging,&report)==CS_RFS_OK);
        for(j=0;j<entry.data.length;++j)CHECK(output[j]==(uint8_t)(j*29u+(j>>8)+(i?103u:37u)));
        CHECK(uniform(output+entry.data.length,sizeof output-(size_t)entry.data.length,0xa5));
    }
    return 0;
}
int main(int argc,char **argv)
{
    FILE *input;cs_startup_report report,report_copy;unsigned i,j,total;
    cs_startup_result result;
    CHECK(argc==2);input=fopen(argv[1],"rb");CHECK(input!=NULL);
    CHECK(fread(original,1,sizeof original,input)==sizeof original);CHECK(fgetc(input)==EOF);CHECK(fclose(input)==0);
    reset();CHECK(cs_startup_files_open(&device,&work,&guarded.files,&report)==CS_STARTUP_OK);
    total=io.calls;CHECK(total>20);
    CHECK(report.stage==CS_STARTUP_COMPLETE && report.file==2 && report.domain==0 && report.status==0);
    CHECK(report.opened==2 && report.types==6 && report.references==8 && report.warnings==127);
    CHECK(guarded.files.header.version==18 && guarded.files.header.heap_128k==17152);
    CHECK(guarded.files.files[0].file_id==10 && guarded.files.files[1].file_id==11);
    CHECK(guarded.files.files[1].resource_logical==66438);
    CHECK(sources()==0);
    CHECK(uniform(guarded.before,8,0xa5) && uniform(guarded.after,8,0xa5));
    /* Every provider read, including late second-fork metadata, can fail after
     * writing its whole destination. No session byte can then be published. */
    for(i=0;i<total;++i)for(j=0;j<3;++j){
        reset();io.fail_at=i;io.failure=j==0?CS_BLOCK_IO_ERROR:j==1?CS_BLOCK_IO_UNAVAILABLE:(cs_block_io_result)88;
        CHECK(cs_startup_files_open(&device,&work,&guarded.files,&report)==CS_STARTUP_REFUSED);
        CHECK(io.calls==i+1 && report.status!=0 && report.stage!=CS_STARTUP_COMPLETE);
        CHECK(uniform(&guarded,sizeof guarded,0xa5));
    }
    reset();memset(&report,0x47,sizeof report);report_copy=report;
    CHECK(cs_startup_files_open(NULL,&work,&guarded.files,&report)==CS_STARTUP_ARGUMENT);
    CHECK(cs_startup_files_open(&device,NULL,&guarded.files,&report)==CS_STARTUP_ARGUMENT);
    CHECK(cs_startup_files_open(&device,&work,NULL,&report)==CS_STARTUP_ARGUMENT);
    CHECK(cs_startup_files_open(&device,&work,&guarded.files,NULL)==CS_STARTUP_ARGUMENT);
    CHECK(cs_startup_files_open(&device,&work,(cs_startup_files *)&work,&report)==CS_STARTUP_ARGUMENT);
    CHECK(memcmp(&report,&report_copy,sizeof report)==0 && io.calls==0 && uniform(&guarded,sizeof guarded,0xa5));
    device.blocks=0;CHECK(cs_startup_files_open(&device,&work,&guarded.files,&report)==CS_STARTUP_ARGUMENT);device.blocks=800;
    device.read=NULL;CHECK(cs_startup_files_open(&device,&work,&guarded.files,&report)==CS_STARTUP_ARGUMENT);device.read=read_one;
    CHECK(memcmp(&report,&report_copy,sizeof report)==0);
    for(i=0;i<8;++i){
        reset();
        switch(i){
        case 0:media[0]=0;break;
        case 1:media[106]=16;break;
        case 2:media[1024]=0;break;
        case 3:media[11]='o';break; /* exact counted lookup, no case folding */
        case 4:media[27]='X';break; /* missing Shell after System opened */
        case 5:media[4*512+56+38]=0xff;break; /* physical fork length rejects binding */
        case 6:media[18*512+4]=0xff;break; /* second fork map offset outside fork */
        default:media[26]=0;break; /* exact empty-name entry has an empty resource fork */
        }
        result=cs_startup_files_open(&device,&work,&guarded.files,&report);
        CHECK(result==CS_STARTUP_REFUSED && uniform(&guarded,sizeof guarded,0xa5));
        if(i<2)CHECK(report.stage==CS_STARTUP_HEADER && report.domain==CS_STARTUP_BOOT);
        if(i==2)CHECK(report.stage==CS_STARTUP_MOUNT);
        if(i==3 || i==4)CHECK(report.stage==CS_STARTUP_FIND && report.status==CS_MFSV_NOT_FOUND);
        if(i==5)CHECK(report.stage==CS_STARTUP_BIND && report.file==1);
        if(i>=6)CHECK(report.stage==CS_STARTUP_OPEN && report.file==1 && report.domain==CS_STARTUP_RESOURCE);
        if(i>=4)CHECK(report.opened==1);
    }
    reset();memcpy(media+26,media+10,16);
    /* A later duplicate also matches; existing first-match selection wins. */
    media[4*512+56+50]=5;memcpy(media+4*512+56+51,media+11,5);
    CHECK(cs_startup_files_open(&device,&work,&guarded.files,&report)==CS_STARTUP_OK);
    CHECK(guarded.files.files[0].file_id==10 && guarded.files.files[1].file_id==10);
    CHECK(report.types==4 && report.references==6 && report.warnings==30);
    reset();CHECK(cs_startup_files_open(&device,&work,&guarded.files,&report)==CS_STARTUP_OK);
    memcpy(&snapshot,&guarded.files,sizeof snapshot);io.calls=0;io.fail_at=total-1;
    CHECK(cs_startup_files_open(&device,&work,&guarded.files,&report)==CS_STARTUP_REFUSED);
    CHECK(memcmp(&snapshot,&guarded.files,sizeof snapshot)==0);
    io.fail_at=(unsigned)-1;CHECK(sources()==0); /* same immutable medium, prior session still live */
    reset();CHECK(cs_startup_files_probe(&device)==0xb8);
    CHECK(cs_startup_files_report[12]==2 && cs_startup_files_report[15]==1);
    printf("Startup files: %u checks passed\n",checks);return 0;
}
