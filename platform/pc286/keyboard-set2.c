/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "keyboard-set2.h"

int cs_pc286_set2_begin(cs_pc286_set2_pointer *s)
{
    if(!s)return 0;
    s->held=0;s->release=0;s->extended=0;s->pause=0;return 1;
}

int cs_pc286_set2_feed(cs_pc286_set2_pointer *s,uint8_t b,cs_pointer_sample *out)
{
    cs_pointer_sample next;unsigned release,extended;
    if(!s || !out || s->held>1u || s->release>1u || s->extended>1u || s->pause>7u)return -1;
    if(b==0u || b==0xffu || b==0xfcu || b==0xfdu)return -1;
    if(s->pause) {--s->pause;return 0;}
    if(b==0xe1u) {s->pause=7;s->release=0;s->extended=0;return 0;}
    if(b==0xe0u) {s->extended=1;return 0;}
    if(b==0xf0u) {s->release=1;return 0;}
    release=s->release;extended=s->extended;s->release=0;s->extended=0;
    next.dx=0;next.dy=0;next.buttons=s->held;
    if(b==0x29u && !extended) {
        if(release!=s->held)return 0;
        s->held=release?0u:1u;next.buttons=s->held;*out=next;return 1;
    }
    if(release)return 0;
    if(b==0x6bu)next.dx=-16;
    else if(b==0x74u)next.dx=16;
    else if(b==0x75u)next.dy=-16;
    else if(b==0x72u)next.dy=16;
    else return 0;
    *out=next;return 1;
}

int cs_pc286_set1_feed(cs_pc286_set2_pointer *s,uint8_t b,cs_pointer_sample *out)
{
    cs_pointer_sample next;unsigned release,extended,code;
    if(!s || !out || s->held>1u || s->release>1u || s->extended>1u || s->pause>7u)return -1;
    if(b==0u || b==0xffu || b==0xfcu || b==0xfdu)return -1;
    if(s->pause) {--s->pause;return 0;}
    if(b==0xe1u) {s->pause=5;s->release=0;s->extended=0;return 0;}
    if(b==0xe0u) {s->extended=1;return 0;}
    release=(b&0x80u)!=0u;code=b&0x7fu;extended=s->extended;s->extended=0;s->release=0;
    next.dx=0;next.dy=0;next.buttons=s->held;
    if(code==0x39u && !extended) {
        if(release!=s->held)return 0;
        s->held=release?0u:1u;next.buttons=s->held;*out=next;return 1;
    }
    if(release)return 0;
    if(code==0x4bu)next.dx=-16;
    else if(code==0x4du)next.dx=16;
    else if(code==0x48u)next.dy=-16;
    else if(code==0x50u)next.dy=16;
    else return 0;
    *out=next;return 1;
}
