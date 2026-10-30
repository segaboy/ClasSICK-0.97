/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* End-to-end host check of the CSPSEQ01 recorder over the unchanged SPEC-0040
 * bus and the authored reactive 8042 fixture. The loop mirrors the native
 * window loop's order: observe, clear delivery, poll once, drain. Optional
 * argv[1]: directory for registers.txt (r8..r15 dump) and truth.txt (one line
 * per READY AUX byte: byte, delivery kind, dx, dy, buttons, error step), used
 * by the reader's conformance test. */
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include "../platform/pc/input-probe.h"
#include "ps2-bus-fixture.h"
#if (CLASSICK_PC_INPUT_PROBE_SEQUENCE+0)!=1
#error build with CLASSICK_PC_INPUT_PROBE_SEQUENCE=1
#endif
static jmp_buf done;
static uint64_t actual[8];
static unsigned checks;
#define CHECK(x) do { ++checks; if(!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); exit(1); } } while(0)
__attribute__((noreturn)) void cs_pc_input_probe_halt(const uint64_t values[8])
{ memcpy(actual,values,sizeof actual); longjmp(done,1); }

static cs_pc_window_session s;
static cs_pc_input_sequence rec;
static bus_fixture hw;
static cs_ps2_io io;
typedef struct { uint8_t byte; uint32_t kind; int32_t dx, dy; uint32_t buttons, err; } truth;
static truth seen[96];
static unsigned nseen;
static uint8_t fed[96];
static unsigned nfed;

static void aux(uint8_t b) { pf_byte(&hw.key,b,0x20); fed[nfed++]=b; }
static void aux_bad(uint8_t b) { pf_byte(&hw.key,b,0x20|0x80); fed[nfed++]=b; }
static void report(uint8_t h, uint8_t x, uint8_t y) { aux(h); aux(x); aux(y); }

int main(int argc, char **argv)
{
    static uint64_t tick;
    static unsigned turns;
    unsigned i, halted=0;
    cs_input_record record;
    memset(&s,0,sizeof s);
    bf_init(&hw); io=bf_io(&hw);
    CHECK(cs_input_init(&s.queue,s.records,CS_PWS_QUEUE_CAPACITY)==CS_INPUT_OK);
    CHECK(cs_ps2_bus_begin(&s.bus)==CS_PS2_ACTIVE);
    CHECK(cs_pc_input_sequence_begin(&rec)==0);

    /* Stream fed once READY: 48 recorded AUX bytes plus a tail that must not be
     * needed. Interleaved keyboard bytes must be ignored. */
    if(setjmp(done)==0) {
        for(;;) {
            uint32_t aux_before=s.bus.auxiliary, err_before=s.bus.errors, r;
            uint32_t was_ready=s.bus.phase==CS_PS2_BUS_READY && s.bus.result==CS_PS2_READY;
            CHECK(cs_pc_input_probe_observe(&rec,NULL,&s)==0);
            s.delivery.kind=CS_PS2_DELIVERY_NONE;
            s.delivery.packet.dx=0; s.delivery.packet.dy=0; s.delivery.packet.buttons=0;
            tick+=1000u;
            r=cs_ps2_bus_poll_delivery(&s.bus,&io,tick,&s.queue,&s.delivery);
            CHECK(r<=CS_PS2_READY);
            while(cs_input_pop(&s.queue,&record)==CS_INPUT_OK) { }
            if(s.bus.result==CS_PS2_READY && nfed==0) {
                report(0x08,0x05,0x03);                  /* +5,+3 */
                pf_byte(&hw.key,0x1E,0);                 /* keyboard make, between reports */
                report(0x48,0xFF,0x00);                  /* X overflow: whole report dropped */
                aux(0x18); pf_byte(&hw.key,0x9E,0); aux(0xF0); aux(0x02);  /* -16,+2, keyboard inside */
                aux(0x01);                               /* bit 3 clear at sync: dropped */
                aux_bad(0x08);                           /* parity/timeout status: lost */
                report(0x09,0x00,0x00);                  /* left down */
                report(0x28,0x7F,0x81);                  /* +127,-127 */
                report(0x88,0x01,0x01);                  /* Y overflow: dropped */
                for(i=0;i<9u;++i) report(0x08,0x01,0x00);
                aux(0x08);                               /* byte 48: incomplete header */
                aux(0x02); aux(0x00);                    /* tail, never recorded */
                CHECK(nfed==50);
            }
            if(was_ready && s.bus.phase==CS_PS2_BUS_READY && s.bus.auxiliary!=aux_before) {
                CHECK(s.bus.auxiliary-aux_before==1u && nseen<96u);
                seen[nseen].byte=(uint8_t)s.bus.last_byte; seen[nseen].kind=s.delivery.kind;
                seen[nseen].dx=s.delivery.packet.dx; seen[nseen].dy=s.delivery.packet.dy;
                seen[nseen].buttons=s.delivery.packet.buttons;
                seen[nseen].err=s.bus.errors-err_before; ++nseen;
            }
            CHECK(++turns<5000u);
        }
    } else halted=1;

    CHECK(halted && !hw.key.violations);
    CHECK(nseen==48u);                                   /* halted at the observation after byte 48 */
    for(i=0;i<48u;++i) {
        CHECK(seen[i].byte==fed[i]);
        CHECK(rec.bytes[i]==fed[i]);
        CHECK((((rec.flags>>i)&1u)!=0u)==(seen[i].err!=0u));
    }
    CHECK(rec.gaps==0u && rec.count==48u);
    CHECK(actual[0]==CS_PC_INPUT_PROBE_SEQUENCE_MAGIC);
    CHECK((actual[1]>>56)==48u && ((actual[1]>>48)&255u)==0u);
    CHECK((actual[1]&((UINT64_C(1)<<48)-1u))==rec.flags);
    for(i=0;i<48u;++i) CHECK(((actual[2u+i/8u]>>((i%8u)*8u))&255u)==fed[i]);
    /* Literal expectations for the decoder's verdicts on this stream. */
    CHECK(seen[2].kind==CS_PS2_DELIVERY_PACKET && seen[2].dx==5 && seen[2].dy==3);
    CHECK(seen[5].kind==CS_PS2_DELIVERY_LOST && seen[5].err==1u);          /* X overflow */
    CHECK(seen[8].kind==CS_PS2_DELIVERY_PACKET && seen[8].dx==-16 && seen[8].dy==2);
    CHECK(seen[9].kind==CS_PS2_DELIVERY_LOST && seen[10].kind==CS_PS2_DELIVERY_LOST);
    CHECK(seen[13].kind==CS_PS2_DELIVERY_PACKET && seen[13].buttons==1u);
    CHECK(seen[16].dx==127 && seen[16].dy==-127);
    CHECK(seen[19].kind==CS_PS2_DELIVERY_LOST);                              /* Y overflow */
    CHECK(rec.flags==((UINT64_C(1)<<5)|(UINT64_C(1)<<9)|(UINT64_C(1)<<10)|(UINT64_C(1)<<19)));

    if(argc>1) {
        char path[512]; FILE *f;
        snprintf(path,sizeof path,"%s/registers.txt",argv[1]); f=fopen(path,"w"); CHECK(f!=NULL);
        for(i=0;i<8u;++i) fprintf(f,"r%u=%016llx\n",8u+i,(unsigned long long)actual[i]);
        fclose(f);
        snprintf(path,sizeof path,"%s/truth.txt",argv[1]); f=fopen(path,"w"); CHECK(f!=NULL);
        for(i=0;i<48u;++i) fprintf(f,"%02x %u %d %d %u %u\n",seen[i].byte,seen[i].kind,
            seen[i].dx,seen[i].dy,seen[i].buttons,seen[i].err);
        fclose(f);
    }
    printf("sequence bus:%u assertions,%u turns\n",checks,turns);
    return 0;
}
