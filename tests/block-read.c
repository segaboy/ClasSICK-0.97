/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../core/storage/block-read.h"
#include <stdio.h>
#include <string.h>

#define PAYLOAD (63u * 512u)
#define STORAGE 32768u
typedef struct { uint8_t before[16], data[STORAGE], after[16]; } guarded;
typedef struct {
    uint32_t first, calls, fail_at;
    cs_block_io_result failure;
    int bad_order;
} provider;
static guarded destination, scratch;
static unsigned checks;
#define CHECK(value) do { ++checks; if (!(value)) { \
    fprintf(stderr,"block-read check failed at line %u\n",(unsigned)__LINE__); \
    return 1; } } while (0)

static uint8_t content(uint32_t lba, unsigned byte)
{
    return (uint8_t)((lba ^ (lba >> 8) ^ (lba >> 16) ^ (lba >> 24)) +
        (uint32_t)(byte & 255u) + (uint32_t)(byte >> 8));
}
static cs_block_io_result read_one(void *context, uint32_t lba, uint8_t *block)
{
    provider *p = context;
    unsigned i, limit = p->calls + 1 == p->fail_at ? 23u : 512u;
    if (lba != p->first + p->calls) p->bad_order = 1;
    ++p->calls;
    for (i = 0; i < limit; ++i) block[i] = content(lba, i);
    return p->calls == p->fail_at ? p->failure : CS_BLOCK_IO_OK;
}
static int uniform(const uint8_t *data, size_t n, uint8_t value)
{
    size_t i;
    for (i = 0; i < n; ++i) if (data[i] != value) return 0;
    return 1;
}
static int guards(void)
{
    return uniform(destination.before,16,0xa5) && uniform(destination.after,16,0xa5) &&
        uniform(scratch.before,16,0x5a) && uniform(scratch.after,16,0x5a) &&
        uniform(destination.data+PAYLOAD,STORAGE-PAYLOAD,0xa5) &&
        uniform(scratch.data+PAYLOAD,STORAGE-PAYLOAD,0x5a);
}
static void prepare(provider *p, uint32_t first)
{
    memset(&destination,0xa5,sizeof destination);
    memset(&scratch,0x5a,sizeof scratch);
    p->first=first;p->calls=0;p->fail_at=0;p->failure=CS_BLOCK_IO_ERROR;p->bad_order=0;
}

int main(void)
{
    provider p;
    cs_block_device device={UINT32_MAX,read_one,&p};
    uint32_t count, fail;
    size_t i;
    cs_block_result result;
    /* Independent byte-position oracle, including LBA carries and the maximum
     * admitted request, without storing any external or historical media. */
    for (count=1;count<=63;++count) {
        uint32_t first = count == 63 ? UINT32_MAX-63 : UINT32_C(0x1234fffe);
        prepare(&p,first);
        CHECK(cs_block_read(&device,first,count,destination.data,PAYLOAD,
            scratch.data,PAYLOAD)==CS_BLOCK_OK);
        CHECK(p.calls==count && !p.bad_order);
        for (i=0;i<(size_t)count*512;++i) {
            uint32_t lba=first+(uint32_t)(i/512);
            unsigned byte=(unsigned)(i%512);
            uint32_t sum;
            /* XOR the four LBA bytes, then add the within-block byte index. */
            sum=(lba&255u)^((lba>>8)&255u)^((lba>>16)&255u)^((lba>>24)&255u);
            CHECK(destination.data[i]==(uint8_t)(sum+(byte%256u)+(byte/256u)));
        }
        CHECK(uniform(destination.data+(size_t)count*512,PAYLOAD-(size_t)count*512,0xa5));
        CHECK(uniform(scratch.data+(size_t)count*512,PAYLOAD-(size_t)count*512,0x5a));
        CHECK(guards());
    }
    /* Each possible failure position includes a 23-byte failed-block write.
     * Every output byte must remain original even after 62 successful reads. */
    for (fail=1;fail<=63;++fail) {
        prepare(&p,17);p.fail_at=fail;
        CHECK(cs_block_read(&device,17,63,destination.data,PAYLOAD,
            scratch.data,PAYLOAD)==CS_BLOCK_IO);
        CHECK(p.calls==fail && !p.bad_order);
        CHECK(uniform(destination.data,PAYLOAD,0xa5));
        CHECK(scratch.data[(size_t)(fail-1)*512]==content(17+fail-1,0));
        CHECK(scratch.data[(size_t)(fail-1)*512+23]==0x5a);
        CHECK(guards());
    }
    prepare(&p,1);p.fail_at=1;p.failure=CS_BLOCK_IO_UNAVAILABLE;
    CHECK(cs_block_read(&device,1,1,destination.data,512,scratch.data,512)==CS_BLOCK_UNAVAILABLE);
    CHECK(uniform(destination.data,PAYLOAD,0xa5) && p.calls==1 && guards());
    prepare(&p,1);p.fail_at=1;p.failure=(cs_block_io_result)77;
    CHECK(cs_block_read(&device,1,1,destination.data,512,scratch.data,512)==CS_BLOCK_PROVIDER);
    CHECK(uniform(destination.data,PAYLOAD,0xa5) && p.calls==1 && guards());

#define REFUSE(expr, expected) do { prepare(&p,0);result=(expr); \
    CHECK(result==(expected));CHECK(p.calls==0); \
    CHECK(uniform(destination.data,PAYLOAD,0xa5)); \
    CHECK(uniform(scratch.data,PAYLOAD,0x5a));CHECK(guards()); } while(0)
#define READ(lba,n,oc,sc) cs_block_read(&device,(lba),(n),destination.data,(oc),scratch.data,(sc))
    REFUSE(READ(0,0,512,512),CS_BLOCK_ARGUMENT);
    REFUSE(READ(0,64,PAYLOAD,PAYLOAD),CS_BLOCK_RANGE);
    REFUSE(READ(0,UINT32_MAX,PAYLOAD,PAYLOAD),CS_BLOCK_RANGE);
    REFUSE(READ(UINT32_MAX,1,512,512),CS_BLOCK_RANGE);
    REFUSE(READ(UINT32_MAX-1,2,1024,1024),CS_BLOCK_RANGE);
    REFUSE(READ(UINT32_MAX-62,63,PAYLOAD,PAYLOAD),CS_BLOCK_RANGE);
    REFUSE(READ(0,1,511,512),CS_BLOCK_CAPACITY);
    REFUSE(READ(0,1,512,511),CS_BLOCK_CAPACITY);
    REFUSE(READ(0,63,PAYLOAD-1,PAYLOAD),CS_BLOCK_CAPACITY);
    REFUSE(READ(0,63,PAYLOAD,PAYLOAD-1),CS_BLOCK_CAPACITY);
    REFUSE(READ(0,1,0,512),CS_BLOCK_CAPACITY);
    REFUSE(READ(0,1,512,0),CS_BLOCK_CAPACITY);
    REFUSE(READ(0,1,32768u,512),CS_BLOCK_ARGUMENT);
    REFUSE(READ(0,1,512,32768u),CS_BLOCK_ARGUMENT);
    REFUSE(cs_block_read(NULL,0,1,destination.data,512,scratch.data,512),CS_BLOCK_ARGUMENT);
    REFUSE(cs_block_read(&device,0,1,NULL,512,scratch.data,512),CS_BLOCK_ARGUMENT);
    REFUSE(cs_block_read(&device,0,1,destination.data,512,NULL,512),CS_BLOCK_ARGUMENT);
    REFUSE(cs_block_read(&device,0,1,destination.data,512,destination.data,512),CS_BLOCK_ARGUMENT);
    device.blocks=0;REFUSE(READ(0,1,512,512),CS_BLOCK_ARGUMENT);
    device.blocks=1;REFUSE(READ(1,1,512,512),CS_BLOCK_RANGE);
    device.read=NULL;REFUSE(READ(0,1,512,512),CS_BLOCK_ARGUMENT);
    /* A null provider context is permitted when the callback does not need it;
     * this test provider does need one, so its null case is not a valid fixture. */
    printf("block-read: %u checks passed\n",checks);
    return 0;
}
