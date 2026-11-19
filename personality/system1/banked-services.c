/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "banked-services.h"

typedef struct {uintptr_t at;size_t bytes;} native_span;
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define CS_BANK_SERVICE_ALIGNMENT(type) _Alignof(type)
#else
#define CS_BANK_SERVICE_TYPE(type) \
    typedef struct {unsigned char prefix;type value;} cs_bank_service_align_##type; \
    typedef char cs_bank_service_check_##type[(offsetof(cs_bank_service_align_##type,value)!=0 && \
        (offsetof(cs_bank_service_align_##type,value)&(offsetof(cs_bank_service_align_##type,value)-1u))==0)?1:-1]
CS_BANK_SERVICE_TYPE(cs_m68k_transfer_backend);
CS_BANK_SERVICE_TYPE(cs_trap_gateway);
CS_BANK_SERVICE_TYPE(cs_trap_frame);
CS_BANK_SERVICE_TYPE(cs_m68k_bus_report);
CS_BANK_SERVICE_TYPE(cs_m68k_stack_limits);
CS_BANK_SERVICE_TYPE(cs_m68k_guard);
CS_BANK_SERVICE_TYPE(cs_s1_patch_state);
#define CS_BANK_SERVICE_ALIGNMENT(type) offsetof(cs_bank_service_align_##type,value)
#endif
static int span(native_span *out,const void *object,size_t bytes)
{
    uintptr_t at=(uintptr_t)object;
    if(object==0 || bytes==0 || bytes>UINTPTR_MAX-at)return 0;
    out->at=at;out->bytes=bytes;return 1;
}
static int envelope(const cs_m68k_transfer_backend *b,const cs_trap_gateway *gateway,
    const cs_trap_frame *frame,const cs_m68k_bus_report *report,
    const cs_m68k_stack_limits *limits,const cs_m68k_guard *guard,const cs_s1_patch_state *patch)
{
    native_span objects[9];size_t count=7;
    if(!span(&objects[0],b,sizeof *b) || !span(&objects[1],gateway,sizeof *gateway) ||
        !span(&objects[2],frame,sizeof *frame) || !span(&objects[3],report,sizeof *report) ||
        !span(&objects[4],limits,sizeof *limits) || !span(&objects[5],guard,sizeof *guard))return 0;
    if(((uintptr_t)b&(CS_BANK_SERVICE_ALIGNMENT(cs_m68k_transfer_backend)-1u))!=0 ||
        ((uintptr_t)gateway&(CS_BANK_SERVICE_ALIGNMENT(cs_trap_gateway)-1u))!=0 ||
        ((uintptr_t)frame&(CS_BANK_SERVICE_ALIGNMENT(cs_trap_frame)-1u))!=0 ||
        ((uintptr_t)report&(CS_BANK_SERVICE_ALIGNMENT(cs_m68k_bus_report)-1u))!=0 ||
        ((uintptr_t)limits&(CS_BANK_SERVICE_ALIGNMENT(cs_m68k_stack_limits)-1u))!=0 ||
        ((uintptr_t)guard&(CS_BANK_SERVICE_ALIGNMENT(cs_m68k_guard)-1u))!=0)return 0;
    if(b->scalar.validate==0 || b->scalar.span==0 || b->scalar.read==0 || b->scalar.write==0 ||
        b->write_bytes==0 || !span(&objects[6],b->scalar.context,b->scalar.context_bytes) ||
        guard->check==0 || ((guard->context==0)!=(guard->context_bytes==0)))return 0;
    if(guard->context!=0 && !span(&objects[count++],guard->context,guard->context_bytes))return 0;
    if(patch!=0 && (((uintptr_t)patch&(CS_BANK_SERVICE_ALIGNMENT(cs_s1_patch_state)-1u))!=0 ||
        !span(&objects[count++],patch,sizeof *patch)))return 0;
    for(size_t i=0;i<count;++i)for(size_t j=0;j<i;++j)
        if(objects[i].at<objects[j].at+objects[j].bytes && objects[j].at<objects[i].at+objects[i].bytes)return 0;
    return 1;
}
static void copy_frame(cs_trap_frame *to,const cs_trap_frame *from)
{
    uint8_t *d=(uint8_t *)to;const uint8_t *s=(const uint8_t *)from;
    for(size_t i=0;i<sizeof *to;++i)d[i]=s[i];
}
static void begin(cs_m68k_bus_report *r,const cs_trap_frame *f)
{
    uint8_t *bytes=(uint8_t *)r;
    for(size_t i=0;i<sizeof *r;++i)bytes[i]=0;
    r->cpu.status=CS_M68K_OK;r->cpu.pc=f->pc;r->cpu.service.status=CS_TRAP_OK;
    r->cpu.service.pc=f->pc;r->memory=CS_GUEST_OK;
}
static cs_m68k_result done(cs_m68k_bus_report *r,cs_m68k_result code)
{r->cpu.status=code;return code;}
static int permit(const cs_m68k_guard *guard,cs_m68k_guard_kind kind,
    const cs_trap_frame *before,const cs_trap_frame *proposed,uint16_t word,
    uint32_t address,uint32_t bytes,uint32_t access)
{
    cs_m68k_guard_event event;
    event.kind=kind;event.address=address;event.bytes=bytes;event.access=access;
    event.word=word;event.before=before;event.proposed=proposed;
    return guard->check(guard->context,&event)!=0;
}
static cs_m68k_result patch_read(const cs_m68k_bus_backend *b,cs_m68k_bus_report *report,
    const cs_m68k_guard *guard,const cs_trap_frame *frame,const cs_trap_frame *proposed,
    cs_m68k_guard_kind kind,uint32_t address,unsigned bytes,unsigned access,uint32_t *value)
{
    report->address=address;report->bytes=bytes;report->access=access;
    if(!permit(guard,kind,frame,proposed,report->cpu.word,address,bytes,access))
        return done(report,CS_M68K_POLICY);
    report->memory=b->read(b->context,address,bytes,access,value);
    return report->memory==CS_GUEST_OK?CS_M68K_OK:done(report,CS_M68K_ADDRESS);
}
static cs_m68k_result enter_patch(const cs_m68k_transfer_backend *backend,const cs_m68k_guard *guard,
    cs_trap_frame *frame,cs_trap_frame *proposed,cs_m68k_bus_report *report,
    uint32_t base,cs_s1_patch_state *state,uint32_t target)
{
    uint32_t next=frame->pc+2,unused,slot;
    if(state->depth==CS_S1_PATCH_DEPTH)return done(report,CS_M68K_POLICY);
    if(frame->a[7]-base<6u || frame->a[7]<0x404u)return done(report,CS_M68K_ADDRESS);
    slot=frame->a[7]-4;
    if((slot<=target && target<slot+4) || (slot<=next && next<slot+4))
        return done(report,CS_M68K_POLICY);
    cs_m68k_result status=patch_read(&backend->scalar,report,guard,frame,proposed,
        CS_M68K_GUARD_TARGET,target,2,CS_GUEST_READ|CS_GUEST_EXECUTE,&unused);
    if(status!=CS_M68K_OK)return status;
    report->address=slot;report->bytes=4;report->access=CS_GUEST_WRITE;
    if(!permit(guard,CS_M68K_GUARD_WRITE,frame,proposed,report->cpu.word,slot,4,CS_GUEST_WRITE))
        return done(report,CS_M68K_POLICY);
    report->memory=backend->scalar.span(backend->scalar.context,slot,4,CS_GUEST_WRITE);
    if(report->memory!=CS_GUEST_OK)return done(report,CS_M68K_ADDRESS);
    proposed->pc=target;proposed->a[7]=slot;
    if(!permit(guard,CS_M68K_GUARD_COMMIT,frame,proposed,report->cpu.word,target,0,0))
        return done(report,CS_M68K_POLICY);
    uint8_t bytes[4];
    for(unsigned i=0;i<4u;++i)bytes[i]=(uint8_t)(next>>((3u-i)*8u));
    report->memory=backend->write_bytes(backend->scalar.context,slot,4,bytes);
    if(report->memory!=CS_GUEST_OK)return done(report,CS_M68K_ADDRESS);
    cs_s1_patch_call *call=&state->calls[state->depth++];
    call->slot=slot;call->continuation=next;call->word=report->cpu.word;
    call->a0=frame->a[0];call->a1=frame->a[1];call->d1=frame->d[1];call->d2=frame->d[2];
    report->branch_taken=1;report->subroutine=1;
    copy_frame(frame,proposed);
    return done(report,CS_M68K_OK);
}
static cs_m68k_result leave_patch(const cs_m68k_bus_backend *b,const cs_m68k_guard *guard,
    cs_trap_frame *frame,cs_trap_frame *proposed,cs_m68k_bus_report *report,cs_s1_patch_state *state)
{
    const cs_s1_patch_call *call=&state->calls[state->depth-1u];
    uint32_t next,unused;
    cs_m68k_result status=patch_read(b,report,guard,frame,proposed,CS_M68K_GUARD_READ,
        call->slot,4,CS_GUEST_READ,&next);
    if(status!=CS_M68K_OK)return status;
    if(next!=call->continuation)return done(report,CS_M68K_POLICY);
    status=patch_read(b,report,guard,frame,proposed,CS_M68K_GUARD_TARGET,next,2,
        CS_GUEST_READ|CS_GUEST_EXECUTE,&unused);
    if(status!=CS_M68K_OK)return status;
    proposed->pc=next;proposed->a[7]+=4;
    proposed->d[1]=call->d1;proposed->d[2]=call->d2;proposed->a[1]=call->a1;
    if(!(call->word&0x0100u))proposed->a[0]=call->a0;
    uint16_t low=(uint16_t)frame->d[0];
    proposed->sr=(uint16_t)((frame->sr&0xfff0u)|(low==0?4u:((low&0x8000u)?8u:0u)));
    if(!permit(guard,CS_M68K_GUARD_COMMIT,frame,proposed,report->cpu.word,next,0,0))
        return done(report,CS_M68K_POLICY);
    --state->depth;
    copy_frame(frame,proposed);
    return done(report,CS_M68K_OK);
}
static cs_m68k_result banked_step(const cs_m68k_transfer_backend *backend,
    const cs_trap_gateway *gateway,cs_trap_gateway *publish_gateway,cs_trap_frame *frame,cs_m68k_bus_report *report,
    const cs_m68k_stack_limits *limits,const cs_m68k_guard *guard,cs_s1_patch_state *patch)
{
    if(!envelope(backend,gateway,frame,report,limits,guard,patch))return CS_M68K_ARGUMENT;
    if(patch!=0 && patch->depth>CS_S1_PATCH_DEPTH)return CS_M68K_ARGUMENT;
    const cs_m68k_bus_backend *b=&backend->scalar;
    begin(report,frame);report->memory=b->validate(b->context);
    if(report->memory!=CS_GUEST_OK)return done(report,CS_M68K_ADDRESS);
    if(limits->size==0 || ((limits->base|limits->size)&1u)!=0 ||
        limits->base>=0x1000000UL || limits->size>0x1000000UL-limits->base ||
        (frame->a[7]&1u)!=0 || frame->a[7]<limits->base || frame->a[7]>limits->base+limits->size){
        report->memory=CS_GUEST_ADDRESS;return done(report,CS_M68K_ADDRESS);
    }
    report->memory=b->span(b->context,limits->base,limits->size,CS_GUEST_READ|CS_GUEST_WRITE);
    if(report->memory!=CS_GUEST_OK)return done(report,CS_M68K_ADDRESS);
    if((frame->pc&1u)!=0 || frame->pc>0xfffffcUL)return done(report,CS_M68K_ADDRESS);
    if((frame->sr&0xa700u)!=0x2700u)return done(report,CS_M68K_MODE);
    report->address=frame->pc;report->bytes=2;report->access=CS_GUEST_READ|CS_GUEST_EXECUTE;
    if(!permit(guard,CS_M68K_GUARD_OPCODE,frame,frame,0,frame->pc,2,5))return done(report,CS_M68K_POLICY);
    uint32_t fetched;
    report->memory=b->read(b->context,frame->pc,2,5,&fetched);
    if(report->memory!=CS_GUEST_OK)return done(report,CS_M68K_ADDRESS);
    uint16_t word=(uint16_t)fetched;report->cpu.word=word;
    cs_m68k_stack_limits active;
    int returning=0;
    if(patch!=0 && patch->depth!=0){
        const cs_s1_patch_call *call=&patch->calls[patch->depth-1u];
        if((call->slot&1u) || call->slot<=limits->base || limits->size<4 ||
            call->slot-limits->base>limits->size-4 || (call->continuation&1u) ||
            call->continuation>0xfffffeUL || (call->word&0xfe00u)!=0xa000u ||
            frame->a[7]>call->slot)return done(report,CS_M68K_POLICY);
        active.base=limits->base;active.size=call->slot-limits->base;
        limits=&active;
        returning=word==0x4e75u && frame->a[7]==call->slot;
    }
    if((word&0xf000u)!=0xa000u && !returning)
        return cs_m68k_bus_step_transfer(backend,frame,report,limits,guard);
    cs_trap_frame proposed;copy_frame(&proposed,frame);
    if(!permit(guard,CS_M68K_GUARD_INSTRUCTION,frame,&proposed,word,frame->pc,2,5))
        return done(report,CS_M68K_POLICY);
    if(returning)return leave_patch(b,guard,frame,&proposed,report,patch);
    (void)cs_trap_decode(word,&report->cpu.service.decoded);
    if(patch!=0 && (word&0xfe00u)==0xa000u){
        uint32_t installed;
        cs_trap_result lookup=cs_trap_get(gateway,(uint16_t)(word&0xffu),&installed);
        if(lookup!=CS_TRAP_OK){report->cpu.service.status=lookup;return done(report,CS_M68K_SERVICE);}
        report->cpu.service.target=installed;
        if(installed>=gateway->config.ram_begin && installed<gateway->config.ram_end)
            return enter_patch(backend,guard,frame,&proposed,report,limits->base,patch,installed);
    }
    int setting=word==0xa047u && publish_gateway!=0;
    if(word!=0xa146u && word!=0xa046u && !setting){
        report->cpu.service.status=CS_TRAP_UNSUPPORTED_FORMAT;return done(report,CS_M68K_POLICY);
    }
    if(frame->a[7]<6 || frame->a[7]-6<limits->base || frame->a[7]-6<0x400UL){
        report->memory=CS_GUEST_ADDRESS;return done(report,CS_M68K_ADDRESS);
    }
    uint32_t scratch=frame->a[7]-6;
    report->address=scratch;report->bytes=6;report->access=CS_GUEST_WRITE;
    if(!permit(guard,CS_M68K_GUARD_WRITE,frame,&proposed,word,scratch,6,CS_GUEST_WRITE))
        return done(report,CS_M68K_POLICY);
    report->memory=b->span(b->context,scratch,6,CS_GUEST_WRITE);
    if(report->memory!=CS_GUEST_OK)return done(report,CS_M68K_ADDRESS);
    uint32_t target;
    cs_trap_result service=cs_trap_get(gateway,(uint16_t)(setting?CS_TRAP_SET:CS_TRAP_GET),&target);
    if(service!=CS_TRAP_OK){report->cpu.service.status=service;return done(report,CS_M68K_SERVICE);}
    report->cpu.service.target=target;
    if(setting){
        if(target!=gateway->config.set_token){
            report->cpu.service.status=CS_TRAP_UNSUPPORTED_FORMAT;return done(report,CS_M68K_POLICY);
        }
    }else{
        if(target!=gateway->config.get_token){
            report->cpu.service.status=CS_TRAP_UNSUPPORTED_FORMAT;return done(report,CS_M68K_POLICY);
        }
    }
    report->cpu.service_called=1;
    uint16_t set_index=(uint16_t)frame->d[0];uint32_t set_address=frame->a[0],value=0;
    service=setting?cs_trap_check_set(gateway,set_index,set_address):cs_trap_get(gateway,(uint16_t)frame->d[0],&value);
    report->cpu.service.status=service;
    if(service!=CS_TRAP_OK)return done(report,CS_M68K_SERVICE);
    if(!setting && (word&0x0100u)!=0)proposed.a[0]=value;
    uint16_t low=(uint16_t)frame->d[0];
    proposed.sr=(uint16_t)((frame->sr&0xfff0u)|(low==0?4u:((low&0x8000u)?8u:0u)));
    proposed.pc=frame->pc+2;
    if(proposed.pc>0xfffffeUL || (proposed.pc&1u)!=0)return done(report,CS_M68K_ADDRESS);
    if(!permit(guard,CS_M68K_GUARD_COMMIT,frame,&proposed,word,proposed.pc,0,0))
        return done(report,CS_M68K_POLICY);
    uint8_t bytes[6];bytes[0]=(uint8_t)(proposed.sr>>8);bytes[1]=(uint8_t)proposed.sr;
    for(unsigned i=0;i<4u;++i)bytes[i+2u]=(uint8_t)(proposed.pc>>((3u-i)*8u));
    report->memory=backend->write_bytes(b->context,scratch,6,bytes);
    if(report->memory!=CS_GUEST_OK)return done(report,CS_M68K_ADDRESS);
    if(setting)publish_gateway->entries[set_index]=set_address;
    copy_frame(frame,&proposed);return done(report,CS_M68K_OK);
}
cs_m68k_result cs_s1_banked_step(const cs_m68k_transfer_backend *backend,
    const cs_trap_gateway *gateway,cs_trap_frame *frame,cs_m68k_bus_report *report,
    const cs_m68k_stack_limits *limits,const cs_m68k_guard *guard)
{return banked_step(backend,gateway,0,frame,report,limits,guard,0);}
cs_m68k_result cs_s1_banked_trap_step(const cs_m68k_transfer_backend *backend,
    cs_trap_gateway *gateway,cs_trap_frame *frame,cs_m68k_bus_report *report,
    const cs_m68k_stack_limits *limits,const cs_m68k_guard *guard)
{return banked_step(backend,gateway,gateway,frame,report,limits,guard,0);}
cs_m68k_result cs_s1_banked_patch_step(const cs_m68k_transfer_backend *backend,
    cs_trap_gateway *gateway,cs_trap_frame *frame,cs_m68k_bus_report *report,
    const cs_m68k_stack_limits *limits,const cs_m68k_guard *guard,cs_s1_patch_state *state)
{
    if(state==0)return CS_M68K_ARGUMENT;
    return banked_step(backend,gateway,gateway,frame,report,limits,guard,state);
}
