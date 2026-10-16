# Focused rectangle boundary review - 2026-10-16

Owner SRC-0202 continues the next visible loaded68k drawing milestone. Standing
SRC-0149 allows this reviewed-public/original-code check. Codex is sole writer.
Read only THIS frozen packet, ranges1..120,121..240,241..360,361..480, at most4
Reads, no EOF probes/other files. Return <=450 words: concrete demonstrated
ownership/atomicity bypasses, line/function and falsifying test; otherwise say
no demonstrated bypass and exact limits. Do not perform a general review.

No commands/edits/tests/web/MCP/extra agents/chats/automation/OneNote. No HQ,
raw research/manual/credential/Apple/MS implementation/ROM/binary/assets/media,
emulator source/manual sample/private heap reconstruction. Only original code
and the reviewed engineering contract below. No historical/provenance certification.
Native extents are live/aligned/truthful/synchronized; no other host entry or
mutation runs during the synchronous mask7 root. No hostile native-code sandbox.
The existing execution_safe helper is unchanged: its added spans are mutually
disjoint and checked against all image/lifecycle/manager/map/descriptors/backings/
inventory controls. The new port/scratch spans use that same checked envelope.
Only opt-in coordinator accepts canonical A8A2/A8A3. Directory entries must be
unsupported; global Get/Set/direct QuickDraw installation remains deferred.
Stop effects survive until explicit existing retirement. Only existing actual
RTS/root-slot validation can complete a root. Ordinary CPU guard still rejects
all A-lines, including fallback. Focus on new explicit interception/commit path.

## personality/system1/quickdraw-rects.h

```c
/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original fixed-port SPEC-0036 profile. */
#ifndef CLASSICK_SYSTEM1_QUICKDRAW_RECTS_H
#define CLASSICK_SYSTEM1_QUICKDRAW_RECTS_H
#include <stdint.h>
#define CS_QD_RECT_WIDTH 128u
#define CS_QD_RECT_HEIGHT 64u
#define CS_QD_RECT_BYTES 1024u
#define CS_QD_PAINT_RECT UINT16_C(0xA8A2)
#define CS_QD_ERASE_RECT UINT16_C(0xA8A3)
typedef struct {
    uint8_t pixels[CS_QD_RECT_BYTES];
    uint32_t paints,erases;
} cs_qd_rect_port;
/* Native borrowed eight-byte BE signed16 top,left,bottom,right record.
 * Objects/buffers live/aligned/disjoint/synchronized. No guest pointers,
 * callbacks or retention. Unknown word/null returns0 unchanged; success1.
 * Fixed128x64/stride16 black patCopy/white background; not a guest GrafPort.
 * Empty/inverted/clipped calls succeed; diagnostic counts saturate.
 * Original algorithm, no libc/mutable global/allocator. */
int cs_qd_rect(cs_qd_rect_port *port,const uint8_t rect[8],uint16_t word);
#endif
```

## personality/system1/code-run-internal.h

```c
/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Private coordinator boundary. */
#ifndef CLASSICK_SYSTEM1_CODE_RUN_INTERNAL_H
#define CLASSICK_SYSTEM1_CODE_RUN_INTERNAL_H
#include "code-run.h"
typedef struct {
    cs_guest_view argument,rect,scratch;
    cs_trap_frame next;
    uint8_t bytes[8];
} cs_image_rect_scratch;
int cs_image_execution_rect_native(cs_image_service *s,const cs_image_run_request *q,
    cs_trap_frame *caller,cs_image_run_report *r,cs_qd_rect_port *port,
    cs_image_rect_scratch *scratch);
int cs_image_execution_native(cs_image_service *s,const cs_image_run_request *q,
    cs_trap_frame *caller,cs_image_run_report *r);
int cs_image_execution_retire_native(cs_image_service *s,const cs_image_run_request *q,
    cs_image_workspace *w,size_t capacity,cs_image_description *out,cs_image_report *r);
int cs_image_execution_integrity(const cs_image_service *s,const cs_image_context *c,int dirty);
#endif
```

## personality/system1/code-run.c

```c
/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original dedicated execution coordinator. */
#include "code-run-internal.h"
typedef struct {cs_image_service *image;const cs_image_run_request *request;
    uint32_t root;cs_qd_rect_port *port;} policy;
static void clear(void *p,size_t n){uint8_t *b=p;for(size_t i=0;i<n;++i)b[i]=0;}
static void copy(void *p,const void *q,size_t n)
{uint8_t *a=p;const uint8_t *b=q;for(size_t i=0;i<n;++i)a[i]=b[i];}
static void frame_copy(cs_trap_frame *a,const cs_trap_frame *b)
{for(unsigned i=0;i<8;++i){a->a[i]=b->a[i];a->d[i]=b->d[i];}a->pc=b->pc;a->sr=b->sr;}
static int inside(uint32_t a,uint32_t n,uint32_t base,uint32_t size)
{return n && a>=base && n<=size && a-base<=size-n;}
static int opcode(const cs_image_service *s,uint32_t pc)
{
    for(uint32_t i=0;i<s->manifest.segment_count;++i){const cs_image_segment *p=&s->segments[i];
        if(p->length>=6 && inside(pc,2,p->address+4u,p->length-4u))return 1;}
    return pc>=s->layout.table_begin+2u && pc<s->layout.table_end &&
        (pc-s->layout.table_begin)%8u==2u;
}
static int extension(const cs_image_service *s,const cs_m68k_guard_event *e)
{
    uint32_t pc=e->before->pc;
    if(pc>=s->layout.table_begin+2u && pc<s->layout.table_end &&
        (pc-s->layout.table_begin)%8u==2u)
        return e->word==0x4EF9u && e->address==pc+2u && e->bytes==4;
    for(uint32_t i=0;i<s->manifest.segment_count;++i){const cs_image_segment *p=&s->segments[i];
        if(p->length>=6 && inside(pc,2,p->address+4u,p->length-4u) &&
            inside(e->address,e->bytes,p->address+4u,p->length-4u))return 1;}
    return 0;
}
static int memory(const cs_image_service *s,const cs_m68k_guard_event *e,int writing)
{
    if(inside(e->address,e->bytes,s->layout.globals_begin,s->manifest.below_a5) ||
        inside(e->address,e->bytes,s->layout.stack_base,s->layout.stack_top-s->layout.stack_base))return 1;
    if(writing)return 0;
    if(inside(e->address,e->bytes,s->layout.a5,s->manifest.above_a5))return 1;
    for(uint32_t i=0;i<s->manifest.segment_count;++i){const cs_image_segment *p=&s->segments[i];
        if(p->length>=4 && inside(e->address,e->bytes,p->address+4u,p->length-4u))return 1;}
    return 0;
}
static int check(const void *opaque,const cs_m68k_guard_event *e)
{
    const policy *p=opaque;const cs_image_service *s=p->image;
    int entry=s->execution_state==1 && e->kind==CS_M68K_GUARD_WRITE && e->word==0 &&
        e->address==p->root && e->bytes==4 && e->before->a[7]==s->layout.stack_top;
    if((s->execution_state!=2 && !entry) || s->execution_generation!=s->generation ||
        !cs_image_execution_integrity(s,&p->request->context,1))return 0;
    switch(e->kind){
        case CS_M68K_GUARD_OPCODE:return e->bytes==2 && opcode(s,e->address);
        case CS_M68K_GUARD_EXTENSION:return extension(s,e);
        case CS_M68K_GUARD_READ:return memory(s,e,0);
        case CS_M68K_GUARD_WRITE:return memory(s,e,1);
        case CS_M68K_GUARD_INSTRUCTION:
            if((e->word&0xF000u)==0xA000u)return p->port &&
                (e->word==CS_QD_PAINT_RECT || e->word==CS_QD_ERASE_RECT);
            return !(e->before->pc>=s->layout.table_begin && e->before->pc<s->layout.table_end) || e->word==0x4EF9u;
        case CS_M68K_GUARD_TARGET:
            if(e->address==p->request->token)return e->word==0x4E75u && e->before->a[7]==p->root;
            return e->bytes==2 && opcode(s,e->address);
        case CS_M68K_GUARD_COMMIT:
            if(e->proposed->a[5]!=s->layout.a5)return 0;
            if(e->proposed->pc==p->request->token)return e->word==0x4E75u &&
                e->before->a[7]==p->root && e->proposed->a[7]==s->layout.stack_top;
            return opcode(s,e->proposed->pc);
        default:return 0;
    }
}
static int event(const policy *p,cs_m68k_guard_kind kind,const cs_trap_frame *before,
    const cs_trap_frame *next,uint16_t word,uint32_t at,uint32_t bytes,uint32_t access)
{
    cs_m68k_guard_event e;e.kind=kind;e.before=before;e.proposed=next;
    e.word=word;e.address=at;e.bytes=bytes;e.access=access;return check(p,&e);
}
static int rectangle_directory(const cs_image_run_request *q)
{
    uint32_t paint,erase;
    return cs_trap_get(q->gateway,CS_QD_PAINT_RECT&0x1FFu,&paint)==CS_TRAP_OK &&
        cs_trap_get(q->gateway,CS_QD_ERASE_RECT&0x1FFu,&erase)==CS_TRAP_OK &&
        paint==q->gateway->config.unsupported_token && erase==q->gateway->config.unsupported_token;
}
/* Only the owning coordinator can enter this path. Every potentially failing
 * check precedes native pixels, exception scratch and frame publication. */
static cs_m68k_result rectangle(const policy *p,cs_trap_frame *f,
    cs_m68k_bus_report *r,cs_image_rect_scratch *w,uint16_t word)
{
    cs_image_service *s=p->image;uint32_t sp=f->a[7],pointer;
    r->cpu.pc=f->pc;r->cpu.word=word;r->cpu.service_called=1;
    r->cpu.service.pc=f->pc;r->cpu.service.decoded.word=word;
    r->cpu.service.decoded.number=(uint16_t)(word&0x1FFu);r->cpu.service.decoded.toolbox=1;
    if((f->sr&0xA700u)!=0x2700u || f->a[5]!=s->layout.a5 || (sp&1u) || sp<6 ||
        sp>p->root || p->root-sp<4u ||
        !inside(sp-6u,10,s->layout.stack_base,s->layout.stack_top-s->layout.stack_base) ||
        !rectangle_directory(p->request) ||
        !event(p,CS_M68K_GUARD_OPCODE,f,f,0,f->pc,2,5) ||
        !event(p,CS_M68K_GUARD_INSTRUCTION,f,f,word,f->pc,2,5) ||
        !event(p,CS_M68K_GUARD_READ,f,f,word,sp,4,1))return CS_M68K_POLICY;
    r->address=sp;r->bytes=4;r->access=1;
    r->memory=cs_guest_window(s->map,sp,4,CS_GUEST_READ,&w->argument);
    if(r->memory!=CS_GUEST_OK)return CS_M68K_ADDRESS;
    pointer=0;for(unsigned i=0;i<4;++i)pointer=(pointer<<8)|w->argument.bytes[i];
    if((pointer&1u) || !event(p,CS_M68K_GUARD_READ,f,f,word,pointer,8,1))return CS_M68K_POLICY;
    r->address=pointer;r->bytes=8;
    r->memory=cs_guest_window(s->map,pointer,8,CS_GUEST_READ,&w->rect);
    if(r->memory!=CS_GUEST_OK)return CS_M68K_ADDRESS;
    for(unsigned i=0;i<8;++i)w->bytes[i]=w->rect.bytes[i];
    frame_copy(&w->next,f);w->next.pc+=2u;w->next.a[7]+=4u;
    if(!event(p,CS_M68K_GUARD_WRITE,f,&w->next,word,sp-6u,6,2) ||
        !event(p,CS_M68K_GUARD_COMMIT,f,&w->next,word,w->next.pc,0,0))return CS_M68K_POLICY;
    r->address=sp-6u;r->bytes=6;r->access=2;
    r->memory=cs_guest_window(s->map,sp-6u,6,CS_GUEST_WRITE,&w->scratch);
    if(r->memory!=CS_GUEST_OK)return CS_M68K_ADDRESS;
    /* Snapshot precedes scratch writes even when the guest Rect overlaps them.
     * Canonical operation and nonnull port are already established. Helper
     * refusal is pre-effect; after it succeeds all remaining writes are total. */
    if(!cs_qd_rect(p->port,w->bytes,word))return CS_M68K_POLICY;
    w->scratch.writable[0]=(uint8_t)(f->sr>>8);w->scratch.writable[1]=(uint8_t)f->sr;
    for(unsigned i=0;i<4;++i)w->scratch.writable[2u+i]=(uint8_t)(w->next.pc>>((3u-i)*8u));
    frame_copy(f,&w->next);r->cpu.service.status=CS_TRAP_OK;
    return CS_M68K_OK;
}
static int token_valid(const cs_image_service *s,uint32_t at)
{
    cs_guest_view v;
    if((at&1u) || at>0xFFFFF8u || cs_guest_window(s->map,at,8,CS_GUEST_READ|CS_GUEST_EXECUTE,&v)!=CS_GUEST_OK)return 0;
    if(inside(at,8,s->config.system.begin,s->config.system.end-s->config.system.begin) ||
        inside(at,8,s->config.application.begin,s->config.application.end-s->config.application.begin) ||
        inside(at,8,s->layout.stack_base,s->layout.b-s->layout.stack_base))return 0;
    for(unsigned i=0;i<8;++i)if(v.bytes[i]!=(i&1u?0x71u:0x4Eu))return 0;
    return 1;
}
static cs_image_run_result finish(cs_image_service *s,cs_image_run_report *r,
    cs_image_run_result result,const cs_trap_frame *guest)
{
    frame_copy(&r->guest,guest);r->status=result;
    if(result==CS_IMAGE_RUN_RETURNED)s->execution_state=3;
    else if(result==CS_IMAGE_RUN_QUARANTINED)s->execution_state=5;
    else s->execution_state=4;
    return result;
}
static cs_image_run_result run(cs_image_service *s,const cs_image_run_request *q,
    cs_trap_frame *caller,cs_image_run_report *r,cs_qd_rect_port *port)
{
    cs_image_rect_scratch rectangle_scratch;
    if(port?!cs_image_execution_rect_native(s,q,caller,r,port,&rectangle_scratch):
        !cs_image_execution_native(s,q,caller,r))return CS_IMAGE_RUN_ARGUMENT;
    clear(r,sizeof *r);r->status=CS_IMAGE_RUN_REFUSED;
    if(s->execution_state || s->execution_generation==s->generation ||
        q->generation!=s->generation || q->epoch!=s->epoch || !q->budget || q->budget>1000000u ||
        !cs_image_execution_integrity(s,&q->context,0) || !token_valid(s,q->token) ||
        (port && !rectangle_directory(q)) ||
        s->layout.stack_top<s->layout.stack_base+4u)return r->status;
    /* X-1's envelope rejects all distinct backing aliases and known native
     * controls. Check the same invariant before claiming or writing the token. */
    for(size_t i=0;i<s->map->count;++i)for(size_t j=0;j<i;++j){
        uintptr_t a=(uintptr_t)s->map->regions[i].bytes,b=(uintptr_t)s->map->regions[j].bytes;
        size_t n=s->map->regions[i].size,m=s->map->regions[j].size;
        if(a<=b?b-a<n:a-b<m)return r->status;
    }
    cs_trap_frame guest;clear(&guest,sizeof guest);
    guest.pc=s->layout.table_begin+2u;guest.sr=0x2700;guest.a[5]=s->layout.a5;guest.a[7]=s->layout.stack_top;
    r->generation=s->generation;r->epoch=s->epoch;
    policy p;clear(&p,sizeof p);p.image=s;p.request=q;p.root=s->layout.stack_top-4u;p.port=port;
    cs_m68k_guard guard;guard.check=check;guard.context=&p;guard.context_bytes=sizeof p;
    cs_m68k_stack_limits limits;limits.base=s->layout.stack_base;limits.size=s->layout.stack_top-s->layout.stack_base;
    cs_m68k_bus_report step;clear(&step,sizeof step);
    if(cs_m68k_bus_guard_validate(s->map,q->gateway,&guest,&step,&limits,&guard)!=CS_M68K_OK)return r->status;
    s->execution_generation=s->generation;s->execution_state=1;s->execution_scrubbed=0;
    /* Entry is a separately admitted, whole bounded stack write. There is no
     * fallible operation after its store and before publishing RUNNING/A7. */
    cs_m68k_guard_event entry;clear(&entry,sizeof entry);entry.kind=CS_M68K_GUARD_WRITE;
    entry.address=p.root;entry.bytes=4;entry.access=CS_GUEST_WRITE;entry.before=&guest;entry.proposed=&guest;
    if(!check(&p,&entry) || cs_guest_write(s->map,p.root,4,q->token)!=CS_GUEST_OK)
        return finish(s,r,CS_IMAGE_RUN_QUARANTINED,&guest);
    guest.a[7]=p.root;r->entered=1;s->execution_state=2;
    for(uint32_t i=0;i<q->budget;++i){
        if(!cs_image_execution_integrity(s,&q->context,1))return finish(s,r,CS_IMAGE_RUN_QUARANTINED,&guest);
        clear(&step,sizeof step);
        cs_m68k_result cpu;uint32_t word=0;
        if(port && opcode(s,guest.pc) &&
            cs_guest_read(s->map,guest.pc,2,CS_GUEST_READ,&word)==CS_GUEST_OK &&
            (word==CS_QD_PAINT_RECT || word==CS_QD_ERASE_RECT))
            cpu=rectangle(&p,&guest,&step,&rectangle_scratch,(uint16_t)word);
        else cpu=cs_m68k_bus_step_admitted(s->map,q->gateway,&guest,&step,&limits,&guard,q->token,p.root);
        step.cpu.status=cpu;
        if(step.cpu.service_called && cpu!=CS_M68K_OK)
            step.cpu.service.status=cpu==CS_M68K_ADDRESS?CS_TRAP_ADDRESS:CS_TRAP_UNSUPPORTED_FORMAT;
        r->cpu=cpu;r->memory=step.memory;
        if(cpu!=CS_M68K_OK){
            if(!cs_image_execution_integrity(s,&q->context,1) || cpu==CS_M68K_ARGUMENT)
                return finish(s,r,CS_IMAGE_RUN_QUARANTINED,&guest);
            return finish(s,r,CS_IMAGE_RUN_CPU,&guest);
        }
        ++r->steps;
        if(guest.pc==q->token){
            if(step.cpu.word!=0x4E75u || guest.a[7]!=s->layout.stack_top)
                return finish(s,r,CS_IMAGE_RUN_QUARANTINED,&guest);
            return finish(s,r,CS_IMAGE_RUN_RETURNED,&guest);
        }
    }
    return finish(s,r,CS_IMAGE_RUN_BUDGET,&guest);
}
cs_image_run_result cs_image_run(cs_image_service *s,const cs_image_run_request *q,
    cs_trap_frame *caller,cs_image_run_report *r)
{return run(s,q,caller,r,0);}
cs_image_run_result cs_image_run_rectangles(cs_image_service *s,const cs_image_run_request *q,
    cs_trap_frame *caller,cs_image_run_report *r,cs_qd_rect_port *port)
{return port?run(s,q,caller,r,port):CS_IMAGE_RUN_ARGUMENT;}
cs_image_result cs_image_run_retire(cs_image_service *s,const cs_image_run_request *q,
    cs_image_workspace *w,size_t capacity,cs_image_description *out,cs_image_report *r)
{
    if(!cs_image_execution_retire_native(s,q,w,capacity,out,r))return CS_IMAGE_ARGUMENT;
    if((s->execution_state!=3 && s->execution_state!=4) || q->generation!=s->generation ||
        q->epoch!=s->epoch || s->execution_generation!=s->generation)return CS_IMAGE_STATE;
    if(!cs_image_execution_integrity(s,&q->context,1)){s->execution_state=5;return CS_IMAGE_FAULT;}
    /* Establish every scrub destination before changing any byte. Only below-A5
     * becomes canonical; stack and the caller-owned stop snapshot are retained. */
    for(uint32_t i=0;i<s->manifest.below_a5;++i){cs_guest_view v;
        if(cs_guest_window(s->map,s->layout.globals_begin+i,1,CS_GUEST_WRITE,&v)!=CS_GUEST_OK){s->execution_state=5;return CS_IMAGE_FAULT;}}
    for(uint32_t i=0;i<s->manifest.below_a5;++i){cs_guest_view v;
        if(cs_guest_window(s->map,s->layout.globals_begin+i,1,CS_GUEST_WRITE,&v)!=CS_GUEST_OK){s->execution_state=5;return CS_IMAGE_FAULT;}
        v.writable[0]=0;
        ++s->execution_scrubbed;
    }
    if(!cs_image_execution_integrity(s,&q->context,0)){s->execution_state=5;return CS_IMAGE_FAULT;}
    cs_image_request release;clear(&release,sizeof release);release.operation=CS_IMAGE_RELEASE;
    copy(&release.context,&q->context,sizeof release.context);release.generation=s->generation;release.epoch=s->epoch;
    release.b=s->layout.b;release.s=s->layout.s;release.h_min=s->layout.h_min;
    s->execution_state=0;
    cs_image_result result=cs_image_execute(s,&release,w,capacity,out,r);
    r->execution_scrubbed=s->execution_scrubbed;
    if(result!=CS_IMAGE_OK)s->execution_state=5;
    return result;
}
```

## code-image.c new envelope adapter

```c
int cs_image_execution_rect_native(cs_image_service *s,const cs_image_run_request *q,
    cs_trap_frame *caller,cs_image_run_report *r,cs_qd_rect_port *port,
    cs_image_rect_scratch *scratch)
{
    if(!OBJ(caller,cs_trap_frame) || !OBJ(r,cs_image_run_report) ||
        !OBJ(port,cs_qd_rect_port) || !OBJ(scratch,cs_image_rect_scratch))return 0;
    span outputs[4]={{caller,sizeof *caller},{r,sizeof *r},
        {port,sizeof *port},{scratch,sizeof *scratch}};
    return execution_safe(s,q,outputs,4);
}
```
