/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../apps/product-scene/scene.h"
#include "../apps/boot-scene/scene.h"
#include "../apps/drawing-rom-app/probe.h"
#include "../apps/service-probe/probe.h"
#include "scene-oracle.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"line%d: %s\n",__LINE__,#x);exit(1);}}while(0)
static void *space(size_t n){void *p=malloc(n);CHECK(p);memset(p,0xA5,n);return p;}
static void compare(const char *directory)
{
    const size_t mono_size=70u*342u+19u;
    uint8_t *mono=space(mono_size);cs_surface mac;
    CHECK(cs_surface_init(&mac,mono,mono_size,512,342,70,CS_SURFACE_MONO1_MSB)==CS_SURFACE_OK);
    cs_trap_gateway gateway;cs_service_probe_result service;cs_loaded_app_result result;
    cs_drawing_app_workspace *w=calloc(1,sizeof *w);CHECK(w);
    CHECK(cs_service_probe(&gateway,&service) && cs_drawing_app_probe(w,&gateway,&result));
    CHECK(result.success && result.value==97 && result.phase==6 && result.retired==1);
    for(unsigned mode=0;mode<2;++mode){
        size_t pitch=647u,bytes=pitch*4u*480u;
        uint8_t *pc=space(bytes+31u);void *backing=space(CS_BOOT_STAGING_LIMIT*2u+16u);
        cs_arena arena;cs_fb_target target;cs_boot_scene scene;cs_boot_report report;
        CHECK(cs_arena_init(&arena,backing,CS_BOOT_STAGING_LIMIT*2u)==CS_ARENA_OK);
        CHECK(cs_fb_init(&target,pc,bytes,640,480,(uint32_t)pitch,mode)==CS_FB_OK);
        CHECK(cs_boot_scene_prepare(&arena,&target,&scene)==CS_BOOT_OK);
        CHECK(scene.state.value==result.value && scene.state.phases==result.phase && scene.state.success==result.success);
        CHECK(!memcmp(scene.state.pixels,w->port.pixels,1024));
        CHECK(cs_drawing_app_scene(&mac,&result,&w->port,1234));
        CHECK(cs_boot_scene_draw_input(&scene,0,1234,&report)==CS_BOOT_OK && report.rows==480);
        for(unsigned y=0;y<480;++y){
            for(unsigned x=0;x<640;++x){
                const uint8_t *p=pc+(size_t)y*pitch*4u+x*4u;
                uint32_t expected=oracle_scene_inputs(640,480,0,1234,(long)x,(long)y);
                uint8_t white=expected==0xFFFFFFFFu?255u:0u;
                CHECK(p[0]==white && p[1]==white && p[2]==white && p[3]==0);
                if(x>=64 && x<576 && y>=69 && y<411){
                    unsigned xx=x-64,yy=y-69;
                    unsigned black=(mono[yy*70u+xx/8u]>>(7u-(xx&7u)))&1u;
                    CHECK(black==(white==0));
                }else CHECK(white==255);
            }
            for(size_t pad=640u*4u;pad<pitch*4u;++pad)CHECK(pc[(size_t)y*pitch*4u+pad]==0xA5);
        }
        for(size_t i=bytes;i<bytes+31u;++i)CHECK(pc[i]==0xA5);
        for(size_t y=0;y<342;++y)for(size_t i=64;i<70;++i)CHECK(mono[y*70u+i]==0xA5);
        for(size_t i=70u*342u;i<mono_size;++i)CHECK(mono[i]==0xA5);
        if(directory && mode==0){
            char path[1024];CHECK(snprintf(path,sizeof path,"%s/mac.pbm",directory)>0);
            FILE *f=fopen(path,"wb");CHECK(f);fprintf(f,"P4\n512 342\n");
            for(unsigned y=0;y<342;++y)CHECK(fwrite(mono+y*70u,1,64,f)==64);
            CHECK(!fclose(f));CHECK(snprintf(path,sizeof path,"%s/pc.ppm",directory)>0);
            f=fopen(path,"wb");CHECK(f);fprintf(f,"P6\n640 480\n255\n");
            for(unsigned y=0;y<480;++y)for(unsigned x=0;x<640;++x)
                CHECK(fwrite(pc+(size_t)y*pitch*4u+x*4u,1,3,f)==3);
            CHECK(!fclose(f));
            printf("PC arena=%zu, staging=%zu, workspace=%zu; 68k fixture steps=%u\n",
                arena.used,scene.staging_bytes,scene.workspace_bytes,result.steps);
        }
        free(pc);free(backing);
    }
    free(mono);free(w);
}
static void updates(void)
{
    const int widths[3]={512,640,641},heights[3]={342,480,483};
    for(unsigned mode=1;mode<=2;++mode)for(unsigned size=0;size<3;++size){
        int width=widths[size],height=heights[size];size_t row=mode==1?((size_t)width+7u)/8u:(size_t)width*4u;
        size_t stride=row+5u,n=stride*(size_t)height+23u;
        uint8_t *a=space(n),*b=space(n),*saved=space(n);cs_surface sa,sb;
        CHECK(cs_surface_init(&sa,a,n,width,height,stride,(cs_surface_format)mode)==CS_SURFACE_OK);
        CHECK(cs_surface_init(&sb,b,n,width,height,stride,(cs_surface_format)mode)==CS_SURFACE_OK);
        cs_product_state state={97,0,0,6,1,NULL};
        CHECK(cs_product_scene(&sa,width,height,0,&state));memcpy(saved,a,n);
        state.inputs=UINT32_MAX;CHECK(cs_product_scene(&sb,width,height,0,&state));
        CHECK(cs_product_scene_input(&sa,state.inputs));CHECK(!memcmp(a,b,n));
        int ox=(width-512)/2,oy=(height-342)/2;
        for(int y=0;y<height;++y)for(size_t col=0;col<stride;++col){
            size_t left=(size_t)(ox+352)*(mode==1?1u:4u),right=(size_t)(ox+432)*(mode==1?1u:4u);
            if(mode==1){left/=8u;right=(right+7u)/8u;}
            if(y<oy+286 || y>=oy+306 || col<left || col>=right)CHECK(a[(size_t)y*stride+col]==saved[(size_t)y*stride+col]);
        }
        free(a);free(b);free(saved);
    }
}
static void errors(void)
{
    uint8_t pixels[64*342];memset(pixels,0xA5,sizeof pixels);cs_surface s;
    CHECK(cs_surface_init(&s,pixels,sizeof pixels,512,342,64,CS_SURFACE_MONO1_MSB)==CS_SURFACE_OK);
    cs_product_state state={97,0,0,6,1,NULL};
    CHECK(!cs_product_scene(NULL,512,342,0,&state));CHECK(!cs_product_scene(&s,0,342,0,&state));
    state.phases=7;CHECK(!cs_product_scene(&s,512,342,0,&state));state.phases=6;
    state.success=2;CHECK(!cs_product_scene(&s,512,342,0,&state));state.success=1;
    CHECK(!cs_product_scene(&s,512,342,342,&state));s.storage_size=1;
    CHECK(!cs_product_scene(&s,512,342,0,&state));
    for(size_t i=0;i<sizeof pixels;++i)CHECK(pixels[i]==0xA5);
}
int main(int argc,char **argv)
{
    if(argc==3 && !strcmp(argv[1],"preview")){compare(argv[2]);return 0;}
    if(argc!=2)return 2;
    if(!strcmp(argv[1],"parity"))compare(NULL);
    else if(!strcmp(argv[1],"updates"))updates();else if(!strcmp(argv[1],"errors"))errors();else return 2;
    puts("PASS");return 0;
}
