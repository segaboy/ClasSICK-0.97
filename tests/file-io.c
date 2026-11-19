/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../personality/system1/file-io.h"
#include <stdio.h>
#include <string.h>

static uint8_t disk[800][512], before[800][512], ram[2048], saved[2048], staging[1100];
static cs_mfsv_volume volume;
static cs_mfsv_workspace work;
static cs_s1_open_fork file;
static cs_trap_frame frame;
static unsigned checks,reads,writes;
static uint32_t fail_lba;
#define CHECK(p) do { ++checks; if(!(p)){fprintf(stderr,"file I/O line %d: %s\n",__LINE__,#p);return 1;} } while(0)

static cs_block_io_result read_one(void *context,uint32_t lba,uint8_t *out)
{
    (void)context; ++reads;
    if(lba>=800) return CS_BLOCK_IO_ERROR;
    memcpy(out,disk[lba],512); return CS_BLOCK_IO_OK;
}
static cs_block_io_result write_one(void *context,uint32_t lba,const uint8_t *source)
{
    (void)context; ++writes;
    if(lba>=800) return CS_BLOCK_IO_ERROR;
    memcpy(disk[lba],source,512);
    return lba==fail_lba ? CS_BLOCK_IO_ERROR : CS_BLOCK_IO_OK;
}
static void word32(uint8_t *p,uint32_t value)
{
    p[0]=(uint8_t)(value>>24);p[1]=(uint8_t)(value>>16);p[2]=(uint8_t)(value>>8);p[3]=(uint8_t)value;
}

int main(int argc,char **argv)
{
    cs_block_device device={800,read_one,NULL};
    cs_block_writer writer={800,write_one,NULL};
    cs_guest_region region={ram,ram,0x8000,sizeof ram,CS_GUEST_READ|CS_GUEST_WRITE};
    cs_guest_map map={&region,1};
    FILE *input;
    int published;
    if(argc!=2 || (input=fopen(argv[1],"rb"))==NULL) return 1;
    CHECK(fread(disk,1,sizeof disk,input)==sizeof disk && fgetc(input)==EOF);
    CHECK(fclose(input)==0);
    memcpy(before,disk,sizeof disk);
    CHECK(cs_mfsv_mount(&device,&work,&volume)==CS_MFSV_OK);
    CHECK(cs_s1_open_named_fork(&file,&volume,&work,(const uint8_t *)"Start",5,CS_MFSV_DATA,7,&writer)==CS_MFSV_OK);
    CHECK(cs_s1_open_named_fork(&file,&volume,&work,(const uint8_t *)"NoSuch",6,CS_MFSV_DATA,8,&writer)==CS_MFSV_NOT_FOUND);
    CHECK(file.reference==7 && file.mark==0);
    frame.a[0]=0x8000;frame.d[0]=89;
    ram[25]=7;ram[45]=1; word32(ram+36,1100);word32(ram+46,500);word32(ram+32,0x8080);
    memset(ram+128,0x33,1100);
    reads=writes=0;
#define REFUSE() do { memcpy(saved,ram,sizeof ram); \
    CHECK(!cs_s1_file_io(&file,&work,&map,&frame,0xa003,staging,sizeof staging)); \
    CHECK(!reads && !writes && !file.mark && frame.d[0]==89 && memcmp(ram,saved,sizeof ram)==0); } while(0)
    frame.a[0]++; REFUSE(); frame.a[0]--;
    word32(ram+32,0x8001); REFUSE();
    word32(ram+32,0x8700); REFUSE();
    word32(ram+32,0x8080); word32(ram+46,2400); REFUSE();
    word32(ram+46,500);ram[45]=64; REFUSE();ram[45]=1;
    region.flags=CS_GUEST_READ;REFUSE();region.flags=CS_GUEST_READ|CS_GUEST_WRITE;
    fail_lba=17;
    published=cs_s1_file_io(&file,&work,&map,&frame,0xa003,staging,sizeof staging);
    CHECK(published && frame.d[0]==UINT32_C(0xffffffdc) && file.mark==512 && writes==2);
    CHECK(ram[16]==255 && ram[17]==220 && !memcmp(ram+40,"\0\0\0\014",4));
    CHECK(!memcmp(ram+46,"\0\0\002\0",4));
    memset(before[16]+500,0x33,12);memset(before[17],0x33,512);
    CHECK(!memcmp(before,disk,sizeof disk));
    word32(ram+36,UINT32_MAX);
    CHECK(cs_s1_file_io(&file,&work,&map,&frame,0xa002,staging,sizeof staging) &&
          frame.d[0]==UINT32_C(0xffffffce) && file.mark==512);
    word32(ram+36,7);word32(ram+46,UINT32_C(0xfffffc00));ram[45]=3;
    CHECK(cs_s1_file_io(&file,&work,&map,&frame,0xa002,staging,sizeof staging) &&
          frame.d[0]==UINT32_C(0xffffffd8) && file.mark==512);
    printf("File I/O: %u checks passed\n",checks);
    return 0;
}
