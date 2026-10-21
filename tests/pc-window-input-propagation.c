/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../platform/pc/window-input.h"
static unsigned calls,checks;
static int result;
static cs_pointer_sample seen;
#define CHECK(x) do { ++checks; if(!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); exit(1); } } while(0)
int cs_test_window_input_sample(cs_window_input *owner,const cs_pointer_sample *sample)
{ CHECK(owner!=NULL); ++calls; seen=*sample; return result; }
int main(void)
{
    cs_window_input owner,saved; memset(&owner,0xA5,sizeof(owner)); saved=owner;
    for(unsigned buttons=0;buttons<8u;++buttons) for(result=0;result<=5;++result) {
        const cs_ps2_mouse_packet raw={-256,255,buttons}; unsigned before=calls;
        CHECK(cs_pc_window_packet(&owner,&raw)==result);
        CHECK(calls==before+1u && seen.dx==-256 && seen.dy==-255 && seen.buttons==buttons);
        CHECK(memcmp(&owner,&saved,sizeof(owner))==0);
    }
    const cs_ps2_mouse_packet raw={255,-256,7u}; result=CS_WINDOW_INPUT_FAULT;
    CHECK(cs_pc_window_packet(&owner,&raw)==CS_WINDOW_INPUT_FAULT);
    CHECK(seen.dx==255 && seen.dy==256 && seen.buttons==7u);
    unsigned before=calls;
    const cs_ps2_mouse_packet bad={0,INT32_MIN,0};
    CHECK(cs_pc_window_packet(&owner,&bad)==CS_WINDOW_INPUT_ARGUMENT && calls==before);
    printf("%u boundary checks/%u single deliveries PASS\n",checks,calls); return 0;
}
