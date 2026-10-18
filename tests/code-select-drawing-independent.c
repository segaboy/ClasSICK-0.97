/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Independent SPEC-0046 selected-port drawing oracle. */
#define main cs_old_selection_main
#include "code-select-independent.c"
#undef main
#define RECT UINT32_C(0x7EC0)
#define PAINT 0xA8A2u
#define ERASE 0xA8A3u
/* Distinct physical offsets/clips (top,left,bottom,right) per port. */
static const int OX[2]={1,3},OY[2]={2,5};
static const int16_t CL[2][4]={{0,0,64,128},{4,6,40,60}};
static const int16_t RC[2][4]={{10,20,30,50},{0,0,60,100}};
static void p16(uint8_t *b,int16_t v){b[0]=(uint8_t)((uint16_t)v>>8);b[1]=(uint8_t)v;}
static void r8(uint8_t o[8],const int16_t r[4]){for(unsigned i=0;i<4;++i)p16(o+2*i,r[i]);}
static int px(const uint8_t *m,int x,int y){return (m[y*16+x/8]>>(7-x%8))&1;}
/* Independent per-pixel model: black=1, MSB-leftmost, physical=logical+offset. */
static void expect_port(const cs_qd_rect_port *p,const uint8_t *init,const int16_t *r,
    unsigned i,int value)
{
    for(int y=0;y<64;++y)for(int x=0;x<128;++x){
        int in=r && y>=r[0]+OY[i] && y<r[2]+OY[i] && x>=r[1]+OX[i] && x<r[3]+OX[i]
            && y>=CL[i][0] && y<CL[i][2] && x>=CL[i][1] && x<CL[i][3];
        REQUIRE(px(p->pixels,x,y)==(in?value:px(init,x,y)));
    }
}
static void go2(fx *f)
{
    cs_trap_frame saved=f->caller;
    memset(f->ws,0xA5,sizeof *f->ws);
    f->res=cs_image_run_selection_draw(&f->w->image,&f->req,&f->caller,f->ws,sizeof *f->ws,&f->rep);
    REQUIRE(!memcmp(&saved,&f->caller,sizeof saved));
}
/* First op writes current directly then draws at RECT; optional second op repeats
 * with c2/ra2/t2. The rect is written by guest stores only at RECT. */
static fx *mk(uint16_t t1,uint32_t c1,uint32_t c2,uint32_t ra2,uint16_t t2,
    const int16_t r[4],uint8_t init[2][1024])
{
    prog p;memset(&p,0,sizeof p);
    store(&p,((uint32_t)(uint16_t)r[0]<<16)|(uint16_t)r[1],RECT);
    store(&p,((uint32_t)(uint16_t)r[2]<<16)|(uint16_t)r[3],RECT+4);
    store(&p,c1,GV);push(&p,RECT,t1);
    if(ra2){store(&p,c2,GV);push(&p,ra2,t2);}
    raw(&p,0x4E75);
    fx *f=make(&p);
    uint8_t cr[8];
    for(unsigned i=0;i<2;++i){
        r8(cr,CL[i]);
        REQUIRE(cs_qd_view_offset(&f->views[i],OX[i],OY[i]));
        REQUIRE(cs_qd_view_clip(&f->views[i],cr));
        memcpy(init[i],f->ports[i].pixels,1024);
    }
    f->caller=f->w->caller;
    return f;
}
static void once_only(fx *f)
{
    retire(f);
    REQUIRE(cs_image_run_selection_retire(&f->w->image,&f->req.run,&f->w->image_workspace,
        sizeof f->w->image_workspace,&f->desc,&f->rep2)!=CS_IMAGE_OK);
    go2(f);REQUIRE(f->res!=CS_IMAGE_RUN_RETURNED);
}
static void clean_draw(unsigned sel,uint16_t trap,const int16_t *r)
{
    uint8_t init[2][1024];
    uint32_t id=sel?REC2:REC1;
    fx *f=mk(trap,id,0,0,0,r,init);
    go2(f);
    REQUIRE(f->res==CS_IMAGE_RUN_RETURNED);
    REQUIRE(f->rep.services==1 && f->rep.last_word==trap);
    REQUIRE(rd(f,GV)==id);
    expect_port(&f->ports[sel],init[sel],r,sel,trap==PAINT);
    expect_port(&f->ports[!sel],init[!sel],NULL,!sel,0);
    REQUIRE(f->ports[sel].paints==(trap==PAINT) && f->ports[sel].erases==(trap==ERASE));
    REQUIRE(f->ports[!sel].paints==0 && f->ports[!sel].erases==0);
    REQUIRE(frame_eq(&f->caller,&f->w->caller));
    once_only(f);
    release(f);
}
static void refused_second(uint32_t c2,uint32_t ra2)
{
    uint8_t init[2][1024];
    fx *f=mk(PAINT,REC1,c2,ra2,PAINT,RC[0],init);
    go2(f);
    REQUIRE(f->res==CS_IMAGE_RUN_CPU && f->rep.run.cpu==CS_M68K_POLICY);
    REQUIRE(f->rep.quarantined==0 && f->rep.borrow_unresolved==0);
    REQUIRE(f->rep.services==1 && f->rep.last_word==PAINT);
    expect_port(&f->ports[0],init[0],RC[0],0,1);
    expect_port(&f->ports[1],init[1],NULL,1,0);
    REQUIRE(f->ports[0].paints==1 && f->ports[1].paints==0 && f->ports[1].erases==0);
    REQUIRE(rd(f,GV)==c2);
    /* Lead: exact rejected frame and previous completed service scratch. */
    uint32_t entry=f->w->image.segments[0].address+4u;
    REQUIRE(f->rep.run.guest.pc==entry+54u && f->rep.run.steps==8u);
    REQUIRE(f->rep.run.guest.a[7]==f->w->image.layout.stack_top-8u);
    REQUIRE(rd(f,f->rep.run.guest.a[7]-4u)==entry+38u);
    cs_guest_view sr;
    REQUIRE(cs_guest_window(&f->w->map,f->rep.run.guest.a[7]-6u,2,CS_GUEST_READ,&sr)==CS_GUEST_OK);
    REQUIRE(sr.bytes[0]==0x27 && sr.bytes[1]==0);
    retire(f);
    release(f);
}
static void empty_draw(unsigned sel)
{
    uint8_t init[2][1024];
    int16_t e[4]={20,20,20,50};
    fx *f=mk(PAINT,sel?REC2:REC1,0,0,0,e,init);
    go2(f);
    REQUIRE(f->res==CS_IMAGE_RUN_RETURNED && f->rep.services==1);
    expect_port(&f->ports[sel],init[sel],NULL,sel,0);
    REQUIRE(f->ports[sel].paints==1 && f->ports[!sel].paints==0);
    retire(f);
    release(f);
}
static void patched_slot(uint16_t number)
{
    uint8_t init[2][1024];
    fx *f=mk(PAINT,REC1,0,0,0,RC[0],init);
    REQUIRE(cs_trap_set(&f->gw,number,0x10100)==CS_TRAP_OK);
    go2(f);
    REQUIRE(f->res==CS_IMAGE_RUN_REFUSED && f->rep.cause==CS_IMAGE_SELECT_DIRECTORY);
    REQUIRE(!f->rep.run.entered && !f->w->image.execution_state);
    expect_port(&f->ports[0],init[0],NULL,0,0);
    expect_port(&f->ports[1],init[1],NULL,1,0);
    REQUIRE(f->ports[0].paints==0 && f->ports[1].paints==0);
    release(f);
}
static void prf1_refuses(uint16_t trap)
{
    uint8_t init[2][1024];
    fx *f=mk(trap,REC1,0,0,0,RC[0],init);
    go(f);
    REQUIRE(f->res==CS_IMAGE_RUN_CPU && f->rep.run.cpu==CS_M68K_POLICY);
    expect_port(&f->ports[0],init[0],NULL,0,0);
    expect_port(&f->ports[1],init[1],NULL,1,0);
    REQUIRE(f->ports[0].paints==0 && f->ports[0].erases==0);
    retire(f);release(f);
}
/* Lead contract gap cases; original Copilot oracle above kept independent of
 * the implementation. These additions use the frozen contract and old helpers. */
static void lead_switch_and_saturation(void)
{
    uint8_t init[2][1024];fx *f=mk(PAINT,REC1,REC2,RECT,ERASE,RC[0],init);
    f->ports[0].paints=UINT32_MAX;f->ports[1].erases=UINT32_MAX;
    memset(&f->caller,0x6B,sizeof f->caller);go2(f);
    REQUIRE(f->res==CS_IMAGE_RUN_RETURNED && f->rep.services==2 && f->rep.last_word==ERASE);
    REQUIRE(f->rep.profile==CS_IMAGE_SELECT_DRAW_PROFILE && rd(f,GV)==REC2);
    expect_port(&f->ports[0],init[0],RC[0],0,1);
    expect_port(&f->ports[1],init[1],RC[0],1,0);
    REQUIRE(f->ports[0].paints==UINT32_MAX && f->ports[1].erases==UINT32_MAX);
    REQUIRE(!f->ports[0].erases && !f->ports[1].paints);
    REQUIRE(f->rep.run.guest.a[7]==f->w->image.layout.stack_top);
    REQUIRE(zero(f->ws,sizeof *f->ws));
    /* Resolved native borrows may end before scalar-only retirement. */
    f->views[0].port=NULL;f->views[1].port=NULL;f->req.selection=NULL;
    retire(f);release(f);
}
static void lead_rect_scratch_alias(void)
{
    prog p;memset(&p,0,sizeof p);
    /* root=0x7DD4, parameter SP=0x7DD0; Rect ends at SP and aliases
     * six scratch bytes. The return token at root is not part of Rect. */
    const uint32_t at=UINT32_C(0x7DC8);const int16_t rect[4]={1,2,3,4};
    store(&p,0x00010002u,at);store(&p,0x00030004u,at+4u);
    store(&p,REC1,GV);push(&p,at,PAINT);raw(&p,0x4E75);
    fx *f=make(&p);uint8_t init[1024],clip[8];memcpy(init,f->ports[0].pixels,1024);
    REQUIRE(cs_qd_view_offset(&f->views[0],OX[0],OY[0]));r8(clip,CL[0]);
    REQUIRE(cs_qd_view_clip(&f->views[0],clip));go2(f);
    REQUIRE(f->res==CS_IMAGE_RUN_RETURNED && f->rep.services==1);
    expect_port(&f->ports[0],init,rect,0,1);
    REQUIRE(rd(f,at+4u)==f->w->image.segments[0].address+4u+38u);
    REQUIRE(f->ports[0].paints==1 && !f->ports[1].paints);
    retire(f);release(f);
}
static void lead_misc(void)
{
    static const uint16_t variants[3]={0xACA2u,0xA9A2u,0xA8A4u};
    for(unsigned i=0;i<3;++i){uint8_t init[2][1024];
        fx *f=mk(variants[i],REC1,0,0,0,RC[0],init);go2(f);
        REQUIRE(f->res==CS_IMAGE_RUN_CPU && !f->rep.services);
        expect_port(&f->ports[0],init[0],NULL,0,0);retire(f);release(f);}
    {uint8_t init[2][1024];fx *f=mk(PAINT,REC1,0,0,0,RC[0],init);
        f->views[1].right=129;go2(f);
        REQUIRE(f->res==CS_IMAGE_RUN_REFUSED && f->rep.cause==CS_IMAGE_SELECT_CANDIDATE);
        REQUIRE(!f->w->image.execution_state);expect_port(&f->ports[0],init[0],NULL,0,0);release(f);}
    {uint8_t init[2][1024];fx *f=mk(PAINT,REC1,0,0,0,RC[0],init);
        REQUIRE(cs_image_selection_test_fault(&f->w->image,6,CS_IMAGE_SELECT_TEST_BINDING)==CS_IMAGE_OK);
        go2(f);REQUIRE(f->res==CS_IMAGE_RUN_QUARANTINED && f->rep.services==1);
        REQUIRE(f->rep.first_fault_cause==CS_IMAGE_SELECT_BINDING && !f->rep.borrow_unresolved);
        expect_port(&f->ports[0],init[0],RC[0],0,1);release(f);}
    {uint8_t init[2][1024];fx *f=mk(PAINT,REC1,0,0,0,RC[0],init);
        REQUIRE(cs_trap_set(&f->gw,0xA2,0x10100)==CS_TRAP_OK);
        /* PRF1 does not depend on drawing slots; its Set/Get remain usable. */
        go(f);REQUIRE(f->res==CS_IMAGE_RUN_CPU && f->rep.run.entered && !f->rep.services);
        REQUIRE(f->rep.profile==CS_IMAGE_SELECT_PROFILE);retire(f);release(f);}
    for(unsigned k=0;k<2;++k){uint8_t init[2][1024];fx *f=mk(PAINT,REC1,0,0,0,RC[0],init);
        go2(f);REQUIRE(f->res==CS_IMAGE_RUN_RETURNED);
        if(k)++f->w->image.select.admitted_profile;
        else f->w->image.select.profile=CS_IMAGE_SELECT_PROFILE;
        uint8_t *ram=snap(f->w->ram,sizeof f->w->ram);
        REQUIRE(cs_image_run_selection_retire(&f->w->image,&f->req.run,&f->w->image_workspace,
            sizeof f->w->image_workspace,&f->desc,&f->rep2)==CS_IMAGE_FAULT);
        REQUIRE(f->rep2.quarantined && !f->rep2.scrubbed_bytes);
        same(f->w->ram,ram,sizeof f->w->ram);release(f);}
}
int main(void)
{
    static const uint16_t traps[2]={PAINT,ERASE};
    static const uint16_t slots[4]={0x73,0x74,0xA2,0xA3};
    (void)cs_old_selection_main;(void)unresolved_fixture;(void)load;(void)snap;(void)same;
    for(unsigned s=0;s<2;++s)for(unsigned t=0;t<2;++t)clean_draw(s,traps[t],RC[s]);
    empty_draw(0);empty_draw(1);
    refused_second(0,RECT);                      /* NIL current */
    refused_second(0x1234,RECT);                 /* unknown current */
    refused_second(REC2,RECT+1);                 /* odd */
    refused_second(REC2,REC1+8);                 /* inside record */
    refused_second(REC2,UINT32_C(0x7F10));       /* reservation */
    refused_second(REC2,UINT32_C(0x7FE0));       /* above A5 */
    refused_second(REC2,UINT32_C(0x80000000)|RECT);/* high byte */
    for(unsigned i=0;i<4;++i)patched_slot(slots[i]);
    prf1_refuses(PAINT);prf1_refuses(ERASE);
    lead_switch_and_saturation();lead_rect_scratch_alias();lead_misc();
    printf("code-select-drawing-independent: %u checks\n",checks);
    return 0;
}
