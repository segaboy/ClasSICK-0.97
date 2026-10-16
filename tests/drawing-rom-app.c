/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original pixel oracle and loaded calls. */
#include "../apps/drawing-rom-app/probe.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define REQUIRE(x) do{if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);exit(1);}}while(0)
static void word(uint8_t *p,int32_t n)
{uint32_t v=(uint32_t)n;p[0]=(uint8_t)(v>>8);p[1]=(uint8_t)v;}
static void rectangle_bytes(uint8_t *p,int32_t t,int32_t l,int32_t b,int32_t r)
{word(p,t);word(p+2,l);word(p+4,b);word(p+6,r);}
/* Independent set-of-pixels oracle; no production clipping/masks reused. */
static void oracle(uint8_t *p,int32_t t,int32_t l,int32_t b,int32_t r,unsigned paint)
{
    for(int32_t y=0;y<64;++y)for(int32_t x=0;x<128;++x)if(y>=t && y<b && x>=l && x<r){
        size_t at=(size_t)y*16u+(size_t)x/8u;uint8_t mask=(uint8_t)(128u>>((uint32_t)x%8u));
        p[at]=paint?(uint8_t)(p[at]|mask):(uint8_t)(p[at]&(uint8_t)~mask);
    }
}
static void expected(uint8_t *p)
{memset(p,0,1024);oracle(p,8,8,56,120,1);oracle(p,16,16,48,112,0);}
static uint32_t random_value(uint32_t *n)
{*n=*n*UINT32_C(1664525)+UINT32_C(1013904223);return *n;}
static void pixels(void)
{
    struct {uint8_t before[16];cs_qd_rect_port port;uint8_t after[16];} box;
    memset(&box,0xA5,sizeof box);box.port.paints=0;box.port.erases=0;
    uint8_t reference[1024],r[8];memcpy(reference,box.port.pixels,1024);
    const int32_t edges[]={-32768,-1,0,1,7,8,9,31,63,64,65,127,128,129,32767};
    uint32_t seed=13;unsigned paints=0,erases=0;
    for(unsigned i=0;i<1500;++i){int32_t t,l,b,right;
        if(i<500){t=edges[random_value(&seed)%15u];l=edges[random_value(&seed)%15u];
            b=edges[random_value(&seed)%15u];right=edges[random_value(&seed)%15u];}
        else {t=(int32_t)(random_value(&seed)%100u)-20;l=(int32_t)(random_value(&seed)%170u)-20;
            b=t+(int32_t)(random_value(&seed)%90u)-10;right=l+(int32_t)(random_value(&seed)%140u)-10;}
        rectangle_bytes(r,t,l,b,right);uint8_t original[8];memcpy(original,r,8);
        unsigned paint=i&1u;REQUIRE(cs_qd_rect(&box.port,r,paint?CS_QD_PAINT_RECT:CS_QD_ERASE_RECT));
        oracle(reference,t,l,b,right,paint);REQUIRE(!memcmp(reference,box.port.pixels,1024));
        REQUIRE(!memcmp(r,original,8));if(paint)++paints;else ++erases;
        REQUIRE(box.port.paints==paints && box.port.erases==erases);
        for(unsigned j=0;j<16;++j)REQUIRE(box.before[j]==0xA5 && box.after[j]==0xA5);
    }
    cs_qd_rect_port saved=box.port;
    REQUIRE(!cs_qd_rect(NULL,r,CS_QD_PAINT_RECT));REQUIRE(!cs_qd_rect(&box.port,NULL,CS_QD_PAINT_RECT));
    REQUIRE(!cs_qd_rect(&box.port,r,0xACA2));REQUIRE(!memcmp(&saved,&box.port,sizeof saved));
    box.port.paints=UINT32_MAX;box.port.erases=UINT32_MAX;rectangle_bytes(r,0,0,0,0);
    REQUIRE(cs_qd_rect(&box.port,r,CS_QD_PAINT_RECT) && box.port.paints==UINT32_MAX);
    REQUIRE(cs_qd_rect(&box.port,r,CS_QD_ERASE_RECT) && box.port.erases==UINT32_MAX);
}
static cs_drawing_app_workspace *fresh(cs_trap_gateway *g)
{
    cs_drawing_app_workspace *w=calloc(1,sizeof *w);REQUIRE(w);
    cs_trap_config c={0x400000,0x10000,0x20000,0x400100,0x400120,0x400140};
    REQUIRE(cs_trap_init(g,&c)==CS_TRAP_OK);return w;
}
static void prepare(cs_drawing_app_workspace *w,cs_trap_gateway *g,const uint8_t *p,size_t n)
{REQUIRE(cs_drawing_app_prepare(w,g,p,n));}
static cs_image_run_result run(cs_drawing_app_workspace *w)
{return cs_image_run_rectangles(&w->app.image,&w->app.run_request,&w->app.caller,&w->app.run_report,&w->port);}
static void retire(cs_drawing_app_workspace *w)
{
    cs_loaded_app_workspace *a=&w->app;
    REQUIRE(cs_image_run_retire(&a->image,&a->run_request,&a->image_workspace,sizeof a->image_workspace,
        &a->description,&a->image_report)==CS_IMAGE_OK);
    REQUIRE(a->image.state==CS_IMAGE_EMPTY && a->image_report.execution_scrubbed==16);
}
static void success(void)
{
    cs_trap_gateway g;cs_drawing_app_workspace *w=fresh(&g);cs_loaded_app_result r;uint8_t want[1024];
    cs_trap_gateway saved=g;int ok=cs_drawing_app_probe(w,&g,&r);
    if(!ok)fprintf(stderr,"drawing phase%u run%u cpu%u steps%u pc%08x paint%u erase%u\n",
        (unsigned)r.phase,(unsigned)w->app.run_report.status,(unsigned)w->app.run_report.cpu,
        (unsigned)r.steps,(unsigned)w->app.run_report.guest.pc,(unsigned)w->port.paints,(unsigned)w->port.erases);
    REQUIRE(ok);expected(want);
    REQUIRE(r.success && r.phase==6 && r.value==97 && r.steps==13 && r.retired==1);
    REQUIRE(w->port.paints==1 && w->port.erases==1 && !memcmp(want,w->port.pixels,1024));
    REQUIRE(!memcmp(&saved,&g,sizeof g) && cs_image_external_admit(&w->app.image)==CS_IMAGE_OK);
    REQUIRE(w->app.run_report.guest.a[7]==w->app.run_report.guest.a[5]-16u);
    for(size_t i=0;i<sizeof w->app.caller;++i)REQUIRE(!((uint8_t *)&w->app.caller)[i]);
    uint32_t value;REQUIRE(cs_guest_read(&w->app.map,w->app.run_report.guest.a[5]-8u,4,1,&value)==CS_GUEST_OK && !value);
    free(w);
    w=fresh(&g);prepare(w,&g,cs_drawing_app_program,40);
    REQUIRE(cs_image_run(&w->app.image,&w->app.run_request,&w->app.caller,&w->app.run_report)==CS_IMAGE_RUN_CPU);
    REQUIRE(w->app.run_report.cpu==CS_M68K_POLICY && w->app.run_report.steps==5 && !w->port.paints);
    retire(w);free(w);
}
/* Compare a failed trap with an independently loaded identical program stopped
 * by budget just before that trap: exact earlier RAM effects must be retained. */
static void atomic_failure(const uint8_t *program,size_t length,uint32_t before)
{
    cs_trap_gateway g,h;cs_drawing_app_workspace *a=fresh(&g),*b=fresh(&h);
    prepare(a,&g,program,length);prepare(b,&h,program,length);
    a->app.run_request.budget=before;b->app.run_request.budget=100;
    REQUIRE(run(a)==CS_IMAGE_RUN_BUDGET && a->app.run_report.steps==before);
    cs_trap_gateway saved=h;REQUIRE(run(b)==CS_IMAGE_RUN_CPU && b->app.run_report.steps==before);
    REQUIRE(b->app.run_report.cpu==CS_M68K_POLICY || b->app.run_report.cpu==CS_M68K_ADDRESS);
    REQUIRE(!memcmp(a->app.ram,b->app.ram,sizeof a->app.ram) && !memcmp(a->app.low,b->app.low,sizeof a->app.low));
    REQUIRE(!memcmp(&a->port,&b->port,sizeof a->port) && !memcmp(&h,&saved,sizeof h));
    REQUIRE(!memcmp(&a->app.run_report.guest,&b->app.run_report.guest,sizeof(cs_trap_frame)));
    retire(a);retire(b);free(a);free(b);
}
static void encode32(uint8_t *p,uint32_t value)
{for(unsigned i=0;i<4;++i)p[i]=(uint8_t)(value>>((3u-i)*8u));}
static void faults(void)
{
    uint8_t p[40];const uint16_t words[]={0xA8A5,0xACA2,0xAAA2,0xA9A2,0xA9F4,0xA046};
    for(unsigned i=0;i<6;++i){memcpy(p,cs_drawing_app_program,40);p[16]=(uint8_t)(words[i]>>8);p[17]=(uint8_t)words[i];atomic_failure(p,40,5);}
    const uint32_t pointers[]={0x7FD1,0x114,0x500000,0xAB007FD0,0x7FD4,0x2000};
    for(unsigned i=0;i<6;++i){uint8_t bad[]={0x2F,0x3C,0,0,0,0,0xA8,0xA2,0x4E,0x75};
        encode32(bad+2,pointers[i]);atomic_failure(bad,sizeof bad,2);}
    uint8_t no_argument[]={0xA8,0xA2,0x4E,0x75};atomic_failure(no_argument,sizeof no_argument,1);
    uint8_t bottom_stack[]={0x2E,0x7C,0,0,0,0,0xA8,0xA2,0x4E,0x75};
    /* layout B=8000,above40,below16,stack512 => stack base7DC8. */
    encode32(bottom_stack+2,0x7DCC);atomic_failure(bottom_stack,sizeof bottom_stack,2);
    /* Last instruction has no owned continuation; empty geometry cannot mask it. */
    for(unsigned i=0;i<40;++i)p[i]=(uint8_t)(i&1u?0x71:0x4E);
    p[0]=0x20;p[1]=0x4D;p[2]=0x2F;p[3]=8;p[38]=0xA8;p[39]=0xA2;
    atomic_failure(p,sizeof p,20);
}
static void effects(void)
{
    cs_trap_gateway g;cs_drawing_app_workspace *w=fresh(&g);prepare(w,&g,cs_drawing_app_program,40);
    w->app.caller.d[7]=0xDEADBEEF;w->app.caller.a[5]=0x12345678;w->app.caller.sr=0xFFFF;
    cs_trap_frame caller=w->app.caller;w->app.run_request.budget=6;
    REQUIRE(run(w)==CS_IMAGE_RUN_BUDGET && w->app.run_report.steps==6);
    uint8_t want[1024];memset(want,0,1024);oracle(want,8,8,56,120,1);
    REQUIRE(!memcmp(want,w->port.pixels,1024) && w->port.paints==1 && !w->port.erases);
    REQUIRE(!memcmp(&caller,&w->app.caller,sizeof caller));
    cs_qd_rect_port saved=w->port;REQUIRE(run(w)==CS_IMAGE_RUN_REFUSED && !memcmp(&saved,&w->port,sizeof saved));
    retire(w);REQUIRE(!memcmp(&saved,&w->port,sizeof saved));free(w);
    w=fresh(&g);uint8_t bad[40];memcpy(bad,cs_drawing_app_program,40);bad[35]=0xA5;
    prepare(w,&g,bad,40);REQUIRE(run(w)==CS_IMAGE_RUN_CPU && w->app.run_report.steps==10);
    REQUIRE(!memcmp(want,w->port.pixels,1024) && w->port.paints==1 && !w->port.erases);retire(w);free(w);
    w=fresh(&g);memcpy(bad,cs_drawing_app_program,40);bad[18]=0x4E;bad[19]=0xF9;encode32(bad+20,0x500000);
    prepare(w,&g,bad,40);REQUIRE(run(w)==CS_IMAGE_RUN_CPU && w->app.run_report.steps==6);
    REQUIRE(!memcmp(want,w->port.pixels,1024));retire(w);free(w);
    /* A real guest Rect overlaps the trap's scratch: painting must use the
     * pre-commit snapshot, not the later saved SR/PC bytes. */
    const uint8_t overlapping[]={0x20,0x4F,0x21,0x3C,0,0,0,0,
        0x21,0x3C,0,1,0,1,0x21,0x3C,0,0,0,0,0x2F,8,0xA8,0xA2,0x70,0x61,0x4E,0x75};
    w=fresh(&g);prepare(w,&g,overlapping,sizeof overlapping);REQUIRE(run(w)==CS_IMAGE_RUN_RETURNED);
    memset(want,0,1024);want[0]=0x80;
    REQUIRE(w->port.paints==1 && !w->port.erases && !memcmp(want,w->port.pixels,1024));retire(w);free(w);
    const uint8_t empty[]={0x2F,0x0D,0xA8,0xA3,0x70,0x61,0x4E,0x75};
    w=fresh(&g);prepare(w,&g,empty,sizeof empty);REQUIRE(run(w)==CS_IMAGE_RUN_RETURNED);
    memset(want,0,1024);REQUIRE(!w->port.paints && w->port.erases==1 && !memcmp(want,w->port.pixels,1024));retire(w);free(w);
}
static void aliases(void)
{
    cs_trap_gateway g;cs_drawing_app_workspace *w=fresh(&g);prepare(w,&g,cs_drawing_app_program,40);
    cs_loaded_app_workspace *a=&w->app;memset(&a->run_report,0xA5,sizeof a->run_report);
    cs_image_run_report before=a->run_report;
    REQUIRE(cs_image_run_rectangles(&a->image,&a->run_request,&a->caller,&a->run_report,NULL)==CS_IMAGE_RUN_ARGUMENT);
    REQUIRE(!memcmp(&before,&a->run_report,sizeof before));
    REQUIRE(cs_image_run_rectangles(&a->image,&a->run_request,&a->caller,&a->run_report,
        (cs_qd_rect_port *)(void *)a->ram)==CS_IMAGE_RUN_ARGUMENT);
    REQUIRE(!memcmp(&before,&a->run_report,sizeof before) && !a->image.execution_state);
    REQUIRE(cs_image_run_rectangles(&a->image,&a->run_request,(cs_trap_frame *)(void *)&w->port,
        &a->run_report,&w->port)==CS_IMAGE_RUN_ARGUMENT);
    REQUIRE(!memcmp(&before,&a->run_report,sizeof before));
    REQUIRE(cs_image_run_rectangles(&a->image,&a->run_request,&a->caller,&a->run_report,
        (cs_qd_rect_port *)(void *)(a->ram+1))==CS_IMAGE_RUN_ARGUMENT);
    REQUIRE(!memcmp(&before,&a->run_report,sizeof before) && !a->image.execution_state);
    REQUIRE(cs_trap_set(&g,0xA2,0x18000)==CS_TRAP_OK);
    REQUIRE(run(w)==CS_IMAGE_RUN_REFUSED && !a->image.execution_state && !w->port.paints);
    REQUIRE(cs_trap_set(&g,0xA2,g.config.unsupported_token)==CS_TRAP_OK);
    REQUIRE(cs_trap_set(&g,0xA3,g.config.get_token)==CS_TRAP_OK);
    REQUIRE(run(w)==CS_IMAGE_RUN_REFUSED && !a->image.execution_state && !w->port.erases);
    REQUIRE(cs_trap_set(&g,0xA3,g.config.unsupported_token)==CS_TRAP_OK);
    REQUIRE(run(w)==CS_IMAGE_RUN_RETURNED);retire(w);free(w);
}
static void scene(const char *path)
{
    cs_trap_gateway g;cs_drawing_app_workspace *w=fresh(&g);cs_loaded_app_result result;
    REQUIRE(cs_drawing_app_probe(w,&g,&result));size_t size=72u*342u+32u;
    uint8_t *pixels=malloc(size),*want=malloc(size);REQUIRE(pixels && want);
    memset(pixels,0xA5,size);memset(want,0xA5,size);cs_surface a,b;
    REQUIRE(cs_surface_init(&a,pixels,size,512,342,72,CS_SURFACE_MONO1_MSB)==CS_SURFACE_OK);
    REQUIRE(cs_surface_init(&b,want,size,512,342,72,CS_SURFACE_MONO1_MSB)==CS_SURFACE_OK);
    REQUIRE(cs_loaded_app_scene(&b,&result,0));REQUIRE(cs_drawing_app_scene(&a,&result,&w->port,0));
    for(unsigned y=0;y<342;++y)for(unsigned x=0;x<512;++x){size_t at=(size_t)y*72u+x/8u;uint8_t mask=(uint8_t)(128u>>(x%8u));
        if(y>=132 && y<204 && x>=52 && x<460)want[at]=(uint8_t)(want[at]&(uint8_t)~mask);
        if(y>=136 && y<200 && x>=192 && x<320){unsigned yy=y-136u,xx=x-192u;
            unsigned bit=(w->port.pixels[yy*16u+xx/8u]>>(7u-(xx%8u)))&1u;
            want[at]=bit?(uint8_t)(want[at]|mask):(uint8_t)(want[at]&(uint8_t)~mask);}}
    REQUIRE(!memcmp(want,pixels,size));REQUIRE(cs_loaded_app_scene_input(&a,1));
    for(unsigned y=0;y<64;++y)REQUIRE(!memcmp(pixels+(136u+y)*72u+24u,w->port.pixels+y*16u,16));
    if(path){FILE *f=fopen(path,"wb");REQUIRE(f);fprintf(f,"P4\n512 342\n");
        for(unsigned y=0;y<342;++y){REQUIRE(fwrite(pixels+y*72u,1,64,f)==64);}
        REQUIRE(!fclose(f));}
    free(pixels);free(want);free(w);
}
int main(int argc,char **argv)
{
    if(argc==3 && !strcmp(argv[1],"preview")){scene(argv[2]);return 0;}
    if(argc!=2)return 2;
    if(!strcmp(argv[1],"pixels"))pixels();else if(!strcmp(argv[1],"success"))success();
    else if(!strcmp(argv[1],"faults"))faults();else if(!strcmp(argv[1],"effects"))effects();
    else if(!strcmp(argv[1],"aliases"))aliases();else if(!strcmp(argv[1],"scene"))scene(NULL);else return 2;
    puts("PASS");return 0;
}
