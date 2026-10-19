/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Independent SPEC-0049 oracle; never calls native view/outline for expectations. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../apps/outline-drawing/workload.h"
#include "../apps/boot-scene/scene.h"
#define G 64u
static int bad;
static unsigned checks;
#define CK(c) do{++checks;if(!(c)){printf("FAIL %d: %s\n",__LINE__,#c);bad=1;exit(1);}}while(0)

typedef struct{int ox,oy,ct,cl,cb,cr,t,l,b,r,w,h;}spec;
/* Logical rectangle t,l,b,r; w is left/right thickness, h is top/bottom. */
static const spec SP[2]={
 {8,8,8,8,56,56,0,0,48,48,3,2},
 {72,8,8,72,56,120,-8,0,56,48,4,3}};

/* Inverse map: physical pixel to logical, then edge distances to the rectangle. */
static int want(const spec *s,int x,int y){
    int64_t lx,ly,dl,dr,dt,db;
    if(x<s->cl||x>=s->cr||y<s->ct||y>=s->cb)return 0;
    lx=x-s->ox;ly=y-s->oy;
    dl=lx-s->l;dr=s->r-1-lx;dt=ly-s->t;db=s->b-1-ly;
    if(dl<0||dr<0||dt<0||db<0)return 0;
    return dl<s->w||dr<s->w||dt<s->h||db<s->h;}
static int bit(const uint8_t *r,int x){return (r[x>>3]>>(7-(x&7)))&1;}
static int portok(const uint8_t *px,const spec *s,uint32_t *n){
    int x,y;*n=0;
    for(y=0;y<64;y++)for(x=0;x<128;x++){
        int b=bit(px+y*16,x);*n+=(uint32_t)b;if(b!=want(s,x,y))return 0;}
    return 1;}
static int guards(const uint8_t *m,size_t n){size_t i;
    for(i=0;i<G;i++){if(m[i]!=0xA5||m[G+n+i]!=0x5A)return 0;}
    return 1;}
static uint8_t *mk(size_t n){uint8_t *m=malloc(n+2*G);
    if(m){memset(m,0xA5,G);memset(m+G,0xCC,n);memset(m+G+n,0x5A,G);}return m;}
static uint32_t fnv(const uint8_t *p,size_t n){uint32_t h=2166136261u;size_t i;
    for(i=0;i<n;i++){h^=p[i];h*=16777619u;}return h;}
static void check(const cs_selected_draw_workspace *w,const cs_selected_draw_result *r){
    uint8_t u[CS_QD_RECT_BYTES];uint32_t a,b,n=0;size_t i;int x,y;
    CK(portok(w->ports[0].pixels,&SP[0],&a)&&a==456);
    CK(portok(w->ports[1].pixels,&SP[1],&b)&&b==384);
    for(i=0;i<sizeof u;i++)u[i]=(uint8_t)(w->ports[0].pixels[i]|w->ports[1].pixels[i]);
    for(y=0;y<64;y++)for(x=0;x<128;x++){
        int e=want(&SP[0],x,y)|want(&SP[1],x,y);
        CK(bit(w->pixels+y*16,x)==e);n+=(uint32_t)e;}
    CK(n==840);CK(n==a+b);CK(memcmp(u,w->pixels,sizeof u)==0);
    CK(w->ports[0].paints==1&&w->ports[0].erases==0);
    CK(w->ports[1].paints==1&&w->ports[1].erases==0);
    CK(r->draw.success==1&&r->draw.phase==6&&r->draw.value==97);
    CK(r->draw.black_pixels==840u&&r->draw.pattern==fnv(u,sizeof u));
    CK(r->selections==2&&r->current==0x706Cu);}
static int nearly(const uint8_t *q,uint8_t v){
    return q[0]==v&&q[1]==v&&q[2]==v&&q[3]==0;}
static void frame_vs_mono(const uint8_t *fb,const uint8_t *mono){
    enum{MW=512,MH=342,PW=640,PH=480};int x,y;
    for(y=0;y<MH;y++)for(x=0;x<MW;x++){
        const uint8_t *q=fb+((size_t)(y+69)*PW+(size_t)(x+64))*4;
        CK(nearly(q,(uint8_t)(bit(mono+y*64,x)?0u:255u)));}
    for(y=0;y<PH;y++)for(x=0;x<PW;x++)if(x<64||x>=576||y<69||y>=411){
        const uint8_t *q=fb+((size_t)y*PW+(size_t)x)*4;
        CK(nearly(q,255));}}
static void scene(const cs_selected_draw_workspace *w){
    enum{MW=512,MH=342,PW=640,PH=480};
    uint8_t *mono=calloc(64u*MH,1),*m2=calloc(64u*MH,1),*arena=calloc(70000,1);
    uint8_t *fbm=calloc((size_t)PW*PH*4,1),*fb0=calloc((size_t)PW*PH*4,1);
    cs_surface ms,m2s;cs_arena ar;cs_fb_target t;cs_boot_scene s;cs_boot_report rp;
    cs_product_state st;int x,y,pc=0,minx=MW,miny=MH,maxx=-1,maxy=-1;
    uint32_t steps[2]={0,9},inp[2]={0,2};int si,ii;
    if(!mono||!m2||!arena||!fbm||!fb0){CK(0);goto done;}
    CK(cs_arena_init(&ar,arena,70000)==CS_ARENA_OK);
    CK(cs_fb_init(&t,(volatile unsigned char*)fbm,(size_t)PW*PH*4,PW,PH,PW,CS_FB_RGB_RESERVED)==CS_FB_OK);
    CK(cs_boot_scene_prepare(&ar,&t,&s)==CS_BOOT_OK);
    CK(s.state.pixels&&memcmp(s.state.pixels,w->pixels,CS_QD_RECT_BYTES)==0);
    CK(s.state.progress==0);
    st=s.state;st.pixels=w->pixels;
    CK(cs_surface_init(&ms,mono,64u*MH,MW,MH,64,CS_SURFACE_MONO1_MSB)==CS_SURFACE_OK);
    CK(cs_product_scene(&ms,MW,MH,0,&st)==1);
    memcpy(m2,mono,64u*MH);
    CK(cs_surface_init(&m2s,m2,64u*MH,MW,MH,64,CS_SURFACE_MONO1_MSB)==CS_SURFACE_OK);
    CK(cs_product_scene_input(&m2s,2)==1);
    for(y=0;y<MH;y++)for(x=0;x<MW;x++)if(bit(mono+y*64,x)!=bit(m2+y*64,x)){
        CK(x>=352&&x<432&&y>=286&&y<306);
        pc++;if(x<minx)minx=x;if(x>maxx)maxx=x;if(y<miny)miny=y;if(y>maxy)maxy=y;}
    CK(pc>0&&maxx-minx<80&&maxy-miny<20);
    for(si=0;si<2;si++)for(ii=0;ii<2;ii++){
        memset(fbm,0x7E,(size_t)PW*PH*4);
        if(inp[ii]==0)CK(cs_boot_scene_draw(&s,steps[si],&rp)==CS_BOOT_OK);
        else CK(cs_boot_scene_draw_input(&s,steps[si],inp[ii],&rp)==CS_BOOT_OK);
        CK(rp.step==steps[si]);
        CK(s.state.progress==0);
        frame_vs_mono(fbm,inp[ii]?m2:mono);
        if(inp[ii]==0)memcpy(fb0,fbm,(size_t)PW*PH*4);
        else for(y=0;y<PH;y++)for(x=0;x<PW;x++){
            int cx=x-64,cy=y-69;
            int ctr=cx>=352&&cx<432&&cy>=286&&cy<306;
            if(!ctr)CK(memcmp(fbm+((size_t)y*PW+(size_t)x)*4,fb0+((size_t)y*PW+(size_t)x)*4,4)==0);}}
done:free(mono);free(m2);free(arena);free(fbm);free(fb0);}
int main(void){
    size_t wn=sizeof(cs_selected_draw_workspace),rn=sizeof(cs_selected_draw_result);
    uint8_t *wm=mk(wn),*rm=mk(rn),*wm2=mk(wn),*rm2=mk(rn),*snap,*keep;
    cs_selected_draw_workspace *w,*w2;cs_selected_draw_result *r,*r2;int i;
    if(!wm||!rm||!wm2||!rm2)return 2;
    w=(cs_selected_draw_workspace*)(wm+G);r=(cs_selected_draw_result*)(rm+G);
    w2=(cs_selected_draw_workspace*)(wm2+G);r2=(cs_selected_draw_result*)(rm2+G);
    snap=malloc(wn+rn);keep=malloc(wn+rn);if(!snap||!keep)return 2;
    for(i=0;i<2;i++){
        if(i)memset(wm+G,0x3C,wn),memset(rm+G,0xC3,rn);
        memcpy(snap,wm+G,wn);memcpy(snap+wn,rm+G,rn);
        CK(cs_outline_draw(NULL,r)==0);
        CK(cs_outline_draw(w,NULL)==0);
        CK(cs_outline_draw(NULL,NULL)==0);
        CK(memcmp(snap,wm+G,wn)==0&&memcmp(snap+wn,rm+G,rn)==0);
        CK(guards(wm,wn)&&guards(rm,rn));}
    for(i=0;i<3;i++){
        memset(wm+G,i==1?0xFF:0x3C,wn);memset(rm+G,i==1?0xFF:0xC3,rn);
        CK(cs_outline_draw(w,r)==1);check(w,r);
        CK(guards(wm,wn)&&guards(rm,rn));
        if(i==0){memcpy(keep,wm+G,wn);memcpy(keep+wn,rm+G,rn);}
        else CK(memcmp(keep,wm+G,wn)==0&&memcmp(keep+wn,rm+G,rn)==0);}
    memcpy(keep,wm+G,wn);memcpy(keep+wn,rm+G,rn);
    memset(wm2+G,0x00,wn);memset(rm2+G,0xFF,rn);
    CK(cs_outline_draw(w2,r2)==1);check(w2,r2);
    CK(guards(wm2,wn)&&guards(rm2,rn));
    CK(memcmp(keep,wm+G,wn)==0&&memcmp(keep+wn,rm+G,rn)==0);
    CK(memcmp(w->pixels,w2->pixels,CS_QD_RECT_BYTES)==0);
    scene(w);
    free(snap);free(keep);free(wm);free(rm);free(wm2);free(rm2);
    puts(bad?"outline-drawing-independent FAIL":"outline-drawing-independent PASS");
    printf("%u checks\n",checks);
    return bad;}
