/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../core/storage/mfs-read.h"
#include <stdio.h>
#include <string.h>

static unsigned checks;
#define CHECK(x) do { ++checks;if(!(x)){fprintf(stderr,"MFS check failed at line %u\n",(unsigned)__LINE__);return 1;} } while(0)
static void put16(uint8_t *p,uint16_t x){p[0]=(uint8_t)(x>>8);p[1]=(uint8_t)x;}
static void put32(uint8_t *p,uint32_t x){p[0]=(uint8_t)(x>>24);p[1]=(uint8_t)(x>>16);p[2]=(uint8_t)(x>>8);p[3]=(uint8_t)x;}
static int uniform(const void *p,size_t length,uint8_t value)
{
    const uint8_t *b=p;size_t i;for(i=0;i<length;++i)if(b[i]!=value)return 0;return 1;
}
static void header(uint8_t *b)
{
    memset(b,0x6c,512);
    put16(b,0xd2d7);put32(b+2,UINT32_C(0x12345678));put32(b+6,UINT32_C(0x89abcdef));
    put16(b+10,0xc081);put16(b+12,0x1234);put16(b+14,4);put16(b+16,1);
    put16(b+18,4);put32(b+20,1024);put32(b+24,UINT32_C(0x87654321));
    put16(b+28,4);put32(b+30,UINT32_C(0xfedcba98));put16(b+34,3);
    b[36]=3;b[37]='N';b[38]=0;b[39]=0xfe;
}
static void entry(uint8_t *b,unsigned offset,unsigned n)
{
    uint8_t *p=b+offset;unsigned i;
    p[0]=0xc1;p[1]=0xa7;for(i=0;i<16;++i)p[2+i]=(uint8_t)(0x40u+i);
    put32(p+18,UINT32_C(0x12345678));put16(p+22,0x3456);
    put32(p+24,UINT32_C(0xf1234567));put32(p+28,0);
    put16(p+32,0);put32(p+34,0);put32(p+38,UINT32_C(0x87654321));
    put32(p+42,UINT32_C(0x10203040));put32(p+46,UINT32_C(0x50607080));
    p[50]=(uint8_t)n;for(i=0;i<n;++i)p[51+i]=(uint8_t)(i^0xa5u);
}
typedef struct {const uint8_t *block;unsigned calls;uint32_t lba;cs_block_io_result result;} source;
static cs_block_io_result read_one(void *ctx,uint32_t lba,uint8_t *out)
{
    source *s=ctx;unsigned i,n=s->result==CS_BLOCK_IO_OK?512u:17u;
    ++s->calls;s->lba=lba;for(i=0;i<n;++i)out[i]=s->block[i];return s->result;
}
int main(void)
{
    /* An odd native input address proves bytewise decoding; guard bytes and
     * snapshots independently check that raw source bytes never change. */
    uint8_t storage[516],original[512];uint8_t *b=storage+1;
    struct {uint8_t before[8];cs_mfs_volume value;uint8_t after[8];} v;
    struct {uint8_t before[8];cs_mfs_entry value;uint8_t after[8];} e;
    cs_mfs_workspace work;
    source s;
    cs_block_device dev={32,read_one,&s};
    unsigned i,n,count;
    const uint32_t units[]={1,2,3,127,65535,UINT32_C(8388607)};
    memset(storage,0xb6,sizeof storage);header(b);memcpy(original,b,512);
    memset(&v,0xa5,sizeof v);
    CHECK(cs_mfs_header_decode(b,512,32,&v.value)==CS_MFS_OK);
    CHECK(v.value.created==UINT32_C(0x12345678) && v.value.backup==UINT32_C(0x89abcdef));
    CHECK(v.value.attributes==0xc081 && v.value.file_count==0x1234);
    CHECK(v.value.dir_first==4 && v.value.dir_blocks==1 && v.value.alloc_count==4);
    CHECK(v.value.alloc_bytes==1024 && v.value.clump_bytes==UINT32_C(0x87654321));
    CHECK(v.value.alloc_first==4 && v.value.next_file==UINT32_C(0xfedcba98) && v.value.free_blocks==3);
    CHECK(v.value.name_length==3 && v.value.name[0]=='N' && v.value.name[1]==0 && v.value.name[2]==0xfe);
    CHECK(uniform(v.value.name+3,24,0));CHECK(memcmp(b,original,512)==0);
    CHECK(uniform(v.before,8,0xa5) && uniform(v.after,8,0xa5));
    for(i=0;i<2;++i){put16(b+10,(uint16_t)(i?0x8000u:0x0080u));CHECK(cs_mfs_header_decode(b,512,32,&v.value)==CS_MFS_OK);CHECK(v.value.attributes==(i?0x8000u:0x0080u));}
    for(n=0;n<=27;++n){b[36]=(uint8_t)n;CHECK(cs_mfs_header_decode(b,512,32,&v.value)==CS_MFS_OK);CHECK(v.value.name_length==n);CHECK(memcmp(v.value.name,b+37,n)==0);CHECK(uniform(v.value.name+n,27-n,0));}

#define VFAIL(expr,expected) do{memset(&v.value,0xa5,sizeof v.value);CHECK((expr)==(expected));CHECK(uniform(&v.value,sizeof v.value,0xa5));}while(0)
#define VH(blocks) cs_mfs_header_decode(b,512,(blocks),&v.value)
    VFAIL(cs_mfs_header_decode(NULL,512,32,&v.value),CS_MFS_ARGUMENT);
    CHECK(cs_mfs_header_decode(b,512,32,NULL)==CS_MFS_ARGUMENT);
    VFAIL(cs_mfs_header_decode(b,511,32,&v.value),CS_MFS_ARGUMENT);
    VFAIL(cs_mfs_header_decode(b,513,32,&v.value),CS_MFS_ARGUMENT);
    VFAIL(VH(3),CS_MFS_GEOMETRY);VFAIL(VH(0),CS_MFS_GEOMETRY);
    header(b);b[0]=0;VFAIL(VH(32),CS_MFS_SIGNATURE);
    header(b);b[36]=28;VFAIL(VH(32),CS_MFS_GEOMETRY);
#define BAD16(at,value) do{header(b);put16(b+(at),(value));VFAIL(VH(32),CS_MFS_GEOMETRY);}while(0)
    BAD16(14,3);BAD16(14,32);BAD16(16,0);BAD16(16,29);
    BAD16(18,0);BAD16(18,641);BAD16(28,3);BAD16(28,32);BAD16(34,5);
    header(b);put32(b+20,0);VFAIL(VH(32),CS_MFS_GEOMETRY);
    header(b);put32(b+20,513);VFAIL(VH(32),CS_MFS_GEOMETRY);
    /* Independent 64-bit product oracle for the <=640-count shift/add guard.
     * Synthetic uint32 device sizes need no volume-sized memory allocation. */
    for(count=1;count<=640;++count)for(i=0;i<sizeof units/sizeof units[0];++i)for(n=0;n<3;++n) {
        uint64_t product=(uint64_t)count*units[i];
        uint64_t candidate=product+4;
        uint32_t blocks;
        cs_mfs_result expected;
        if(n==0)--candidate;else if(n==2)++candidate;
        blocks=candidate>UINT32_MAX?UINT32_MAX:(uint32_t)candidate;
        expected=product<=(uint64_t)blocks-4?CS_MFS_OK:CS_MFS_GEOMETRY;
        header(b);put16(b+18,(uint16_t)count);put16(b+34,0);put32(b+20,units[i]<<9);
        memset(&v.value,0xa5,sizeof v.value);
        CHECK(VH(blocks)==expected);
        if(expected!=CS_MFS_OK)CHECK(uniform(&v.value,sizeof v.value,0xa5));
        else CHECK(v.value.alloc_count==count && v.value.alloc_bytes==(units[i]<<9));
    }
    header(b);s.block=b;s.calls=0;s.lba=0;s.result=CS_BLOCK_IO_OK;
    CHECK(cs_mfs_header_read(&dev,&work,&v.value)==CS_MFS_OK && s.calls==1 && s.lba==2);
    CHECK(v.value.created==UINT32_C(0x12345678));
    for(i=0;i<3;++i){s.calls=0;s.result=i==0?CS_BLOCK_IO_UNAVAILABLE:i==1?CS_BLOCK_IO_ERROR:(cs_block_io_result)77;
        VFAIL(cs_mfs_header_read(&dev,&work,&v.value),i==0?CS_MFS_UNAVAILABLE:i==1?CS_MFS_IO:CS_MFS_PROVIDER);CHECK(s.calls==1 && s.lba==2);}
    s.calls=0;dev.blocks=3;VFAIL(cs_mfs_header_read(&dev,&work,&v.value),CS_MFS_GEOMETRY);CHECK(s.calls==0);
    dev.blocks=0;VFAIL(cs_mfs_header_read(&dev,&work,&v.value),CS_MFS_ARGUMENT);CHECK(s.calls==0);
    dev.blocks=32;dev.read=NULL;VFAIL(cs_mfs_header_read(&dev,&work,&v.value),CS_MFS_ARGUMENT);CHECK(s.calls==0);
    dev.read=read_one;VFAIL(cs_mfs_header_read(NULL,&work,&v.value),CS_MFS_ARGUMENT);
    VFAIL(cs_mfs_header_read(&dev,NULL,&v.value),CS_MFS_ARGUMENT);
    CHECK(cs_mfs_header_read(&dev,&work,NULL)==CS_MFS_ARGUMENT && s.calls==0);
    s.result=CS_BLOCK_IO_OK;b[0]=0;VFAIL(cs_mfs_header_read(&dev,&work,&v.value),CS_MFS_SIGNATURE);CHECK(s.calls==1);

    memset(b,0x6c,512);entry(b,100,3);b[152]=0;b[153]=0xfe;memcpy(original,b,512);
    memset(&e,0xa5,sizeof e);
    CHECK(cs_mfs_entry_decode(b,512,100,&e.value)==CS_MFS_OK);
    CHECK(e.value.offset==100 && e.value.extent==54 && e.value.flags==0xc1 && e.value.version==0xa7);
    for(i=0;i<16;++i)CHECK(e.value.finder[i]==(uint8_t)(0x40u+i));
    CHECK(e.value.file_id==UINT32_C(0x12345678) && e.value.data_start==0x3456);
    CHECK(e.value.data_logical==UINT32_C(0xf1234567) && e.value.data_physical==0);
    CHECK(e.value.resource_start==0 && e.value.resource_logical==0 && e.value.resource_physical==UINT32_C(0x87654321));
    CHECK(e.value.created==UINT32_C(0x10203040) && e.value.modified==UINT32_C(0x50607080));
    CHECK(e.value.name_length==3 && e.value.name[0]==0xa5 && e.value.name[1]==0 && e.value.name[2]==0xfe);
    CHECK(uniform(e.value.name+3,252,0) && memcmp(b,original,512)==0);
    CHECK(uniform(e.before,8,0xa5) && uniform(e.after,8,0xa5));
    for(n=0;n<=255;++n){unsigned extent=(52u+n)&~1u;unsigned offset=512u-extent;
        memset(b,0x6c,512);entry(b,offset,n);memcpy(original,b,512);
        CHECK(cs_mfs_entry_decode(b,512,(uint16_t)offset,&e.value)==CS_MFS_OK);
        CHECK(e.value.extent==extent && e.value.offset==offset && e.value.name_length==n);
        CHECK(memcmp(e.value.name,b+offset+51,n)==0 && uniform(e.value.name+n,255-n,0));
        CHECK(memcmp(b,original,512)==0);}
#define EFAIL(expr,expected) do{memset(&e.value,0xa5,sizeof e.value);CHECK((expr)==(expected));CHECK(uniform(&e.value,sizeof e.value,0xa5));}while(0)
    EFAIL(cs_mfs_entry_decode(NULL,512,0,&e.value),CS_MFS_ARGUMENT);
    CHECK(cs_mfs_entry_decode(b,512,0,NULL)==CS_MFS_ARGUMENT);
    EFAIL(cs_mfs_entry_decode(b,511,0,&e.value),CS_MFS_ARGUMENT);
    EFAIL(cs_mfs_entry_decode(b,513,0,&e.value),CS_MFS_ARGUMENT);
    EFAIL(cs_mfs_entry_decode(b,512,1,&e.value),CS_MFS_ARGUMENT);
    EFAIL(cs_mfs_entry_decode(b,512,512,&e.value),CS_MFS_ENTRY_BOUNDS);
    EFAIL(cs_mfs_entry_decode(b,512,65534,&e.value),CS_MFS_ENTRY_BOUNDS);
    b[510]=0x7f;EFAIL(cs_mfs_entry_decode(b,512,510,&e.value),CS_MFS_INACTIVE);
    b[510]=0x80;EFAIL(cs_mfs_entry_decode(b,512,510,&e.value),CS_MFS_ENTRY_BOUNDS);
    memset(b,0x6c,512);entry(b,208,0);b[258]=255;
    EFAIL(cs_mfs_entry_decode(b,512,208,&e.value),CS_MFS_ENTRY_BOUNDS);
    CHECK(storage[0]==0xb6 && storage[515]==0xb6);
    printf("MFS metadata: %u checks passed\n",checks);return 0;
}
