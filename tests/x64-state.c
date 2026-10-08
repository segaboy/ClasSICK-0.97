/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../platform/pc/x64/state.h"
#include <stdio.h>
#include <string.h>
static unsigned failures;
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); ++failures; } } while(0)
/* Decode bits independently of the implementation's byte emitter. */
static uint64_t bits(const unsigned char *p,unsigned start,unsigned count)
{
    uint64_t value=0;
    for(unsigned i=0;i<count;++i)
        if((p[(start+i)/8]&(1u<<((start+i)%8)))!=0) value|=UINT64_C(1)<<i;
    return value;
}
static cs_x64_layout fixture(void)
{
    cs_x64_layout l={UINT64_C(0x123456780000),UINT64_C(0x234567890000),
        UINT64_C(0x3456789A0000),{UINT64_C(0x456789AB0000),UINT64_C(0x56789ABC0000),
        UINT64_C(0x6789ABCD0000),UINT64_C(0x789ABCDE0000)}};
    return l;
}
static void decode(const unsigned char *p,const cs_x64_layout *l)
{
    unsigned char used[8192]={0};
    const unsigned char code[8]={0xFF,0xFF,0,0,0,0x9B,0xAF,0};
    const unsigned char data[8]={0xFF,0xFF,0,0,0,0x93,0xCF,0};
    CHECK(memcmp(p+8,code,8)==0); CHECK(memcmp(p+16,data,8)==0);
    for(unsigned i=8;i<40;++i) used[i]=1;
    const unsigned char *t=p+24;
    CHECK(bits(t,0,16)==103); CHECK(bits(t,48,4)==0);
    CHECK(bits(t,40,8)==0x89); CHECK(bits(t,52,4)==0);
    uint64_t base=bits(t,16,24)|(bits(t,56,8)<<24)|(bits(t,64,32)<<32);
    CHECK(base==l->state_base+64); CHECK(bits(t,96,32)==0);
    CHECK(bits(p+40,0,16)==39); CHECK(bits(p+42,0,64)==l->state_base);
    CHECK(bits(p+50,0,16)==4095); CHECK(bits(p+52,0,64)==l->state_base+256);
    for(unsigned i=40;i<60;++i) used[i]=1;
    CHECK(bits(p+68,0,64)==l->stack_top);
    for(unsigned i=68;i<76;++i) used[i]=1;
    for(unsigned j=0;j<4;++j) {
        CHECK(bits(p+100+j*8,0,64)==l->ist_top[j]);
        for(unsigned i=100+j*8;i<108+j*8;++i) used[i]=1;
    }
    CHECK(bits(p+166,0,16)==104); used[166]=1; used[167]=1;
    for(unsigned v=0;v<256;++v) {
        const unsigned char *g=p+256+v*16;
        uint64_t address=bits(g,0,16)|(bits(g,48,16)<<16)|(bits(g,64,32)<<32);
        CHECK(address-l->vector_base==v*32u); CHECK(bits(g,16,16)==8);
        unsigned ist=1;
        switch(v) { case 2:ist=2;break;case 8:ist=3;break;case 18:ist=4;break;default:break; }
        CHECK(bits(g,32,3)==ist); CHECK(bits(g,35,5)==0);
        CHECK(bits(g,40,8)==0x8E); CHECK(bits(g,96,32)==0);
        for(unsigned i=0;i<12;++i) used[256+v*16+i]=1;
    }
    for(unsigned i=0;i<8192;++i) if(!used[i]) CHECK(p[i]==0);
}
static void layout(void)
{
    unsigned char buffer[8192+37]; cs_x64_layout l=fixture();
    for(unsigned offset=0;offset<17;++offset) {
        memset(buffer,0xA7,sizeof(buffer));
        CHECK(cs_x64_tables_init(buffer+offset,8192+3,&l)==CS_X64_OK);
        decode(buffer+offset,&l);
        for(unsigned i=0;i<sizeof(buffer);++i)
            if(i<offset || i>=offset+8192) CHECK(buffer[i]==0xA7);
    }
}
static void reject(const cs_x64_layout *l,size_t size,cs_x64_result result)
{
    unsigned char a[8193],b[8193]; memset(a,0xBD,sizeof(a)); memcpy(b,a,sizeof(a));
    CHECK(cs_x64_tables_init(a,size,l)==result); CHECK(memcmp(a,b,sizeof(a))==0);
}
static void errors(void)
{
    cs_x64_layout l=fixture(),bad;
    CHECK(cs_x64_tables_init(NULL,8192,&l)==CS_X64_ARGUMENT);
    reject(NULL,8192,CS_X64_ARGUMENT); reject(&l,8191,CS_X64_LIMIT);
    const uint64_t invalid[]={0,1,UINT64_MAX,UINT64_C(1)<<47,(UINT64_C(1)<<47)-16};
    for(unsigned i=0;i<sizeof(invalid)/sizeof(invalid[0]);++i) {
        bad=l; bad.state_base=invalid[i]; reject(&bad,8192,CS_X64_LIMIT);
        bad=l; bad.vector_base=invalid[i]; reject(&bad,8192,CS_X64_LIMIT);
    }
    for(unsigned stack=0;stack<5;++stack) {
        for(unsigned i=0;i<sizeof(invalid)/sizeof(invalid[0]);++i) {
            /* End exactly 2^47 is a valid stack bound. */
            if(invalid[i]==(UINT64_C(1)<<47)) continue;
            bad=l;
            if(stack==0) bad.stack_top=invalid[i]; else bad.ist_top[stack-1]=invalid[i];
            reject(&bad,8192,CS_X64_LIMIT);
        }
        bad=l;
        if(stack==0) bad.stack_top=65536; else bad.ist_top[stack-1]=4096;
        reject(&bad,8192,CS_X64_LIMIT);
    }
    bad=l; bad.vector_base=l.state_base; reject(&bad,8192,CS_X64_OVERLAP);
    bad=l; bad.stack_top=l.state_base+65536; reject(&bad,8192,CS_X64_OVERLAP);
    for(unsigned i=0;i<4;++i) {
        bad=l; bad.ist_top[i]=l.state_base+4096; reject(&bad,8192,CS_X64_OVERLAP);
        bad=l; bad.ist_top[i]=l.vector_base+8192; reject(&bad,8192,CS_X64_OVERLAP);
        bad=l; bad.ist_top[i]=l.stack_top; reject(&bad,8192,CS_X64_OVERLAP);
        for(unsigned j=0;j<i;++j) {
            bad=l; bad.ist_top[i]=l.ist_top[j]; reject(&bad,8192,CS_X64_OVERLAP);
        }
    }
}
static void matrix(void)
{
    unsigned char p[8192];
    for(unsigned i=0;i<512;++i) {
        cs_x64_layout l=fixture(); uint64_t delta=i*UINT64_C(65536);
        l.state_base+=delta; l.vector_base+=delta; l.stack_top+=delta;
        for(unsigned j=0;j<4;++j) l.ist_top[j]+=delta;
        CHECK(cs_x64_tables_init(p,sizeof(p),&l)==CS_X64_OK); decode(p,&l);
    }
    for(unsigned which=0;which<7;++which) {
        cs_x64_layout l=fixture(); uint64_t end=UINT64_C(1)<<47;
        if(which==0) l.state_base=end-8192;
        else if(which==1) l.vector_base=end-8192;
        else if(which==2) l.stack_top=end;
        else l.ist_top[which-3]=end;
        CHECK(cs_x64_tables_init(p,sizeof(p),&l)==CS_X64_OK); decode(p,&l);
    }
    /* All neighboring ranges may touch without overlap. */
    cs_x64_layout l={4096,12288,86016,{90112,94208,98304,102400}};
    CHECK(cs_x64_tables_init(p,sizeof(p),&l)==CS_X64_OK); decode(p,&l);
}
static void independent(void)
{
    unsigned char a[8192],saved[8192],b[8192]; cs_x64_layout l=fixture(),r=fixture();
    r.state_base+=8192; r.vector_base+=8192;
    CHECK(cs_x64_tables_init(a,sizeof(a),&l)==CS_X64_OK); memcpy(saved,a,sizeof(a));
    CHECK(cs_x64_tables_init(b,sizeof(b),&r)==CS_X64_OK);
    CHECK(memcmp(a,saved,sizeof(a))==0); CHECK(memcmp(a,b,sizeof(a))!=0);
    decode(a,&l); decode(b,&r);
}
int main(int argc,char **argv)
{
    if(argc!=2) return 2;
    if(strcmp(argv[1],"layout")==0) layout();
    else if(strcmp(argv[1],"errors")==0) errors();
    else if(strcmp(argv[1],"matrix")==0) matrix();
    else if(strcmp(argv[1],"independent")==0) independent();
    else return 2;
    if(failures) return 1;
    printf("x64 table bytes %s: PASS (no privileged execution)\n",argv[1]); return 0;
}
