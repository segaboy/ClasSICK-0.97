/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_TEST_PS2_FIXTURE_H
#define CLASSICK_TEST_PS2_FIXTURE_H
#include "../platform/pc/ps2.h"
#include <string.h>
/* Original reactive hardware transcript, from SPEC-0012. No device is accessed. */
typedef struct {
    uint8_t bytes[512],flags[512],ports[128],values[128];
    unsigned head,tail,writes,status_reads,data_reads,violations;
    unsigned config_pending,live,blocked,disable_reply,resend_value,resend_left;
    uint8_t configuration,self_test,interface_test,bat;
    uint8_t live_bytes[64]; unsigned live_count;
} ps2_fixture;
static void pf_byte(ps2_fixture *f,uint8_t byte,uint8_t flags)
{
    if(f->tail>=512) { ++f->violations; return; }
    f->bytes[f->tail]=byte; f->flags[f->tail++]=flags;
}
static void pf_init(ps2_fixture *f)
{
    memset(f,0,sizeof *f); f->configuration=0x47; f->self_test=0x55; f->bat=0xAA;
}
static uint8_t pf_read(void *context,uint16_t port)
{
    ps2_fixture *f=context;
    if(port==0x64) {
        ++f->status_reads;
        return (uint8_t)((f->blocked?2u:0u)|(f->head<f->tail?1u|f->flags[f->head]:0u));
    }
    if(port!=0x60 || f->head>=f->tail) { ++f->violations; return 0; }
    ++f->data_reads;
    { uint8_t byte=f->bytes[f->head++];
      if(f->head==f->tail) f->head=f->tail=0;
      if(byte==0xFA && f->writes!=0 && f->values[f->writes-1]==0xF4 && !f->live) {
          f->live=1; for(unsigned i=0;i<f->live_count;++i) pf_byte(f,f->live_bytes[i],0);
      }
      return byte; }
}
static void pf_write(void *context,uint16_t port,uint8_t byte)
{
    ps2_fixture *f=context;
    if(f->blocked || f->writes>=128 || (port!=0x60 && port!=0x64)) { ++f->violations; return; }
    f->ports[f->writes]=(uint8_t)port; f->values[f->writes++]=byte;
    if(port==0x64) {
        if(byte==0x60) f->config_pending=1;
        else if(!f->disable_reply) {
            if(byte==0x20) pf_byte(f,f->configuration,0);
            else if(byte==0xAA) { f->configuration=0x47; pf_byte(f,f->self_test,0); }
            else if(byte==0xAB) pf_byte(f,f->interface_test,0);
        }
    } else if(f->config_pending) { f->configuration=byte; f->config_pending=0; }
    else if(!f->disable_reply) {
        if(byte==f->resend_value && f->resend_left!=0) { --f->resend_left; pf_byte(f,0xFE,0); }
        else { pf_byte(f,0xFA,0); if(byte==0xFF) pf_byte(f,f->bat,0); }
    }
}
static cs_ps2_io pf_io(ps2_fixture *f)
{
    cs_ps2_io io; io.read=pf_read; io.write=pf_write; io.context=f; return io;
}
#endif
