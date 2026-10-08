/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../platform/pc/framebuffer.h"
#include <stdio.h>
#include <string.h>
static unsigned failures;
static unsigned long cases;
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); ++failures; } } while(0)
enum { GUARD=29, FB_BYTES=4096, SRC_BYTES=2048 };
static uint32_t seed=0x2468ACE1u;
static unsigned char next_byte(void)
{
    seed=seed*1103515245u+12345u;
    return (unsigned char)(seed>>16);
}
/* Expected destination from an independent per-pixel placement test. */
static void expect(unsigned char *out,const unsigned char *before,size_t total,uint32_t width,
    uint32_t height,uint32_t pitch,uint32_t format,const unsigned char *src,size_t stride,
    long sw,long sh,long long left,long long top)
{
    memcpy(out,before,total);
    for(uint32_t y=0;y<height;++y) for(uint32_t x=0;x<width;++x) {
        long long sx=(long long)x-left,sy=(long long)y-top;
        if(sx<0 || sy<0 || sx>=sw || sy>=sh) continue;
        const unsigned char *s=src+(size_t)sy*stride+(size_t)sx*4u;
        unsigned char *d=out+GUARD+(size_t)y*pitch*4u+(size_t)x*4u;
        d[0]=format==0?s[0]:s[2]; d[1]=s[1]; d[2]=format==0?s[2]:s[0]; d[3]=0;
    }
}
static void conversion(void)
{
    static const uint32_t widths[]={1,3,7}, heights[]={1,2,5}, extra[]={0,1,3};
    static const long sizes[][2]={{1,1},{2,3},{5,4},{9,6}};
    static const long long places[]={-10,-3,-1,0,1,2,6,INT32_MIN,INT32_MAX,INT32_MAX-1L};
    static unsigned char fb[FB_BYTES],before[FB_BYTES],want[FB_BYTES],srcbuf[SRC_BYTES],srccopy[SRC_BYTES];
    for(unsigned f=0;f<2;++f) for(unsigned w=0;w<3;++w) for(unsigned h=0;h<3;++h) for(unsigned e=0;e<3;++e)
    for(unsigned s=0;s<4;++s) for(unsigned a=0;a<10;++a) for(unsigned b=0;b<10;++b) {
        uint32_t width=widths[w],height=heights[h],pitch=width+extra[e];
        size_t span=(size_t)pitch*4u*height,total=span+2u*GUARD;
        size_t stride=(size_t)sizes[s][0]*4u+(size_t)(s%3u);
        unsigned char *storage=srcbuf+1u+s;   /* deliberately unaligned */
        size_t srclen=stride*(size_t)sizes[s][1];
        cs_fb_target target; cs_surface source;
        for(size_t i=0;i<total;++i) fb[i]=next_byte();
        for(size_t i=0;i<SRC_BYTES;++i) srcbuf[i]=next_byte();
        memcpy(before,fb,total); memcpy(srccopy,srcbuf,SRC_BYTES);
        CHECK(cs_fb_init(&target,fb+GUARD,span,width,height,pitch,f)==CS_FB_OK);
        CHECK(cs_surface_init(&source,storage,srclen,(int32_t)sizes[s][0],(int32_t)sizes[s][1],
            stride,CS_SURFACE_RGBA8)==CS_SURFACE_OK);
        CHECK(cs_fb_present(&target,&source,(int32_t)places[a],(int32_t)places[b])==CS_FB_OK);
        expect(want,before,total,width,height,pitch,f,storage,stride,sizes[s][0],sizes[s][1],
            places[a],places[b]);
        CHECK(memcmp(fb,want,total)==0);
        CHECK(memcmp(srcbuf,srccopy,SRC_BYTES)==0);
        ++cases;
    }
    /* Literal check of the published byte orders, independent of the oracle above. */
    {
        unsigned char rgba[8]={0x11,0x22,0x33,0x44,0xA1,0xB2,0xC3,0xD4},out[8];
        cs_fb_target t; cs_surface s;
        static const unsigned char rgb[8]={0x11,0x22,0x33,0,0xA1,0xB2,0xC3,0};
        static const unsigned char bgr[8]={0x33,0x22,0x11,0,0xC3,0xB2,0xA1,0};
        CHECK(cs_surface_init(&s,rgba,8,2,1,8,CS_SURFACE_RGBA8)==CS_SURFACE_OK);
        memset(out,0xEE,8);
        CHECK(cs_fb_init(&t,out,8,2,1,2,CS_FB_RGB_RESERVED)==CS_FB_OK);
        CHECK(cs_fb_present(&t,&s,0,0)==CS_FB_OK); CHECK(memcmp(out,rgb,8)==0);
        memset(out,0xEE,8);
        CHECK(cs_fb_init(&t,out,8,2,1,2,CS_FB_BGR_RESERVED)==CS_FB_OK);
        CHECK(cs_fb_present(&t,&s,0,0)==CS_FB_OK); CHECK(memcmp(out,bgr,8)==0);
    }
}
static void errors(void)
{
    static unsigned char fb[4*4*3+8],copy[sizeof fb];
    unsigned char pixels[16]={0};
    cs_fb_target t,saved; cs_surface s,bad;
    memset(fb,0x5A,sizeof fb); memcpy(copy,fb,sizeof fb);
    memset(&t,0x77,sizeof t); saved=t;
    CHECK(cs_fb_init(NULL,fb,48,4,3,4,0)==CS_FB_ARGUMENT);
    CHECK(cs_fb_init(&t,NULL,48,4,3,4,7)==CS_FB_ARGUMENT);
    CHECK(cs_fb_init(&t,fb,0,0,0,0,2)==CS_FB_FORMAT);
    CHECK(cs_fb_init(&t,fb,0,0,3,4,1)==CS_FB_DIMENSION);
    CHECK(cs_fb_init(&t,fb,0,4,0,4,1)==CS_FB_DIMENSION);
    CHECK(cs_fb_init(&t,fb,0,8193,1,8193,0)==CS_FB_DIMENSION);
    CHECK(cs_fb_init(&t,fb,0,1,8193,1,0)==CS_FB_DIMENSION);
    CHECK(cs_fb_init(&t,fb,0,4,3,3,0)==CS_FB_DIMENSION);
    CHECK(cs_fb_init(&t,fb,0,4,3,16385,0)==CS_FB_DIMENSION);
    CHECK(cs_fb_init(&t,fb,47,4,3,4,0)==CS_FB_STORAGE);
    CHECK(t.base==saved.base && t.size==saved.size && t.width==saved.width
        && t.height==saved.height && t.pitch==saved.pitch && t.format==saved.format);
    CHECK(cs_fb_init(&t,fb,48,4,3,4,0)==CS_FB_OK);
    CHECK(t.base==fb && t.size==48 && t.width==4 && t.height==3 && t.pitch==4 && t.format==0);
    /* Validation only: the largest profile span is 2^29 bytes and fits every host
       width, so OVERFLOW is unreachable within the profile; nothing is accessed. */
    CHECK(cs_fb_init(&t,fb,(size_t)16384*4u*8192u,8192,8192,16384,1)==CS_FB_OK);
    CHECK(cs_fb_init(&t,fb,(size_t)16384*4u*8192u-1u,8192,8192,16384,1)==CS_FB_STORAGE);
    CHECK(cs_fb_init(&t,fb,48,4,3,4,0)==CS_FB_OK);
    CHECK(cs_surface_init(&s,pixels,16,2,2,8,CS_SURFACE_RGBA8)==CS_SURFACE_OK);
    CHECK(cs_fb_present(NULL,&s,0,0)==CS_FB_ARGUMENT);
    CHECK(cs_fb_present(&t,NULL,0,0)==CS_FB_ARGUMENT);
    saved=t; saved.base=NULL; CHECK(cs_fb_present(&saved,&s,0,0)==CS_FB_ARGUMENT);
    saved=t; saved.format=3; CHECK(cs_fb_present(&saved,&s,0,0)==CS_FB_FORMAT);
    saved=t; saved.pitch=3; CHECK(cs_fb_present(&saved,&s,0,0)==CS_FB_DIMENSION);
    saved=t; saved.size=47; CHECK(cs_fb_present(&saved,&s,0,0)==CS_FB_STORAGE);
    bad=s; bad.format=CS_SURFACE_MONO1_MSB; CHECK(cs_fb_present(&t,&bad,0,0)==CS_FB_SOURCE);
    bad=s; bad.stride=7; CHECK(cs_fb_present(&t,&bad,0,0)==CS_FB_SOURCE);
    bad=s; bad.storage_size=15; CHECK(cs_fb_present(&t,&bad,0,0)==CS_FB_SOURCE);
    bad=s; bad.storage=NULL; CHECK(cs_fb_present(&t,&bad,0,0)==CS_FB_SOURCE);
    bad=s; bad.width=0; CHECK(cs_fb_present(&t,&bad,0,0)==CS_FB_SOURCE);
    /* Target errors precede source errors. */
    saved=t; saved.format=9; CHECK(cs_fb_present(&saved,&bad,0,0)==CS_FB_FORMAT);
    {
        unsigned char mono[4]={0};
        cs_surface m;
        CHECK(cs_surface_init(&m,mono,4,8,2,2,CS_SURFACE_MONO1_MSB)==CS_SURFACE_OK);
        CHECK(cs_fb_present(&t,&m,0,0)==CS_FB_SOURCE);
    }
    CHECK(memcmp(fb,copy,sizeof fb)==0);
    /* Empty/disjoint intersections succeed without writing. */
    CHECK(cs_fb_present(&t,&s,4,0)==CS_FB_OK); CHECK(cs_fb_present(&t,&s,0,3)==CS_FB_OK);
    CHECK(cs_fb_present(&t,&s,-2,0)==CS_FB_OK); CHECK(cs_fb_present(&t,&s,0,-2)==CS_FB_OK);
    CHECK(cs_fb_present(&t,&s,INT32_MAX,INT32_MAX)==CS_FB_OK);
    CHECK(cs_fb_present(&t,&s,INT32_MIN,INT32_MIN)==CS_FB_OK);
    CHECK(memcmp(fb,copy,sizeof fb)==0);
}
static void independent(void)
{
    unsigned char a[4*3*2+4],b[4*3*2+4],src[4*3*2];
    cs_fb_target ta,tb; cs_surface s;
    for(unsigned i=0;i<sizeof src;++i) src[i]=(unsigned char)(i*7u+1u);
    memset(a,0xC1,sizeof a); memset(b,0xC2,sizeof b);
    CHECK(cs_fb_init(&ta,a,24,3,2,3,CS_FB_RGB_RESERVED)==CS_FB_OK);
    CHECK(cs_fb_init(&tb,b,24,3,2,3,CS_FB_BGR_RESERVED)==CS_FB_OK);
    CHECK(cs_surface_init(&s,src,24,3,2,12,CS_SURFACE_RGBA8)==CS_SURFACE_OK);
    CHECK(cs_fb_present(&ta,&s,0,0)==CS_FB_OK);
    for(unsigned i=0;i<24;++i) CHECK(b[i]==0xC2);
    CHECK(cs_fb_present(&tb,&s,0,0)==CS_FB_OK);
    for(unsigned p=0;p<6;++p) {
        CHECK(a[p*4]==src[p*4] && a[p*4+2]==src[p*4+2] && a[p*4+3]==0);
        CHECK(b[p*4]==src[p*4+2] && b[p*4+2]==src[p*4] && b[p*4+3]==0);
        CHECK(a[p*4+1]==src[p*4+1] && b[p*4+1]==src[p*4+1]);
    }
    for(unsigned i=24;i<28;++i) { CHECK(a[i]==0xC1); CHECK(b[i]==0xC2); }
    /* Repeating a presentation is idempotent. */
    {
        unsigned char copy[sizeof a];
        memcpy(copy,a,sizeof a);
        CHECK(cs_fb_present(&ta,&s,0,0)==CS_FB_OK);
        CHECK(memcmp(copy,a,sizeof a)==0);
    }
}
int main(int argc,char **argv)
{
    const char *suite=argc>1?argv[1]:"";
    if(strcmp(suite,"conversion")==0) conversion();
    else if(strcmp(suite,"errors")==0) errors();
    else if(strcmp(suite,"independent")==0) independent();
    else { fprintf(stderr,"unknown suite\n"); return 2; }
    if(cases!=0) printf("native framebuffer %s: %lu cases\n",suite,cases);
    if(failures!=0) { fprintf(stderr,"%u failures\n",failures); return 1; }
    printf("native framebuffer %s: PASS\n",suite);
    return 0;
}
