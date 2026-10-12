/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../platform/mac128k/rom.h"
#include "../arch/m68k/runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#define CHECK(x) do { if(!(x)) {fprintf(stderr,"check failed line %d\n",__LINE__); return 1;} } while(0)

static uint32_t random32(uint32_t *s)
{
    *s=*s*UINT32_C(1664525)+UINT32_C(1013904223); return *s;
}
static int helpers(void)
{
    uint32_t seed=123;
    for(unsigned i=0;i<100000u;++i) {
        uint32_t a=random32(&seed),b=random32(&seed)|1u;
        int32_t sa=(int32_t)(a & UINT32_C(0x7FFFFFFF));
        int32_t sb=(int32_t)(b & UINT32_C(0x7FFFFFFF));
        if(sb==0) sb=1;
        if((i & 1u)!=0u) sa=-sa;
        if((i & 2u)!=0u) sb=-sb;
        CHECK(__mulsi3(a,b)==(uint32_t)((uint64_t)a*b));
        CHECK(__udivsi3(a,b)==(uint32_t)((uint64_t)a/b));
        CHECK(__umodsi3(a,b)==(uint32_t)((uint64_t)a%b));
        CHECK(__divsi3(sa,sb)==(int32_t)((int64_t)sa/sb));
    }
    CHECK(__divsi3(INT32_MIN,-1)==INT32_MIN);
    CHECK(__divsi3(INT32_MIN,1)==INT32_MIN);
    CHECK(__divsi3(INT32_MIN,INT32_MIN)==1);
    CHECK(__divsi3(1,INT32_MIN)==0);
    for(unsigned bit=0;bit<32u;++bit) {
        uint32_t divisor=UINT32_C(1)<<bit;
        CHECK(__udivsi3(UINT32_MAX,divisor)==(uint32_t)(UINT64_C(0xFFFFFFFF)/divisor));
        CHECK(__umodsi3(UINT32_MAX,divisor)==(uint32_t)(UINT64_C(0xFFFFFFFF)%divisor));
    }
    for(size_t count=0;count<=64;++count) {
        unsigned char source[64],destination[66];
        for(size_t i=0;i<64;++i) source[i]=(unsigned char)(i^0x53u);
        for(size_t i=0;i<66;++i) destination[i]=0xA5;
        CHECK(memcpy(destination+1,source,count)==destination+1);
        CHECK(destination[0]==0xA5 && destination[count+1]==0xA5);
        for(size_t i=0;i<count;++i) CHECK(destination[i+1]==source[i]);
    }
    puts("helpers: 100000 independent arithmetic pairs and boundaries PASS"); return 0;
}
typedef struct { uint8_t regs[16]; unsigned acknowledgments,writes; uint32_t touched; } fixture;
static uint8_t read_reg(void *context,unsigned reg)
{
    fixture *f=context;
    if(reg>=16u) abort();
    if(reg==1u) {++f->acknowledgments; f->regs[13]&=(uint8_t)~2u;}
    return f->regs[reg];
}
static void write_reg(void *context,unsigned reg,uint8_t value)
{
    fixture *f=context; if(reg>=16u) abort();
    f->regs[reg]=value; ++f->writes; f->touched|=UINT32_C(1)<<reg;
}
static int controller(void)
{
    fixture f={{0},0,0,0}; cs_mac_io io={&f,read_reg,write_reg};
    f.regs[13]=0x42;
    cs_mac_via_init(&io);
    CHECK(f.writes==5u && f.acknowledgments==1u);
    CHECK(f.touched==((1u<<14)|(1u<<11)|(1u<<12)|(1u<<0)|(1u<<2)));
    CHECK(f.regs[14]==0x7Fu && f.regs[0]==0x80u && f.regs[2]==0x80u);
    CHECK(f.regs[11]==0 && f.regs[12]==0 && f.regs[13]==0x40);
    return 0;
}
static int mouse(void)
{
    fixture f={{0},0,0,0}; cs_mac_io io={&f,read_reg,write_reg}; cs_mac_loop loop;
    cs_mac_loop_begin(&loop,0);
    CHECK(!cs_mac_loop_poll(&loop,&io) && loop.presses==0u);
    f.regs[0]=8; CHECK(!cs_mac_loop_poll(&loop,&io));
    f.regs[0]=0; CHECK(cs_mac_loop_poll(&loop,&io) && loop.presses==1u);
    for(unsigned i=0;i<100u;++i) CHECK(!cs_mac_loop_poll(&loop,&io));
    f.regs[0]=8; (void)cs_mac_loop_poll(&loop,&io);
    f.regs[0]=0; CHECK(cs_mac_loop_poll(&loop,&io) && loop.presses==2u);
    loop.presses=UINT32_MAX; f.regs[0]=8; (void)cs_mac_loop_poll(&loop,&io);
    f.regs[0]=0; CHECK(cs_mac_loop_poll(&loop,&io) && loop.presses==UINT32_MAX);
    CHECK(loop.ticks==0 && f.acknowledgments==0 && f.writes==0);
    return 0;
}
static int clock_loop(void)
{
    fixture f={{0},0,0,0}; cs_mac_io io={&f,read_reg,write_reg}; cs_mac_loop loop;
    f.regs[0]=8; cs_mac_loop_begin(&loop,8);
    for(unsigned i=0;i<1000u;++i) CHECK(!cs_mac_loop_poll(&loop,&io));
    CHECK(loop.ticks==0u && loop.synchronized==0u);
    f.regs[13]=2; (void)cs_mac_loop_poll(&loop,&io);
    CHECK(loop.synchronized==1u && loop.ticks==0u);
    for(unsigned i=1;i<=CS_MAC_TOTAL_TICKS+100u;++i) {
        f.regs[13]=2; (void)cs_mac_loop_poll(&loop,&io);
        CHECK(loop.ticks==(i<CS_MAC_TOTAL_TICKS ? i : CS_MAC_TOTAL_TICKS));
        CHECK((f.regs[13]&2u)==0u);
        CHECK(!cs_mac_loop_poll(&loop,&io));
    }
    CHECK(f.acknowledgments==CS_MAC_TOTAL_TICKS+101u && f.writes==0);
    f.regs[0]=0; CHECK(cs_mac_loop_poll(&loop,&io) && loop.presses==1u);
    return 0;
}
static int pixel(const unsigned char *b,unsigned x,unsigned y)
{
    return (b[y*64u+x/8u] & (0x80u>>(x%8u)))!=0u;
}
static int pixels(void)
{
    unsigned char a[CS_MAC_SCREEN_BYTES+16],b[CS_MAC_SCREEN_BYTES]; cs_surface surface;
    for(size_t i=0;i<sizeof(a);++i) a[i]=0xA5;
    CHECK(cs_surface_init(&surface,a+8,CS_MAC_SCREEN_BYTES,512,342,64,CS_SURFACE_MONO1_MSB)==CS_SURFACE_OK);
    CHECK(cs_mac_scene(&surface,0,0));
    for(unsigned i=0;i<8u;++i) CHECK(a[i]==0xA5 && a[CS_MAC_SCREEN_BYTES+8u+i]==0xA5);
    CHECK(!pixel(a+8,0,0) && pixel(a+8,8,8) && !pixel(a+8,255,200));
    CHECK(!pixel(a+8,54,150) && !pixel(a+8,70,150) && pixel(a+8,118,150));
    for(unsigned i=0;i<16u;++i) CHECK(!pixel(a+8,52+i*26u,240));
    for(size_t i=0;i<sizeof(b);++i) b[i]=a[i+8];
    CHECK(cs_mac_scene(&surface,0,1));
    CHECK(pixel(a+8,70,150) && !pixel(a+8,118,150));
    CHECK(memcmp(a+8,b,sizeof(b))!=0);
    CHECK(cs_mac_scene(&surface,CS_MAC_TOTAL_TICKS,1));
    for(unsigned i=0;i<16u;++i) CHECK(pixel(a+8,52+i*26u,240));
    CHECK(!cs_mac_scene(NULL,0,0));
    surface.width=511; CHECK(!cs_mac_scene(&surface,0,0));
    return 0;
}
static int crc_test(void)
{
    const unsigned char digits[]="123456789";
    unsigned char b[CS_MAC_ROM_BYTES]; uint32_t crc;
    CHECK(cs_mac_image_crc(digits,9)==UINT32_C(0xCBF43926));
    for(size_t i=0;i<sizeof(b);++i) b[i]=(unsigned char)(i & 255u);
    crc=cs_mac_image_crc(b,sizeof(b));
    b[CS_MAC_SEAL_OFFSET]^=1u; CHECK(cs_mac_image_crc(b,sizeof(b))==crc);
    b[900]^=1u; CHECK(cs_mac_image_crc(b,sizeof(b))!=crc);
    return 0;
}
int main(int argc,char **argv)
{
    if(argc!=2) return 2;
    if(strcmp(argv[1],"zero")==0) {
#if defined(_WIN32)
        (void)SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
#endif
        return (int)__udivsi3(7,0);
    }
    if(strcmp(argv[1],"helpers")==0) return helpers();
    if(strcmp(argv[1],"controller")==0) return controller();
    if(strcmp(argv[1],"mouse")==0) return mouse();
    if(strcmp(argv[1],"clock")==0) return clock_loop();
    if(strcmp(argv[1],"pixels")==0) return pixels();
    if(strcmp(argv[1],"crc")==0) return crc_test();
    return 2;
}
