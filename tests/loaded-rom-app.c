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
static uint32_t fnv(const uint8_t *p,size_t n)
{
    uint32_t h=2166136261u;for(size_t i=0;i<n;++i)h=(h^p[i])*16777619u;return h;
}
static int bit_at(const uint8_t *p,size_t stride,unsigned x,unsigned y)
{
    return (p[y*stride+x/8u]>>(7u-x%8u))&1u;
}
static int in_counter(unsigned x,unsigned y){return x>=352u && x<432u && y>=286u && y<306u;}
static uint32_t mix(uint32_t *s){*s^=*s<<13;*s^=*s>>17;*s^=*s<<5;return *s;}
/* Full frame from the existing renderer into a fresh 0xCC tight buffer. */
static void full_frame(uint8_t *out,const cs_loaded_app_result *r,uint32_t presses)
{
    cs_surface s;memset(out,0xCC,21888);
    REQUIRE(cs_surface_init(&s,out,21888,512,342,64,CS_SURFACE_MONO1_MSB)==CS_SURFACE_OK);
    REQUIRE(cs_loaded_app_scene(&s,r,presses));
}
static void redraw(void)
{
    static const struct {uint32_t value,pattern,phase,success,presses,hash;} frozen[]={
        {97,0x5A5A5A5A,6,1,0,3049100689u},{97,0x5A5A5A5A,6,1,1,144985001u},
        {97,0x5A5A5A5A,6,1,2,2718142593u},{97,0x5A5A5A5A,6,1,9999,1255755017u},
        {97,0x5A5A5A5A,6,1,10000,3049100689u},{97,0x5A5A5A5A,6,1,UINT32_MAX,3866680433u},
        {0,0,0,0,0,705968457u},{97,0x5A5A5A5A,4,0,0,182201649u},
        {97,UINT32_MAX,5,0,7,3247734721u},{3610,0x80000001,6,1,17,666794281u},
        {UINT32_MAX,0,6,1,10001,3122324361u},{97,0x01010101,2,0,8,2519077233u}};
    static uint8_t a[21888+8],b[21888+8];
    cs_loaded_app_result r;cs_surface s;
    /* 1. Frozen full-frame fingerprints, tail untouched. */
    for(size_t i=0;i<sizeof frozen/sizeof frozen[0];++i){
        memset(&r,0,sizeof r);r.value=frozen[i].value;r.pattern=frozen[i].pattern;
        r.phase=frozen[i].phase;r.success=frozen[i].success;
        memset(a,0xCC,sizeof a);
        REQUIRE(cs_surface_init(&s,a,21888,512,342,64,CS_SURFACE_MONO1_MSB)==CS_SURFACE_OK);
        REQUIRE(cs_loaded_app_scene(&s,&r,frozen[i].presses));
        REQUIRE(fnv(a,21888)==frozen[i].hash);
        for(size_t k=21888;k<sizeof a;++k)REQUIRE(a[k]==0xCC);
    }
    /* 2. Incremental sequence matches full frames byte-for-byte, incl. wrap,
     * UINT32_MAX, repeats and decreasing values; tail canary preserved. */
    static const uint32_t seq[]={0,1,1,2,9999,10000,10000,10001,UINT32_MAX,UINT32_MAX,17,0};
    const cs_loaded_app_result *bases[3];cs_loaded_app_result rb[3];
    memset(rb,0,sizeof rb);
    rb[0].value=97;rb[0].pattern=0x5A5A5A5A;rb[0].phase=6;rb[0].success=1;
    rb[1].value=97;rb[1].pattern=UINT32_MAX;rb[1].phase=5;
    rb[2].value=UINT32_MAX;rb[2].phase=6;rb[2].success=1;
    for(int j=0;j<3;++j)bases[j]=&rb[j];
    for(int j=0;j<3;++j){
        memset(a,0xCC,sizeof a);
        REQUIRE(cs_surface_init(&s,a,21888,512,342,64,CS_SURFACE_MONO1_MSB)==CS_SURFACE_OK);
        REQUIRE(cs_loaded_app_scene(&s,bases[j],seq[0]));
        for(size_t i=1;i<sizeof seq/sizeof seq[0];++i){
            REQUIRE(cs_loaded_app_scene_input(&s,seq[i]));
            full_frame(b,bases[j],seq[i]);
            REQUIRE(!memcmp(a,b,21888));
            for(size_t k=21888;k<sizeof a;++k)REQUIRE(a[k]==0xCC);
        }
    }
    /* 3. Arbitrary non-counter pixels preserved; counter rect equals full frame
     * rect, including cleared zero pixels (start from all-ones noise too). */
    for(uint32_t seed=1;seed<=3;++seed){
        uint32_t st=0x9E3779B9u*seed;
        for(size_t k=0;k<sizeof a;++k)a[k]=seed==3?0xFF:(uint8_t)mix(&st);
        uint8_t before[21888+8];memcpy(before,a,sizeof a);
        REQUIRE(cs_surface_init(&s,a,21888,512,342,64,CS_SURFACE_MONO1_MSB)==CS_SURFACE_OK);
        uint32_t presses=seed==2?9999u:1234u*seed;
        REQUIRE(cs_loaded_app_scene_input(&s,presses));
        full_frame(b,&rb[0],presses);
        for(unsigned y=0;y<342u;++y)for(unsigned x=0;x<512u;++x){
            if(in_counter(x,y))REQUIRE(bit_at(a,64,x,y)==bit_at(b,64,x,y));
            else REQUIRE(bit_at(a,64,x,y)==bit_at(before,64,x,y));
        }
        for(size_t k=21888;k<sizeof a;++k)REQUIRE(a[k]==before[k]);
    }
    /* 4. Refusal without any write: null, zeroed descriptor, wrong geometry
     * (valid surfaces of other sizes), then valid padded updates. */
    {
        static uint8_t big[342*72+16];uint8_t snap[sizeof big];
        uint32_t st=0xC0FFEEu;for(size_t k=0;k<sizeof big;++k)big[k]=(uint8_t)mix(&st);
        memcpy(snap,big,sizeof big);
        REQUIRE(!cs_loaded_app_scene_input(NULL,5));
        cs_surface z;memset(&z,0,sizeof z);
        REQUIRE(!cs_loaded_app_scene_input(&z,5));
        REQUIRE(cs_surface_init(&s,big,21888,511,342,64,CS_SURFACE_MONO1_MSB)==CS_SURFACE_OK);
        REQUIRE(!cs_loaded_app_scene_input(&s,5));
        REQUIRE(cs_surface_init(&s,big,342*64-64,512,341,64,CS_SURFACE_MONO1_MSB)==CS_SURFACE_OK);
        REQUIRE(!cs_loaded_app_scene_input(&s,5));
        REQUIRE(!memcmp(big,snap,sizeof big));
        /* Padded rows are valid. Check their logical output and every padding byte. */
        REQUIRE(cs_surface_init(&s,big,342*72,512,342,72,CS_SURFACE_MONO1_MSB)==CS_SURFACE_OK);
        REQUIRE(cs_loaded_app_scene_input(&s,5));
        full_frame(b,&rb[0],5);
        for(unsigned y=0;y<342u;++y)for(unsigned x=0;x<512u;++x){
            if(in_counter(x,y))REQUIRE(bit_at(big,72,x,y)==bit_at(b,64,x,y));
            else REQUIRE(bit_at(big,72,x,y)==bit_at(snap,72,x,y));
        }
        for(size_t k=0;k<sizeof big;++k)
            if(k>=342u*72u || k%72u>=64u)REQUIRE(big[k]==snap[k]);
        /* Corrupt only the descriptor; rejected updates cannot change backing. */
        memcpy(snap,big,sizeof big);s.stride=63;
        REQUIRE(!cs_loaded_app_scene_input(&s,6));REQUIRE(!memcmp(big,snap,sizeof big));
        s.stride=72;s.storage_size=342u*72u-1u;
        REQUIRE(!cs_loaded_app_scene_input(&s,6));REQUIRE(!memcmp(big,snap,sizeof big));
        s.storage_size=342u*72u;s.format=CS_SURFACE_RGBA8;
        REQUIRE(!cs_loaded_app_scene_input(&s,6));REQUIRE(!memcmp(big,snap,sizeof big));
        s.format=CS_SURFACE_MONO1_MSB;s.storage=NULL;
        REQUIRE(!cs_loaded_app_scene_input(&s,6));REQUIRE(!memcmp(big,snap,sizeof big));
    }
    /* 5. Refused call after a valid frame leaves that frame intact. */
    full_frame(a,&rb[0],42);memcpy(b,a,sizeof a);
    REQUIRE(!cs_loaded_app_scene_input(NULL,43));
    REQUIRE(!memcmp(a,b,sizeof a));
}
int main(int argc,char **argv)
{
    if(argc==3 && !strcmp(argv[1],"preview")){render(argv[2]);return 0;}
    if(argc!=2)return 2;
    if(!strcmp(argv[1],"redraw"))redraw();else if(!strcmp(argv[1],"success"))success();else if(!strcmp(argv[1],"failures"))failures();
    else if(!strcmp(argv[1],"access"))access();else if(!strcmp(argv[1],"retire"))retire();else if(!strcmp(argv[1],"scene"))render(NULL);
    else if(!strcmp(argv[1],"claims"))claims();else return 2;
    puts("PASS");return 0;
}
