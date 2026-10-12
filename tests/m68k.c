/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../apps/aline-probe/probe.h"
#define REQUIRE(e) do {if(!(e)){fprintf(stderr,"line %d: %s\n",__LINE__,#e);exit(1);}} while(0)
static cs_trap_gateway gateway(void)
{cs_trap_gateway g;cs_trap_config c={0x400000,0x10000,0x20000,0x400100,0x400120,0x400140};
 REQUIRE(cs_trap_init(&g,&c)==CS_TRAP_OK);return g;}
static cs_trap_frame frame(void)
{cs_trap_frame f;memset(&f,0xCC,sizeof f);for(unsigned i=0;i<8;++i){f.d[i]=0xDEAD0100u+i;f.a[i]=0xCAFE0200u+i;}
 f.pc=0x401000;f.a[7]=0x300010;f.sr=0x271F;return f;}
static void instructions(void)
{
 cs_trap_gateway g=gateway();uint8_t code[4]={0},stack[32]={0};
 cs_m68k_memory m={code,0x401000,4,stack,0x300000,32};cs_m68k_report r;
 for(unsigned reg=0;reg<8;++reg)for(unsigned byte=0;byte<256;++byte){
  uint16_t w=(uint16_t)(28672u+reg*512u+byte);code[0]=(uint8_t)(w>>8);code[1]=(uint8_t)w;
  cs_trap_frame f=frame(),before=f;REQUIRE(cs_m68k_step(&m,&g,&f,&r)==CS_M68K_OK);
  uint32_t expected=byte<128?byte:UINT32_C(0xFFFFFF00)+byte;
  REQUIRE(f.d[reg]==expected && f.pc==before.pc+2 && f.sr==(uint16_t)(0x2710u+(byte==0?4u:(byte>=128?8u:0u))));
  f.d[reg]=before.d[reg];f.pc=before.pc;f.sr=before.sr;REQUIRE(memcmp(&f,&before,sizeof f)==0);
 }
 for(uint32_t w=0;w<65536;++w){
  if((w>=40960 && w<45056) || (w/4096==7 && (w/256)%2==0) || w==20081 || w==20085)continue;
  code[0]=(uint8_t)(w>>8);code[1]=(uint8_t)w;cs_trap_frame f=frame(),before=f;
  REQUIRE(cs_m68k_step(&m,&g,&f,&r)==CS_M68K_OPCODE && r.word==w);
  REQUIRE(memcmp(&f,&before,sizeof f)==0);
 }
 code[0]=0x4E;code[1]=0x71;cs_trap_frame f=frame(),before=f;
 REQUIRE(cs_m68k_step(&m,&g,&f,&r)==CS_M68K_OK && f.pc==before.pc+2);f.pc=before.pc;
 REQUIRE(memcmp(&f,&before,sizeof f)==0);
}
static void calls(void)
{
 cs_trap_gateway g=gateway();cs_m68k_report r;
 for(unsigned word=0xA000;word<=0xAFFF;++word){
  cs_trap_frame f=frame(),before;cs_trap_gateway old=g;f.d[0]=0x46;f.a[0]=0x400100;before=f;
  cs_m68k_result result=cs_m68k_aline(&g,(uint16_t)word,&f,&r);
  REQUIRE(r.word==word && r.pc==before.pc && r.service.pc==before.pc && r.service_called==1);
  unsigned number=(word>=0xA800?word%512:word%256);
  int supported=(word<0xA200 && (number==0x46 || number==0x47));
  REQUIRE(result==(supported?CS_M68K_OK:CS_M68K_SERVICE));
  if(!supported){REQUIRE(memcmp(&f,&before,sizeof f)==0);REQUIRE(memcmp(&g,&old,sizeof g)==0);}
  else{
   REQUIRE(f.pc==before.pc+2 && f.a[7]==before.a[7] && f.sr==0x2710);
   REQUIRE(f.a[0]==((number==0x46 && word>=0xA100)?g.config.get_token:before.a[0]));
  }
 }
 /* Failed Set has an invalid A0 and must not mutate, including instruction PC. */
 cs_trap_frame f=frame(),before=f;cs_trap_gateway old=g;
 REQUIRE(cs_m68k_aline(&g,0xA047,&f,&r)==CS_M68K_SERVICE);
 REQUIRE(memcmp(&f,&before,sizeof f)==0 && memcmp(&g,&old,sizeof g)==0);
}
static void bounds(void)
{
 uint8_t code[4]={0xA1,0x46,0x4E,0x71},stack[32];memset(stack,0xCC,sizeof stack);
 cs_m68k_memory m={code,0x401000,4,stack,0x300000,32};cs_trap_gateway g=gateway();
 cs_m68k_report r;cs_trap_frame f=frame();f.d[0]=0x46;
 for(unsigned n=0;n<12;++n){cs_trap_frame v=f,old;cs_m68k_memory bad=m;cs_trap_gateway saved=g;uint8_t bytes[32];memcpy(bytes,stack,32);
  switch(n){case 0:v.pc++;break;case 1:v.pc=0xFFFFFF;break;case 2:v.pc=0x401004;break;
   case 3:v.a[7]=0x300004;break;case 4:v.a[7]++;break;case 5:v.sr=0x071F;break;
   case 6:v.sr=0xA71F;break;case 7:bad.code_size=3;break;case 8:bad.stack_base=0x401000;break;
   case 9:bad.code_base=0xFFFFFE;bad.code_size=4;break;case 10:bad.code=NULL;break;default:bad.stack=NULL;break;}
  old=v;REQUIRE(cs_m68k_step(&bad,&g,&v,&r)!=CS_M68K_OK);
  REQUIRE(memcmp(&v,&old,sizeof v)==0 && memcmp(&g,&saved,sizeof g)==0 && memcmp(stack,bytes,32)==0);
 }
 REQUIRE(cs_m68k_step(&m,&g,&f,&r)==CS_M68K_OK && f.pc==0x401002);
 REQUIRE(stack[9]==0xCC && stack[10]==0x27 && stack[11]==0x10 && stack[15]==2 && stack[16]==0xCC);
 f=frame();f.d[0]=0x46;g.entries[0x46]=0x10000;uint8_t saved[32];memcpy(saved,stack,32);cs_trap_frame old=f;
 REQUIRE(cs_m68k_step(&m,&g,&f,&r)==CS_M68K_SERVICE && r.service.status==CS_TRAP_NEEDS_EXECUTION);
 REQUIRE(memcmp(&f,&old,sizeof f)==0 && memcmp(stack,saved,32)==0);
 REQUIRE(cs_m68k_step(NULL,&g,&f,&r)==CS_M68K_ARGUMENT);
 REQUIRE(cs_m68k_aline(NULL,0xA146,&f,&r)==CS_M68K_ARGUMENT);
 REQUIRE(cs_m68k_aline(&g,0x4E71,&f,&r)==CS_M68K_OPCODE);
}
static void returns(void)
{
 uint8_t code[4]={0x4E,0x75,0x4E,0x71},stack[32]={0};cs_m68k_memory m={code,0x401000,4,stack,0x300000,32};
 cs_trap_gateway g=gateway();cs_m68k_report r;
 for(unsigned n=0;n<5;++n){cs_trap_frame f=frame(),before;stack[16]=0;stack[17]=0x40;stack[18]=0x10;stack[19]=2;
  if(n==1)stack[19]=3;
  if(n==2)stack[19]=4;
  if(n==3)f.a[7]=0x30001E;
  if(n==4)stack[16]=1;
  before=f;cs_m68k_result result=cs_m68k_step(&m,&g,&f,&r);
  if(n==0){REQUIRE(result==CS_M68K_OK && f.pc==0x401002 && f.a[7]==before.a[7]+4 && f.sr==before.sr);}
  else{REQUIRE(result==CS_M68K_ADDRESS && memcmp(&f,&before,sizeof f)==0);}
 }
}
static void probe(void)
{cs_trap_gateway a=gateway(),b=gateway();cs_aline_probe_result r;REQUIRE(cs_aline_probe(&a,&r));
 REQUIRE(r.checks==8 && r.status==0 && r.signature==0x414C4E31 && r.steps==6 && r.calls==3);
 REQUIRE(a.entries[0x20]==a.config.get_token && b.entries[0x20]==b.config.unsupported_token);
 REQUIRE(cs_aline_probe(NULL,&r)==0 && cs_aline_probe(&a,NULL)==0);}
int main(int argc,char **argv)
{
 if(argc!=2)return 2;
 if(!strcmp(argv[1],"instructions"))instructions();
 else if(!strcmp(argv[1],"calls"))calls();
 else if(!strcmp(argv[1],"bounds"))bounds();
 else if(!strcmp(argv[1],"returns"))returns();
 else if(!strcmp(argv[1],"probe"))probe();
 else return 2;
 return 0;
}
