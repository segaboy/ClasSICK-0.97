/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Independent SPEC-0047 oracle; never calls native view/draw for expectations. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../apps/selected-drawing/workload.h"
#include "../apps/boot-scene/scene.h"
#define G 64u
static int bad;
static unsigned checks;
#define CK(c) do{++checks;if(!(c)){printf("FAIL %d: %s\n",__LINE__,#c);bad=1;exit(1);}}while(0)
static int in(int x,int y,int t,int l,int b,int r){return y>=t&&y<b&&x>=l&&x<r;}
static int p0(int x,int y){return in(x,y,8,8,56,56)&&!in(x,y,16,16,48,48);}
static int p1(int x,int y){return in(x,y,8,72,56,120)&&!in(x,y,8,88,56,104);}
static int bit(const uint8_t *r,int x){return (r[x>>3]>>(7-(x&7)))&1;}
static int portok(const uint8_t *px,int(*pr)(int,int),uint32_t *n){
    int x,y;*n=0;
    for(y=0;y<64;y++)for(x=0;x<128;x++){
        int b=bit(px+y*16,x);*n+=(uint32_t)b;if(b!=pr(x,y))return 0;}
    return 1;}
static int guards(const uint8_t *m,size_t n){size_t i;
    for(i=0;i<G;i++){if(m[i]!=0xA5||m[G+n+i]!=0x5A)return 0;}
    return 1;}
static uint8_t *mk(size_t n){uint8_t *m=malloc(n+2*G);
    if(m){memset(m,0xA5,G);memset(m+G,0xCC,n);memset(m+G+n,0x5A,G);}return m;}
static uint32_t fnv(const uint8_t *p,size_t n){uint32_t h=2166136261u;size_t i;
    for(i=0;i<n;i++){h^=p[i];h*=16777619u;}return h;}
static void check(const cs_selected_draw_workspace *w,const cs_selected_draw_result *r){
    uint8_t u[CS_QD_RECT_BYTES];uint32_t a,b,k,n=0;size_t i;
    CK(portok(w->ports[0].pixels,p0,&a)&&a==1280);
    CK(portok(w->ports[1].pixels,p1,&b)&&b==1536);
    for(i=0;i<sizeof u;i++)u[i]=(uint8_t)(w->ports[0].pixels[i]|w->ports[1].pixels[i]);
    for(i=0;i<sizeof u;i++)for(k=0;k<8;k++)n+=(uint32_t)((u[i]>>k)&1);
    CK(n==2816);CK(memcmp(u,w->pixels,sizeof u)==0);
    CK(w->ports[0].paints==1&&w->ports[0].erases==1);
    CK(w->ports[1].paints==1&&w->ports[1].erases==1);
    CK(r->draw.success==1&&r->draw.phase==6&&r->draw.value==97);
    CK(r->draw.black_pixels==n&&r->draw.pattern==fnv(u,sizeof u));
    CK(r->selections==2&&r->current==0x706Cu);}
static void scene(const cs_selected_draw_workspace *w){
    enum{MW=512,MH=342,PW=640,PH=480};
    uint8_t *mono=calloc(64u*MH,1),*arena=calloc(70000,1),*fbm=calloc((size_t)PW*PH*4,1);
    uint8_t *m2=calloc(64u*MH,1);
    cs_surface ms,m2s;cs_arena ar;cs_fb_target t;cs_boot_scene s;cs_boot_report rp;
    cs_product_state st;int x,y,minx=MW,miny=MH,maxx=-1,maxy=-1,pc=0;
    if(!mono||!arena||!fbm||!m2){CK(0);goto done;}
    CK(cs_arena_init(&ar,arena,70000)==CS_ARENA_OK);
    CK(cs_fb_init(&t,(volatile unsigned char*)fbm,(size_t)PW*PH*4,PW,PH,PW,CS_FB_RGB_RESERVED)==CS_FB_OK);
    CK(cs_boot_scene_prepare(&ar,&t,&s)==CS_BOOT_OK);
    CK(s.state.pixels&&memcmp(s.state.pixels,w->pixels,CS_QD_RECT_BYTES)==0);
    st=s.state;st.pixels=w->pixels;
    CK(cs_surface_init(&ms,mono,64u*MH,MW,MH,64,CS_SURFACE_MONO1_MSB)==CS_SURFACE_OK);
    CK(cs_product_scene(&ms,MW,MH,0,&st)==1);
    CK(cs_boot_scene_draw(&s,0,&rp)==CS_BOOT_OK);
    for(y=0;y<MH;y++)for(x=0;x<MW;x++){
        const uint8_t *q=fbm+((size_t)(y+69)*PW+(size_t)(x+64))*4;
        uint8_t want=(uint8_t)(bit(mono+y*64,x)?0u:255u);
        CK(q[0]==want&&q[1]==want&&q[2]==want&&q[3]==0);}
    memcpy(m2,mono,64u*MH);
    CK(cs_surface_init(&m2s,m2,64u*MH,MW,MH,64,CS_SURFACE_MONO1_MSB)==CS_SURFACE_OK);
    CK(cs_product_scene_input(&m2s,7)==1);
    for(y=0;y<MH;y++)for(x=0;x<MW;x++)if(bit(mono+y*64,x)!=bit(m2+y*64,x)){
        CK(x>=352&&x<432&&y>=286&&y<306);
        pc++;if(x<minx)minx=x;if(x>maxx)maxx=x;if(y<miny)miny=y;if(y>maxy)maxy=y;}
    CK(pc>0&&maxx-minx<80&&maxy-miny<20);
    CK(cs_boot_scene_draw_input(&s,0,7,&rp)==CS_BOOT_OK);
    for(y=0;y<MH;y++)for(x=0;x<MW;x++){
        const uint8_t *q=fbm+((size_t)(y+69)*PW+(size_t)(x+64))*4;
        uint8_t want=(uint8_t)(bit(m2+y*64,x)?0u:255u);
        CK(q[0]==want&&q[1]==want&&q[2]==want&&q[3]==0);}
    for(y=0;y<PH;y++)for(x=0;x<PW;x++)if(x<64||x>=576||y<69||y>=411){
        const uint8_t *q=fbm+((size_t)y*PW+(size_t)x)*4;
        CK(q[0]==255&&q[1]==255&&q[2]==255&&q[3]==0);}
done:free(mono);free(m2);free(arena);free(fbm);}
int main(void){
    size_t wn=sizeof(cs_selected_draw_workspace),rn=sizeof(cs_selected_draw_result);
    uint8_t *wm=mk(wn),*rm=mk(rn),*snap;
    cs_selected_draw_workspace *w;cs_selected_draw_result *r;int i;
    if(!wm||!rm)return 2;
    w=(cs_selected_draw_workspace*)(wm+G);r=(cs_selected_draw_result*)(rm+G);
    snap=malloc(wn+rn);if(!snap)return 2;
    memcpy(snap,wm+G,wn);memcpy(snap+wn,rm+G,rn);
    CK(cs_selected_draw(NULL,r)==0&&cs_selected_draw(w,NULL)==0&&cs_selected_draw(NULL,NULL)==0);
    CK(memcmp(snap,wm+G,wn)==0&&memcmp(snap+wn,rm+G,rn)==0);
    for(i=0;i<3;i++){
        memset(wm+G,i==1?0xFF:0x3C,wn);memset(rm+G,i==1?0xFF:0xC3,rn);
        CK(cs_selected_draw(w,r)==1);check(w,r);
        CK(guards(wm,wn)&&guards(rm,rn));}
    scene(w);
    free(snap);free(wm);free(rm);
    puts(bad?"selected-drawing-independent FAIL":"selected-drawing-independent PASS");
    printf("%u checks\n",checks);
    return bad;}
