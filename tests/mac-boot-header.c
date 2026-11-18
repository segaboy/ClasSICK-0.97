/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../core/storage/mac-boot-header.h"
#include <stdio.h>
#include <string.h>

static unsigned long checks;
#define CHECK(x) do { ++checks;if(!(x)){fprintf(stderr,"boot header check failed: %u\n",(unsigned)__LINE__);return 1;} } while(0)
static void put16(uint8_t *p,uint16_t n){p[0]=(uint8_t)(n>>8);p[1]=(uint8_t)n;}
static void put32(uint8_t *p,uint32_t n){p[0]=(uint8_t)(n>>24);p[1]=(uint8_t)(n>>16);p[2]=(uint8_t)(n>>8);p[3]=(uint8_t)n;}
static int uniform(const void *p,size_t n,uint8_t value)
{
    const uint8_t *b=p;size_t i;for(i=0;i<n;++i)if(b[i]!=value)return 0;return 1;
}
static void fixture(uint8_t *b)
{
    unsigned i,j;memset(b,0x6b,1024);put16(b,0x4c4b);put16(b+6,0xa1d3);
    put16(b+8,0x83e7);put16(b+122,0x1234);put16(b+124,0xfedc);
    put32(b+126,UINT32_C(0x10203040));put32(b+130,UINT32_C(0x89abcdef));
    put32(b+134,UINT32_C(0xfedcba98));
    for(i=0;i<7;++i){b[10+16*i]=(uint8_t)(2*i+1);for(j=0;j<15;++j)b[11+16*i+j]=(uint8_t)(17*i+j);}
    b[11]=0;b[27]=0xff;
}
typedef struct {const uint8_t *input;unsigned calls,fail_at;uint32_t lba[2];cs_block_io_result result;} source;
static cs_block_io_result read_one(void *ctx,uint32_t lba,uint8_t *out)
{
    source *s=ctx;unsigned i,call=s->calls++;if(call<2)s->lba[call]=lba;
    if(lba>1)return CS_BLOCK_IO_ERROR;
    for(i=0;i<512;++i)out[i]=s->input[(size_t)lba*512u+i];
    return call==s->fail_at?s->result:CS_BLOCK_IO_OK;
}
int main(void)
{
    uint8_t storage[1026],snapshot[1024],*area=storage+1;
    struct {uint8_t before[8];cs_mac_boot_header value;uint8_t after[8];} guarded;
    cs_mac_boot_header baseline;cs_mac_boot_workspace work;
    source s;cs_block_device device={2,read_one,&s};unsigned i,j,n,slot;uint32_t version;
    memset(storage,0x5a,sizeof storage);fixture(area);memcpy(snapshot,area,1024);
    memset(&guarded,0xa5,sizeof guarded);
    CHECK(cs_mac_boot_header_decode(area,1024,&guarded.value)==CS_MAC_BOOT_OK);
    CHECK(guarded.value.signature==0x4c4b && guarded.value.version==0xa1d3 && guarded.value.page_flags==0x83e7);
    CHECK(guarded.value.fcb_count==0x1234 && guarded.value.event_count==0xfedc);
    CHECK(guarded.value.heap_128k==UINT32_C(0x10203040) && guarded.value.heap_256k==UINT32_C(0x89abcdef));
    CHECK(guarded.value.heap_other==UINT32_C(0xfedcba98));
    for(i=0;i<7;++i){CHECK(guarded.value.names[i].length==2*i+1);CHECK(memcmp(guarded.value.names[i].bytes,area+11+16*i,2*i+1)==0);CHECK(uniform(guarded.value.names[i].bytes+2*i+1,14-2*i,0));}
    CHECK(memcmp(area,snapshot,1024)==0 && storage[0]==0x5a && storage[1025]==0x5a);
    CHECK(uniform(guarded.before,8,0xa5) && uniform(guarded.after,8,0xa5));
    memcpy(&baseline,&guarded.value,sizeof baseline);
    /* Every opaque entry byte and trailing byte can change without changing
     * any output member, including a high-bit arbitrary version. */
    for(i=2;i<6;++i)area[i]^=0xff;
    for(i=138;i<1024;++i)area[i]^=0xff;
    CHECK(cs_mac_boot_header_decode(area,1024,&guarded.value)==CS_MAC_BOOT_OK);
    CHECK(memcmp(&baseline,&guarded.value,sizeof baseline)==0);
    for(version=0;version<=UINT32_C(65535);++version){put16(area+6,(uint16_t)version);CHECK(cs_mac_boot_header_decode(area,1024,&guarded.value)==CS_MAC_BOOT_OK);CHECK(guarded.value.version==version);}

#define FAIL(call,expected) do{memset(&guarded.value,0xa5,sizeof guarded.value);CHECK((call)==(expected));CHECK(uniform(&guarded.value,sizeof guarded.value,0xa5));}while(0)
    for(n=0;n<1024;++n)FAIL(cs_mac_boot_header_decode(area,n,&guarded.value),CS_MAC_BOOT_ARGUMENT);
    FAIL(cs_mac_boot_header_decode(area,1025,&guarded.value),CS_MAC_BOOT_ARGUMENT);
    FAIL(cs_mac_boot_header_decode(area,(size_t)-1,&guarded.value),CS_MAC_BOOT_ARGUMENT);
    FAIL(cs_mac_boot_header_decode(NULL,1024,&guarded.value),CS_MAC_BOOT_ARGUMENT);
    CHECK(cs_mac_boot_header_decode(area,1024,NULL)==CS_MAC_BOOT_ARGUMENT);
    FAIL(cs_mac_boot_header_decode((const uint8_t *)&guarded.value,1024,&guarded.value),CS_MAC_BOOT_ARGUMENT);
    for(n=0;n<4;++n){fixture(area);area[n/2]^=(uint8_t)(1u<<(n%2));FAIL(cs_mac_boot_header_decode(area,1024,&guarded.value),CS_MAC_BOOT_SIGNATURE);}
    for(slot=0;slot<7;++slot)for(n=0;n<256;++n){fixture(area);area[10+16*slot]=(uint8_t)n;
        if(n>15){FAIL(cs_mac_boot_header_decode(area,1024,&guarded.value),CS_MAC_BOOT_NAME);}
        else {CHECK(cs_mac_boot_header_decode(area,1024,&guarded.value)==CS_MAC_BOOT_OK);CHECK(guarded.value.names[slot].length==n);CHECK(memcmp(guarded.value.names[slot].bytes,area+11+16*slot,n)==0);CHECK(uniform(guarded.value.names[slot].bytes+n,15-n,0));}}
    fixture(area);s.input=area;s.calls=0;s.fail_at=2;s.result=CS_BLOCK_IO_OK;
    memset(&work,0x73,sizeof work);
    CHECK(cs_mac_boot_header_read(&device,&work,&guarded.value)==CS_MAC_BOOT_OK);
    CHECK(s.calls==2 && s.lba[0]==0 && s.lba[1]==1 && memcmp(work.area,area,1024)==0);
    CHECK(guarded.value.heap_other==UINT32_C(0xfedcba98));
    for(i=0;i<2;++i)for(j=0;j<3;++j){s.calls=0;s.fail_at=i;s.result=j==0?CS_BLOCK_IO_UNAVAILABLE:j==1?CS_BLOCK_IO_ERROR:(cs_block_io_result)91;memset(&work,0x73,sizeof work);
        FAIL(cs_mac_boot_header_read(&device,&work,&guarded.value),j==0?CS_MAC_BOOT_UNAVAILABLE:j==1?CS_MAC_BOOT_IO:CS_MAC_BOOT_PROVIDER);
        CHECK(s.calls==i+1 && uniform(work.area,1024,0x73));CHECK(s.lba[0]==0);if(i)CHECK(s.lba[1]==1);}
    s.calls=0;s.fail_at=2;device.blocks=1;
    FAIL(cs_mac_boot_header_read(&device,&work,&guarded.value),CS_MAC_BOOT_RANGE);CHECK(s.calls==0);
    device.blocks=0;FAIL(cs_mac_boot_header_read(&device,&work,&guarded.value),CS_MAC_BOOT_ARGUMENT);CHECK(s.calls==0);
    device.blocks=2;device.read=NULL;FAIL(cs_mac_boot_header_read(&device,&work,&guarded.value),CS_MAC_BOOT_ARGUMENT);
    device.read=read_one;FAIL(cs_mac_boot_header_read(NULL,&work,&guarded.value),CS_MAC_BOOT_ARGUMENT);
    FAIL(cs_mac_boot_header_read(&device,NULL,&guarded.value),CS_MAC_BOOT_ARGUMENT);
    CHECK(cs_mac_boot_header_read(&device,&work,NULL)==CS_MAC_BOOT_ARGUMENT);
    FAIL(cs_mac_boot_header_read((const cs_block_device *)&work,&work,&guarded.value),CS_MAC_BOOT_ARGUMENT);
    FAIL(cs_mac_boot_header_read((const cs_block_device *)&guarded.value,&work,&guarded.value),CS_MAC_BOOT_ARGUMENT);
    FAIL(cs_mac_boot_header_read(&device,(cs_mac_boot_workspace *)&guarded.value,&guarded.value),CS_MAC_BOOT_ARGUMENT);CHECK(s.calls==0);
    for(i=0;i<2;++i){fixture(area);if(i)area[106]=16;else area[0]=0;s.calls=0;
        FAIL(cs_mac_boot_header_read(&device,&work,&guarded.value),i?CS_MAC_BOOT_NAME:CS_MAC_BOOT_SIGNATURE);CHECK(s.calls==2 && memcmp(work.area,area,1024)==0);}
    CHECK(uniform(guarded.before,8,0xa5) && uniform(guarded.after,8,0xa5));
    printf("Mac boot header: %lu checks passed\n",checks);return 0;
}
