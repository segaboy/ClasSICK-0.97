/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Independent prepared-port selection owner tests. */
#include "../apps/loaded-rom-app/probe.h"
#include "../personality/system1/code-select.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define REQUIRE(x) do{++checks;if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);exit(1);}}while(0)
#define A5V UINT32_C(0x7FD8)
#define GV UINT32_C(0x7FD4)
#define BELOW UINT32_C(0x7DD8)
#define REC1 UINT32_C(0x7DD8)
#define REC2 UINT32_C(0x7E44)
#define OUT0 UINT32_C(0x7EB0)
#define OUT1 UINT32_C(0x7EB4)
#define OUT2 UINT32_C(0x7EB8)
#define GET 0xA874u
#define SET 0xA873u
static unsigned checks;
typedef struct {uint8_t b[100];size_t n;} prog;
typedef struct {
    cs_trap_gateway gw;cs_loaded_app_workspace *w;
    cs_qd_rect_port ports[2];cs_qd_view views[2];cs_qd_selection sel;
    cs_image_select_request req;cs_trap_frame caller;
    cs_image_select_workspace *ws;cs_image_select_report rep,rep2;
    cs_image_description desc;uint8_t src[512];size_t srclen;
    cs_image_run_result res;
} fx;
/* Unresolved native borrows remain live until process termination. */
static fx *unresolved_fixture;
static void put32(uint8_t *p,uint32_t v)
{for(unsigned i=0;i<4;++i)p[i]=(uint8_t)(v>>(24u-8u*i));}
static void raw(prog *p,uint16_t w)
{REQUIRE(p->n+2<=sizeof p->b);p->b[p->n++]=(uint8_t)(w>>8);p->b[p->n++]=(uint8_t)w;}
static void raw32(prog *p,uint32_t v)
{REQUIRE(p->n+4<=sizeof p->b);put32(p->b+p->n,v);p->n+=4;}
static void push(prog *p,uint32_t v,uint16_t trap){raw(p,0x2F3C);raw32(p,v);raw(p,trap);}
static void store(prog *p,uint32_t v,uint32_t a){raw(p,0x23FC);raw32(p,v);raw32(p,a);}
static void load(prog *p,uint32_t a){raw(p,0x2039);raw32(p,a);}
/* Original resource fork: CODE0 below512 above40 table8 offset32; CODE1 header+program. */
static size_t build_source(uint8_t *s,const uint8_t *code,size_t n)
{
    static const uint8_t table[8]={0,4,0x3F,0x3C,0,1,0xA9,0xF0};
    size_t d=36u+n;uint8_t *c0=s+260,*m=s+256u+d;
    memset(s,0,512);
    put32(s,256);put32(s+4,(uint32_t)(256u+d));put32(s+8,(uint32_t)d);put32(s+12,62);
    put32(s+256,24);put32(c0,40);put32(c0+4,512);put32(c0+8,8);put32(c0+12,32);
    memcpy(c0+16,table,8);
    put32(s+284,(uint32_t)(4u+n));s[291]=1;memcpy(s+292,code,n);
    m[25]=28;m[27]=62;m[30]=0x43;m[31]=0x4F;m[32]=0x44;m[33]=0x45;m[35]=1;m[37]=10;
    m[40]=0xFF;m[41]=0xFF;m[51]=1;m[52]=0xFF;m[53]=0xFF;m[57]=28;
    return 256u+d+62u;
}
static fx *make(const prog *p)
{
    fx *f=calloc(1,sizeof *f);REQUIRE(f);
    cs_trap_config c={0x400000,0x10000,0x20000,0x400100,0x400120,0x400140};
    f->w=calloc(1,sizeof *f->w);f->ws=calloc(1,sizeof *f->ws);REQUIRE(f->w && f->ws);
    REQUIRE(cs_trap_init(&f->gw,&c)==CS_TRAP_OK);
    f->srclen=build_source(f->src,p->b,p->n);
    REQUIRE(f->srclen<=4096);
    REQUIRE(cs_loaded_app_prepare_source(f->w,&f->gw,f->src,f->srclen));
    for(unsigned i=0;i<2;++i){
        memset(f->ports[i].pixels,0x3C,sizeof f->ports[i].pixels);
        REQUIRE(cs_qd_view_init(&f->views[i],&f->ports[i]));
    }
    f->sel.globals_begin=BELOW;f->sel.a5=A5V;f->sel.g=GV;f->sel.count=2;
    f->sel.bindings[0].id=REC1;f->sel.bindings[0].view=&f->views[0];
    f->sel.bindings[1].id=REC2;f->sel.bindings[1].view=&f->views[1];
    f->req.run=f->w->run_request;f->req.run.budget=64;f->req.selection=&f->sel;
    return f;
}
static void go(fx *f)
{
    memset(f->ws,0xA5,sizeof *f->ws);
    f->res=cs_image_run_selection(&f->w->image,&f->req,&f->caller,f->ws,sizeof *f->ws,&f->rep);
}
static int zero(const void *p,size_t n)
{const uint8_t *b=p;for(size_t i=0;i<n;++i)if(b[i])return 0;return 1;}
static int frame_eq(const cs_trap_frame *a,const cs_trap_frame *b)
{
    for(unsigned i=0;i<8;++i)if(a->d[i]!=b->d[i] || a->a[i]!=b->a[i])return 0;
    return a->pc==b->pc && a->sr==b->sr;
}
static uint32_t rd(fx *f,uint32_t at)
{
    uint32_t v=UINT32_MAX;
    REQUIRE(cs_guest_read(&f->w->map,at,4,CS_GUEST_READ,&v)==CS_GUEST_OK);return v;
}
static uint8_t *snap(const void *p,size_t n)
{uint8_t *s=malloc(n);REQUIRE(s);memcpy(s,p,n);return s;}
static void same(const void *p,uint8_t *s,size_t n)
{REQUIRE(!memcmp(p,s,n));free(s);}
static void below_zero(fx *f)
{
    cs_guest_view v;
    REQUIRE(cs_guest_window(&f->w->map,BELOW,516,CS_GUEST_READ,&v)==CS_GUEST_OK);
    REQUIRE(zero(v.bytes,516));
}
static void retire(fx *f)
{
    cs_image_select_report *r=&f->rep2;
    REQUIRE(cs_image_run_selection_retire(&f->w->image,&f->req.run,&f->w->image_workspace,
        sizeof f->w->image_workspace,&f->desc,r)==CS_IMAGE_OK);
    REQUIRE(r->retired==1 && r->scrubbed_bytes==512 && r->anchor_bytes==4 && r->handback_started==1);
    REQUIRE(frame_eq(&r->run.guest,&f->rep.run.guest));
    below_zero(f);
    REQUIRE(f->w->image.state==CS_IMAGE_EMPTY);
    REQUIRE(f->w->image.select.consumed_generation==f->rep.consumed_generation);
    REQUIRE(f->rep.consumed_generation!=0);
    REQUIRE(cs_image_external_admit(&f->w->image)==CS_IMAGE_OK);
}
/* Old APIs must refuse on a stopped selection, then retire. */
static void finish(fx *f,cs_image_run_result want)
{
    cs_image_run_report old;
    REQUIRE(f->res==want && f->rep.run.status==want);
    REQUIRE(zero(f->ws,sizeof *f->ws));
    REQUIRE(!f->rep.borrow_unresolved && !f->rep.quarantined && f->rep.first_stop_valid);
    REQUIRE(f->rep.first_stop_pc==f->rep.run.guest.pc && f->rep.first_stop_a7==f->rep.run.guest.a[7]);
    REQUIRE(frame_eq(&f->w->image.select.stop_frame,&f->rep.run.guest));
    REQUIRE(f->w->image.select.phase==CS_IMAGE_SELECT_STOPPED);
    REQUIRE(cs_image_external_admit(&f->w->image)!=CS_IMAGE_OK);
    REQUIRE(cs_image_run(&f->w->image,&f->req.run,&f->caller,&old)==CS_IMAGE_RUN_REFUSED);
    REQUIRE(cs_image_execute(&f->w->image,&f->w->image_request,&f->w->image_workspace,
        sizeof f->w->image_workspace,&f->desc,&f->w->image_report)!=CS_IMAGE_OK);
    REQUIRE(cs_image_run_retire(&f->w->image,&f->req.run,&f->w->image_workspace,
        sizeof f->w->image_workspace,&f->desc,&f->w->image_report)!=CS_IMAGE_OK);
    for(unsigned i=0;i<2;++i){
        for(unsigned k=0;k<sizeof f->ports[i].pixels;++k)REQUIRE(f->ports[i].pixels[k]==0x3C);
        REQUIRE(!f->ports[i].paints && !f->ports[i].erases);
    }
    retire(f);
}
static void release(fx *f){free(f->ws);free(f->w);free(f);}
static void canonical_prog(prog *p)
{
    memset(p,0,sizeof *p);
    store(p,0x11223344u,OUT0); /* proves that GetPort(NIL) writes zero */
    push(p,OUT0,GET);push(p,REC1,SET);push(p,OUT1,GET);
    store(p,REC2,GV);push(p,OUT2,GET);
    load(p,OUT2); /* D0 is read from the third output; all three outputs are checked exactly */
    raw(p,0x4E75);
}
static void test_canonical(void)
{
    prog p;canonical_prog(&p);
    fx *f=make(&p);cs_trap_frame saved;
    for(unsigned i=0;i<8;++i){f->caller.d[i]=i+100u;f->caller.a[i]=i+200u;}
    f->caller.pc=0x12345678u;f->caller.sr=0x1234u;saved=f->caller;go(f);
    REQUIRE(f->res==CS_IMAGE_RUN_RETURNED);
    REQUIRE(rd(f,OUT0)==0 && rd(f,OUT1)==REC1 && rd(f,OUT2)==REC2);
    REQUIRE(rd(f,GV)==REC2 && rd(f,A5V)==GV);
    REQUIRE(f->rep.run.guest.d[0]==REC2);
    REQUIRE(f->rep.services==4 && f->rep.last_word==GET && f->rep.binding_count==2);
    REQUIRE(f->rep.profile==CS_IMAGE_SELECT_PROFILE && f->rep.g==GV);
    REQUIRE(f->rep.run.steps>=13 && f->rep.run.entered==1);
    REQUIRE(!f->rep.first_fault_valid && f->rep.cause==CS_IMAGE_SELECT_OK);
    REQUIRE(f->rep.generation==f->rep.consumed_generation);
    REQUIRE(frame_eq(&f->caller,&saved));
    /* former native objects are never dereferenced after quiescence */
    memset(f->ws,0xEE,sizeof *f->ws);
    for(unsigned i=0;i<2;++i)memset(f->ports[i].pixels,0x3C,sizeof f->ports[i].pixels);
    memset(f->views,0xEE,sizeof f->views);memset(&f->sel,0xEE,sizeof f->sel);
    memset(f->ws,0,sizeof *f->ws);
    f->views[0].port=NULL;
    finish(f,CS_IMAGE_RUN_RETURNED);
    /* one generation only */
    memset(&f->caller,0,sizeof f->caller);
    REQUIRE(f->w->image.select.consumed_generation!=0);
    release(f);
}
static void arg_case(fx *f,int k)
{
    cs_image_service *im=&f->w->image;const cs_image_select_request *rq=&f->req;
    cs_trap_frame *cl=&f->caller;cs_image_select_workspace *ws=f->ws;size_t cap=sizeof *ws;
    cs_image_select_report *rp=&f->rep;cs_qd_rect_port *sp=f->views[1].port;
    size_t bn=sizeof *ws+16u;uint8_t *buf=calloc(1,bn);REQUIRE(buf);
    memset(f->ws,0xA5,sizeof *f->ws);memset(buf,0xA5,bn);
    memset(&f->rep,0x5A,sizeof f->rep);memset(&f->caller,0x11,sizeof f->caller);
    uint8_t *s0=snap(f->w,sizeof *f->w),*s1=snap(f->ws,sizeof *f->ws),*s2=snap(&f->rep,sizeof f->rep);
    uint8_t *s3=snap(&f->caller,sizeof f->caller),*s4=snap(buf,bn);
    switch(k){
    case 0:im=NULL;break;case 1:rq=NULL;break;case 2:cl=NULL;break;case 3:ws=NULL;break;
    case 4:rp=NULL;break;case 5:cap=sizeof *ws-1u;break;
    case 6:ws=(cs_image_select_workspace *)(void *)(buf+1);break;
    case 7:f->sel.count=0;break;case 8:f->sel.count=3;break;
    case 9:rp=(cs_image_select_report *)(void *)f->ws;break;
    case 10:cl=(cs_trap_frame *)(void *)f->ws;break;
    default:f->views[1].port=f->views[0].port;break;
    }
    REQUIRE(cs_image_run_selection(im,rq,cl,ws,cap,rp)==CS_IMAGE_RUN_ARGUMENT);
    f->sel.count=2;f->views[1].port=sp;
    same(f->w,s0,sizeof *f->w);same(f->ws,s1,sizeof *f->ws);same(&f->rep,s2,sizeof f->rep);
    same(&f->caller,s3,sizeof f->caller);same(buf,s4,bn);
    free(buf);memset(&f->caller,0,sizeof f->caller);
    REQUIRE(f->w->image.execution_state==0 && f->w->image.select.phase==CS_IMAGE_SELECT_NONE);
}
#define REFUSED(mod,undo) do{memset(f->ws,0xA5,sizeof *f->ws);mod;\
    REQUIRE(cs_image_run_selection(&f->w->image,&f->req,&f->caller,f->ws,sizeof *f->ws,&f->rep)\
        ==CS_IMAGE_RUN_REFUSED);undo;REQUIRE(zero(f->ws,sizeof *f->ws));\
    REQUIRE(f->w->image.execution_state==0 && f->w->image.select.phase==CS_IMAGE_SELECT_NONE);}while(0)
static void test_preclaim(void)
{
    prog p;canonical_prog(&p);fx *f=make(&p);cs_trap_word tw;
    for(int k=0;k<=11;++k)arg_case(f,k);
    REQUIRE(cs_trap_decode(0xA874,&tw)==CS_TRAP_OK);
    uint32_t old=f->sel.bindings[0].id,sg=f->sel.g,sa=f->sel.a5,sb=f->sel.globals_begin;
    REFUSED(f->sel.bindings[0].id=0,f->sel.bindings[0].id=old);
    REFUSED(f->sel.bindings[0].id=REC1+1u,f->sel.bindings[0].id=old);
    REFUSED(f->sel.bindings[1].id=REC1,f->sel.bindings[1].id=REC2);
    REFUSED(f->sel.bindings[1].id=REC1+2u,f->sel.bindings[1].id=REC2);
    REFUSED(f->sel.bindings[0].id=0x7F10,f->sel.bindings[0].id=old);
    REFUSED(f->sel.bindings[0].id=0x7FE0,f->sel.bindings[0].id=old);
    REFUSED(f->sel.g=BELOW+200u,f->sel.g=sg); /* reservation underflows below-A5 */
    REFUSED(f->sel.a5=0x7FDA,f->sel.a5=sa);
    REFUSED(f->sel.globals_begin=0x7DDA,f->sel.globals_begin=sb);
    REFUSED(f->req.run.generation+=1u,f->req.run.generation-=1u);
    REFUSED(f->req.run.epoch+=1u,f->req.run.epoch-=1u);
    REFUSED(f->req.run.budget=0,f->req.run.budget=64);
    REFUSED(f->req.run.token+=2u,f->req.run.token-=2u);
    REFUSED(f->req.run.context.active_root=1,f->req.run.context.active_root=0);
    REFUSED(REQUIRE(cs_trap_set(&f->gw,tw.number,0x400100)==CS_TRAP_OK),
        REQUIRE(cs_trap_set(&f->gw,tw.number,0x400140)==CS_TRAP_OK));
    REFUSED(f->w->regions[2].flags=CS_GUEST_READ,f->w->regions[2].flags=CS_GUEST_READ|CS_GUEST_EXECUTE);
    /* even a nonselected binding is fully validated; no claim either way */
    f->views[1].port=NULL;
    memset(f->ws,0xA5,sizeof *f->ws);go(f);
    REQUIRE(f->res==CS_IMAGE_RUN_ARGUMENT);
    REQUIRE(f->w->image.execution_state==0);
    f->views[1].port=&f->ports[1];
    /* preclaim failures consumed nothing: the unchanged request now runs */
    go(f);REQUIRE(f->res==CS_IMAGE_RUN_RETURNED);
    finish(f,CS_IMAGE_RUN_RETURNED);
    release(f);
}
static fx *one(uint32_t v,uint16_t trap)
{
    prog p;memset(&p,0,sizeof p);push(&p,v,trap);raw(&p,0x4E75);
    fx *f=make(&p);go(f);return f;
}
static void cpu_case(uint32_t v,uint16_t trap)
{
    fx *f=one(v,trap);
    REQUIRE(rd(f,GV)==0 && rd(f,OUT0)==0 && rd(f,A5V)==GV);
    finish(f,CS_IMAGE_RUN_CPU);release(f);
}
static void test_guest_faults(void)
{
    prog p;fx *f;
    cpu_case(0,SET);                 /* SetPort NIL */
    cpu_case(REC1+2u,SET);           /* unknown ID */
    cpu_case(REC1+1u,SET);           /* odd ID */
    cpu_case(OUT0+1u,GET);           /* odd destination */
    cpu_case(GV,GET);                /* current cell alias */
    cpu_case(0x7F10,GET);            /* reservation */
    cpu_case(REC1,GET);              /* record */
    cpu_case(A5V,GET);               /* above A5 / anchor */
    cpu_case(0x7FD0,GET);
    cpu_case(OUT0,0xA875);           /* unknown word */
    cpu_case(OUT0,0xA8A2);           /* PaintRect is not serviced here */
    cpu_case(OUT0,0xAC74);           /* auto-pop */
    cpu_case(OUT0,0xA9F0);
    memset(&p,0,sizeof p);raw(&p,GET);raw(&p,0x4E75);   /* no stack parameter */
    f=make(&p);go(f);REQUIRE(rd(f,GV)==0);finish(f,CS_IMAGE_RUN_CPU);release(f);
    memset(&p,0,sizeof p);store(&p,REC1+2u,GV);push(&p,OUT0,GET);raw(&p,0x4E75);
    f=make(&p);go(f);                /* unknown current ID */
    REQUIRE(rd(f,GV)==REC1+2u && rd(f,OUT0)==0);
    finish(f,CS_IMAGE_RUN_CPU);release(f);
    memset(&p,0,sizeof p);store(&p,0x12345678u,A5V);raw(&p,0x4E75);
    f=make(&p);go(f);REQUIRE(rd(f,A5V)==GV);   /* A5 write refused */
    finish(f,CS_IMAGE_RUN_CPU);release(f);
    memset(&p,0,sizeof p);store(&p,REC1,GV);push(&p,OUT0,GET);raw(&p,0x4E75);
    f=make(&p);go(f);                /* a valid direct store is honored */
    REQUIRE(rd(f,OUT0)==REC1);finish(f,CS_IMAGE_RUN_RETURNED);release(f);
    canonical_prog(&p);f=make(&p);f->req.run.budget=2;go(f);
    REQUIRE(f->res==CS_IMAGE_RUN_BUDGET);finish(f,CS_IMAGE_RUN_BUDGET);
    /* a retired generation cannot be selected again */
    go(f);REQUIRE(f->res==CS_IMAGE_RUN_REFUSED);
    release(f);
}
/* Lead contract-only gap cases, authored before implementation review. */
static void test_lead_gaps(void)
{
    prog p;memset(&p,0,sizeof p);push(&p,REC1,SET);push(&p,OUT0,GET);raw(&p,0x4E75);
    fx *f=make(&p);uint32_t alternative=GV-4u;
    f->sel.g=alternative;go(f);
    REQUIRE(f->res==CS_IMAGE_RUN_RETURNED && rd(f,A5V)==alternative);
    REQUIRE(rd(f,alternative)==REC1 && rd(f,OUT0)==REC1);
    finish(f,CS_IMAGE_RUN_RETURNED);
    /* A later loaded generation is allowed; old consumed generation is not. */
    uint32_t consumed=f->w->image.select.consumed_generation;
    REQUIRE(cs_loaded_app_prepare_source(f->w,&f->gw,f->src,f->srclen));
    f->req.run=f->w->run_request;f->req.run.budget=64;go(f);
    REQUIRE(f->res==CS_IMAGE_RUN_RETURNED && f->rep.generation>consumed);
    finish(f,CS_IMAGE_RUN_RETURNED);release(f);

    memset(&p,0,sizeof p);store(&p,UINT32_MAX,GV);push(&p,REC1,SET);raw(&p,0x4E75);
    f=make(&p);go(f);
    REQUIRE(rd(f,GV)==UINT32_MAX && f->rep.services==0);
    finish(f,CS_IMAGE_RUN_CPU);release(f);

    memset(&p,0,sizeof p);push(&p,OUT0,GET);raw(&p,0x4E75);
    f=make(&p);f->sel.count=1;
    f->sel.bindings[1].view=NULL;f->sel.bindings[1].id=UINT32_MAX;
    go(f);REQUIRE(f->res==CS_IMAGE_RUN_RETURNED && f->rep.binding_count==1);
    finish(f,CS_IMAGE_RUN_RETURNED);release(f);

    f=make(&p);go(f);REQUIRE(f->res==CS_IMAGE_RUN_RETURNED);
    cs_trap_frame stop=f->rep.run.guest;
    f->w->image.select.owner_id+=1u;
    uint8_t *ram=snap(f->w->ram,sizeof f->w->ram);
    REQUIRE(cs_image_run_selection_retire(&f->w->image,&f->req.run,&f->w->image_workspace,
        sizeof f->w->image_workspace,&f->desc,&f->rep2)==CS_IMAGE_FAULT);
    same(f->w->ram,ram,sizeof f->w->ram);
    REQUIRE(f->rep2.quarantined && !f->rep2.scrubbed_bytes && !f->rep2.anchor_bytes);
    REQUIRE(frame_eq(&f->w->image.select.stop_frame,&stop));release(f);
}
/* Lead implementation-aware adversarial coverage of the full-record review gap.
 * Expectations still follow SPEC45: a mismatched owned scalar cannot authorize g. */
static void test_owner_record(void)
{
    prog p;memset(&p,0,sizeof p);push(&p,OUT0,GET);raw(&p,0x4E75);
    for(unsigned k=0;k<18;++k){
        fx *f=make(&p);go(f);REQUIRE(f->res==CS_IMAGE_RUN_RETURNED);
        cs_image_select_state *o=&f->w->image.select;
        uint32_t *fields[]={&o->profile,&o->consumed_generation,&o->inventory_kind,
            &o->inventory_bytes,&o->globals_begin,&o->a5,&o->below_a5,&o->above_a5,
            &o->stack_base,&o->stack_top,&o->root,&o->reservation_begin,&o->count,
            &o->extent,&o->layout.b,&o->ids[0],&o->anchor_cleared,&o->generation};
        ++*fields[k];uint8_t *ram=snap(f->w->ram,sizeof f->w->ram);
        REQUIRE(cs_image_run_selection_retire(&f->w->image,&f->req.run,&f->w->image_workspace,
            sizeof f->w->image_workspace,&f->desc,&f->rep2)==CS_IMAGE_FAULT);
        REQUIRE(f->rep2.quarantined && !f->rep2.scrubbed_bytes && !f->rep2.handback_started);
        same(f->w->ram,ram,sizeof f->w->ram);release(f);
    }
}
#ifdef CLASSICK_IMAGE_TEST_FAULTS
static void test_faults(void)
{
    prog p;canonical_prog(&p);fx *f=make(&p);
    REQUIRE(cs_image_selection_test_fault(&f->w->image,0,CS_IMAGE_SELECT_TEST_HEADER)==CS_IMAGE_ARGUMENT);
    REQUIRE(cs_image_selection_test_fault(&f->w->image,3,4u)==CS_IMAGE_ARGUMENT);
    REQUIRE(cs_image_selection_test_fault(&f->w->image,3,CS_IMAGE_SELECT_TEST_HEADER)==CS_IMAGE_OK);
    REQUIRE(cs_image_selection_test_fault(&f->w->image,0,0)==CS_IMAGE_OK);   /* kind0 clears */
    go(f);REQUIRE(f->res==CS_IMAGE_RUN_RETURNED);finish(f,CS_IMAGE_RUN_RETURNED);release(f);
    for(uint32_t kind=1;kind<=3;++kind){
        f=make(&p);
        REQUIRE(cs_image_selection_test_fault(&f->w->image,3,kind)==CS_IMAGE_OK);
        go(f);
        REQUIRE(f->res==CS_IMAGE_RUN_QUARANTINED && f->rep.run.status==CS_IMAGE_RUN_QUARANTINED);
        REQUIRE(f->rep.quarantined==1 && f->rep.first_fault_valid==1);
        REQUIRE(f->rep.first_fault_pc==f->w->image.select.fault_pc);
        REQUIRE(f->rep.first_fault_cause==f->w->image.select.fault_cause);
        uint8_t *ram=snap(f->w->ram,sizeof f->w->ram);
        uint8_t *ws=snap(f->ws,sizeof *f->ws);
        if(kind==CS_IMAGE_SELECT_TEST_QUIESCENCE){
            REQUIRE(f->rep.borrow_unresolved==1 && !zero(f->ws,sizeof *f->ws));
            uint8_t *saved_report=snap(&f->rep,sizeof f->rep);
            REQUIRE(cs_image_run_selection(&f->w->image,&f->req,&f->caller,f->ws,
                sizeof *f->ws,&f->rep)==CS_IMAGE_RUN_REFUSED);
            same(&f->rep,saved_report,sizeof f->rep);
            REQUIRE(!memcmp(f->ws,ws,sizeof *f->ws));
        } else {
            REQUIRE(f->rep.borrow_unresolved==0 && zero(f->ws,sizeof *f->ws));
            REQUIRE(f->rep.first_fault_cause==(kind==CS_IMAGE_SELECT_TEST_BINDING?
                CS_IMAGE_SELECT_BINDING:f->rep.first_fault_cause));
        }
        REQUIRE(cs_image_run_selection_retire(&f->w->image,&f->req.run,&f->w->image_workspace,
            sizeof f->w->image_workspace,&f->desc,&f->rep2)!=CS_IMAGE_OK);
        if(kind==CS_IMAGE_SELECT_TEST_QUIESCENCE){
            same(f->w->ram,ram,sizeof f->w->ram);same(f->ws,ws,sizeof *f->ws);
            REQUIRE(f->w->image.state==CS_IMAGE_LOADED && f->rep2.retired==0);
        } else {free(ram);free(ws);}
        REQUIRE(cs_image_external_admit(&f->w->image)!=CS_IMAGE_OK);
        if(kind==CS_IMAGE_SELECT_TEST_QUIESCENCE)unresolved_fixture=f;
        else release(f);
    }
    /* failed original RELEASE after a normal stop: scrub done, ownership kept */
    f=make(&p);go(f);REQUIRE(f->res==CS_IMAGE_RUN_RETURNED);
    REQUIRE(cs_image_test_fault(&f->w->image,0,3,1,0)==CS_IMAGE_OK);
    REQUIRE(cs_image_run_selection_retire(&f->w->image,&f->req.run,&f->w->image_workspace,
        sizeof f->w->image_workspace,&f->desc,&f->rep2)!=CS_IMAGE_OK);
    REQUIRE(f->rep2.quarantined==1 && f->rep2.retired==0 && f->rep2.handback_started==1);
    REQUIRE(f->rep2.scrubbed_bytes==512 && f->rep2.anchor_bytes==4);
    REQUIRE(f->w->image.select.consumed_generation!=0 && f->w->image.state!=CS_IMAGE_EMPTY);
    REQUIRE(frame_eq(&f->rep2.run.guest,&f->rep.run.guest));
    below_zero(f);
    REQUIRE(cs_image_external_admit(&f->w->image)!=CS_IMAGE_OK);
    release(f);
}
#endif
int main(void)
{
    test_canonical();
    test_preclaim();
    test_guest_faults();
    test_lead_gaps();
    test_owner_record();
#ifdef CLASSICK_IMAGE_TEST_FAULTS
    test_faults();
    REQUIRE(unresolved_fixture && unresolved_fixture->rep.borrow_unresolved);
#endif
    REQUIRE(checks>0);
    printf("PASS %u checks", checks);
    puts("");
    return 0;
}
