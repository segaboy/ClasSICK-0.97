/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "memory-registers.h"
static cs_mm_result finish(cs_mm_register_report *r,cs_mm_result status)
{r->status=status;return status;}
cs_mm_result cs_mm_register_call(cs_mm_manager *m,uint16_t word,unsigned interrupt_context,
    cs_trap_frame *frame,cs_mm_register_report *report)
{
    if(m==0 || frame==0 || report==0) return CS_MM_ARGUMENT;
    report->status=CS_MM_OK;report->word=word;report->reply.code=0;
    report->reply.value=0;report->reply.moved=0;
    cs_trap_word decoded;
    if(cs_trap_decode(word,&decoded)!=CS_TRAP_OK || decoded.toolbox) return finish(report,CS_MM_OPERATION);
    cs_mm_request q;q.address=frame->a[0];q.other=frame->a[1];q.size=frame->d[0];
    q.flags=decoded.flags;q.interrupt_context=interrupt_context;
    switch(decoded.number) {
        case 0x1E:q.operation=CS_MM_NEW_PTR;break;case 0x1F:q.operation=CS_MM_DISPOS_PTR;break;
        case 0x20:q.operation=CS_MM_SET_PTR_SIZE;break;case 0x21:q.operation=CS_MM_GET_PTR_SIZE;break;
        case 0x22:q.operation=CS_MM_NEW_HANDLE;break;case 0x23:q.operation=CS_MM_DISPOS_HANDLE;break;
        case 0x24:q.operation=CS_MM_SET_HANDLE_SIZE;break;case 0x25:q.operation=CS_MM_GET_HANDLE_SIZE;break;
        case 0x29:q.operation=CS_MM_HLOCK;break;case 0x2A:q.operation=CS_MM_HUNLOCK;break;
        case 0x2E:q.operation=CS_MM_BLOCK_MOVE;break;default:return finish(report,CS_MM_OPERATION);
    }
    cs_mm_result result=cs_mm_execute(m,&q,&report->reply);
    if(result!=CS_MM_OK) return finish(report,result);
    if(q.operation==CS_MM_GET_PTR_SIZE || q.operation==CS_MM_GET_HANDLE_SIZE)
        frame->d[0]=report->reply.code==0?report->reply.value:(uint32_t)(int32_t)report->reply.code;
    else frame->d[0]=(frame->d[0]&0xFFFF0000u)|(uint16_t)report->reply.code;
    if(decoded.a0_result && (q.operation==CS_MM_NEW_PTR || q.operation==CS_MM_NEW_HANDLE)) frame->a[0]=report->reply.value;
    uint16_t low=(uint16_t)frame->d[0];
    frame->sr=(uint16_t)((frame->sr&0xFFF0u)|(low==0?4u:((low&0x8000u)?8u:0u)));
    return CS_MM_OK;
}
