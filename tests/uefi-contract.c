/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../platform/uefi/contract.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do { if(!(x)){fprintf(stderr,"UEFI contract failure at line %d\n",__LINE__);return 1;} } while(0)

static void put32(unsigned char *p,uint32_t n)
{
    for(unsigned i=0;i<4;++i) p[i]=(unsigned char)((n>>(i*8))&255u);
}
static void put64(unsigned char *p,uint64_t n)
{
    put32(p,(uint32_t)n); put32(p+4,(uint32_t)(n>>32));
}
static void descriptor(unsigned char *p,uint32_t type,uint64_t base,uint64_t pages,uint64_t attributes)
{
    put32(p,type); put64(p+8,base); put64(p+16,0); put64(p+24,pages); put64(p+32,attributes);
}
static cs_uefi_framebuffer good_fb(void)
{
    cs_uefi_framebuffer f={0x100000,4096,9,2,12,1}; return f;
}
static int framebuffer(void)
{
    size_t cases=0;
    for(uint32_t w=1;w<=8;++w) for(uint32_t h=1;h<=8;++h)
    for(uint32_t padding=0;padding<4;++padding) for(uint32_t format=0;format<5;++format)
    for(unsigned tail=0;tail<3;++tail) for(unsigned address=0;address<3;++address) {
        uint64_t needed=(uint64_t)(w+padding)*4*h;
        cs_uefi_framebuffer f={address==0?0:(address==1?4096:(UINT64_C(1)<<47)-needed),
            needed+tail-1,w,h,w+padding,format};
        cs_uefi_framebuffer_layout out={127,129};
        cs_uefi_result expected;
        if(format>1) expected=CS_UEFI_FORMAT;
        else if(address==0) expected=CS_UEFI_LIMIT;
        else if(address==2 && tail==2) expected=CS_UEFI_OVERFLOW;
        else if(tail==0) expected=CS_UEFI_LIMIT;
        else expected=CS_UEFI_OK;
        CHECK(cs_uefi_check_framebuffer(&f,&out)==expected);
        if(expected==CS_UEFI_OK) CHECK(out.stride==(uint64_t)(w+padding)*4 && out.required==needed);
        else CHECK(out.stride==127 && out.required==129);
        ++cases;
    }
    cs_uefi_framebuffer f=good_fb(); cs_uefi_framebuffer_layout out={127,129};
    CHECK(cs_uefi_check_framebuffer(NULL,&out)==CS_UEFI_ARGUMENT);
    CHECK(cs_uefi_check_framebuffer(&f,NULL)==CS_UEFI_ARGUMENT);
    f.width=0; CHECK(cs_uefi_check_framebuffer(&f,&out)==CS_UEFI_LIMIT);
    f=good_fb(); f.pitch=8; CHECK(cs_uefi_check_framebuffer(&f,&out)==CS_UEFI_LIMIT);
    f=good_fb(); f.size=UINT64_C(268435457); CHECK(cs_uefi_check_framebuffer(&f,&out)==CS_UEFI_LIMIT);
    f=good_fb(); f.base=UINT64_MAX; CHECK(cs_uefi_check_framebuffer(&f,&out)==CS_UEFI_OVERFLOW);
    CHECK(out.stride==127 && out.required==129);
    printf("UEFI framebuffer: %zu independent cases PASS\n",cases); return 0;
}
static int maps(void)
{
    unsigned char raw[514],before[514];
    const size_t strides[]={40,48,56,64,256};
    size_t cases=0;
    for(size_t k=0;k<5;++k) for(size_t unaligned=0;unaligned<2;++unaligned)
    for(unsigned a=1;a<=8;++a) for(unsigned b=1;b<=8;++b)
    for(unsigned na=1;na<=4;++na) for(unsigned nb=1;nb<=4;++nb) {
        unsigned char occupied[16]={0}; int conflict=0;
        size_t stride=strides[k],count=123;
        memset(raw,0xA5,sizeof(raw));
        descriptor(raw+unaligned,UINT32_MAX,(uint64_t)a*4096,na,0);
        descriptor(raw+unaligned+stride,7,(uint64_t)b*4096,nb,0);
        memcpy(before,raw,sizeof(raw));
        for(unsigned p=a;p<a+na;++p) occupied[p]=1;
        for(unsigned p=b;p<b+nb;++p) if(occupied[p]) conflict=1;
        CHECK(cs_uefi_check_map(raw+unaligned,2*stride,stride,1,&count)==(conflict?CS_UEFI_OVERLAP:CS_UEFI_OK));
        CHECK(count==(conflict?123u:2u));
        CHECK(memcmp(raw,before,sizeof(raw))==0); ++cases;
    }
    memset(raw,0,sizeof(raw)); descriptor(raw,2,4096,1,0); size_t count=123;
    CHECK(cs_uefi_check_map(NULL,40,40,1,&count)==CS_UEFI_ARGUMENT);
    CHECK(cs_uefi_check_map(raw,40,40,1,NULL)==CS_UEFI_ARGUMENT);
    CHECK(cs_uefi_check_map(raw,40,39,1,&count)==CS_UEFI_FORMAT);
    CHECK(cs_uefi_check_map(raw,40,40,2,&count)==CS_UEFI_FORMAT);
    CHECK(cs_uefi_check_map(raw,41,40,1,&count)==CS_UEFI_LIMIT);
    CHECK(cs_uefi_check_map(raw,0,40,1,&count)==CS_UEFI_LIMIT);
    put64(raw+8,4097); CHECK(cs_uefi_check_map(raw,40,40,1,&count)==CS_UEFI_FORMAT);
    put64(raw+8,4096); put64(raw+16,1); CHECK(cs_uefi_check_map(raw,40,40,1,&count)==CS_UEFI_FORMAT);
    put64(raw+16,0); put64(raw+24,0); CHECK(cs_uefi_check_map(raw,40,40,1,&count)==CS_UEFI_FORMAT);
    put64(raw+24,UINT64_MAX); CHECK(cs_uefi_check_map(raw,40,40,1,&count)==CS_UEFI_OVERFLOW);
    descriptor(raw,2,UINT64_MAX-4095,1,0); CHECK(cs_uefi_check_map(raw,40,40,1,&count)==CS_UEFI_OVERFLOW);
    descriptor(raw,2,4096,1,0); put64(raw+16,UINT64_MAX-4095);
    CHECK(cs_uefi_check_map(raw,40,40,1,&count)==CS_UEFI_OVERFLOW);
    CHECK(count==123);
    unsigned char *large=malloc(262145);
    CHECK(large!=NULL);
    memset(large,0xA5,262145);
    for(size_t i=0;i<1024;++i) descriptor(large+i*256,7,((uint64_t)i+1)*4096,1,0);
    int limits=cs_uefi_check_map(large,262144,256,1,&count)==CS_UEFI_OK && count==1024;
    count=123;
    limits=limits && cs_uefi_check_map(large,262145,256,1,&count)==CS_UEFI_LIMIT
        && cs_uefi_check_map(large,1025*40,40,1,&count)==CS_UEFI_LIMIT && count==123
        && large[262144]==0xA5;
    free(large); CHECK(limits);
    printf("UEFI map: %zu bitmap-oracle cases and maximum map PASS\n",cases); return 0;
}
static int ownership(void)
{
    unsigned char map[96],before[96];
    cs_uefi_owned_span spans[2]={{4096,8192,1},{16384,8192,2}},saved[2];
    cs_uefi_framebuffer f=good_fb();
    memset(map,0xA5,sizeof(map)); descriptor(map,2,16384,4,0); descriptor(map+48,1,4096,3,0);
    memcpy(before,map,sizeof(map)); memcpy(saved,spans,sizeof(spans));
    CHECK(cs_uefi_check_owned(map,96,48,1,spans,2,&f)==CS_UEFI_OK);
    CHECK(memcmp(before,map,sizeof(map))==0 && memcmp(saved,spans,sizeof(spans))==0);
    spans[0].kind=2; CHECK(cs_uefi_check_owned(map,96,48,1,spans,2,&f)==CS_UEFI_OWNERSHIP);
    memcpy(spans,saved,sizeof(spans)); put64(map+32,UINT64_C(1)<<63);
    CHECK(cs_uefi_check_owned(map,96,48,1,spans,2,&f)==CS_UEFI_OWNERSHIP);
    memcpy(map,before,sizeof(map)); put32(map,7);
    CHECK(cs_uefi_check_owned(map,96,48,1,spans,2,&f)==CS_UEFI_OWNERSHIP);
    memcpy(map,before,sizeof(map)); spans[1].base=8192;
    CHECK(cs_uefi_check_owned(map,96,48,1,spans,2,&f)==CS_UEFI_OVERLAP);
    memcpy(spans,saved,sizeof(spans)); spans[1].size=20480;
    CHECK(cs_uefi_check_owned(map,96,48,1,spans,2,&f)==CS_UEFI_OWNERSHIP);
    memcpy(spans,saved,sizeof(spans)); f.base=16384;
    CHECK(cs_uefi_check_owned(map,96,48,1,spans,2,&f)==CS_UEFI_OVERLAP);
    f=good_fb(); spans[1].base=(UINT64_C(1)<<47)-4096;
    CHECK(cs_uefi_check_owned(map,96,48,1,spans,2,&f)==CS_UEFI_OVERFLOW);
    memcpy(spans,saved,sizeof(spans)); spans[1].size=1;
    CHECK(cs_uefi_check_owned(map,96,48,1,spans,2,&f)==CS_UEFI_FORMAT);
    cs_uefi_owned_span excessive[9]={{0,0,0}};
    CHECK(cs_uefi_check_owned(map,96,48,1,excessive,9,&f)==CS_UEFI_LIMIT);
    CHECK(cs_uefi_check_owned(map,96,48,1,NULL,2,&f)==CS_UEFI_ARGUMENT);
    descriptor(map,2,16384,16,0);
    cs_uefi_owned_span eight[8];
    for(size_t i=0;i<8;++i) { eight[i].base=16384+(uint64_t)i*4096; eight[i].size=4096; eight[i].kind=2; }
    CHECK(cs_uefi_check_owned(map,96,48,1,eight,8,&f)==CS_UEFI_OK);
    puts("UEFI allocation ownership: PASS"); return 0;
}
static int exit_transactions(void)
{
    size_t cases=0;
    for(unsigned code=0;code<27;++code) {
        cs_uefi_exit_state state; unsigned sequence=code; int terminal=0;
        CHECK(cs_uefi_exit_init(&state)==CS_UEFI_OK);
        CHECK(cs_uefi_exit_allowed(&state,CS_UEFI_OTHER_BOOT));
        CHECK(!cs_uefi_exit_allowed(&state,CS_UEFI_NATIVE_LOOP));
        for(uint32_t attempt=1;attempt<=3;++attempt) {
            uint32_t outcome=sequence%3; sequence/=3;
            CHECK(cs_uefi_exit_allowed(&state,CS_UEFI_GET_MAP));
            CHECK(cs_uefi_exit_snapshot(&state,attempt==1?0:UINT64_MAX-attempt)==CS_UEFI_OK);
            CHECK(state.key==(attempt==1?0:UINT64_MAX-attempt));
            CHECK(cs_uefi_exit_allowed(&state,CS_UEFI_EXIT_BOOT));
            CHECK(!cs_uefi_exit_allowed(&state,CS_UEFI_GET_MAP));
            CHECK(!cs_uefi_exit_allowed(&state,CS_UEFI_OTHER_BOOT));
            cs_uefi_exit_state before=state;
            CHECK(cs_uefi_exit_observe(&state,3)==CS_UEFI_FORMAT);
            CHECK(state.key==before.key && state.phase==before.phase && state.attempts==before.attempts);
            CHECK(cs_uefi_exit_snapshot(&state,123)==CS_UEFI_STATE);
            CHECK(cs_uefi_exit_observe(&state,outcome)==CS_UEFI_OK && state.attempts==attempt);
            if(outcome==0) { CHECK(state.phase==CS_UEFI_EXITED && cs_uefi_exit_allowed(&state,CS_UEFI_NATIVE_LOOP)); terminal=1; }
            else if(outcome==2 || attempt==3) { CHECK(state.phase==CS_UEFI_FAILED && !cs_uefi_exit_allowed(&state,CS_UEFI_NATIVE_LOOP)); terminal=1; }
            else CHECK(state.phase==CS_UEFI_RETRY && !cs_uefi_exit_allowed(&state,CS_UEFI_OTHER_BOOT));
            if(terminal) break;
        }
        CHECK(terminal);
        cs_uefi_exit_state before=state;
        CHECK(cs_uefi_exit_observe(&state,0)==CS_UEFI_STATE);
        CHECK(cs_uefi_exit_snapshot(&state,5)==CS_UEFI_STATE);
        CHECK(state.phase==before.phase && state.attempts==before.attempts && state.key==before.key);
        CHECK(!cs_uefi_exit_allowed(&state,CS_UEFI_OTHER_BOOT) && !cs_uefi_exit_allowed(&state,CS_UEFI_GET_MAP));
        ++cases;
    }
    CHECK(cs_uefi_exit_init(NULL)==CS_UEFI_ARGUMENT);
    CHECK(cs_uefi_exit_snapshot(NULL,0)==CS_UEFI_ARGUMENT);
    CHECK(cs_uefi_exit_observe(NULL,0)==CS_UEFI_ARGUMENT);
    CHECK(!cs_uefi_exit_allowed(NULL,0));
    cs_uefi_exit_state malformed={CS_UEFI_RETRY,0,17};
    CHECK(cs_uefi_exit_snapshot(&malformed,0)==CS_UEFI_STATE && !cs_uefi_exit_allowed(&malformed,CS_UEFI_GET_MAP));
    printf("UEFI exit transaction: %zu finite outcome traces PASS\n",cases); return 0;
}
static int independent(void)
{
    unsigned char map[40],copy[40]; size_t first=77,second=88;
    cs_uefi_framebuffer f=good_fb(); cs_uefi_framebuffer_layout a={1,2},b={3,4};
    memset(map,0xA5,sizeof(map)); descriptor(map,2,4096,1,0); memcpy(copy,map,sizeof(map));
    CHECK(cs_uefi_check_map(map,40,40,1,&first)==CS_UEFI_OK && first==1 && second==88);
    CHECK(cs_uefi_check_framebuffer(&f,&a)==CS_UEFI_OK && b.stride==3 && b.required==4);
    CHECK(memcmp(copy,map,sizeof(map))==0);
    cs_uefi_exit_state x,y;
    CHECK(cs_uefi_exit_init(&x)==CS_UEFI_OK && cs_uefi_exit_init(&y)==CS_UEFI_OK);
    CHECK(cs_uefi_exit_snapshot(&x,9)==CS_UEFI_OK && y.phase==CS_UEFI_READY && y.attempts==0 && y.key==0);
    CHECK(cs_uefi_exit_observe(&x,0)==CS_UEFI_OK && y.phase==CS_UEFI_READY);
    puts("UEFI independent caller ownership: PASS"); return 0;
}
int main(int argc,char **argv)
{
    if(argc!=2) return 2;
    if(strcmp(argv[1],"framebuffer")==0) return framebuffer();
    if(strcmp(argv[1],"map")==0) return maps();
    if(strcmp(argv[1],"ownership")==0) return ownership();
    if(strcmp(argv[1],"exit")==0) return exit_transactions();
    if(strcmp(argv[1],"independent")==0) return independent();
    return 2;
}
