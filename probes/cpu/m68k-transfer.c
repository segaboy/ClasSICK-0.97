/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "m68k-transfer.h"
#include "../../core/cpu/m68k-transfer.h"
typedef struct {cs_guest_region regions[3];cs_guest_map map;unsigned fail_write;uint32_t fail_read;} memory;
typedef struct {cs_m68k_guard_kind kind;unsigned bytes,deny;} policy;
static uint8_t code[64],data[128],stack[128];
static memory backing;
static policy rules;
static cs_m68k_transfer_backend bus;
static cs_trap_frame frame,before;
static cs_m68k_bus_report report;
static cs_m68k_stack_limits limits;
static cs_m68k_guard guard;
static void fill(void *p,size_t n,uint8_t value){uint8_t *b=p;size_t i;for(i=0;i<n;++i)b[i]=value;}
static int same(const void *p,const void *q,size_t n){const uint8_t *a=p,*b=q;size_t i;for(i=0;i<n;++i)if(a[i]!=b[i])return 0;return 1;}
static int all(const uint8_t *p,size_t n,uint8_t value){size_t i;for(i=0;i<n;++i)if(p[i]!=value)return 0;return 1;}
static void save(void){unsigned i;for(i=0;i<8u;++i){before.d[i]=frame.d[i];before.a[i]=frame.a[i];}before.pc=frame.pc;before.sr=frame.sr;}
static void p16(uint8_t *p,uint16_t value){p[0]=(uint8_t)(value>>8);p[1]=(uint8_t)value;}
static void p32(uint8_t *p,uint32_t value){p16(p,(uint16_t)(value>>16));p16(p+2,(uint16_t)value);}
static uint32_t g32(const uint8_t *p){return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|((uint32_t)p[2]<<8)|p[3];}
static cs_guest_result valid(const void *context){const memory *m=context;return cs_guest_validate(&m->map);}
static cs_guest_result span(const void *context,uint32_t at,uint32_t n,uint32_t access)
{const memory *m=context;cs_guest_view v;return cs_guest_window(&m->map,at,n,access,&v);}
static cs_guest_result read_scalar(const void *context,uint32_t at,unsigned width,uint32_t access,uint32_t *out)
{const memory *m=context;if(at==m->fail_read)return CS_GUEST_PERMISSION;return cs_guest_read(&m->map,at,width,access,out);}
static cs_guest_result write_scalar(const void *context,uint32_t at,unsigned width,uint32_t value)
{const memory *m=context;return cs_guest_write(&m->map,at,width,value);}
static cs_guest_result write_bytes(const void *context,uint32_t at,unsigned n,const uint8_t *source)
{
    const memory *m=context;cs_guest_view v;unsigned i;
    cs_guest_result result=cs_guest_window(&m->map,at,n,CS_GUEST_WRITE,&v);
    if(result!=CS_GUEST_OK)return result;
    if(m->fail_write)return CS_GUEST_PERMISSION;
    for(i=0;i<n;++i)v.writable[i]=source[i];
    return CS_GUEST_OK;
}
static int permit(const void *context,const cs_m68k_guard_event *event)
{const policy *p=context;return !p->deny || event->kind!=p->kind || event->bytes!=p->bytes;}
static void setup(void)
{
    unsigned i;fill(&frame,sizeof frame,0);fill(code,sizeof code,0);fill(data,sizeof data,0x6d);fill(stack,sizeof stack,0xa5);
    backing.regions[0].bytes=code;backing.regions[0].writable=0;backing.regions[0].base=0x401000;backing.regions[0].size=64;backing.regions[0].flags=5;
    backing.regions[1].bytes=data;backing.regions[1].writable=data;backing.regions[1].base=0x2000;backing.regions[1].size=128;backing.regions[1].flags=3;
    backing.regions[2].bytes=stack;backing.regions[2].writable=stack;backing.regions[2].base=0x3000;backing.regions[2].size=128;backing.regions[2].flags=3;
    backing.map.regions=backing.regions;backing.map.count=3;backing.fail_read=UINT32_MAX;backing.fail_write=0;
    bus.scalar.context=&backing;bus.scalar.context_bytes=sizeof backing;bus.scalar.validate=valid;bus.scalar.span=span;
    bus.scalar.read=read_scalar;bus.scalar.write=write_scalar;bus.write_bytes=write_bytes;
    for(i=0;i<8u;++i){frame.d[i]=UINT32_C(0x81000000)+i;frame.a[i]=UINT32_C(0x52000000)+i;}
    frame.pc=0x401000;frame.a[7]=0x3080;frame.sr=0x271f;limits.base=0x3000;limits.size=128;
    rules.deny=0;rules.kind=CS_M68K_GUARD_COMMIT;rules.bytes=0;
    guard.check=permit;guard.context=&rules;guard.context_bytes=sizeof rules;
}
static cs_m68k_result step(void){return cs_m68k_bus_step_transfer(&bus,&frame,&report,&limits,&guard);}
unsigned cs_m68k_transfer_probe(void)
{
    unsigned i;setup();p16(code,0x51c8);p16(code+2,0xfffe);frame.d[0]=UINT32_C(0x12340002);
    for(i=0;i<3u;++i) {
        if(step()!=CS_M68K_OK || frame.d[0]!=(i==2u?UINT32_C(0x1234ffff):UINT32_C(0x12340001)-i) ||
            frame.pc!=(i==2u?UINT32_C(0x401004):UINT32_C(0x401000)) || frame.sr!=0x271f)return 0xe1;
    }
    setup();p16(code,0x48e7);p16(code+2,0xffff);save();
    if(step()!=CS_M68K_OK || frame.a[7]!=0x3040 || frame.sr!=before.sr)return 0xe2;
    for(i=0;i<16u;++i)if(g32(stack+64u+i*4u)!=(i<8u?before.d[i]:before.a[i-8u]))return i==15u?0xe3u:0xe2u;
    p16(code+4,0x4cdf);p16(code+6,0xffff);
    for(i=0;i<8u;++i){frame.d[i]=0; if(i<7u)frame.a[i]=0;}
    if(step()!=CS_M68K_OK || frame.a[7]!=0x3080 || frame.pc!=0x401008)return 0xe2;
    for(i=0;i<8u;++i)if(frame.d[i]!=before.d[i] || frame.a[i]!=before.a[i])return 0xe2;
    setup();p16(code,0x48e3);p16(code+2,0x0010);frame.a[3]=0x2050;
    if(step()!=CS_M68K_OK || frame.a[3]!=0x204c || g32(data+76)!=0x2050)return 0xe3;
    setup();p16(code,0x4c99);p16(code+2,0x0204);p16(data,0x8001);p16(data+2,0x7fff);frame.a[1]=0x2000;
    if(step()!=CS_M68K_OK || frame.d[2]!=UINT32_C(0xffff8001) || frame.a[1]!=0x2004 || frame.sr!=0x271f)return 0xe4;
    setup();p16(code,0x4cdf);p16(code+2,0x8000);p32(stack+32,UINT32_MAX);frame.a[7]=0x3020;
    if(step()!=CS_M68K_OK || frame.a[7]!=0x3024)return 0xe4;
    setup();p16(code,0x48e7);p16(code+2,0xffff);backing.fail_write=1;save();
    if(step()!=CS_M68K_ADDRESS || report.memory!=CS_GUEST_PERMISSION || report.bytes!=64u ||
        !same(&frame,&before,sizeof frame) || !all(stack,sizeof stack,0xa5))return 0xe5;
    backing.fail_write=0;rules.deny=1;rules.kind=CS_M68K_GUARD_WRITE;rules.bytes=64;
    if(step()!=CS_M68K_POLICY || !same(&frame,&before,sizeof frame) || !all(stack,sizeof stack,0xa5))return 0xe6;
    rules.kind=CS_M68K_GUARD_COMMIT;rules.bytes=0;
    if(step()!=CS_M68K_POLICY || !same(&frame,&before,sizeof frame) || !all(stack,sizeof stack,0xa5))return 0xe6;
    setup();p16(code,0x4cd0);p16(code+2,0xffff);frame.a[0]=0x2000;backing.fail_read=0x203c;save();
    if(step()!=CS_M68K_ADDRESS || !same(&frame,&before,sizeof frame))return 0xe7;
    setup();p16(code,0x48e7);p16(code+2,0);save();
    if(step()!=CS_M68K_OPCODE || !same(&frame,&before,sizeof frame) || !all(stack,sizeof stack,0xa5))return 0xe8;
    return 0xba;
}
