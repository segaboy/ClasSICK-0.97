/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "toolbox.h"
#define TB_TAG 0x54425031u
static unsigned width(cs_tb_type type)
{return type==CS_TB_NONE?0u:(type==CS_TB_WORD || type==CS_TB_BOOL || type==CS_TB_CHAR?2u:4u);}
static int overlap(uint32_t a,uint32_t n,uint32_t b,uint32_t m)
{return n!=0 && m!=0 && (a<=b?b-a<n:a-b<m);}
static int inside(uint32_t base,uint32_t size,uint32_t at,uint32_t bytes)
{return bytes<=size && at>=base && at-base<=size-bytes;}
static int readonly(const cs_guest_map *map,uint32_t at)
{
    for(size_t i=0;i<map->count;++i)
        if(map->regions[i].flags==5 && inside(map->regions[i].base,map->regions[i].size,at,2)) return 1;
    return 0;
}
static int argument_valid(const cs_tb_argument *a)
{
    if(a->type<CS_TB_WORD || a->type>CS_TB_STRING || a->kept>1) return 0;
    if(a->type<CS_TB_SPAN) return a->bytes==0 && a->access==0 && a->alignment==0 && a->kept==0;
    if(a->type==CS_TB_STRING) return a->bytes==0 && a->access==1 && a->alignment==1;
    return a->bytes!=0 && a->bytes<=256 && a->access!=0 && a->access<=3 &&
        (a->alignment==1 || a->alignment==2) && ((a->access&2u)==0 || a->bytes<=CS_TB_OUTPUT_BYTES);
}
static int records_valid(const cs_m68k_session *s,const cs_tb_record *records,size_t count)
{
    if(s==0 || s->calls.map==0 || s->calls.gateway==0 || records==0 || count==0 || count>CS_TB_RECORDS ||
        cs_guest_validate(s->calls.map)!=CS_GUEST_OK) return 0;
    const cs_m68k_call_config *config=&s->calls.config;cs_guest_view stack;
    if(config->stack_base<0x400u || config->stack_base>=0x1000000u || config->stack_size==0 ||
        config->stack_size>0x1000000u-config->stack_base || ((config->stack_base|config->stack_size)&1u)!=0 ||
        config->max_depth==0 || config->max_depth>CS_M68K_CALL_DEPTH ||
        cs_guest_window(s->calls.map,config->stack_base,config->stack_size,3,&stack)!=CS_GUEST_OK) return 0;
    int stack_region=0;
    for(size_t i=0;i<s->calls.map->count;++i)
        if(s->calls.map->regions[i].flags==3 && inside(s->calls.map->regions[i].base,
            s->calls.map->regions[i].size,config->stack_base,config->stack_size)) stack_region=1;
    if(!stack_region) return 0;
    for(unsigned i=0;i<CS_M68K_CALL_DEPTH;++i) {
        uint32_t token=config->sentinel_base+i*2u,value;
        if(config->sentinel_base>0xFFFFF8u || !readonly(s->calls.map,token) ||
            cs_guest_read(s->calls.map,token,2,5,&value)!=CS_GUEST_OK || value!=0x4E71u) return 0;
    }
    const cs_trap_config *g=&s->calls.gateway->config;
    for(size_t i=0;i<count;++i) {
        const cs_tb_record *r=&records[i];cs_trap_word word;uint32_t ignored;
        if(cs_trap_decode(r->word,&word)!=CS_TRAP_OK || !word.toolbox || word.auto_pop || word.reserved ||
            word.number==CS_TRAP_GET || word.number==CS_TRAP_SET ||
            cs_trap_get(s->calls.gateway,word.number,&ignored)!=CS_TRAP_OK ||
            r->identity<g->rom_base || r->identity-g->rom_base>=65536u || (r->identity&1u)!=0 ||
            r->identity==g->get_token || r->identity==g->set_token || r->identity==g->unsupported_token ||
            inside(s->calls.config.sentinel_base,CS_M68K_CALL_DEPTH*2u,r->identity,2) ||
            !readonly(s->calls.map,r->identity) ||
            cs_guest_read(s->calls.map,r->identity,2,5,&ignored)!=CS_GUEST_OK || ignored!=0x4E71u ||
            r->count>CS_TB_ARGUMENTS || (r->count!=0 && r->arguments==0) ||
            r->result<CS_TB_NONE || r->result>CS_TB_CHAR) return 0;
        if(r->convention==CS_TB_REGISTER) {
            if(r->count!=0 || r->result!=CS_TB_NONE || r->completion!=CS_TB_RETURNS || r->handler!=0) return 0;
        } else if(r->convention==CS_TB_STACK) {
            if(r->completion==CS_TB_RETURNS) {if(r->handler==0) return 0;}
            else if(r->completion==CS_TB_EXITS) {
                if(r->word!=0xA9F4u || r->count!=0 || r->result!=CS_TB_NONE || r->handler!=0) return 0;
            } else if(r->completion==CS_TB_CONTINUES) {
                if(r->word!=0xA9F0u || r->result!=CS_TB_NONE || r->handler!=0) return 0;
            } else return 0;
        } else return 0;
        uint32_t outputs=0;
        for(unsigned j=0;j<r->count;++j) {
            const cs_tb_argument *a=&r->arguments[j];if(!argument_valid(a)) return 0;
            if((a->access&2u)!=0) outputs+=a->bytes;
        }
        if(outputs>CS_TB_OUTPUT_BYTES) return 0;
        for(size_t j=0;j<i;++j)
            if(records[j].word==r->word || records[j].identity==r->identity) return 0;
    }
    return 1;
}
cs_tb_result cs_tb_init(cs_tb_runtime *rt,cs_m68k_session *s,
    const cs_tb_record *records,size_t count,const cs_m68k_bus_options *options)
{
    if(rt==0 || s==0 || records==0) return CS_TB_ARGUMENT;
    if(s->active!=0 || s->calls.depth!=0) return CS_TB_CONTEXT;
    if(!records_valid(s,records,count) || (options!=0 &&
        options->data_policy!=CS_M68K_STRICT && options->data_policy!=CS_M68K_DATA24)) return CS_TB_CONFIG;
    for(size_t i=0;i<count;++i) {
        uint32_t current;uint16_t number=(uint16_t)(records[i].word&0x1FFu);
        if(cs_trap_get(s->calls.gateway,number,&current)!=CS_TRAP_OK ||
            (current!=s->calls.gateway->config.unsupported_token && current!=records[i].identity)) return CS_TB_SIGNATURE;
    }
    rt->map=s->calls.map;rt->gateway=s->calls.gateway;rt->session=s;rt->records=records;rt->count=count;
    rt->options.data_policy=options==0?CS_M68K_STRICT:options->data_policy;
    rt->options.dispatch=options==0?0:options->dispatch;rt->options.context=options==0?0:options->context;
    rt->interrupt_context=0;rt->last.status=CS_TB_OK;rt->last.convention=CS_TB_CONVENTION_NONE;
    rt->last.completion=CS_TB_COMPLETION_NONE;rt->last.normalized_sp=0;
    rt->last.parameters=0;rt->last.outputs=0;rt->last.invoked=0;rt->tag=TB_TAG;
    for(size_t i=0;i<count;++i) rt->gateway->entries[records[i].word&0x1FFu]=records[i].identity;
    return CS_TB_OK;
}
static void begin(cs_tb_runtime *rt,cs_m68k_bus_report *r,const cs_trap_frame *f)
{
    rt->last.status=CS_TB_OK;rt->last.convention=CS_TB_CONVENTION_NONE;rt->last.completion=CS_TB_COMPLETION_NONE;
    rt->last.normalized_sp=0;rt->last.parameters=0;rt->last.outputs=0;rt->last.invoked=0;
    r->cpu.status=CS_M68K_OK;r->cpu.pc=f->pc;r->cpu.word=0;r->cpu.service_called=0;
    r->cpu.service.status=CS_TRAP_OK;r->cpu.service.pc=f->pc;r->cpu.service.target=0;
    r->cpu.service.decoded.word=0;r->cpu.service.decoded.number=0;
    r->cpu.service.decoded.toolbox=0;r->cpu.service.decoded.a0_result=0;
    r->cpu.service.decoded.auto_pop=0;r->cpu.service.decoded.reserved=0;r->cpu.service.decoded.flags=0;
    r->memory=CS_GUEST_OK;r->address=0;r->access=0;r->bytes=0;r->branch_taken=0;r->subroutine=0;
}
static cs_m68k_result failure(cs_tb_runtime *rt,cs_m68k_bus_report *r,cs_tb_result result)
{
    rt->last.status=result;r->cpu.status=CS_M68K_SERVICE;
    if(result==CS_TB_ADDRESS && r->memory==CS_GUEST_OK) r->memory=CS_GUEST_ADDRESS;
    r->cpu.service.status=result==CS_TB_PATCH?CS_TRAP_NEEDS_EXECUTION:
        result==CS_TB_UNKNOWN?CS_TRAP_UNIMPLEMENTED:
        result==CS_TB_ADDRESS?CS_TRAP_ADDRESS:CS_TRAP_UNSUPPORTED_FORMAT;
    return CS_M68K_SERVICE;
}
static cs_guest_result read_value(cs_tb_runtime *rt,cs_m68k_bus_report *r,
    uint32_t at,unsigned bytes,uint32_t flags,uint32_t *out)
{
    r->address=at;r->bytes=bytes;r->access=flags;
    r->memory=cs_guest_read(rt->map,at,bytes,flags,out);return r->memory;
}
static cs_guest_result window(cs_tb_runtime *rt,cs_m68k_bus_report *r,
    uint32_t at,uint32_t bytes,uint32_t flags,cs_guest_view *out)
{
    r->address=at;r->bytes=(unsigned)bytes;r->access=flags;
    r->memory=cs_guest_window(rt->map,at,bytes,flags,out);return r->memory;
}
static void store(uint8_t *p,unsigned bytes,uint32_t value)
{for(unsigned i=0;i<bytes;++i) p[i]=(uint8_t)(value>>((bytes-1u-i)*8u));}
cs_m68k_result cs_tb_step(void *context,cs_trap_frame *f,
    cs_m68k_bus_report *r,const cs_m68k_stack_limits *limits)
{
    cs_tb_runtime *rt=context;
    if(rt==0 || f==0 || r==0 || limits==0) return CS_M68K_ARGUMENT;
    begin(rt,r,f);
    if(rt->tag!=TB_TAG || rt->session==0 || rt->session->calls.map!=rt->map ||
        rt->session->calls.gateway!=rt->gateway || !records_valid(rt->session,rt->records,rt->count) ||
        (rt->options.data_policy!=CS_M68K_STRICT && rt->options.data_policy!=CS_M68K_DATA24))
        return failure(rt,r,CS_TB_CONFIG);
    uint32_t fetched;
    if(read_value(rt,r,f->pc,2,5,&fetched)!=CS_GUEST_OK) {r->cpu.status=CS_M68K_ADDRESS;return CS_M68K_ADDRESS;}
    r->cpu.word=(uint16_t)fetched;
    for(size_t i=0;i<rt->count;++i) if(f->pc==rt->records[i].identity) {
        r->cpu.service.target=f->pc;return failure(rt,r,CS_TB_DIRECT);
    }
    if((fetched&0xF800u)!=0xA800u)
        return cs_m68k_bus_step_bounded(rt->map,rt->gateway,f,r,&rt->options,limits);
    r->cpu.service_called=1;
    (void)cs_trap_decode((uint16_t)fetched,&r->cpu.service.decoded);
    if((f->sr&0xA000u)!=0x2000u) {r->cpu.status=CS_M68K_MODE;return CS_M68K_MODE;}
    if(rt->interrupt_context!=0) return failure(rt,r,CS_TB_CONTEXT);
    if(r->cpu.service.decoded.reserved) return failure(rt,r,CS_TB_FLAGS);
    uint16_t canonical=(uint16_t)(fetched&0xFBFFu);const cs_tb_record *record=0;
    for(size_t i=0;i<rt->count;++i) if(rt->records[i].word==canonical) record=&rt->records[i];
    if(record==0) return failure(rt,r,CS_TB_UNKNOWN);
    rt->last.convention=record->convention;rt->last.completion=record->completion;
    if(record->convention!=CS_TB_STACK) return failure(rt,r,CS_TB_CONVENTION);
    if(record->completion==CS_TB_CONTINUES) return failure(rt,r,CS_TB_CONTINUATION);
    if(cs_trap_get(rt->gateway,r->cpu.service.decoded.number,&r->cpu.service.target)!=CS_TRAP_OK)
        return failure(rt,r,CS_TB_CONFIG);
    if(r->cpu.service.target!=record->identity) {
        for(size_t i=0;i<rt->count;++i) if(r->cpu.service.target==rt->records[i].identity)
            return failure(rt,r,CS_TB_SIGNATURE);
        return failure(rt,r,CS_TB_PATCH);
    }
    unsigned autopop=r->cpu.service.decoded.auto_pop;
    if(autopop && (record->completion!=CS_TB_RETURNS || record->result!=CS_TB_NONE))
        return failure(rt,r,CS_TB_FLAGS);
    uint32_t parameters=0;
    for(unsigned i=0;i<record->count;++i) {
        if(record->arguments[i].kept) return failure(rt,r,CS_TB_RETAINED);
        parameters+=width(record->arguments[i].type);
    }
    unsigned result_bytes=width(record->result);
    uint32_t frame_bytes=6u+parameters+result_bytes+(autopop?4u:0u),sp=f->a[7];
    const cs_m68k_call_config *config=&rt->session->calls.config;
    cs_guest_view frame;
    if(config->stack_base<0x400u || config->stack_base>=0x1000000u || config->stack_size>0x1000000u-config->stack_base ||
        !inside(config->stack_base,config->stack_size,limits->base,limits->size) ||
        ((limits->base|limits->size|sp)&1u)!=0 || sp<6 || f->pc>0xFFFFFCu ||
        !inside(limits->base,limits->size,sp-6,frame_bytes) ||
        window(rt,r,sp-6,frame_bytes,3,&frame)!=CS_GUEST_OK) return failure(rt,r,CS_TB_ADDRESS);
    rt->last.normalized_sp=sp-4;rt->last.parameters=parameters;
    uint32_t next=f->pc+2,ignored;
    if(autopop) {
        if(read_value(rt,r,sp+parameters,4,1,&next)!=CS_GUEST_OK) return failure(rt,r,CS_TB_ADDRESS);
        if(inside(config->sentinel_base,CS_M68K_CALL_DEPTH*2u,next,2)) return failure(rt,r,CS_TB_CONTEXT);
    }
    if(read_value(rt,r,next,2,5,&ignored)!=CS_GUEST_OK) return failure(rt,r,CS_TB_ADDRESS);
    for(size_t i=0;i<rt->count;++i)
        if(next==rt->records[i].identity) return failure(rt,r,CS_TB_DIRECT);
    if(record->completion==CS_TB_EXITS) {
        if(cs_m68k_session_request_exit(rt->session,0)!=CS_M68K_SESSION_OK) return failure(rt,r,CS_TB_CONTEXT);
        /* Accepted exit request is the last fallible action. */
    } else {
        cs_tb_input input;cs_tb_reply reply;cs_guest_view views[CS_TB_ARGUMENTS];
        uint32_t addresses[CS_TB_ARGUMENTS],remaining=parameters,outputs=0;
        input.count=record->count;input.a5=f->a[5];reply.result=0;reply.bytes=0;
        for(unsigned i=0;i<CS_TB_OUTPUT_BYTES;++i) reply.output[i]=0;
        for(unsigned i=0;i<CS_TB_ARGUMENTS;++i) {
            input.value[i]=0;input.length[i]=0;input.span[i]=0;addresses[i]=0;
            views[i].bytes=0;views[i].writable=0;
        }
        for(unsigned i=0;i<record->count;++i) {
            const cs_tb_argument *a=&record->arguments[i];uint32_t value;
            remaining-=width(a->type);
            if(read_value(rt,r,sp+remaining,width(a->type),1,&value)!=CS_GUEST_OK)
                return failure(rt,r,CS_TB_ADDRESS);
            input.value[i]=a->type==CS_TB_BOOL?(value>>8)&1u:a->type==CS_TB_CHAR?value&0xFFu:value;
            if(a->type<CS_TB_SPAN) continue;
            addresses[i]=value;input.length[i]=a->bytes;
            if(a->alignment==2 && (value&1u)!=0) return failure(rt,r,CS_TB_ADDRESS);
            if(a->type==CS_TB_STRING) {
                if(read_value(rt,r,value,1,1,&input.length[i])!=CS_GUEST_OK) return failure(rt,r,CS_TB_ADDRESS);
                ++input.length[i];
            }
            if(overlap(value,input.length[i],config->stack_base,config->stack_size)) return failure(rt,r,CS_TB_ALIAS);
            if(window(rt,r,value,input.length[i],a->access,&views[i])!=CS_GUEST_OK) return failure(rt,r,CS_TB_ADDRESS);
            if((a->access&2u)!=0) for(size_t j=0;j<rt->map->count;++j) {
                const cs_guest_region *region=&rt->map->regions[j];
                if((region->flags&CS_GUEST_EXECUTE)!=0 && overlap(value,input.length[i],region->base,region->size)) {
                    r->memory=CS_GUEST_PERMISSION;return failure(rt,r,CS_TB_ADDRESS);
                }
            }
            for(unsigned j=0;j<i;++j)
                if(overlap(value,input.length[i],addresses[j],input.length[j])) return failure(rt,r,CS_TB_ALIAS);
            if((a->access&1u)!=0) input.span[i]=views[i].bytes;
            if((a->access&2u)!=0) outputs+=input.length[i];
        }
        rt->last.outputs=outputs;rt->last.invoked=1;
        if(record->handler(record->context,&input,&reply)!=0) return failure(rt,r,CS_TB_HANDLER);
        if(reply.bytes!=outputs) return failure(rt,r,CS_TB_REPLY);
        /* Immutable borrowed config; no fallible operations after this point. */
        uint32_t offset=0;
        for(unsigned i=0;i<record->count;++i) if((record->arguments[i].access&2u)!=0) {
            for(uint32_t j=0;j<input.length[i];++j) views[i].writable[j]=reply.output[offset+j];
            offset+=input.length[i];
        }
        uint32_t value=record->result==CS_TB_BOOL?(reply.result!=0?0x100u:0u):
            record->result==CS_TB_CHAR?reply.result&0xFFu:reply.result;
        if(result_bytes!=0) store(frame.writable+6u+parameters,result_bytes,value);
    }
    store(frame.writable,2,f->sr);store(frame.writable+2,4,next);
    f->pc=next;f->a[7]=sp+parameters+(autopop?4u:0u);
    r->cpu.status=CS_M68K_OK;r->cpu.service.status=CS_TRAP_OK;r->memory=CS_GUEST_OK;
    r->branch_taken=autopop;return CS_M68K_OK;
}
