/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CS_MAC_CAPTURE_QUEUE_H
#define CS_MAC_CAPTURE_QUEUE_H
#include <stdint.h>
/* Caller excludes interrupts for foreground access. No hidden atomics. */
#ifndef CLASSICK_MAC_BURST_CAPTURE
#define CLASSICK_MAC_BURST_CAPTURE 0
#endif
#if CLASSICK_MAC_BURST_CAPTURE != 0 && CLASSICK_MAC_BURST_CAPTURE != 1
#error CLASSICK_MAC_BURST_CAPTURE must be 0 or 1
#endif
#ifndef CLASSICK_MAC_BURST_CAPACITY
#define CLASSICK_MAC_BURST_CAPACITY 512
#endif
#if CLASSICK_MAC_BURST_CAPACITY != 512 && CLASSICK_MAC_BURST_CAPACITY != 1024 && CLASSICK_MAC_BURST_CAPACITY != 2048
#error CLASSICK_MAC_BURST_CAPACITY must be 512, 1024 or 2048
#endif
enum { CS_MAC_CAPTURE_CAPACITY = CLASSICK_MAC_BURST_CAPTURE ? CLASSICK_MAC_BURST_CAPACITY : 128,
       CS_MAC_CAPTURE_BATCH = CLASSICK_MAC_BURST_CAPTURE ? 128 : 32 };
typedef struct { uint32_t raw[4], late; } cs_mac_capture;
typedef struct {
    uint32_t head, count, overflow, peak, losses;
    cs_mac_capture records[CS_MAC_CAPTURE_CAPACITY];
} cs_mac_capture_queue;
/* Five aligned word transfers, avoiding the freestanding byte-copy helper
 * inside interrupt exclusion. Volatile constrains only these accesses;
 * the caller's exclusion still provides synchronization. */
static inline void cs_mac_capture_copy(cs_mac_capture *to,const cs_mac_capture *from)
{
    volatile uint32_t *d=to->raw;
    const volatile uint32_t *s=from->raw;
    d[0]=s[0];d[1]=s[1];d[2]=s[2];d[3]=s[3];
    to->late=from->late;
}
static inline void cs_mac_capture_clear(cs_mac_capture_queue *q)
{ q->head=0; q->count=0; q->overflow=0; }
static inline void cs_mac_capture_push(cs_mac_capture_queue *q, const cs_mac_capture *v)
{
    uint32_t slot;
    if(q->overflow)return;
    if(q->count==CS_MAC_CAPTURE_CAPACITY){
        q->overflow=1;
        if(q->losses!=UINT32_MAX)q->losses++;
        return;
    }
    slot=(q->head+q->count)&(CS_MAC_CAPTURE_CAPACITY-1u);
    cs_mac_capture_copy(&q->records[slot],v); q->count++;
    if(q->count>q->peak)q->peak=q->count;
}
/* Overflow invalidates all queued observations before a new baseline. */
static inline int cs_mac_capture_pop(cs_mac_capture_queue *q, cs_mac_capture *v)
{
    if(q->overflow){cs_mac_capture_clear(q);return -1;}
    if(!q->count)return 0;
    cs_mac_capture_copy(v,&q->records[q->head]);q->head=(q->head+1u)&(CS_MAC_CAPTURE_CAPACITY-1u);
    q->count--;return 1;
}
#endif
