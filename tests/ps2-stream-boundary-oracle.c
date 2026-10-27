/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Lead oracle frozen before worker fix.
 * The wrapper permits legal controller writes during streaming;this oracle
 * tests byte preservation. Other tests enforce the selected command order. */
#include "../platform/pc/ps2-bus.h"
#include "ps2-bus-fixture.h"
#include <stdio.h>
static void send(void *p,uint16_t port,uint8_t v){bus_fixture*f=p;int enable=f->mouse&&port==0x60&&v==0xF4;{unsigned ready=f->ready;f->ready=0;bf_write(p,port,v);f->ready|=ready;}if(enable){pf_byte(&f->key,0x18,0x20);pf_byte(&f->key,0xFF,0x20);pf_byte(&f->key,0x29,0);pf_byte(&f->key,0,0x20);pf_byte(&f->key,0x28,0x20);pf_byte(&f->key,1,0x20);pf_byte(&f->key,0xFF,0x20);}}
int main(void){bus_fixture f;cs_ps2_bus b;cs_input_queue q;cs_input_record data[8];cs_ps2_io io;unsigned i,seen=0,fail=0;uint32_t before;bf_init(&f);io=bf_io(&f);io.write=send;if(cs_input_init(&q,data,8)!=CS_INPUT_OK)return 2;cs_ps2_bus_begin(&b);
for(i=0;i<250;i++){before=b.packets;cs_ps2_bus_poll(&b,&io,(uint64_t)i*1000,&q);if(b.result>CS_PS2_READY){fail++;break;}if(b.packets>before){int x=seen==0?-1:1,y=seen==0?0:-1;if(b.report.dx!=x||b.report.dy!=y||b.report.buttons!=0)fail++;seen++;}if(b.phase==CS_PS2_BUS_READY&&f.key.head==f.key.tail)break;}
if(b.result!=CS_PS2_READY||seen!=2||b.errors||b.keyboard.last_byte!=0x29||f.key.violations)fail++;
printf("stream boundary:ready=%u packets=%u seen=%u aux=%u errors=%u keyboard=%02X violations=%u failures=%u\n",b.result,b.packets,seen,b.auxiliary,b.errors,b.keyboard.last_byte,f.key.violations,fail);return fail?1:0;}
