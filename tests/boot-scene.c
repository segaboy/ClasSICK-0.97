/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../apps/boot-scene/scene.h"
#include "scene-oracle.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned failures;
static unsigned long pixels_checked;
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); ++failures; } } while(0)
static const long sizes[][2]={{1,1},{2,3},{3,2},{31,17},{64,48},{100,7},{7,100},
    {333,211},{640,480},{1024,768}};
static const uint32_t steps[]={0,1,5,16,17,33,UINT32_MAX};
static uint32_t rgba(const unsigned char *p)
{
    return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|((uint32_t)p[2]<<8)|p[3];
}
static unsigned char *checked_alloc(size_t bytes)
{
    unsigned char *p=malloc(bytes);
    if(p==NULL) { fprintf(stderr,"allocation failed\n"); exit(3); }
    return p;
}
static void oracle(void)
{
    for(unsigned s=0;s<sizeof sizes/sizeof sizes[0];++s) for(unsigned k=0;k<7;++k) {
        long w=sizes[s][0],h=sizes[s][1];
        /* Large frames sample two steps to bound Debug/sanitizer runtime. */
        if(w*h>100000L && k!=0 && k!=4) continue;
        size_t stride=(size_t)w*4u+3u,bytes=stride*(size_t)h+1u;
        unsigned char *raw=checked_alloc(bytes+1u),*storage=raw+1;   /* unaligned */
        cs_surface band;
        memset(raw,0x6D,bytes+1u);
        CHECK(cs_surface_init(&band,storage,bytes,(int32_t)w,(int32_t)h,stride,CS_SURFACE_RGBA8)==CS_SURFACE_OK);
        CHECK(cs_scene_render(&band,(int32_t)w,(int32_t)h,0,steps[k])==CS_SCENE_OK);
        for(long y=0;y<h;++y) {
            const unsigned char *row=storage+(size_t)y*stride;
            for(long x=0;x<w;++x) { CHECK(rgba(row+x*4)==oracle_scene(w,h,steps[k],x,y)); ++pixels_checked; }
            for(size_t p=(size_t)w*4u;p<stride;++p) CHECK(row[p]==0x6D);
        }
        CHECK(raw[0]==0x6D && storage[bytes-1u]==0x6D);
        free(raw);
    }
    /* Literal corner/progress spot checks independent of the oracle. */
    {
        static unsigned char px[64*48*4];
        cs_surface s;
        CHECK(cs_surface_init(&s,px,sizeof px,64,48,256,CS_SURFACE_RGBA8)==CS_SURFACE_OK);
        CHECK(cs_scene_render(&s,64,48,0,3)==CS_SCENE_OK);
        CHECK(rgba(px)==0xFF0000FFu); CHECK(rgba(px+63*4)==0x00FF00FFu);
        CHECK(rgba(px+47*256)==0x0000FFFFu); CHECK(rgba(px+47*256+63*4)==0xFFFFFFFFu);
        CHECK(rgba(px+4*256+4*4)==0x1E3A4CFFu);
        /* u=1: panel 32x16 at (16,16); inner (17,17) 30x14; bars rows 17..23. */
        CHECK(rgba(px+16*256+16*4)==0xF2F2F2FFu); CHECK(rgba(px+17*256+17*4)==0xFFFFFFFFu);
        CHECK(rgba(px+17*256+46*4)==0x000000FFu);
        /* Track x 18..45, rows 25..29; segment 0 lit, segment 3 unlit (step 3). */
        CHECK(rgba(px+25*256+18*4)==0x7FD35FFFu); CHECK(rgba(px+25*256+(18+28*3/16)*4)==0x3A3A3AFFu);
    }
}
static void bands(void)
{
    static const long heights[]={1,2,3,7,64};
    for(unsigned s=0;s<sizeof sizes/sizeof sizes[0];++s) {
        long w=sizes[s][0],h=sizes[s][1];
        size_t stride=(size_t)w*4u,frame=stride*(size_t)h;
        unsigned char *whole=checked_alloc(frame),*parts=checked_alloc(frame);
        cs_surface all;
        memset(whole,0,frame); memset(parts,0xFF,frame);
        CHECK(cs_surface_init(&all,whole,frame,(int32_t)w,(int32_t)h,stride,CS_SURFACE_RGBA8)==CS_SURFACE_OK);
        CHECK(cs_scene_render(&all,(int32_t)w,(int32_t)h,0,9)==CS_SCENE_OK);
        for(unsigned b=0;b<5;++b) {
            for(long top=0;top<h;top+=heights[b]) {
                long rows=h-top<heights[b]?h-top:heights[b];
                cs_surface band;
                CHECK(cs_surface_init(&band,parts+(size_t)top*stride,(size_t)rows*stride,(int32_t)w,
                    (int32_t)rows,stride,CS_SURFACE_RGBA8)==CS_SURFACE_OK);
                CHECK(cs_scene_render(&band,(int32_t)w,(int32_t)h,(int32_t)top,9)==CS_SCENE_OK);
            }
            CHECK(memcmp(whole,parts,frame)==0);
            memset(parts,0xFF,frame);
        }
        free(whole); free(parts);
    }
    /* A band wider/taller than the scene leaves pixels outside the scene untouched. */
    {
        static unsigned char wide[20*6*4];
        cs_surface s;
        memset(wide,0x42,sizeof wide);
        CHECK(cs_surface_init(&s,wide,sizeof wide,20,6,80,CS_SURFACE_RGBA8)==CS_SURFACE_OK);
        CHECK(cs_scene_render(&s,12,4,1,0)==CS_SCENE_OK);
        for(long y=0;y<6;++y) for(long x=0;x<20;++x) {
            const unsigned char *p=wide+y*80+x*4;
            if(x<12 && y<3) CHECK(rgba(p)==oracle_scene(12,4,0,x,y+1));
            else CHECK(p[0]==0x42 && p[1]==0x42 && p[2]==0x42 && p[3]==0x42);
        }
    }
}
static void budget(void)
{
    static const uint32_t widths[]={1,3,640,1024,4096,8191,8192};
    static _Alignas(16) unsigned char arena_bytes[131072];
    for(unsigned i=0;i<7;++i) {
        cs_arena arena; cs_fb_target t; cs_boot_scene scene;
        unsigned char dummy[4];
        uint32_t w=widths[i];
        size_t expected=65536u/((size_t)w*4u);
        if(expected>32u) expected=32u;
        CHECK(cs_arena_init(&arena,arena_bytes,sizeof arena_bytes)==CS_ARENA_OK);
        CHECK(cs_fb_init(&t,dummy,(size_t)w*4u*32u,w,32,w,CS_FB_RGB_RESERVED)==CS_FB_OK);
        CHECK(cs_boot_scene_prepare(&arena,&t,&scene)==CS_BOOT_OK);
        CHECK(scene.band_rows==expected && scene.band_rows>=1u);
        CHECK(scene.staging_bytes==expected*(size_t)w*4u && scene.staging_bytes<=65536u);
        CHECK(scene.staging==arena_bytes && ((uintptr_t)scene.staging%(CS_ARENA_MAX_ALIGNMENT<16u?CS_ARENA_MAX_ALIGNMENT:16u))==0u);
        CHECK(arena.used==scene.staging_bytes);
    }
    /* Exhaustion: arena preserved, output untouched. */
    {
        cs_arena arena; cs_fb_target t; cs_boot_scene scene; unsigned char dummy[4];
        CHECK(cs_arena_init(&arena,arena_bytes,4096)==CS_ARENA_OK);
        CHECK(cs_fb_init(&t,dummy,(size_t)8192*4u*4u,8192,4,8192,CS_FB_BGR_RESERVED)==CS_FB_OK);
        memset(&scene,0x3C,sizeof scene);
        CHECK(cs_boot_scene_prepare(&arena,&t,&scene)==CS_BOOT_ARENA);
        CHECK(arena.used==0 && scene.band_rows==0x3C3C3C3Cu);
    }
    /* Whole-frame drawing in both formats against the oracle, with unusual pitch. */
    for(unsigned f=0;f<2;++f) for(unsigned s=0;s<sizeof sizes/sizeof sizes[0];++s) {
        long w=sizes[s][0],h=sizes[s][1];
        uint32_t pitch=(uint32_t)w+5u;
        size_t span=(size_t)pitch*4u*(size_t)h;
        unsigned char *fb=checked_alloc(span+64u);
        cs_arena arena; cs_fb_target t; cs_boot_scene scene; cs_boot_report report;
        memset(fb,0x99,span+64u);
        CHECK(cs_arena_init(&arena,arena_bytes,sizeof arena_bytes)==CS_ARENA_OK);
        CHECK(cs_fb_init(&t,fb+32,span,(uint32_t)w,(uint32_t)h,pitch,f)==CS_FB_OK);
        CHECK(cs_boot_scene_prepare(&arena,&t,&scene)==CS_BOOT_OK);
        CHECK(cs_boot_scene_draw(&scene,12,&report)==CS_BOOT_OK);
        CHECK(report.result==0 && report.rows==(uint32_t)h && report.step==12);
        CHECK(report.band_rows==scene.band_rows);
        CHECK(report.bands==((uint32_t)h+scene.band_rows-1u)/scene.band_rows);
        for(long y=0;y<h;++y) {
            const unsigned char *row=fb+32+(size_t)y*pitch*4u;
            for(long x=0;x<w;++x) {
                uint32_t c=oracle_scene(w,h,12,x,y);
                unsigned char r=(unsigned char)(c>>24),g=(unsigned char)(c>>16),b=(unsigned char)(c>>8);
                const unsigned char *p=row+x*4;
                CHECK(p[0]==(f==0?r:b) && p[1]==g && p[2]==(f==0?b:r) && p[3]==0);
            }
            for(size_t p=(size_t)w*4u;p<(size_t)pitch*4u;++p) CHECK(row[p]==0x99);
        }
        for(unsigned i=0;i<32;++i) { CHECK(fb[i]==0x99); CHECK(fb[32+span+i]==0x99); }
        free(fb);
    }
}
static void errors(void)
{
    static unsigned char px[8*4*4],copy[sizeof px];
    static _Alignas(16) unsigned char arena_bytes[4096];
    cs_surface s,bad; cs_arena arena; cs_fb_target t; cs_boot_scene scene,tampered;
    cs_boot_report report;
    unsigned char fb[8*4*4];
    memset(px,0x31,sizeof px); memcpy(copy,px,sizeof px);
    CHECK(cs_surface_init(&s,px,sizeof px,8,4,32,CS_SURFACE_RGBA8)==CS_SURFACE_OK);
    CHECK(cs_scene_render(NULL,8,4,0,0)==CS_SCENE_ARGUMENT);
    CHECK(cs_scene_render(&s,0,4,0,0)==CS_SCENE_DIMENSION);
    CHECK(cs_scene_render(&s,8,0,0,0)==CS_SCENE_DIMENSION);
    CHECK(cs_scene_render(&s,8193,4,0,0)==CS_SCENE_DIMENSION);
    CHECK(cs_scene_render(&s,8,8193,0,0)==CS_SCENE_DIMENSION);
    CHECK(cs_scene_render(&s,8,4,-1,0)==CS_SCENE_DIMENSION);
    CHECK(cs_scene_render(&s,8,4,4,0)==CS_SCENE_DIMENSION);
    bad=s; bad.stride=31; CHECK(cs_scene_render(&bad,8,4,0,0)==CS_SCENE_SURFACE);
    bad=s; bad.storage=NULL; CHECK(cs_scene_render(&bad,8,4,0,0)==CS_SCENE_SURFACE);
    /* Dimension errors precede descriptor errors. */
    CHECK(cs_scene_render(&bad,0,4,0,0)==CS_SCENE_DIMENSION);
    CHECK(cs_surface_init(&bad,px,sizeof px,8,4,1,CS_SURFACE_MONO1_MSB)==CS_SURFACE_OK);
    CHECK(cs_scene_render(&bad,8,4,0,0)==CS_SCENE_FORMAT);
    CHECK(memcmp(px,copy,sizeof px)==0);
    memset(fb,0x17,sizeof fb);
    CHECK(cs_arena_init(&arena,arena_bytes,sizeof arena_bytes)==CS_ARENA_OK);
    CHECK(cs_fb_init(&t,fb,sizeof fb,8,4,8,0)==CS_FB_OK);
    CHECK(cs_boot_scene_prepare(NULL,&t,&scene)==CS_BOOT_ARGUMENT);
    CHECK(cs_boot_scene_prepare(&arena,NULL,&scene)==CS_BOOT_ARGUMENT);
    CHECK(cs_boot_scene_prepare(&arena,&t,NULL)==CS_BOOT_ARGUMENT);
    t.format=4;
    CHECK(cs_boot_scene_prepare(&arena,&t,&scene)==CS_BOOT_TARGET && arena.used==0);
    t.format=0;
    CHECK(cs_boot_scene_prepare(&arena,&t,&scene)==CS_BOOT_OK);
    CHECK(cs_boot_scene_draw(NULL,1,&report)==CS_BOOT_ARGUMENT && report.result==1 && report.step==1);
    CHECK(cs_boot_scene_draw(NULL,1,NULL)==CS_BOOT_ARGUMENT);
    tampered=scene; tampered.target.size=1;
    CHECK(cs_boot_scene_draw(&tampered,1,&report)==CS_BOOT_TARGET && report.rows==0);
    tampered=scene; tampered.staging=NULL;
    CHECK(cs_boot_scene_draw(&tampered,1,&report)==CS_BOOT_ARENA);
    tampered=scene; tampered.band_rows=0;
    CHECK(cs_boot_scene_draw(&tampered,1,&report)==CS_BOOT_ARENA);
    tampered=scene; tampered.band_rows=5;
    CHECK(cs_boot_scene_draw(&tampered,1,&report)==CS_BOOT_ARENA);
    tampered=scene; tampered.staging_bytes=scene.staging_bytes-1u;
    CHECK(cs_boot_scene_draw(&tampered,1,&report)==CS_BOOT_ARENA && report.bands==0);
    for(unsigned i=0;i<sizeof fb;++i) CHECK(fb[i]==0x17);
    CHECK(cs_boot_scene_draw(&scene,1,NULL)==CS_BOOT_OK);
    CHECK(fb[3]==0);
}
int main(int argc,char **argv)
{
    const char *suite=argc>1?argv[1]:"";
    if(strcmp(suite,"oracle")==0) oracle();
    else if(strcmp(suite,"bands")==0) bands();
    else if(strcmp(suite,"budget")==0) budget();
    else if(strcmp(suite,"errors")==0) errors();
    else { fprintf(stderr,"unknown suite\n"); return 2; }
    if(pixels_checked!=0) printf("boot scene %s: %lu oracle pixels\n",suite,pixels_checked);
    if(failures!=0) { fprintf(stderr,"%u failures\n",failures); return 1; }
    printf("boot scene %s: PASS\n",suite);
    return 0;
}
