/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original integration/conformance. */
#include "../apps/loaded-rom-app/probe.h"
#include "../apps/loaded-rom-app/scene.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define REQUIRE(x) do{if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);exit(1);}}while(0)
static cs_loaded_app_workspace *fresh(cs_trap_gateway *g)
{
    cs_loaded_app_workspace *w=calloc(1,sizeof *w);REQUIRE(w);
    cs_trap_config c={0x400000,0x10000,0x20000,0x400100,0x400120,0x400140};
    REQUIRE(cs_trap_init(g,&c)==CS_TRAP_OK);return w;
}
static void load_again(cs_loaded_app_workspace *w)
{REQUIRE(cs_loaded_app_prepare(w,w->run_request.gateway));}
static void encode32(uint8_t *p,uint32_t value)
{for(unsigned i=0;i<4;++i)p[i]=(uint8_t)(value>>((3u-i)*8u));}
static void execute_bad(const uint8_t *program,size_t length,cs_m68k_result expected)
{
    cs_trap_gateway g;cs_loaded_app_workspace *w=fresh(&g);
    REQUIRE(cs_loaded_app_prepare_program(w,&g,program,length));cs_trap_gateway before=g;
    REQUIRE(cs_image_run(&w->image,&w->run_request,&w->caller,&w->run_report)==CS_IMAGE_RUN_CPU);
    REQUIRE(w->run_report.steps==1 && w->run_report.cpu==expected && !memcmp(&g,&before,sizeof g));
    REQUIRE(cs_image_run_retire(&w->image,&w->run_request,&w->image_workspace,sizeof w->image_workspace,
        &w->description,&w->image_report)==CS_IMAGE_OK);free(w);
}
static void success(void)
{
    cs_trap_gateway g;cs_loaded_app_workspace *w=fresh(&g);cs_loaded_app_result r;
    REQUIRE(cs_loaded_app_probe(w,&g,&r));REQUIRE(r.success && r.phase==6 && r.value==97 &&
        r.pattern==0x5A5A5A5Au && r.steps==5 && r.retired==1 && r.generation==1);
    REQUIRE(w->image.state==CS_IMAGE_EMPTY && w->image_report.execution_scrubbed==16);
    REQUIRE(w->run_report.guest.d[0]==97 && w->run_report.guest.a[7]==w->run_report.guest.a[5]-16u);
    for(unsigned i=0;i<sizeof w->caller;++i)REQUIRE(((const uint8_t *)&w->caller)[i]==0);
    cs_guest_view v;REQUIRE(cs_guest_window(&w->map,0x7FC8,56,CS_GUEST_READ,&v)==CS_GUEST_OK);
    for(unsigned i=0;i<56;++i)REQUIRE(v.bytes[i]==0);
    REQUIRE(w->regions[1].flags==3 && cs_image_external_admit(&w->image)==CS_IMAGE_OK);
    REQUIRE(!cs_loaded_app_probe(NULL,&g,&r));free(w);
}
static void failures(void)
{
    cs_trap_gateway g;cs_loaded_app_workspace *w=fresh(&g);cs_loaded_app_result r;
    REQUIRE(cs_loaded_app_probe(w,&g,&r));load_again(w);
    cs_image_run_request *q=&w->run_request;cs_image_run_report report;
    w->caller.d[0]=0xDEADBEEF;w->caller.a[5]=0x12345678;w->caller.sr=0xFFFF;
    cs_trap_frame original=w->caller;
    uint8_t *saved=malloc(sizeof w->ram);REQUIRE(saved);memcpy(saved,w->ram,sizeof w->ram);
    q->budget=0;REQUIRE(cs_image_run(&w->image,q,&w->caller,&report)==CS_IMAGE_RUN_REFUSED);
    REQUIRE(!memcmp(saved,w->ram,sizeof w->ram) && !w->image.execution_state);
    q->budget=3;REQUIRE(cs_image_run(&w->image,q,&w->caller,&report)==CS_IMAGE_RUN_BUDGET);
    REQUIRE(report.entered && report.steps==3 && w->image.execution_state==4);
    REQUIRE(!memcmp(&original,&w->caller,sizeof original) && !report.guest.d[0]);
    uint32_t value;REQUIRE(cs_guest_read(&w->map,w->image.layout.a5-4u,4,CS_GUEST_READ,&value)==CS_GUEST_OK && value==0x5A5A5A5Au);
    REQUIRE(cs_image_run(&w->image,q,&w->caller,&w->run_report)==CS_IMAGE_RUN_REFUSED);
    REQUIRE(cs_image_execute(&w->image,&w->image_request,&w->image_workspace,sizeof w->image_workspace,
        &w->description,&w->image_report)==CS_IMAGE_IN_USE);
    REQUIRE(cs_image_run_retire(&w->image,q,&w->image_workspace,sizeof w->image_workspace,
        &w->description,&w->image_report)==CS_IMAGE_OK);
    free(saved);free(w);
}
static void access(void)
{
    const uint16_t bad[]={0xA046,0xA047,0xA9F4,0xF000,0x4E40};
    for(unsigned i=0;i<sizeof bad/sizeof bad[0];++i){
        uint8_t program[2]={(uint8_t)(bad[i]>>8),(uint8_t)bad[i]};
        execute_bad(program,sizeof program,i<3?CS_M68K_POLICY:CS_M68K_OPCODE);
    }
    /* All malicious instruction bodies are original forks actually PREPAREd
     * and LOADed. No native post-LOAD mutation supplies the test program. */
    uint8_t write_above[]={0x20,0x4D,0x20,0xBC,0x12,0x34,0x56,0x78,0x70,0x61,0x4E,0x75};
    cs_trap_gateway g;cs_loaded_app_workspace *w=fresh(&g);
    REQUIRE(cs_loaded_app_prepare_program(w,&g,write_above,sizeof write_above));
    REQUIRE(cs_image_run(&w->image,&w->run_request,&w->caller,&w->run_report)==CS_IMAGE_RUN_CPU);
    REQUIRE(w->run_report.steps==2 && w->run_report.cpu==CS_M68K_POLICY);
    uint32_t value;REQUIRE(cs_guest_read(&w->map,w->image.layout.a5,4,CS_GUEST_READ,&value)==CS_GUEST_OK && !value);
    free(w);
    uint8_t jump[]={0x4E,0xF9,0,0,0,0};encode32(jump+2,0x500000);execute_bad(jump,sizeof jump,CS_M68K_POLICY);
    uint8_t read[]={0x20,0x39,0,0,0,0};encode32(read+2,0x500000);execute_bad(read,sizeof read,CS_M68K_POLICY);
    encode32(read+2,0x114);execute_bad(read,sizeof read,CS_M68K_POLICY);
    uint8_t write[]={0x23,0xFC,1,2,3,4,0,0,0,0};encode32(write+6,0x118);execute_bad(write,sizeof write,CS_M68K_POLICY);
    uint8_t stack[]={0x2E,0x7C,0,0,0,0};encode32(stack+2,0x7FC9);execute_bad(stack,sizeof stack,CS_M68K_ADDRESS);
    encode32(stack+2,0x9000);execute_bad(stack,sizeof stack,CS_M68K_ADDRESS);
    for(unsigned offset=0;offset<8;offset+=2){
        if(offset==2)continue;
        encode32(jump+2,0x7FF8u+offset);execute_bad(jump,sizeof jump,CS_M68K_POLICY);
    }
    encode32(jump+2,0x8000);execute_bad(jump,sizeof jump,CS_M68K_POLICY);
    /* Dirty-world and forged return paths use valid source input too. */
    uint8_t forged[]={0x2E,0x7C,0,0,0,0,0x4E,0x75};encode32(forged+2,0x7DC8);
    cs_loaded_app_workspace *f=fresh(&g);REQUIRE(cs_loaded_app_prepare_program(f,&g,forged,sizeof forged));
    REQUIRE(cs_guest_write(&f->map,0x7DC8,4,0x500000)==CS_GUEST_OK);
    REQUIRE(cs_image_run(&f->image,&f->run_request,&f->caller,&f->run_report)==CS_IMAGE_RUN_CPU &&
        f->run_report.cpu==CS_M68K_POLICY && f->run_report.steps==2);free(f);
}
static void retire(void)
{
    cs_trap_gateway g;cs_loaded_app_workspace *w=fresh(&g);cs_loaded_app_result r;
    REQUIRE(cs_loaded_app_probe(w,&g,&r));load_again(w);
    REQUIRE(cs_image_run(&w->image,&w->run_request,&w->caller,&w->run_report)==CS_IMAGE_RUN_RETURNED);
    cs_image_run_report saved=w->run_report;uint8_t stack[512];
    memcpy(stack,w->ram+w->image.layout.stack_base-0x1000u,sizeof stack);
    REQUIRE(cs_image_run_retire(&w->image,&w->run_request,&w->image_workspace,sizeof w->image_workspace,
        &w->description,&w->image_report)==CS_IMAGE_OK);
    REQUIRE(!memcmp(&saved,&w->run_report,sizeof saved) && !memcmp(stack,w->ram+0x7DC8u-0x1000u,sizeof stack));
    REQUIRE(w->image_report.execution_scrubbed==16 && cs_image_external_admit(&w->image)==CS_IMAGE_OK);
    free(w);
    w=fresh(&g);REQUIRE(cs_loaded_app_prepare(w,&g));
    REQUIRE(cs_image_run(&w->image,&w->run_request,&w->caller,&w->run_report)==CS_IMAGE_RUN_RETURNED);
    REQUIRE(cs_guest_write(&w->map,w->image.layout.a5,4,0xABCD)==CS_GUEST_OK);
    uint8_t *ram=malloc(sizeof w->ram);REQUIRE(ram);memcpy(ram,w->ram,sizeof w->ram);
    REQUIRE(cs_image_run_retire(&w->image,&w->run_request,&w->image_workspace,sizeof w->image_workspace,
        &w->description,&w->image_report)==CS_IMAGE_FAULT);
    REQUIRE(w->image.execution_state==5 && cs_image_external_admit(&w->image)==CS_IMAGE_FAULT &&
        !memcmp(ram,w->ram,sizeof w->ram));free(ram);free(w);
}
static void claims(void)
{
    cs_trap_gateway g;cs_loaded_app_workspace *w=fresh(&g);REQUIRE(cs_loaded_app_prepare(w,&g));
    cs_image_run_report report;uint8_t *saved=malloc(sizeof w->ram);REQUIRE(saved);memcpy(saved,w->ram,sizeof w->ram);
    w->run_request.token+=2;
    REQUIRE(cs_image_run(&w->image,&w->run_request,&w->caller,&report)==CS_IMAGE_RUN_REFUSED);
    w->run_request.token-=2;w->run_request.context.active_root=1;
    REQUIRE(cs_image_run(&w->image,&w->run_request,&w->caller,&report)==CS_IMAGE_RUN_REFUSED);
    w->run_request.context.active_root=0;w->run_request.context.context=CS_LIFE_INTERRUPT;
    REQUIRE(cs_image_run(&w->image,&w->run_request,&w->caller,&report)==CS_IMAGE_RUN_REFUSED);
    w->run_request.context.context=CS_LIFE_ORDINARY;++w->run_request.generation;
    REQUIRE(cs_image_run(&w->image,&w->run_request,&w->caller,&report)==CS_IMAGE_RUN_REFUSED);
    --w->run_request.generation;w->regions[2].flags=CS_GUEST_READ;
    REQUIRE(cs_image_run(&w->image,&w->run_request,&w->caller,&report)==CS_IMAGE_RUN_REFUSED);
    w->regions[2].flags=CS_GUEST_READ|CS_GUEST_EXECUTE;
    REQUIRE(!w->image.execution_state && !w->image.execution_generation && !memcmp(saved,w->ram,sizeof w->ram));
    union {cs_trap_frame caller;cs_image_run_report report;} overlap;
    memset(&overlap,0x5A,sizeof overlap);uint8_t before[sizeof overlap];memcpy(before,&overlap,sizeof overlap);
    REQUIRE(cs_image_run(&w->image,&w->run_request,&overlap.caller,&overlap.report)==CS_IMAGE_RUN_ARGUMENT &&
        !memcmp(before,&overlap,sizeof overlap));
    REQUIRE(cs_image_run(&w->image,&w->run_request,&w->caller,&report)==CS_IMAGE_RUN_RETURNED);
    free(saved);free(w);
}
static void render(const char *path)
{
    cs_trap_gateway g;cs_loaded_app_workspace *w=fresh(&g);cs_loaded_app_result r;
    REQUIRE(cs_loaded_app_probe(w,&g,&r));uint8_t pixels[21888+8];memset(pixels,0xCC,sizeof pixels);
    cs_surface screen;REQUIRE(cs_surface_init(&screen,pixels,21888,512,342,64,CS_SURFACE_MONO1_MSB)==CS_SURFACE_OK);
    REQUIRE(cs_loaded_app_scene(&screen,&r,0));
    for(unsigned bit=0;bit<32;++bit){unsigned x=64u+bit*12u;
        REQUIRE(((pixels[160u*64u+x/8u]>>(7u-x%8u))&1u)==((0x5A5A5A5Au>>(31u-bit))&1u));}
    for(unsigned phase=1;phase<=6;++phase){unsigned x=64u+(phase-1u)*64u;REQUIRE(pixels[217u*64u+x/8u]&0x80u);}
    for(unsigned i=21888;i<sizeof pixels;++i)REQUIRE(pixels[i]==0xCC);
    if(path){FILE *file=fopen(path,"wb");REQUIRE(file);REQUIRE(fprintf(file,"P4\n512 342\n")>0);
        REQUIRE(fwrite(pixels,1,21888,file)==21888);REQUIRE(!fclose(file));}
    r.success=0;r.phase=3;REQUIRE(cs_loaded_app_scene(&screen,&r,0));REQUIRE(pixels[150u*64u+20u/8u]&0x08u);
    free(w);
}
int main(int argc,char **argv)
{
    if(argc==3 && !strcmp(argv[1],"preview")){render(argv[2]);return 0;}
    if(argc!=2)return 2;
    if(!strcmp(argv[1],"success"))success();else if(!strcmp(argv[1],"failures"))failures();
    else if(!strcmp(argv[1],"access"))access();else if(!strcmp(argv[1],"retire"))retire();else if(!strcmp(argv[1],"scene"))render(NULL);
    else if(!strcmp(argv[1],"claims"))claims();else return 2;
    puts("PASS");return 0;
}
