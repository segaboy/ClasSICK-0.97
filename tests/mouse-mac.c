/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Original authored register transcript; no real device access. */
#include "../core/input/mouse.h"
#include "../platform/mac128k/rom.h"
#include <stdio.h>
typedef struct {uint8_t button;uint32_t reads,writes;} transcript;
static uint8_t read_register(void *p,unsigned reg)
{
    transcript *t=p;++t->reads;
    return reg==0?t->button:0;
}
static void write_register(void *p,unsigned reg,uint8_t value)
{
    transcript *t=p;(void)reg;(void)value;++t->writes;
}
int main(void)
{
    for(uint32_t baseline=0;baseline<2;++baseline){
        for(uint32_t bits=0;bits<1024;++bits){
            transcript t={baseline?0:8,0,0};cs_mac_io io={&t,read_register,write_register};
            cs_mac_loop loop;cs_mouse_button button;
            cs_mac_loop_begin(&loop,t.button);
            if(cs_mouse_button_begin(&button,loop.mouse_down)!=CS_MOUSE_OK)return 1;
            uint32_t expected=0,held=baseline;
            for(uint32_t i=0;i<10;++i){
                uint32_t next=(bits>>i)&1u;t.button=(uint8_t)(next?0:8);
                if(next&&!held)++expected;
                held=next;(void)cs_mac_loop_poll(&loop,&io);
                int result=cs_mouse_button_sample(&button,loop.mouse_down);
                if(result>CS_MOUSE_CHANGED||button.presses!=expected||loop.presses!=expected||button.down!=held)return 2;
            }
            if(t.reads!=20||t.writes!=0)return 3;
        }
    }
    puts("native Mac/shared button transcript: PASS (2048 traces)");return 0;
}
