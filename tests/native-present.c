/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../platform/uefi/native.h"
#include "scene-oracle.h"
#include <stdio.h>
#include <string.h>
static unsigned failures;
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); ++failures; } } while(0)
enum { W=640, H=480, PITCH=656, SPAN=PITCH*4*H, GUARD=64 };
static unsigned char fb[SPAN+2*GUARD];
static _Alignas(16) unsigned char arena[CS_LOADER_ARENA_BYTES];
static unsigned char trace[64];
static uint32_t read32(const unsigned char *p)
{
    return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);
}
static cs_uefi_handoff exited(uint32_t format)
{
    cs_uefi_handoff h;
    memset(&h,0,sizeof h);
    h.stage=CS_UEFI_EXITED; h.firmware_status=0; h.exit_attempts=1;
    h.framebuffer.base=UINT64_C(0x80000000); h.framebuffer.size=SPAN;
    h.framebuffer.width=W; h.framebuffer.height=H; h.framebuffer.pitch=PITCH;
    h.framebuffer.format=format; h.arena_size=CS_LOADER_ARENA_BYTES;
    return h;
}
static void reset(void)
{
    memset(fb,0xA5,sizeof fb); memset(trace,0xE7,sizeof trace); memset(arena,0x3D,sizeof arena);
}
static int fb_untouched(void)
{
    for(size_t i=0;i<sizeof fb;++i) if(fb[i]!=0xA5) return 0;
    return 1;
}
static void expect_trace(uint32_t result,uint32_t stage,uint32_t bands,uint32_t rows)
{
    CHECK(read32(trace)==0x31525043u); CHECK(read32(trace+4)==1u);
    CHECK(read32(trace+8)==result); CHECK(read32(trace+12)==stage);
    CHECK(read32(trace+16)==bands); CHECK(read32(trace+24)==rows);
    CHECK(read32(trace+28)==0u);
    for(unsigned i=32;i<64;++i) CHECK(trace[i]==0xE7);
}
static void gating(void)
{
    cs_uefi_handoff h;
    for(uint32_t stage=0;stage<=4;++stage) {
        if(stage==CS_UEFI_EXITED) continue;
        reset(); h=exited(0); h.stage=stage;
        CHECK(cs_native_present(&h,1,fb+GUARD,arena,trace)==CS_NATIVE_NOT_EXITED);
        CHECK(fb_untouched()); expect_trace(2,stage,0,0); CHECK(read32(trace+20)==0);
    }
    reset(); h=exited(0); h.firmware_status=UINT64_C(0x8000000000000002);
    CHECK(cs_native_present(&h,1,fb+GUARD,arena,trace)==CS_NATIVE_NOT_EXITED);
    CHECK(fb_untouched()); expect_trace(2,3,0,0);
    for(uint32_t ready=0;ready<4;ready+=2) {
        reset(); h=exited(1);
        CHECK(cs_native_present(&h,ready,fb+GUARD,arena,trace)==CS_NATIVE_NOT_READY);
        CHECK(fb_untouched()); expect_trace(3,3,0,0);
    }
    reset(); h=exited(0);
    CHECK(cs_native_present(&h,1,fb+GUARD,arena,NULL)==CS_NATIVE_ARGUMENT);
    for(unsigned i=0;i<64;++i) CHECK(trace[i]==0xE7);
    CHECK(cs_native_present(NULL,1,fb+GUARD,arena,trace)==CS_NATIVE_ARGUMENT); expect_trace(1,0,0,0);
    reset(); CHECK(cs_native_present(&h,1,NULL,arena,trace)==CS_NATIVE_ARGUMENT); expect_trace(1,3,0,0);
    reset(); CHECK(cs_native_present(&h,1,fb+GUARD,NULL,trace)==CS_NATIVE_ARGUMENT); expect_trace(1,3,0,0);
    CHECK(fb_untouched());
}
static void rejection(void)
{
    cs_uefi_handoff h;
    reset(); h=exited(2);
    CHECK(cs_native_present(&h,1,fb+GUARD,arena,trace)==CS_NATIVE_TARGET); expect_trace(4,3,0,0);
    reset(); h=exited(0); h.framebuffer.size=SPAN-1u;
    CHECK(cs_native_present(&h,1,fb+GUARD,arena,trace)==CS_NATIVE_TARGET); expect_trace(4,3,0,0);
    reset(); h=exited(0); h.framebuffer.base=0;
    CHECK(cs_native_present(&h,1,fb+GUARD,arena,trace)==CS_NATIVE_TARGET); expect_trace(4,3,0,0);
    reset(); h=exited(0); h.framebuffer.pitch=W-1u;
    CHECK(cs_native_present(&h,1,fb+GUARD,arena,trace)==CS_NATIVE_TARGET); expect_trace(4,3,0,0);
    reset(); h=exited(0); h.arena_size=CS_LOADER_ARENA_BYTES-16u;
    CHECK(cs_native_present(&h,1,fb+GUARD,arena,trace)==CS_NATIVE_ARENA); expect_trace(5,3,0,0);
    CHECK(fb_untouched());
    for(size_t i=0;i<sizeof arena;++i) if(arena[i]!=0x3D) { CHECK(arena[i]==0x3D); break; }
}
static void presentation(void)
{
    for(uint32_t format=0;format<2;++format) {
        cs_uefi_handoff h=exited(format),copy;
        uint32_t rows=(uint32_t)(65536/(W*4));
        reset(); memcpy(&copy,&h,sizeof h);
        CHECK(cs_native_present(&h,1,fb+GUARD,arena,trace)==CS_NATIVE_PRESENTED);
        expect_trace(0,3,(H+rows-1u)/rows,H); CHECK(read32(trace+20)==rows);
        CHECK(memcmp(&h,&copy,sizeof h)==0);
        for(unsigned i=0;i<GUARD;++i) { CHECK(fb[i]==0xA5); CHECK(fb[GUARD+SPAN+i]==0xA5); }
        for(long y=0;y<H;++y) {
            const unsigned char *row=fb+GUARD+(size_t)y*PITCH*4u;
            for(long x=0;x<W;++x) {
                uint32_t c=oracle_scene(W,H,0,x,y);
                unsigned char r=(unsigned char)(c>>24),g=(unsigned char)(c>>16),b=(unsigned char)(c>>8);
                const unsigned char *p=row+x*4;
                if(!(p[0]==(format==0?r:b) && p[1]==g && p[2]==(format==0?b:r) && p[3]==0)) {
                    CHECK(0 && "scene pixel"); y=H; break;
                }
            }
            for(size_t p=(size_t)W*4u;p<(size_t)PITCH*4u;++p) CHECK(row[p]==0xA5);
        }
        /* Staging used only the first 64 KiB of the arena; the rest is untouched. */
        for(size_t i=(size_t)rows*W*4u;i<sizeof arena;++i) if(arena[i]!=0x3D) { CHECK(arena[i]==0x3D); break; }
    }
}
int main(int argc,char **argv)
{
    const char *suite=argc>1?argv[1]:"";
    if(strcmp(suite,"gating")==0) gating();
    else if(strcmp(suite,"rejection")==0) rejection();
    else if(strcmp(suite,"presentation")==0) presentation();
    else { fprintf(stderr,"unknown suite\n"); return 2; }
    if(failures!=0) { fprintf(stderr,"%u failures\n",failures); return 1; }
    printf("native presentation %s: PASS\n",suite);
    return 0;
}
