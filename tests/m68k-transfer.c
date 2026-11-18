/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original SPEC-0123 conformance fixtures. */
#include "../core/cpu/m68k-transfer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned checks;
#define REQUIRE(x) do { ++checks; if(!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); exit(1); } } while(0)
enum { CODE_BYTES=128, DATA_BYTES=256, STACK_BYTES=256 };
typedef struct {
    uint8_t code[CODE_BYTES],data[DATA_BYTES],stack[STACK_BYTES];
    uint32_t code_base,data_base,stack_base,code_size,data_size,stack_size;
    uint32_t fail_read_address;
    unsigned fail_read,reject_bulk,reject_scalar,invalid;
} memory;
typedef struct {
    memory mem;
    cs_guest_region regions[3];cs_guest_map map;cs_trap_gateway gateway;
    cs_trap_frame frame;cs_m68k_bus_report report;cs_m68k_stack_limits limits;
    cs_m68k_transfer_backend transfer;
} fixture;
typedef struct {memory mem;cs_trap_frame frame;cs_trap_gateway gateway;} snapshot;
static void word(uint8_t *p,uint32_t value)
{p[0]=(uint8_t)(value>>8);p[1]=(uint8_t)value;}
static void longword(uint8_t *p,uint32_t value)
{word(p,value>>16);word(p+2,value);}
static uint32_t read_number(const uint8_t *p,unsigned bytes)
{uint32_t n=0;for(unsigned i=0;i<bytes;++i)n=(n<<8)|p[i];return n;}
static uint32_t reg_value(const cs_trap_frame *f,unsigned index)
{return index<8?f->d[index]:f->a[index-8u];}
static void set_reg(cs_trap_frame *f,unsigned index,uint32_t value)
{if(index<8)f->d[index]=value;else f->a[index-8u]=value;}
static uint32_t extend_word(uint32_t n)
{return n<32768u?n:UINT32_C(0xffff0000)|n;}

/* This mock is a scalar guest provider with a separate atomic byte-span store.
 * Failure choices are immutable context fields, not callback logging state. */
static cs_guest_result locate(const memory *m,uint32_t at,uint32_t size,uint32_t access,
    const uint8_t **readable,uint8_t **writable)
{
    uint32_t bases[3]={m->code_base,m->data_base,m->stack_base};
    uint32_t sizes[3]={m->code_size,m->data_size,m->stack_size};
    const uint8_t *backing[3]={m->code,m->data,m->stack};
    if(!size || at>UINT32_C(0xffffff) || size>UINT32_C(0x1000000)-at)return CS_GUEST_ADDRESS;
    for(unsigned i=0;i<3;++i)if(at>=bases[i] && at-bases[i]<=sizes[i] && size<=sizes[i]-(at-bases[i])) {
        uint32_t flags=i==0?CS_GUEST_READ|CS_GUEST_EXECUTE:CS_GUEST_READ|CS_GUEST_WRITE;
        if((flags&access)!=access)return CS_GUEST_PERMISSION;
        if(readable)*readable=backing[i]+(at-bases[i]);
        if(writable)*writable=(uint8_t *)(void *)(backing[i]+(at-bases[i]));
        return CS_GUEST_OK;
    }
    return CS_GUEST_ADDRESS;
}
static cs_guest_result mock_validate(const void *context)
{return ((const memory *)context)->invalid?CS_GUEST_MAP:CS_GUEST_OK;}
static cs_guest_result mock_span(const void *context,uint32_t at,uint32_t bytes,uint32_t access)
{return locate(context,at,bytes,access,NULL,NULL);}
static cs_guest_result mock_read(const void *context,uint32_t at,unsigned bytes,uint32_t access,uint32_t *out)
{
    const memory *m=context;const uint8_t *p=NULL;
    if(bytes!=1 && bytes!=2 && bytes!=4)return CS_GUEST_WIDTH;
    if(bytes!=1 && (at&1u))return CS_GUEST_ADDRESS;
    cs_guest_result result=locate(m,at,bytes,access,&p,NULL);
    if(result!=CS_GUEST_OK)return result;
    if(m->fail_read && access==CS_GUEST_READ && at==m->fail_read_address)return CS_GUEST_PERMISSION;
    *out=read_number(p,bytes);return CS_GUEST_OK;
}
static cs_guest_result mock_write(const void *context,uint32_t at,unsigned bytes,uint32_t value)
{
    const memory *m=context;uint8_t *p=NULL;
    if(m->reject_scalar)return CS_GUEST_PERMISSION;
    if(bytes!=1 && bytes!=2 && bytes!=4)return CS_GUEST_WIDTH;
    if(bytes!=1 && (at&1u))return CS_GUEST_ADDRESS;
    cs_guest_result result=locate(m,at,bytes,CS_GUEST_WRITE,NULL,&p);
    if(result!=CS_GUEST_OK)return result;
    for(unsigned i=0;i<bytes;++i)p[i]=(uint8_t)(value>>(8u*(bytes-1u-i)));
    return CS_GUEST_OK;
}
static cs_guest_result mock_bulk(const void *context,uint32_t at,unsigned bytes,const uint8_t *source)
{
    const memory *m=context;uint8_t *p=NULL;
    if(!source || !bytes || bytes>64u)return CS_GUEST_WIDTH;
    cs_guest_result result=locate(m,at,bytes,CS_GUEST_WRITE,NULL,&p);
    if(result!=CS_GUEST_OK)return result;
    if(m->reject_bulk)return CS_GUEST_PERMISSION;
    memcpy(p,source,bytes);return CS_GUEST_OK;
}
static int permit(const void *context,const cs_m68k_guard_event *event)
{(void)context;(void)event;return 1;}
static const cs_m68k_guard unrestricted={permit,NULL,0};
static void synchronize_map(fixture *f)
{
    f->regions[0]=(cs_guest_region){f->mem.code,NULL,f->mem.code_base,f->mem.code_size,5};
    f->regions[1]=(cs_guest_region){f->mem.data,f->mem.data,f->mem.data_base,f->mem.data_size,3};
    f->regions[2]=(cs_guest_region){f->mem.stack,f->mem.stack,f->mem.stack_base,f->mem.stack_size,3};
}
static void setup(fixture *f)
{
    memset(f,0,sizeof *f);memset(f->mem.data,0xa5,sizeof f->mem.data);memset(f->mem.stack,0x3c,sizeof f->mem.stack);
    for(unsigned i=0;i<CODE_BYTES;i+=2)word(f->mem.code+i,0x4e71);
    f->mem.code_base=0x10000;f->mem.data_base=0x2000;f->mem.stack_base=0x3000;
    f->mem.code_size=CODE_BYTES;f->mem.data_size=DATA_BYTES;f->mem.stack_size=STACK_BYTES;
    synchronize_map(f);f->map=(cs_guest_map){f->regions,3};
    cs_trap_config config={0x400000,0x10000,0x20000,0x400100,0x400120,0x400140};
    REQUIRE(cs_trap_init(&f->gateway,&config)==CS_TRAP_OK);
    for(unsigned i=0;i<8;++i) {
        f->frame.d[i]=UINT32_C(0x80112200)+i*UINT32_C(0x010103);
        f->frame.a[i]=UINT32_C(0xc0345500)+i*UINT32_C(0x010105);
    }
    f->frame.a[0]=0x2000;f->frame.a[7]=0x3080;f->frame.pc=f->mem.code_base;f->frame.sr=0x271f;
    f->limits=(cs_m68k_stack_limits){0x3000,STACK_BYTES};
    f->transfer.scalar=(cs_m68k_bus_backend){&f->mem,sizeof f->mem,mock_validate,mock_span,mock_read,mock_write};
    f->transfer.write_bytes=mock_bulk;
}
static void take(const fixture *f,snapshot *saved)
{memcpy(&saved->mem,&f->mem,sizeof f->mem);memcpy(&saved->frame,&f->frame,sizeof f->frame);memcpy(&saved->gateway,&f->gateway,sizeof f->gateway);}
static cs_m68k_result step(fixture *f,unsigned mode,const cs_m68k_guard *guard)
{
    if(mode==0)return cs_m68k_bus_step_guarded(&f->map,&f->gateway,&f->frame,&f->report,&f->limits,guard);
    if(mode==1)return cs_m68k_bus_step_transfer(&f->transfer,&f->frame,&f->report,&f->limits,guard);
    return cs_m68k_bus_step_backend(&f->transfer.scalar,&f->frame,&f->report,&f->limits,guard);
}
static void exact_state(const fixture *f,const snapshot *expected)
{
    REQUIRE(memcmp(&f->frame,&expected->frame,sizeof f->frame)==0);
    REQUIRE(memcmp(&f->mem,&expected->mem,sizeof f->mem)==0);
    REQUIRE(memcmp(&f->gateway,&expected->gateway,sizeof f->gateway)==0);
    REQUIRE(f->report.cpu.service_called==0 && f->report.subroutine==0);
}
static void success(fixture *f,unsigned mode,const cs_m68k_guard *guard,const snapshot *expected,unsigned taken)
{
    REQUIRE(step(f,mode,guard)==CS_M68K_OK);REQUIRE(f->report.cpu.status==CS_M68K_OK);
    REQUIRE(f->report.branch_taken==taken);exact_state(f,expected);
}
static void refused(fixture *f,unsigned mode,const cs_m68k_guard *guard,cs_m68k_result status)
{
    snapshot saved;take(f,&saved);cs_m68k_result actual=step(f,mode,guard);
    if(actual!=status)fprintf(stderr,"got %u expected %u opcode %02x%02x mode %u\n",(unsigned)actual,(unsigned)status,f->mem.code[0],f->mem.code[1],mode);
    REQUIRE(actual==status);REQUIRE(f->report.cpu.status==status);exact_state(f,&saved);
}
static void instruction(fixture *f,unsigned load,unsigned width,unsigned ea,uint16_t mask)
{word(f->mem.code,0x4880u|(load<<10)|(width==4?0x40u:0u)|ea);word(f->mem.code+2,mask);}

/* The oracle is a truth table for each of the16 CCR combinations (X ignored).
 * Bits in each row correspond to condition codes0..15, with DBF always false. */
static const uint16_t predicates[16]={
    0x5555,0x5569,0xa655,0xa669,0x9599,0x95a9,0xa699,0xa6a9,
    0xa955,0xa969,0x5a55,0x5a69,0xa999,0xa9a9,0x9a99,0x9aa9
};
static void dbcc_matrix(void)
{
    static const uint16_t counters[]={0,1,2,0x7fff,0x8000,0xffff};
    for(unsigned mode=0;mode<3;++mode)for(unsigned ccr=0;ccr<32;++ccr)
        for(unsigned condition=0;condition<16;++condition)for(unsigned reg=0;reg<8;++reg)
            for(unsigned n=0;n<sizeof counters/sizeof counters[0];++n) {
                fixture f;setup(&f);f.frame.pc+=32;f.frame.sr=(uint16_t)(0x2700u|ccr);
                f.frame.d[reg]=UINT32_C(0xa5c30000)|counters[n];
                word(f.mem.code+32,0x50c8u|(condition<<8)|reg);word(f.mem.code+34,0xfffc);
                snapshot expected;take(&f,&expected);
                unsigned truth=(predicates[ccr&15u]>>condition)&1u;
                uint32_t next=((uint32_t)counters[n]-1u)&65535u;
                unsigned branch=!truth && next!=65535u;
                if(!truth)expected.frame.d[reg]=UINT32_C(0xa5c30000)|next;
                expected.frame.pc=branch?f.mem.code_base+30u:f.mem.code_base+36u;
                success(&f,mode,&unrestricted,&expected,branch);
            }
}
typedef struct {cs_m68k_guard_kind kind;uint32_t address,bytes;unsigned match_address,match_bytes;} rule;
static int deny(const void *context,const cs_m68k_guard_event *event)
{
    const rule *r=context;
    return event->kind!=r->kind || (r->match_address && event->address!=r->address) ||
        (r->match_bytes && event->bytes!=r->bytes);
}
static void dbcc_failures(void)
{
    for(unsigned mode=0;mode<3;++mode)for(unsigned which=0;which<8;++which) {
        fixture f;setup(&f);f.frame.d[2]=UINT32_C(0xbeef0002);word(f.mem.code,0x51ca);word(f.mem.code+2,1);
        switch(which) {
        case 0:break; /* Odd taken target. */
        case 1:word(f.mem.code+2,0x7ffe);break; /* Unmapped taken target. */
        case 2:f.mem.code_size=2;synchronize_map(&f);break;
        case 3:f.mem.code_base=0;f.frame.pc=0;word(f.mem.code+2,0xfffc);synchronize_map(&f);break;
        case 4:f.mem.code_base=0xffff80;f.frame.pc=0xfffffc;word(f.mem.code+124,0x51ca);word(f.mem.code+126,4);synchronize_map(&f);break;
        case 5:f.mem.code_base=0xffff80;f.frame.pc=0xfffffc;f.frame.d[2]=UINT32_C(0xbeef0000);word(f.mem.code+124,0x51ca);word(f.mem.code+126,0);synchronize_map(&f);break;
        case 6:f.mem.code_base=0x1800;f.frame.pc=0x1800;word(f.mem.code+2,0x07fe);synchronize_map(&f);break; /* RW, non-EXEC target. */
        default:word(f.mem.code+2,0x8000);break;
        }
        refused(&f,mode,&unrestricted,CS_M68K_ADDRESS);
    }
    static const cs_m68k_guard_kind kinds[]={CS_M68K_GUARD_OPCODE,CS_M68K_GUARD_INSTRUCTION,CS_M68K_GUARD_EXTENSION,CS_M68K_GUARD_TARGET,CS_M68K_GUARD_COMMIT};
    for(unsigned mode=0;mode<3;++mode)for(unsigned i=0;i<sizeof kinds/sizeof kinds[0];++i) {
        fixture f;setup(&f);word(f.mem.code,0x51c9);word(f.mem.code+2,4);f.frame.d[1]=1;
        const rule policy={kinds[i],0,0,0,0};const cs_m68k_guard guard={deny,&policy,sizeof policy};
        refused(&f,mode,&guard,CS_M68K_POLICY);
    }
    for(unsigned mode=0;mode<3;++mode)for(unsigned skip=0;skip<2;++skip) {
        fixture f;setup(&f);word(f.mem.code,skip?0x50c8:0x51c8);word(f.mem.code+2,1);f.frame.d[0]=skip?2u:0u;
        snapshot expected;take(&f,&expected);expected.frame.pc+=4;if(!skip)expected.frame.d[0]=65535;
        const rule policy={CS_M68K_GUARD_TARGET,0,0,0,0};const cs_m68k_guard guard={deny,&policy,sizeof policy};
        success(&f,mode,&guard,&expected,0);
    }
}

static uint32_t load_value(unsigned reg,unsigned width)
{
    static const uint16_t words[16]={0,0x7fff,0x8000,0xffff,0xff,0xff00,0x1234,0xabcd,
                                    0x8001,0x7ffe,0xfffe,0x1357,0xace0,1,0x9000,0x3080};
    if(width==2)return words[reg];
    return reg==15?0x3080u:UINT32_C(0x81234500)+reg*UINT32_C(0x010103);
}
static void put_number(uint8_t *to,unsigned width,uint32_t value)
{if(width==2)word(to,value);else longword(to,value);}
static unsigned count_mask(uint16_t mask)
{unsigned n=0;for(unsigned i=0;i<16;++i)n+=(mask>>i)&1u;return n;}
static void movem_masks(void)
{
    static const uint16_t masks[]={1,0x80,0x100,0x8000,0x0101,0xa55a,0xffff};
    for(unsigned mode=0;mode<3;++mode)for(unsigned load=0;load<2;++load)
        for(unsigned width=2;width<=4;width+=2)for(unsigned m=0;m<sizeof masks/sizeof masks[0];++m) {
            fixture f;setup(&f);uint16_t selected=masks[m];instruction(&f,load,width,0x10,selected);
            unsigned at=0;
            if(load)for(unsigned reg=0;reg<16;++reg)if((selected>>reg)&1u) {
                put_number(f.mem.data+at,width,load_value(reg,width));at+=width;
            }
            snapshot expected;take(&f,&expected);expected.frame.pc+=4;at=0;
            for(unsigned reg=0;reg<16;++reg)if((selected>>reg)&1u) {
                if(load)set_reg(&expected.frame,reg,width==2?extend_word(load_value(reg,width)):load_value(reg,width));
                else put_number(expected.mem.data+at,width,reg_value(&f.frame,reg));
                at+=width;
            }
            if(mode==2 && !load)refused(&f,mode,&unrestricted,CS_M68K_OPCODE);
            else success(&f,mode,&unrestricted,&expected,0);
            REQUIRE(at==count_mask(selected)*width);
        }
    /* Every single mask bit exercises both halves independently. */
    for(unsigned mode=0;mode<2;++mode)for(unsigned width=2;width<=4;width+=2)
        for(unsigned reg=0;reg<16;++reg) {
            fixture f;setup(&f);instruction(&f,1,width,0x10,(uint16_t)(1u<<reg));
            put_number(f.mem.data,width,load_value(reg,width));snapshot expected;take(&f,&expected);
            expected.frame.pc+=4;set_reg(&expected.frame,reg,width==2?extend_word(load_value(reg,width)):load_value(reg,width));
            success(&f,mode,&unrestricted,&expected,0);
        }
}
static void movem_updates(void)
{
    for(unsigned mode=0;mode<3;++mode)for(unsigned width=2;width<=4;width+=2)
        for(unsigned base=0;base<8;++base)for(unsigned load=0;load<2;++load) {
            fixture f;setup(&f);unsigned bytes=16u*width;
            uint8_t *memory_bytes=base==7?f.mem.stack:f.mem.data;
            uint32_t memory_base=base==7?f.mem.stack_base:f.mem.data_base;
            f.frame.a[base]=memory_base+(load?0u:bytes);
            instruction(&f,load,width,(load?24u:32u)+base,0xffff);
            if(load)for(unsigned reg=0;reg<16;++reg) {
                uint32_t value=reg==base+8u?UINT32_MAX:load_value(reg,width);
                put_number(memory_bytes+reg*width,width,value);
            }
            snapshot expected;take(&f,&expected);expected.frame.pc+=4;
            uint8_t *expected_bytes=base==7?expected.mem.stack:expected.mem.data;
            for(unsigned reg=0;reg<16;++reg) {
                if(load && reg!=base+8u)set_reg(&expected.frame,reg,width==2?extend_word(load_value(reg,width)):load_value(reg,width));
                if(!load)put_number(expected_bytes+reg*width,width,reg_value(&f.frame,reg));
            }
            expected.frame.a[base]=memory_base+(load?bytes:0u);
            if(mode==2 && !load)refused(&f,mode,&unrestricted,CS_M68K_OPCODE);
            else success(&f,mode,&unrestricted,&expected,0);
        }
    for(unsigned mode=0;mode<2;++mode) {
        fixture f;setup(&f);f.frame.a[2]=0x2010;instruction(&f,0,4,0x22,0xc081);
        snapshot expected;take(&f,&expected);expected.frame.pc+=4;expected.frame.a[2]=0x2000;
        longword(expected.mem.data,f.frame.d[0]);longword(expected.mem.data+4,f.frame.d[1]);
        longword(expected.mem.data+8,f.frame.a[0]);longword(expected.mem.data+12,f.frame.a[7]);
        success(&f,mode,&unrestricted,&expected,0);
    }
}
static void movem_eas(void)
{
    /* All admitted base68000 control modes, including PC+4 after the mask. */
    static const unsigned eas[]={0x10,0x29,0x32,0x32,0x38,0x39,0x3a,0x3b};
    for(unsigned mode=0;mode<3;++mode)for(unsigned load=0;load<2;++load)
        for(unsigned i=0;i<sizeof eas/sizeof eas[0];++i)for(unsigned width=2;width<=4;width+=2) {
            fixture f;setup(&f);instruction(&f,load,width,eas[i],2);
            unsigned extension_bytes=2;
            switch(i) {
            case 0:extension_bytes=0;break;
            case 1:f.frame.a[1]=0x2004;word(f.mem.code+4,0xfffc);break;
            case 2:f.frame.a[2]=0x200c;f.frame.d[3]=0x8000fff8;word(f.mem.code+4,0x30fc);break;
            case 3:f.frame.a[2]=0x1ffc;f.frame.a[3]=8;word(f.mem.code+4,0xb8fc);break;
            case 4:word(f.mem.code+4,0x2000);break;
            case 5:extension_bytes=4;longword(f.mem.code+4,0x2000);break;
            case 6:f.mem.code_base=0x1800;f.frame.pc=0x1800;word(f.mem.code+4,0x07fc);synchronize_map(&f);break;
            default:f.mem.code_base=0x1800;f.frame.pc=0x1800;f.frame.d[3]=0xdead07fc;word(f.mem.code+4,0x3000);synchronize_map(&f);break;
            }
            put_number(f.mem.data,width,0x89abcdef);
            snapshot expected;take(&f,&expected);expected.frame.pc+=4u+extension_bytes;
            if(load)expected.frame.d[1]=width==2?0xffffcdefu:0x89abcdefu;
            else put_number(expected.mem.data,width,f.frame.d[1]);
            if((!load && i>=6) || (!load && mode==2))refused(&f,mode,&unrestricted,CS_M68K_OPCODE);
            else success(&f,mode,&unrestricted,&expected,0);
        }
}
static void movem_refusals(void)
{
    for(unsigned mode=0;mode<3;++mode)for(unsigned load=0;load<2;++load)
        for(unsigned ea=0;ea<64;++ea) {
            unsigned kind=ea>>3,reg=ea&7u;
            unsigned legal=kind==2 || kind==5 || kind==6 || (kind==7 && reg<2) ||
                (load?(kind==3 || (kind==7 && reg<4)):kind==4);
            if(legal)continue;
            fixture f;setup(&f);instruction(&f,load,4,ea,1);longword(f.mem.code+4,0x2000);
            refused(&f,mode,&unrestricted,CS_M68K_OPCODE);
        }
    for(unsigned mode=0;mode<3;++mode)for(unsigned load=0;load<2;++load)
        for(unsigned bits=1;bits<8;++bits) {
            fixture f;setup(&f);instruction(&f,load,4,0x30,1);word(f.mem.code+4,bits<<8);
            refused(&f,mode,&unrestricted,CS_M68K_OPCODE);
        }
    for(unsigned mode=0;mode<3;++mode)for(unsigned load=0;load<2;++load) {
        fixture f;setup(&f);instruction(&f,load,4,0x10,0);f.frame.a[0]=UINT32_MAX;
        const rule policy={load?CS_M68K_GUARD_READ:CS_M68K_GUARD_WRITE,0,0,0,0};
        const cs_m68k_guard guard={deny,&policy,sizeof policy};refused(&f,mode,&guard,CS_M68K_OPCODE);
    }
    for(unsigned mode=0;mode<2;++mode)for(unsigned load=0;load<2;++load)
        for(unsigned which=0;which<14;++which) {
            fixture f;setup(&f);instruction(&f,load,4,0x10,0xffff);
            for(unsigned i=0;i<16;++i)longword(f.mem.data+i*4u,load_value(i,4));
            switch(which) {
            case 0:f.mem.code_size=2;synchronize_map(&f);break;
            case 1:instruction(&f,load,4,0x28,1);f.mem.code_size=4;synchronize_map(&f);break;
            case 2:instruction(&f,load,4,0x39,1);f.mem.code_size=6;synchronize_map(&f);break;
            case 3:f.frame.a[0]=0x2001;break;
            case 4:f.frame.a[0]=0x20c2;break; /* 62 bytes remain,64 required. */
            case 5:f.frame.a[0]=0xfffffffeu;break;
            case 6:instruction(&f,load,4,0x17,0xffff);f.frame.a[7]=0x3080;f.limits=(cs_m68k_stack_limits){0x3080,62};break;
            case 7:instruction(&f,load,4,load?0x18:0x20,0xffff);f.frame.a[0]=load?UINT32_MAX-31u:31u;break;
            case 8:instruction(&f,load,4,0x28,1);word(f.mem.code+4,0xffff);f.frame.a[0]=0;break;
            case 9:instruction(&f,load,4,0x28,1);word(f.mem.code+4,1);f.frame.a[0]=UINT32_MAX;break;
            case 10:instruction(&f,load,4,0x30,1);word(f.mem.code+4,0x1800);f.frame.d[1]=0x80000000u;break;
            case 11:f.mem.code_base=0xffff80;f.frame.pc=0xfffffc;word(f.mem.code+124,load?0x4cd0:0x48d0);word(f.mem.code+126,0xffff);synchronize_map(&f);break;
            case 12:instruction(&f,load,4,0x17,0xffff);f.frame.a[7]=0x30c2;break;
            default:instruction(&f,load,4,0x10,0xffff);f.frame.a[0]=0xdead2000;break;
            }
            refused(&f,mode,&unrestricted,CS_M68K_ADDRESS);
        }
    for(unsigned mode=0;mode<3;++mode)for(unsigned width=2;width<=4;width+=2)
        for(unsigned odd=0;odd<2;++odd) {
            fixture f;setup(&f);instruction(&f,1,width,0x10,0xffff);
            for(unsigned i=0;i<16;++i)put_number(f.mem.data+i*width,width,load_value(i,width));
            put_number(f.mem.data+15u*width,width,odd?0x3081u:0xffffu);
            refused(&f,mode,&unrestricted,CS_M68K_ADDRESS);
        }
}
static void guards_and_backend(void)
{
    static const cs_m68k_guard_kind common[]={CS_M68K_GUARD_OPCODE,CS_M68K_GUARD_INSTRUCTION,CS_M68K_GUARD_EXTENSION,CS_M68K_GUARD_COMMIT};
    for(unsigned mode=0;mode<2;++mode)for(unsigned load=0;load<2;++load)
        for(unsigned i=0;i<sizeof common/sizeof common[0];++i) {
            fixture f;setup(&f);instruction(&f,load,4,0x10,0xffff);
            for(unsigned reg=0;reg<16;++reg)longword(f.mem.data+reg*4u,load_value(reg,4));
            const rule policy={common[i],0,0,0,0};const cs_m68k_guard guard={deny,&policy,sizeof policy};
            refused(&f,mode,&guard,CS_M68K_POLICY);
        }
    for(unsigned mode=0;mode<2;++mode)for(unsigned load=0;load<2;++load) {
        fixture f;setup(&f);instruction(&f,load,4,0x10,0xffff);
        const rule policy={load?CS_M68K_GUARD_READ:CS_M68K_GUARD_WRITE,0x2000,64,1,1};
        const cs_m68k_guard guard={deny,&policy,sizeof policy};refused(&f,mode,&guard,CS_M68K_POLICY);
    }
    for(unsigned mode=0;mode<3;++mode) {
        fixture f;setup(&f);instruction(&f,1,4,0x18,0xffff);
        for(unsigned reg=0;reg<16;++reg)longword(f.mem.data+reg*4u,load_value(reg,4));
        const rule policy={CS_M68K_GUARD_READ,0x203c,4,1,1};
        const cs_m68k_guard guard={deny,&policy,sizeof policy};refused(&f,mode,&guard,CS_M68K_POLICY);
    }
    /* Full-range admission must precede even the first scalar operand read. */
    for(unsigned mode=0;mode<3;++mode) {
        fixture f;setup(&f);instruction(&f,1,4,0x10,0xffff);
        f.mem.data_size=62;f.mem.fail_read=1;f.mem.fail_read_address=0x2000;synchronize_map(&f);
        const rule policy={CS_M68K_GUARD_READ,0x2000,4,1,1};
        const cs_m68k_guard guard={deny,&policy,sizeof policy};refused(&f,mode,&guard,CS_M68K_ADDRESS);
        REQUIRE(f.report.address==0x2000 && f.report.bytes==64 && f.report.memory==CS_GUEST_ADDRESS);
    }
    /* A7-derived displacement/index accesses cannot leave explicit stack bounds. */
    for(unsigned mode=0;mode<2;++mode)for(unsigned load=0;load<2;++load)for(unsigned indexed=0;indexed<2;++indexed) {
        fixture f;setup(&f);instruction(&f,load,4,indexed?0x37u:0x2fu,1);
        if(indexed) {f.frame.d[1]=0xffffef80;word(f.mem.code+4,0x1800);}
        else word(f.mem.code+4,0xef80);
        refused(&f,mode,&unrestricted,CS_M68K_ADDRESS);
    }
    for(unsigned mode=1;mode<3;++mode) {
        fixture f;setup(&f);instruction(&f,1,4,0x18,0xffff);
        for(unsigned reg=0;reg<16;++reg)longword(f.mem.data+reg*4u,load_value(reg,4));
        f.mem.fail_read=1;f.mem.fail_read_address=0x203c;refused(&f,mode,&unrestricted,CS_M68K_ADDRESS);
        REQUIRE(f.report.address==0x203c && f.report.bytes==4 && f.report.memory==CS_GUEST_PERMISSION);
    }
    fixture f;setup(&f);instruction(&f,0,4,0x20,0xffff);f.frame.a[0]=0x2040;f.mem.reject_bulk=1;
    refused(&f,1,&unrestricted,CS_M68K_ADDRESS);
    REQUIRE(f.report.memory==CS_GUEST_PERMISSION && f.report.address==0x2000 && f.report.bytes==64);
    setup(&f);instruction(&f,0,4,0x10,0xffff);f.mem.reject_scalar=1;
    snapshot expected;take(&f,&expected);expected.frame.pc+=4;
    for(unsigned reg=0;reg<16;++reg)longword(expected.mem.data+reg*4u,reg_value(&f.frame,reg));
    success(&f,1,&unrestricted,&expected,0); /* MOVEM must use one bulk publication. */
    setup(&f);word(f.mem.code,0x2080); /* Existing MOVE.L D0,(A0) still uses scalar write. */
    f.mem.reject_bulk=1;take(&f,&expected);expected.frame.pc+=2;
    expected.frame.sr=(uint16_t)((expected.frame.sr&0xfff0u)|8u);
    longword(expected.mem.data,f.frame.d[0]);success(&f,1,&unrestricted,&expected,0);
    for(unsigned mode=0;mode<2;++mode)for(unsigned load=0;load<2;++load)for(unsigned ccr=0;ccr<32;++ccr) {
        setup(&f);instruction(&f,load,4,0x10,1);f.frame.sr=(uint16_t)(0x2700u|ccr);
        longword(f.mem.data,0);take(&f,&expected);expected.frame.pc+=4;
        if(load)expected.frame.d[0]=0;else longword(expected.mem.data,f.frame.d[0]);
        success(&f,mode,&unrestricted,&expected,0);
    }
}
static void data24_and_arguments(void)
{
    cs_m68k_bus_options options={CS_M68K_DATA24,NULL,NULL};
    for(unsigned load=0;load<2;++load) {
        fixture f;setup(&f);instruction(&f,load,4,load?0x18:0x10,0x101);
        f.frame.a[0]=0xab002000;longword(f.mem.data,0x87654321);longword(f.mem.data+4,0x11223344);
        snapshot expected;take(&f,&expected);expected.frame.pc+=4;
        if(load) {expected.frame.d[0]=0x87654321;expected.frame.a[0]=0xab002008;}
        else {longword(expected.mem.data,f.frame.d[0]);longword(expected.mem.data+4,0xab002000);}
        REQUIRE(cs_m68k_bus_step_options(&f.map,&f.gateway,&f.frame,&f.report,&options)==CS_M68K_OK);
        exact_state(&f,&expected);
    }
    fixture f;setup(&f);instruction(&f,0,4,0x10,0xffff);f.mem.data_base=0xffffe0;f.mem.data_size=32;
    f.frame.a[0]=0xabffffe0;synchronize_map(&f);snapshot saved;take(&f,&saved);
    REQUIRE(cs_m68k_bus_step_options(&f.map,&f.gateway,&f.frame,&f.report,&options)==CS_M68K_ADDRESS);
    exact_state(&f,&saved);
    for(unsigned which=0;which<7;++which) {
        setup(&f);instruction(&f,1,4,0x10,1);take(&f,&saved);
        memset(&f.report,0x5a,sizeof f.report);cs_m68k_bus_report old;memcpy(&old,&f.report,sizeof old);
        cs_m68k_transfer_backend bad=f.transfer;
        if(which==0)bad.write_bytes=NULL;
        if(which==1) {bad.scalar.context=&bad.write_bytes;bad.scalar.context_bytes=sizeof bad.write_bytes;}
        cs_m68k_result result=cs_m68k_bus_step_transfer(which==2?NULL:&bad,which==3?NULL:&f.frame,
            which==4?NULL:&f.report,which==5?NULL:&f.limits,which==6?NULL:&unrestricted);
        REQUIRE(result==CS_M68K_ARGUMENT);REQUIRE(memcmp(&f.report,&old,sizeof old)==0);
        REQUIRE(memcmp(&f.frame,&saved.frame,sizeof f.frame)==0 && memcmp(&f.mem,&saved.mem,sizeof f.mem)==0);
    }
}
/* Malformed native controls must be rejected before any callback or member
 * access. These sentinels fail even if a later alignment check returns ARGUMENT. */
static void unexpected_callback(void)
{fputs("misaligned wrapper reached a callback\n",stderr);exit(1);}
static cs_guest_result never_validate(const void *context)
{(void)context;unexpected_callback();return CS_GUEST_ARGUMENT;}
static cs_guest_result never_span(const void *context,uint32_t at,uint32_t bytes,uint32_t access)
{(void)context;(void)at;(void)bytes;(void)access;unexpected_callback();return CS_GUEST_ARGUMENT;}
static cs_guest_result never_read(const void *context,uint32_t at,unsigned bytes,uint32_t access,uint32_t *out)
{(void)context;(void)at;(void)bytes;(void)access;(void)out;unexpected_callback();return CS_GUEST_ARGUMENT;}
static cs_guest_result never_write(const void *context,uint32_t at,unsigned bytes,uint32_t value)
{(void)context;(void)at;(void)bytes;(void)value;unexpected_callback();return CS_GUEST_ARGUMENT;}
static cs_guest_result never_bulk(const void *context,uint32_t at,unsigned bytes,const uint8_t *source)
{(void)context;(void)at;(void)bytes;(void)source;unexpected_callback();return CS_GUEST_ARGUMENT;}
static int never_guard(const void *context,const cs_m68k_guard_event *event)
{(void)context;(void)event;unexpected_callback();return 0;}
static void misaligned_wrapper(void)
{
    fixture f;setup(&f);instruction(&f,1,4,0x10,1);snapshot saved;take(&f,&saved);
    union {cs_m68k_transfer_backend alignment;uint8_t bytes[sizeof(cs_m68k_transfer_backend)+1];} storage;
    cs_m68k_transfer_backend descriptor=f.transfer;
    descriptor.scalar.validate=never_validate;descriptor.scalar.span=never_span;
    descriptor.scalar.read=never_read;descriptor.scalar.write=never_write;descriptor.write_bytes=never_bulk;
    memset(&storage,0,sizeof storage);memcpy(storage.bytes+1,&descriptor,sizeof descriptor);
    memset(&f.report,0x5a,sizeof f.report);cs_m68k_bus_report old;memcpy(&old,&f.report,sizeof old);
    const cs_m68k_guard guard={never_guard,NULL,0};
    const cs_m68k_transfer_backend *bad=(const cs_m68k_transfer_backend *)(const void *)(storage.bytes+1);
    REQUIRE(cs_m68k_bus_step_transfer(bad,&f.frame,&f.report,&f.limits,&guard)==CS_M68K_ARGUMENT);
    REQUIRE(memcmp(&f.report,&old,sizeof old)==0);
    REQUIRE(memcmp(&f.frame,&saved.frame,sizeof f.frame)==0);
    REQUIRE(memcmp(&f.mem,&saved.mem,sizeof f.mem)==0);
    REQUIRE(memcmp(&f.gateway,&saved.gateway,sizeof f.gateway)==0);
}
int main(void)
{
    dbcc_matrix();dbcc_failures();movem_masks();movem_updates();movem_eas();movem_refusals();
    guards_and_backend();data24_and_arguments();misaligned_wrapper();
    printf("M68K transfer: %u checks passed\n",checks);return 0;
}
