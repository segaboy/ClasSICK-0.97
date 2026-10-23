/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../platform/uefi/window.h"
#include "../platform/pc/pmtimer.h"
#include "acpi-fixture.h"
#include "ps2-bus-fixture.h"
#include <stdio.h>
#include <stdlib.h>

enum { W=640,H=480,PITCH=648,FB_BYTES=PITCH*4*H,FB_PAGES=(FB_BYTES+4095)/4096 };
static unsigned char *bundle_raw,*fb_raw,*bundle,*fb,*saved;
static cs_uefi_handoff *h;
static cs_native_memory memory;
static cs_native_devices devices;
static cs_ps2_io io;
static cs_native_window_binding binding;
static bus_fixture bus;
static uint32_t raw_tick,reads,turns,drained,published,loss_seen,start_seen;
static uint64_t stop_ticks;
static int clock_bad,freeze,freeze_ready,stop_first;
static int fail_clock_after_ready;
static int fail_clock_after_start;
#ifdef CS_UWIN_FAULTS
static int injected;
uint32_t cs_test_window_start(cs_pc_window_session *s,const cs_pc_window_config *c)
{
    if(injected==1 || injected==2){
        s->magic=CS_PWS_MAGIC;s->phase=CS_PWS_FAULTED;
        s->result=injected==1?CS_PWS_STARTUP:CS_PWS_PRESENT;
        s->stage=injected==1?CS_PWS_CANCEL:CS_PWS_PUBLISH;s->cause=CS_WINDOW_INPUT_FAULT;
        if(injected==2){
            s->ram_frames=1;s->presentation.phase=CS_MONO_FAILED;
            s->presentation.rows=5;s->presentation.required=H;s->presentation.cause=CS_FB_STORAGE;
            c->target.base[0]=0x42u; /* injected dependency prefix, not five real rows */
        }
        return s->result;
    }
    return cs_pc_window_session_start(s,c);
}
uint32_t cs_test_window_poll(cs_pc_window_session *s,uint64_t now,uint32_t ok)
{
    if(injected>=3){
        s->phase=CS_PWS_FAULTED;s->delivery.kind=CS_PS2_DELIVERY_PACKET;
        s->result=injected==3?CS_PWS_QUEUE:injected==4?CS_PWS_OWNER:CS_PWS_PRESENT;
        s->stage=injected==3?CS_PWS_DRAIN:injected==4?CS_PWS_DISPATCH:CS_PWS_PUBLISH;
        s->cause=injected==3?CS_INPUT_ERR_STATE:CS_WINDOW_INPUT_FAULT;
        if(injected>=4){s->consumed=1;s->owner.pointer.x+=3;}
        if(injected==5){
            s->ram_frames=2;s->presentation.phase=CS_MONO_FAILED;
            s->presentation.rows=5;s->presentation.required=342;s->presentation.cause=CS_FB_STORAGE;
            s->target.base[0]=0x42u;
        }
        return s->result;
    }
    return cs_pc_window_session_poll(s,now,ok);
}
#endif
static unsigned cases,checks,failures;
#define CHECK(x) do { ++checks; if(!(x)){++failures;fprintf(stderr,"uefi-window:%d %s\n",__LINE__,#x);} } while(0)
static uint32_t u32(const unsigned char *p)
{ return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24); }
static uint64_t u64(const unsigned char *p)
{ return (uint64_t)u32(p)|((uint64_t)u32(p+4)<<32); }
static unsigned char *trace(void){return bundle+CS_LOADER_TRACE_OFFSET+CS_UWIN_TRACE_OFFSET;}
static cs_pc_window_session *session(void){return (cs_pc_window_session *)(void *)(bundle+CS_LOADER_ARENA_OFFSET);}
static uint32_t port(void *context,uint16_t address)
{
    CHECK(context==&reads && address==0x608u); ++reads;
    if(start_seen){
        CHECK(session()->delivery.kind==CS_PS2_DELIVERY_NONE);
        CHECK(session()->consumed==0u && session()->drained==0u);
    }
    if(clock_bad)return UINT32_MAX;
    if(!freeze)raw_tick=(raw_tick+(bus.ready?65536u:1000u))&0xFFFFFFu;
    return raw_tick;
}
static int observe(void *context,const unsigned char *t,const cs_pc_window_session *s)
{
    CHECK(context==&turns && s==session()); ++turns;
    CHECK(u32(t)==CS_UWIN_TRACE_MAGIC && u32(t+4)==1u && u32(t+8)==160u);
    CHECK(u32(t+12)==CS_UWIN_ACTIVE && u32(t+16)==CS_UWIN_RUNNING && u32(t+156)==0u);
    CHECK(u32(t+28)==1u);
    CHECK(u32(t+96)==s->owner.product.inputs && u32(t+100)==s->ram_frames);
    CHECK(u32(t+104)==s->published_frames);
    if(!start_seen){
        start_seen=1;
        CHECK(bus.key.writes==0u && bus.mouse_writes==0u && reads==1u);
        CHECK(s->owner.pointer.suppress==1u && s->published_frames==1u);
        CHECK(u32(t+108)==CS_MONO_COMPLETE && u32(t+112)==H);
    }else if(s->stage!=CS_PWS_PUBLISH){
        CHECK(u32(t+108)==0u && u32(t+112)==0u && u32(t+116)==0u && u32(t+120)==0u);
    }
    drained+=s->drained;
    if(s->delivery.kind==CS_PS2_DELIVERY_LOST)++loss_seen;
    published=s->published_frames;
    if(freeze_ready && bus.ready)freeze=1;
    if(fail_clock_after_ready && bus.ready)clock_bad=1;
    if(fail_clock_after_start)clock_bad=1;
    return stop_first || (stop_ticks!=0 && u64(t+136)>=stop_ticks);
}
static void descriptor(unsigned index,uint32_t kind,uint64_t base,uint64_t pages)
{
    unsigned char *p=bundle+CS_LOADER_MAP_OFFSET+48u*index;
    fx_put(p,kind,4);fx_put(p+8,base,8);fx_put(p+16,base,8);fx_put(p+24,pages,8);
}
static void setup(void)
{
    cs_pc_window_session *s=session();
    if(s->magic==CS_PWS_MAGIC){
        cs_rect old;cs_cursor_info info;
        (void)cs_pc_window_session_stop(s);
        if(cs_cursor_query(&s->cursor,&info)==CS_CURSOR_OK && info.phase!=CS_CURSOR_UNBOUND)
            CHECK(cs_cursor_abandon(&s->cursor,&old)==CS_CURSOR_OK);
    }
    memset(bundle,0,CS_LOADER_BUNDLE);memset(fb,0xA5,(size_t)FB_PAGES*4096u);
    memset(bundle+CS_LOADER_ARENA_OFFSET,0xA5,CS_LOADER_ARENA_BYTES);
    memset(bundle+CS_LOADER_TRACE_OFFSET,0xE7,CS_LOADER_TRACE_BYTES);
    h=(cs_uefi_handoff *)(void *)bundle;
    h->magic=UINT64_C(0x4353303937484F46);h->version=2;h->size=(uint32_t)sizeof *h;
    h->stage=CS_UEFI_EXITED;h->bundle_base=(uint64_t)(uintptr_t)bundle;h->bundle_size=CS_LOADER_BUNDLE;
    h->image_base=0x10000000u;h->image_size=4096;
    h->map_base=h->bundle_base+CS_LOADER_MAP_OFFSET;h->map_size=4u*48u;h->map_stride=48;h->map_version=1;
    h->arena_base=h->bundle_base+CS_LOADER_ARENA_OFFSET;h->arena_size=CS_LOADER_ARENA_BYTES;
    h->trace_base=h->bundle_base+CS_LOADER_TRACE_OFFSET;h->trace_size=CS_LOADER_TRACE_BYTES;
    h->stack_top=h->bundle_base+CS_LOADER_FAULT_OFFSET;h->fault_top=h->arena_base;
    h->framebuffer.base=(uint64_t)(uintptr_t)fb;h->framebuffer.size=FB_BYTES;
    h->framebuffer.width=W;h->framebuffer.height=H;h->framebuffer.pitch=PITCH;h->framebuffer.format=1;
    h->span_count=2;h->spans[0].base=h->image_base;h->spans[0].size=4096;h->spans[0].kind=1;
    h->spans[1].base=h->bundle_base;h->spans[1].size=CS_LOADER_BUNDLE;h->spans[1].kind=2;
    descriptor(0,1,h->image_base,1);descriptor(1,2,h->bundle_base,CS_LOADER_BUNDLE/4096);
    descriptor(2,9,FX_PHYS,FX_SIZE/4096);descriptor(3,0,h->framebuffer.base,FB_PAGES);
    fx_build(0,276);fx[FX_FADT+8]=6;fx[FX_FADT+109]=2;fx_seal(fx+FX_FADT);
    h->rsdp=FX_PHYS+FX_RSDP;
    memory.map=bundle+CS_LOADER_MAP_OFFSET;memory.map_size=(size_t)h->map_size;
    memory.map_stride=48;memory.map_version=1;
    memory.window_physical=FX_PHYS;memory.window_size=FX_SIZE;memory.window_base=(uintptr_t)fx;
    devices.memory=&memory;devices.port=port;devices.port_context=&reads;
    devices.framebuffer=fb;devices.arena=bundle+CS_LOADER_ARENA_OFFSET;
    bf_init(&bus);io=bf_io(&bus);
    binding.devices=&devices;binding.keyboard=&io;binding.physical=0;binding.size=UINT64_C(1)<<47;
    binding.base=0;binding.observer=observe;binding.context=&turns;
    raw_tick=reads=turns=drained=published=loss_seen=start_seen=0;
    stop_ticks=0;clock_bad=freeze=freeze_ready=stop_first=0;
    fail_clock_after_ready=fail_clock_after_start=0;
#ifdef CS_UWIN_FAULTS
    injected=0;
#endif
}
static void snapshot(void)
{
    memcpy(saved,devices.arena,CS_LOADER_ARENA_BYTES);
    memcpy(saved+CS_LOADER_ARENA_BYTES,fb,FB_BYTES);
    memcpy(saved+CS_LOADER_ARENA_BYTES+FB_BYTES,bundle+CS_LOADER_TRACE_OFFSET,CS_LOADER_TRACE_BYTES);
}
static void preserved(int raw)
{
    CHECK(memcmp(saved,devices.arena,CS_LOADER_ARENA_BYTES)==0);
    CHECK(memcmp(saved+CS_LOADER_ARENA_BYTES,fb,FB_BYTES)==0);
    const unsigned char *before=saved+CS_LOADER_ARENA_BYTES+FB_BYTES;
    const unsigned char *after=bundle+CS_LOADER_TRACE_OFFSET;
    CHECK(memcmp(before,after,CS_UWIN_TRACE_OFFSET)==0);
    CHECK(memcmp(before+CS_UWIN_TRACE_OFFSET+160,after+CS_UWIN_TRACE_OFFSET+160,
        CS_LOADER_TRACE_BYTES-CS_UWIN_TRACE_OFFSET-160)==0);
    if(raw)CHECK(memcmp(before,after,CS_LOADER_TRACE_BYTES)==0);
    CHECK(bus.key.writes==0u && bus.mouse_writes==0u && turns==0u);
}
static void gate(uint32_t ready,uint32_t expected)
{
    ++cases;snapshot();
    CHECK(cs_native_window_loop(h,ready,&binding,60,trace())==expected);
    preserved(expected==CS_UWIN_ARGUMENT);
    if(expected!=CS_UWIN_ARGUMENT)CHECK(u32(trace()+16)==expected);
}
static void gates(void)
{
    setup();++cases;snapshot();CHECK(cs_native_window_loop(NULL,1,&binding,60,trace())==CS_UWIN_ARGUMENT);preserved(1);
    setup();++cases;snapshot();CHECK(cs_native_window_loop(h,1,NULL,60,trace())==CS_UWIN_ARGUMENT);preserved(1);
    setup();++cases;snapshot();
    CHECK(cs_native_window_loop((const cs_uefi_handoff *)((UINTPTR_MAX-sizeof(*h)/2u)&~(uintptr_t)7u),1,&binding,60,trace())==CS_UWIN_ARGUMENT);preserved(1);
    setup();++cases;snapshot();CHECK(cs_native_window_loop(h,1,&binding,0,trace())==CS_UWIN_ARGUMENT);preserved(1);
    setup();++cases;snapshot();CHECK(cs_native_window_loop(h,1,&binding,60,(unsigned char *)h)==CS_UWIN_ARGUMENT);preserved(1);
    setup();++cases;snapshot();CHECK(cs_native_window_loop(h,1,&binding,60,NULL)==CS_UWIN_ARGUMENT);preserved(1);
    setup();binding.physical=1;binding.size=UINT64_MAX;gate(1,CS_UWIN_ARGUMENT);
    setup();h->stage=CS_UEFI_FAILED;gate(1,CS_UWIN_NOT_EXITED);
    setup();h->firmware_status=1;gate(1,CS_UWIN_NOT_EXITED);
    setup();gate(0,CS_UWIN_NOT_READY);
    setup();h->magic=0;gate(1,CS_UWIN_HANDOFF);
    setup();h->version=1;gate(1,CS_UWIN_HANDOFF);
    setup();h->image_base=UINT64_C(1)<<47;gate(1,CS_UWIN_HANDOFF);
    setup();h->image_base+=4096;gate(1,CS_UWIN_MAP);
    setup();h->span_count=1;gate(1,CS_UWIN_MAP);
    setup();h->spans[1].kind=1;gate(1,CS_UWIN_MAP);
    setup();bundle[CS_LOADER_MAP_OFFSET+24]=0;gate(1,CS_UWIN_MAP);
    setup();h->framebuffer.width=511;gate(1,CS_UWIN_TARGET);
    setup();h->framebuffer.format=3;gate(1,CS_UWIN_TARGET);
    setup();h->rsdp=0;gate(1,CS_UWIN_TIMER);
    setup();fx[FX_FADT+109]=0;fx_seal(fx+FX_FADT);gate(1,CS_UWIN_NO_8042);
    setup();clock_bad=1;gate(1,CS_UWIN_CLOCK);CHECK(reads==1u);
    setup();memory.window_base=(uintptr_t)devices.arena;gate(1,CS_UWIN_TIMER);
    setup();++cases;io.read=NULL;snapshot();
    CHECK(cs_native_window_loop(h,1,&binding,60,trace())==CS_UWIN_ARGUMENT);preserved(1);
    setup();++cases;binding.keyboard=(const cs_ps2_io *)devices.arena;
    memcpy(devices.arena,&io,sizeof io);snapshot();
    CHECK(cs_native_window_loop(h,1,&binding,60,trace())==CS_UWIN_ARGUMENT);preserved(1);
}
static void traffic(void)
{
    static const uint8_t bytes[]={9,0,0,8,0,0,0x18,120,93,9,0,0,0x29,20,246,8,0,0,
        0x28,72,184,9,0,0,8,0,0,0x29,0xF0,0x29,0x76,0xF0,0x76,8};
    memcpy(bus.live_bytes,bytes,sizeof bytes);bus.live_count=(unsigned)sizeof bytes;
    for(unsigned i=0;i<27u;++i)bus.live_flags[i]=0x20u;
    bus.live_flags[33]=0xE0u;
}
static void live(uint32_t seconds)
{
    setup();++cases;traffic();stop_ticks=UINT64_C(62)*CS_PMTIMER_HZ;
    CHECK(cs_native_window_loop(h,1,&binding,seconds,trace())==CS_UWIN_STOPPED);
    cs_pc_window_session *s=session();
    CHECK(s->phase==CS_PWS_STOPPED && u32(trace()+12)==CS_UWIN_DONE);
    CHECK(bus.ready && bus.live_at==bus.live_count && bus.key.violations==0u);
    CHECK(s->owner.product.inputs==2u && s->bus.left.presses==3u);
    CHECK(s->owner.window.x==100 && s->owner.window.y==80 && s->owner.window.pattern==1u);
    CHECK(s->owner.pointer.x==212 && s->owner.pointer.y==160 && s->owner.pointer.suppress==1u);
    /* Startup plus five visual updates: two moves, drag, title count, content toggle.
     * Held/release/cancel receipts with no pixel change now publish nothing. */
    CHECK(drained==4u && loss_seen==1u && published==6u);
    CHECK(u32(trace()+104)==6u && u32(trace()+108)==0u && u32(trace()+112)==0u);
    CHECK(u64(trace()+136)>=UINT64_C(62)*CS_PMTIMER_HZ && reads==u64(trace()+128)+1u);
    CHECK(u32(trace()+144)==((sizeof(*s)+_Alignof(max_align_t)-1u)&~(_Alignof(max_align_t)-1u))
        +CS_PWS_CANVAS+CS_PWS_PAYLOAD+4u*W);
    CHECK(fb[0]==0xFFu && fb[3]==0u);
    for(size_t y=0;y<H;++y)for(size_t x=W*4u;x<PITCH*4u;++x)CHECK(fb[y*PITCH*4u+x]==0xA5u);
    for(size_t i=FB_BYTES;i<(size_t)FB_PAGES*4096u;++i)CHECK(fb[i]==0xA5u);
#ifndef CS_UWIN_FAULTS
    FILE *out=fopen("window-trace-test.bin","wb");CHECK(out!=NULL);
    if(out){CHECK(fwrite(trace(),1,160,out)==160);CHECK(fclose(out)==0);}
#endif
}
int main(void)
{
    bundle_raw=calloc(1,CS_LOADER_BUNDLE+4095u);fb_raw=malloc((size_t)FB_PAGES*4096u+4095u);
    saved=malloc(CS_LOADER_ARENA_BYTES+FB_BYTES+CS_LOADER_TRACE_BYTES);
    if(!bundle_raw||!fb_raw||!saved)return 2;
    bundle=(unsigned char *)(((uintptr_t)bundle_raw+4095u)&~(uintptr_t)4095u);
    fb=(unsigned char *)(((uintptr_t)fb_raw+4095u)&~(uintptr_t)4095u);
    gates();live(1);live(60);live(3600);
    setup();++cases;stop_first=1;
    CHECK(cs_native_window_loop(h,1,&binding,60,trace())==CS_UWIN_STOPPED && reads==1u && turns==1u);
    setup();++cases;stop_first=1;h->image_base+=1;h->image_size-=2;
    CHECK(cs_native_window_loop(h,1,&binding,60,trace())==CS_UWIN_STOPPED);
    setup();++cases;freeze_ready=1;
    CHECK(cs_native_window_loop(h,1,&binding,60,trace())==CS_UWIN_CLOCK);
    CHECK(u32(trace()+48)==CS_PWS_CLOCK && u32(trace()+108)==0u && u32(trace()+112)==0u);
    setup();++cases;bus.bad_test=1;
    CHECK(cs_native_window_loop(h,1,&binding,60,trace())==CS_UWIN_SESSION);
    CHECK(u32(trace()+48)==CS_PWS_CONTROLLER && u32(trace()+104)==1u);
    setup();++cases;fail_clock_after_ready=1;
    CHECK(cs_native_window_loop(h,1,&binding,60,trace())==CS_UWIN_CLOCK);
    CHECK(u32(trace()+20)==CS_UWIN_CLOCK_MODULE && u32(trace()+56)==CS_PS2_DELIVERY_NONE);
    CHECK(u32(trace()+48)==CS_PWS_CLOCK && u32(trace()+108)==0u);
    setup();++cases;fail_clock_after_start=1;
    CHECK(cs_native_window_loop(h,1,&binding,60,trace())==CS_UWIN_CLOCK);
    CHECK(u32(trace()+108)==0u && u32(trace()+112)==0u && bus.key.writes==0u && reads==2u);
#ifdef CS_UWIN_FAULTS
    for(int mode=1;mode<=5;++mode){
        setup();++cases;injected=mode;
        CHECK(cs_native_window_loop(h,1,&binding,60,trace())==CS_UWIN_SESSION);
        CHECK(u32(trace()+12)==CS_UWIN_FAULT && u32(trace()+16)==CS_UWIN_SESSION);
        CHECK(bus.key.writes==0u && bus.mouse_writes==0u);
        if(mode==1)CHECK(u32(trace()+44)==CS_PWS_CANCEL && u32(trace()+108)==0u && reads==1u);
        if(mode==2 || mode==5)CHECK(u32(trace()+108)==CS_MONO_FAILED && u32(trace()+112)==5u && fb[0]==0x42u);
        if(mode==3)CHECK(u32(trace()+56)==CS_PS2_DELIVERY_PACKET && u32(trace()+60)==0u && u32(trace()+108)==0u);
        if(mode==4)CHECK(u32(trace()+64)==259u && u32(trace()+104)==1u && u32(trace()+108)==0u);
        if(mode==5)CHECK(u32(trace()+100)==2u && u32(trace()+104)==1u);
    }
#endif
    setup(); /* Explicit quiescence/abandonment before freeing borrowed storage. */
    printf("TEST-0073 %s: cases=%u checks=%u failures=%u\n",
#ifdef CS_UWIN_FAULTS
        "fault",
#else
        "binding",
#endif
        cases,checks,failures);
    free(saved);free(fb_raw);free(bundle_raw);return failures?1:0;
}
