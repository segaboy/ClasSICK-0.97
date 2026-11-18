/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Independent generated metadata and sector-position oracle for SPEC-0115.
 * No historical disk, protected file bytes or implementation-derived fixture. */
#include "../core/storage/mfs-volume.h"
#include <stdio.h>
#include <string.h>

static unsigned long checks;
#define CHECK(x) do { ++checks; if (!(x)) { fprintf(stderr,"MFS volume test failed at line %u: %s\n",(unsigned)__LINE__,#x); return 1; } } while (0)
typedef struct {
    uint8_t master[1024], directory[3][512];
    uint32_t blocks, fail_lba;
    cs_block_io_result failure;
    unsigned calls;
} source;
static void put16(uint8_t *p,uint16_t n) { p[0]=(uint8_t)(n>>8);p[1]=(uint8_t)n; }
static void put32(uint8_t *p,uint32_t n) { p[0]=(uint8_t)(n>>24);p[1]=(uint8_t)(n>>16);p[2]=(uint8_t)(n>>8);p[3]=(uint8_t)n; }
static uint8_t pattern(uint32_t lba,unsigned byte) {
    return (uint8_t)((lba*73u+byte*29u+(byte>>4)*11u)^0xa7u);
}
static cs_block_io_result read_one(void *context,uint32_t lba,uint8_t *out) {
    source *s=context;unsigned i,limit=(lba==s->fail_lba)?19u:512u;
    ++s->calls;
    if(lba>=s->blocks)return CS_BLOCK_IO_UNAVAILABLE;
    for(i=0;i<limit;++i) {
        if(lba==2u || lba==3u)out[i]=s->master[(lba-2u)*512u+i];
        else if(lba>=4u && lba<=6u)out[i]=s->directory[lba-4u][i];
        else out[i]=pattern(lba,i);
    }
    return lba==s->fail_lba?s->failure:CS_BLOCK_IO_OK;
}
/* Encode via individual bit positions; independent from decoder's byte/nibble
 * expressions and tests both shared nibbles without overwriting adjacent bits. */
static void map_value(source *s,unsigned index,unsigned value) {
    unsigned bit;
    for(bit=0;bit<12u;++bit) {
        unsigned position=index*12u+bit, mask=1u<<(7u-position%8u);
        uint8_t *byte=&s->master[64u+position/8u];
        if(value&(1u<<(11u-bit)))*byte=(uint8_t)(*byte|mask);
        else *byte=(uint8_t)(*byte&~mask);
    }
}
static unsigned make_entry(uint8_t *p,unsigned n,uint32_t id) {
    unsigned i;memset(p,0,(51u+n+1u)&~1u);
    p[0]=0xc1;p[1]=0x27;
    for(i=0;i<16u;++i)p[2u+i]=(uint8_t)(0x61u+i*3u);
    put32(p+18,id);put32(p+42,UINT32_C(0x12345678));put32(p+46,UINT32_C(0xabcdef01));
    p[50]=(uint8_t)n;
    for(i=0;i<n;++i)p[51u+i]=(uint8_t)(0x80u+i);
    return (51u+n+1u)&~1u;
}
static void fixture(source *s) {
    uint8_t *h=s->master,*p=s->directory[0];
    unsigned extent;
    memset(s,0,sizeof *s);s->blocks=64u;s->fail_lba=UINT32_MAX;s->failure=CS_BLOCK_IO_ERROR;
    put16(h,0xd2d7);put32(h+2,UINT32_C(0x12345678));put16(h+12,3);
    put16(h+14,4);put16(h+16,2);put16(h+18,8);put32(h+20,1024);
    put32(h+24,8192);put16(h+28,8);put32(h+30,14);put16(h+34,3);
    h[36]=3;h[37]='V';h[38]=0;h[39]=0xfe;
    map_value(s,0,5);map_value(s,3,3);map_value(s,1,1); /* 2,5,3 */
    map_value(s,4,4);map_value(s,2,1); /* 6,4 */
    extent=make_entry(p,3,UINT32_C(0x10203040));p[51]='A';p[52]=0;p[53]=0xff;
    put16(p+22,2);put32(p+24,2801);put32(p+28,3072);
    put16(p+32,6);put32(p+34,1703);put32(p+38,2048);
    (void)make_entry(p+extent,0,12);
    (void)make_entry(s->directory[1],255,13);
}
static int uniform(const void *object,size_t bytes,uint8_t value) {
    const uint8_t *p=object;size_t i;for(i=0;i<bytes;++i)if(p[i]!=value)return 0;return 1;
}
static cs_block_device device_for(source *s) {
    cs_block_device d;d.blocks=s->blocks;d.read=read_one;d.context=s;return d;
}
static int mount_tests(void) {
    source s;cs_block_device d;cs_mfsv_workspace w;
    struct {uint8_t pre[8];cs_mfsv_volume v;uint8_t post[8];} g;
    source snapshot;unsigned i;
    fixture(&s);snapshot=s;d=device_for(&s);memset(&g,0xa5,sizeof g);
    CHECK(cs_mfsv_mount(&d,&w,&g.v)==CS_MFSV_OK);
    CHECK(s.calls==2u);CHECK(g.v.info.alloc_count==8u && g.v.info.alloc_bytes==1024u);
    CHECK(g.v.info.free_blocks==3u && g.v.info.file_count==3u);
    CHECK(g.v.info.name_length==3u && g.v.info.name[1]==0u && g.v.info.name[2]==0xfe);
    CHECK(g.v.device.context==&s && g.v.device.read==read_one && g.v.device.blocks==64u);
    CHECK(memcmp(g.v.map,s.master+64,960)==0);
    CHECK(memcmp(snapshot.master,s.master,sizeof s.master)==0);
    CHECK(memcmp(snapshot.directory,s.directory,sizeof s.directory)==0);
    CHECK(uniform(g.pre,sizeof g.pre,0xa5) && uniform(g.post,sizeof g.post,0xa5));
#define BAD_MOUNT(expected) do { memset(&g.v,0xa5,sizeof g.v);CHECK(cs_mfsv_mount(&d,&w,&g.v)==(expected));CHECK(uniform(&g.v,sizeof g.v,0xa5)); } while(0)
    s.fail_lba=3u;BAD_MOUNT(CS_MFSV_IO); /* second read partially writes */
    s.failure=CS_BLOCK_IO_UNAVAILABLE;BAD_MOUNT(CS_MFSV_UNAVAILABLE);
    s.failure=(cs_block_io_result)99;BAD_MOUNT(CS_MFSV_PROVIDER);
    fixture(&s);d=device_for(&s);map_value(&s,7,10);BAD_MOUNT(CS_MFSV_FORMAT);
    fixture(&s);put16(s.master+34,2);BAD_MOUNT(CS_MFSV_FORMAT);
    fixture(&s);s.master[0]=0;BAD_MOUNT(CS_MFSV_FORMAT);
    fixture(&s);put16(s.master+28,4);BAD_MOUNT(CS_MFSV_FORMAT); /* directory not reserved */
    map_value(&s,0,0xfff);CHECK(cs_mfsv_mount(&d,&w,&g.v)==CS_MFSV_OK);
    CHECK(g.v.map[0]==0xff && (g.v.map[1]&0xf0)==0xf0);
    fixture(&s);map_value(&s,7,0xfff);put16(s.master+34,2);
    CHECK(cs_mfsv_mount(&d,&w,&g.v)==CS_MFSV_OK); /* other reservations allowed */
    /* Greatest admitted map index and field sharing. All slots free except641. */
    fixture(&s);s.blocks=1300;d=device_for(&s);memset(s.master+64,0,960);
    put16(s.master+18,640);put16(s.master+34,639);map_value(&s,639,1);
    CHECK(cs_mfsv_mount(&d,&w,&g.v)==CS_MFSV_OK);CHECK(g.v.map[959]==1u);
    for(i=0;i<639u;++i) {
        map_value(&s,i,1);put16(s.master+34,638);
        CHECK(cs_mfsv_mount(&d,&w,&g.v)==CS_MFSV_OK);
        map_value(&s,i,0);put16(s.master+34,639);
    }
    fixture(&s);d=device_for(&s);memset(&g.v,0xa5,sizeof g.v);
    CHECK(cs_mfsv_mount(NULL,&w,&g.v)==CS_MFSV_ARGUMENT);
    CHECK(cs_mfsv_mount(&d,NULL,&g.v)==CS_MFSV_ARGUMENT);
    CHECK(cs_mfsv_mount(&d,&w,NULL)==CS_MFSV_ARGUMENT);
    CHECK(uniform(&g.v,sizeof g.v,0xa5));
    d.read=NULL;CHECK(cs_mfsv_mount(&d,&w,&g.v)==CS_MFSV_ARGUMENT);
#undef BAD_MOUNT
    return 0;
}
static int directory_tests(void) {
    source s;cs_block_device d;cs_mfsv_workspace w;cs_mfsv_volume v;
    cs_mfsv_cursor c={0,0,0},saved;cs_mfs_entry e,first;
    uint8_t weird[3]={'A',0,0xff},long_name[255];unsigned i;
    fixture(&s);d=device_for(&s);CHECK(cs_mfsv_mount(&d,&w,&v)==CS_MFSV_OK);
    CHECK(cs_mfsv_directory_next(&v,&w,&c,&e)==CS_MFSV_OK);first=e;
    CHECK(e.file_id==UINT32_C(0x10203040) && e.name_length==3u);
    CHECK(memcmp(e.name,weird,3)==0 && uniform(e.name+3,252,0));
    CHECK(e.data_start==2u && e.data_logical==2801u && e.resource_start==6u);
    CHECK(c.block==0u && c.offset==54u && c.seen==1u);
    CHECK(cs_mfsv_directory_next(&v,&w,&c,&e)==CS_MFSV_OK);
    CHECK(e.name_length==0u && uniform(e.name,255,0) && e.file_id==12u);
    CHECK(cs_mfsv_directory_next(&v,&w,&c,&e)==CS_MFSV_OK);
    CHECK(e.name_length==255u && e.file_id==13u && c.block==1u && c.seen==3u);
    for(i=0;i<255u;++i) {long_name[i]=(uint8_t)(0x80u+i);CHECK(e.name[i]==long_name[i]);}
    memset(&e,0xa5,sizeof e);
    CHECK(cs_mfsv_directory_next(&v,&w,&c,&e)==CS_MFSV_END);
    CHECK(uniform(&e,sizeof e,0xa5) && c.block==2u && c.seen==3u);
    saved=c;CHECK(cs_mfsv_directory_next(&v,&w,&c,&e)==CS_MFSV_END);
    CHECK(memcmp(&saved,&c,sizeof c)==0 && uniform(&e,sizeof e,0xa5));
    CHECK(cs_mfsv_find(&v,&w,weird,3,&e)==CS_MFSV_OK);
    CHECK(e.file_id==first.file_id && e.name_length==3u);
    CHECK(cs_mfsv_find(&v,&w,long_name,255,&e)==CS_MFSV_OK && e.file_id==13u);
    CHECK(cs_mfsv_find(&v,&w,weird,0,&e)==CS_MFSV_OK && e.file_id==12u);
    memset(&e,0xa5,sizeof e);weird[0]='a';
    CHECK(cs_mfsv_find(&v,&w,weird,3,&e)==CS_MFSV_NOT_FOUND && uniform(&e,sizeof e,0xa5));
    CHECK(cs_mfsv_find(&v,&w,weird,2,&e)==CS_MFSV_NOT_FOUND && uniform(&e,sizeof e,0xa5));
#define NEXT_FAIL(expected) do { saved=c;memset(&e,0xa5,sizeof e);CHECK(cs_mfsv_directory_next(&v,&w,&c,&e)==(expected));CHECK(memcmp(&saved,&c,sizeof c)==0 && uniform(&e,sizeof e,0xa5)); }while(0)
    c.block=0;c.offset=0;c.seen=0;s.fail_lba=4;NEXT_FAIL(CS_MFSV_IO);
    s.failure=CS_BLOCK_IO_UNAVAILABLE;NEXT_FAIL(CS_MFSV_UNAVAILABLE);
    s.failure=(cs_block_io_result)88;NEXT_FAIL(CS_MFSV_PROVIDER);
    s.fail_lba=UINT32_MAX;s.directory[0][0]=1;NEXT_FAIL(CS_MFSV_FORMAT);
    s.directory[0][0]=0;s.directory[0][1]=1;NEXT_FAIL(CS_MFSV_FORMAT);
    fixture(&s);d=device_for(&s);put16(s.master+12,2);CHECK(cs_mfsv_mount(&d,&w,&v)==CS_MFSV_OK);
    c.block=0;c.offset=0;c.seen=0;
    CHECK(cs_mfsv_directory_next(&v,&w,&c,&e)==CS_MFSV_OK);
    CHECK(cs_mfsv_directory_next(&v,&w,&c,&e)==CS_MFSV_OK);
    NEXT_FAIL(CS_MFSV_FORMAT); /* recorded count too small */
    put16(s.master+12,4);CHECK(cs_mfsv_mount(&d,&w,&v)==CS_MFSV_OK);
    c.block=0;c.offset=0;c.seen=0;
    CHECK(cs_mfsv_directory_next(&v,&w,&c,&e)==CS_MFSV_OK);
    CHECK(cs_mfsv_directory_next(&v,&w,&c,&e)==CS_MFSV_OK);
    CHECK(cs_mfsv_directory_next(&v,&w,&c,&e)==CS_MFSV_OK);
    NEXT_FAIL(CS_MFSV_FORMAT); /* recorded count too large */
    /* Two entries including a255-byte name, then exact block end: 306+206. */
    fixture(&s);memset(s.directory,0,sizeof s.directory);
    (void)make_entry(s.directory[0],255,1);
    (void)make_entry(s.directory[0]+306,155,2);
    put16(s.master+12,2);CHECK(cs_mfsv_mount(&d,&w,&v)==CS_MFSV_OK);
    c.block=0;c.offset=0;c.seen=0;
    CHECK(cs_mfsv_directory_next(&v,&w,&c,&e)==CS_MFSV_OK);
    CHECK(cs_mfsv_directory_next(&v,&w,&c,&e)==CS_MFSV_OK && c.offset==512u);
    CHECK(cs_mfsv_directory_next(&v,&w,&c,&e)==CS_MFSV_END && c.block==2u);
    /* Active entry with insufficient fixed body at the last permitted even offset. */
    fixture(&s);memset(s.directory,0,sizeof s.directory);put16(s.master+12,1);
    s.directory[0][510]=0x80;CHECK(cs_mfsv_mount(&d,&w,&v)==CS_MFSV_OK);
    c.block=0;c.offset=510;c.seen=0;NEXT_FAIL(CS_MFSV_FORMAT);
    c.block=0;c.offset=1;c.seen=0;NEXT_FAIL(CS_MFSV_ARGUMENT);
    c.block=3;c.offset=0;c.seen=0;NEXT_FAIL(CS_MFSV_ARGUMENT);
#undef NEXT_FAIL
    return 0;
}
static uint32_t logical_lba(unsigned fork,uint32_t position,uint32_t alloc_size) {
    static const unsigned data_allocations[]={2,5,3},resource_allocations[]={6,4};
    unsigned ordinal=(unsigned)(position/alloc_size);
    unsigned allocation=fork==0u?data_allocations[ordinal]:resource_allocations[ordinal];
    return 8u+(allocation-2u)*(alloc_size/512u)+(position%alloc_size)/512u;
}
static int fork_tests(void) {
    source s;cs_block_device d;cs_mfsv_workspace w;cs_mfsv_volume v;cs_mfs_entry e,empty;
    cs_mfsv_cursor c={0,0,0};unsigned i,kind,case_index;
    struct {uint8_t pre[9],data[33000],post[9];} out,staging;
    static const uint32_t offsets[]={0,1,127,511,512,777,1023,1024,1535,1700};
    static const size_t lengths[]={1,2,17,511,512,513,789};
    fixture(&s);d=device_for(&s);CHECK(cs_mfsv_mount(&d,&w,&v)==CS_MFSV_OK);
    CHECK(cs_mfsv_directory_next(&v,&w,&c,&e)==CS_MFSV_OK);
    CHECK(cs_mfsv_directory_next(&v,&w,&c,&empty)==CS_MFSV_OK);
    CHECK(cs_mfsv_fork_validate(&v,&e,CS_MFSV_DATA)==CS_MFSV_OK);
    CHECK(cs_mfsv_fork_validate(&v,&e,CS_MFSV_RESOURCE)==CS_MFSV_OK);
    CHECK(cs_mfsv_fork_validate(&v,&empty,CS_MFSV_DATA)==CS_MFSV_OK);
    for(kind=0;kind<2u;++kind) for(case_index=0;case_index<sizeof offsets/sizeof offsets[0];++case_index)
        for(i=0;i<sizeof lengths/sizeof lengths[0];++i) {
            uint32_t off=offsets[case_index],logical=kind?e.resource_logical:e.data_logical;
            size_t count=lengths[i],j;
            if(off>logical || count>logical-off)continue;
            memset(&out,0xa5,sizeof out);memset(&staging,0x6c,sizeof staging);
            CHECK(cs_mfsv_fork_read(&v,&w,&e,(cs_mfsv_fork)kind,off,count,out.data,32767,staging.data,32767)==CS_MFSV_OK);
            for(j=0;j<count;++j)CHECK(out.data[j]==pattern(logical_lba(kind,off+(uint32_t)j,1024),(unsigned)((off+j)%512)));
            CHECK(uniform(out.pre,sizeof out.pre,0xa5) && uniform(out.post,sizeof out.post,0xa5));
            CHECK(uniform(out.data+count,sizeof out.data-count,0xa5));
            CHECK(uniform(staging.pre,sizeof staging.pre,0x6c) && uniform(staging.post,sizeof staging.post,0x6c));
            CHECK(uniform(staging.data+count,sizeof staging.data-count,0x6c));
        }
#define RF(off,n,expected) do {memset(&out,0xa5,sizeof out);CHECK(cs_mfsv_fork_read(&v,&w,&e,CS_MFSV_DATA,(off),(n),out.data,32767,staging.data,32767)==(expected));CHECK(uniform(&out,sizeof out,0xa5));}while(0)
    RF(e.data_logical,0,CS_MFSV_OK);RF(e.data_logical,1,CS_MFSV_RANGE);
    RF(UINT32_MAX,1,CS_MFSV_RANGE);RF(e.data_logical+1u,0,CS_MFSV_RANGE);
    CHECK(cs_mfsv_fork_read(&v,&w,&empty,CS_MFSV_DATA,0,0,out.data,32767,staging.data,32767)==CS_MFSV_OK);
    RF(0,32768,CS_MFSV_ARGUMENT);
    CHECK(cs_mfsv_fork_read(&v,&w,&e,CS_MFSV_DATA,0,10,out.data,9,staging.data,10)==CS_MFSV_CAPACITY);
    CHECK(cs_mfsv_fork_read(&v,&w,&e,CS_MFSV_DATA,0,10,out.data,10,staging.data,9)==CS_MFSV_CAPACITY);
    CHECK(cs_mfsv_fork_read(&v,&w,&e,CS_MFSV_DATA,0,1,out.data,32768,staging.data,1)==CS_MFSV_ARGUMENT);
    CHECK(cs_mfsv_fork_read(&v,&w,&e,CS_MFSV_DATA,0,1,out.data,1,out.data,1)==CS_MFSV_ARGUMENT);
    CHECK(cs_mfsv_fork_read(&v,&w,&e,CS_MFSV_DATA,0,0,NULL,0,staging.data,0)==CS_MFSV_ARGUMENT);
    /* Late provider fails after prior requested bytes have staged. */
    s.fail_lba=14;s.failure=CS_BLOCK_IO_ERROR;RF(511,1700,CS_MFSV_IO);
    s.failure=CS_BLOCK_IO_UNAVAILABLE;RF(511,1700,CS_MFSV_UNAVAILABLE);
    s.failure=(cs_block_io_result)77;RF(511,1700,CS_MFSV_PROVIDER);s.fail_lba=UINT32_MAX;
    /* Metadata errors must be rejected without any sector reads. */
    {cs_mfs_entry bad=e;unsigned before=s.calls;bad.data_logical=3073;
     CHECK(cs_mfsv_fork_validate(&v,&bad,CS_MFSV_DATA)==CS_MFSV_CHAIN);
     CHECK(s.calls==before);bad=e;bad.data_start=0;
     CHECK(cs_mfsv_fork_validate(&v,&bad,CS_MFSV_DATA)==CS_MFSV_CHAIN);
     bad=e;bad.data_physical=0;bad.data_logical=0;
     CHECK(cs_mfsv_fork_validate(&v,&bad,CS_MFSV_DATA)==CS_MFSV_CHAIN);
     bad=e;bad.data_start=1;CHECK(cs_mfsv_fork_validate(&v,&bad,CS_MFSV_DATA)==CS_MFSV_CHAIN);
     bad=e;bad.data_start=10;CHECK(cs_mfsv_fork_validate(&v,&bad,CS_MFSV_DATA)==CS_MFSV_CHAIN);
     bad=e;bad.data_physical=3071;bad.data_logical=1;CHECK(cs_mfsv_fork_validate(&v,&bad,CS_MFSV_DATA)==CS_MFSV_CHAIN);
     bad=e;bad.data_logical=0;CHECK(cs_mfsv_fork_validate(&v,&bad,CS_MFSV_DATA)==CS_MFSV_OK);
     CHECK(cs_mfsv_fork_validate(&v,&e,(cs_mfsv_fork)2)==CS_MFSV_ARGUMENT);}
    /* All malformed chain maps are structurally valid mounts. */
    fixture(&s);map_value(&s,3,2);CHECK(cs_mfsv_mount(&d,&w,&v)==CS_MFSV_OK);
    CHECK(cs_mfsv_fork_validate(&v,&e,CS_MFSV_DATA)==CS_MFSV_CHAIN);
    RF(0,1,CS_MFSV_CHAIN);RF(0,0,CS_MFSV_CHAIN);
    fixture(&s);map_value(&s,3,0);put16(s.master+34,4);CHECK(cs_mfsv_mount(&d,&w,&v)==CS_MFSV_OK);
    CHECK(cs_mfsv_fork_validate(&v,&e,CS_MFSV_DATA)==CS_MFSV_CHAIN);
    fixture(&s);map_value(&s,3,0xfff);CHECK(cs_mfsv_mount(&d,&w,&v)==CS_MFSV_OK);
    CHECK(cs_mfsv_fork_validate(&v,&e,CS_MFSV_DATA)==CS_MFSV_CHAIN);
    fixture(&s);map_value(&s,0,1);CHECK(cs_mfsv_mount(&d,&w,&v)==CS_MFSV_OK);
    CHECK(cs_mfsv_fork_validate(&v,&e,CS_MFSV_DATA)==CS_MFSV_CHAIN);
    fixture(&s);map_value(&s,1,6);CHECK(cs_mfsv_mount(&d,&w,&v)==CS_MFSV_OK);
    CHECK(cs_mfsv_fork_validate(&v,&e,CS_MFSV_DATA)==CS_MFSV_CHAIN);
    fixture(&s);put16(s.master+28,4);map_value(&s,0,0xfff);
    CHECK(cs_mfsv_mount(&d,&w,&v)==CS_MFSV_OK);
    CHECK(cs_mfsv_fork_validate(&v,&e,CS_MFSV_DATA)==CS_MFSV_CHAIN);
    /* Variable3-sector allocation blocks exercise non-power-of-two mapping. */
    fixture(&s);put32(s.master+20,1536);CHECK(cs_mfsv_mount(&d,&w,&v)==CS_MFSV_OK);
    e.data_physical=4608;e.data_logical=4501;e.resource_physical=3072;e.resource_logical=2999;
    for(kind=0;kind<2u;++kind) {
        size_t j;memset(&out,0xa5,sizeof out);
        CHECK(cs_mfsv_fork_read(&v,&w,&e,(cs_mfsv_fork)kind,511,2400,out.data,32767,staging.data,32767)==CS_MFSV_OK);
        for(j=0;j<2400;++j)CHECK(out.data[j]==pattern(logical_lba(kind,511u+(uint32_t)j,1536),(unsigned)((511u+j)%512u)));
        CHECK(uniform(out.data+2400,sizeof out.data-2400,0xa5));
    }
    /* Greatest map entry641 resolves to the final allocation, not off-by-two. */
    fixture(&s);s.blocks=1300;d=device_for(&s);memset(s.master+64,0,960);
    put16(s.master+18,640);put16(s.master+34,639);map_value(&s,639,1);
    CHECK(cs_mfsv_mount(&d,&w,&v)==CS_MFSV_OK);
    e.data_start=641;e.data_physical=1024;e.data_logical=1024;
    CHECK(cs_mfsv_fork_read(&v,&w,&e,CS_MFSV_DATA,501,523,out.data,32767,staging.data,32767)==CS_MFSV_OK);
    for(i=0;i<523u;++i)CHECK(out.data[i]==pattern(1286u+(501u+i)/512u,(501u+i)%512u));
    /* Maximum native request and high32-bit sector addresses. The independently
     * generated volume has512 very large allocation blocks; only its last one
     * is used. uint64_t in this oracle avoids sharing target arithmetic. */
    fixture(&s);s.blocks=UINT32_MAX;d=device_for(&s);memset(s.master+64,0,960);
    put16(s.master+18,512);put32(s.master+20,UINT32_C(0xfffffe00));
    put16(s.master+34,511);map_value(&s,511,1);
    CHECK(cs_mfsv_mount(&d,&w,&v)==CS_MFSV_OK);
    e.data_start=513;e.data_physical=UINT32_C(0xfffffe00);e.data_logical=e.data_physical;
    {
        uint32_t offset=e.data_logical-UINT32_C(32767);
        uint64_t first_lba=UINT64_C(8)+UINT64_C(511)*(UINT64_C(0xfffffe00)/512u);
        memset(&out,0xa5,sizeof out);memset(&staging,0x6c,sizeof staging);
        CHECK(cs_mfsv_fork_read(&v,&w,&e,CS_MFSV_DATA,offset,32767,out.data,32767,staging.data,32767)==CS_MFSV_OK);
        for(i=0;i<32767u;++i) {
            uint64_t position=(uint64_t)offset+i;
            CHECK(first_lba+position/512u<UINT32_MAX);
            CHECK(out.data[i]==pattern((uint32_t)(first_lba+position/512u),(unsigned)(position%512u)));
        }
        CHECK(uniform(out.data+32767,sizeof out.data-32767,0xa5));
        CHECK(uniform(out.pre,sizeof out.pre,0xa5) && uniform(out.post,sizeof out.post,0xa5));
        CHECK(uniform(staging.data+32767,sizeof staging.data-32767,0x6c));
    }
#undef RF
    return 0;
}
int main(void) {
    if(mount_tests() || directory_tests() || fork_tests())return 1;
    printf("MFS volume: %lu checks passed\n",checks);return 0;
}
