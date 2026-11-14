/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../platform/pc286/keyboard-config.h"
#include <stdio.h>
#include <string.h>

int main(void)
{
    cs_pc286_keyboard_config s,before;unsigned i;
    memset(&s,0x5a,sizeof(s));before=s;
    if(cs_pc286_keyboard_config_begin(NULL,1)!=CS_286_CFG_STATE ||
       cs_pc286_keyboard_config_begin(&s,0)!=CS_286_CFG_STATE || memcmp(&s,&before,sizeof(s)))return 1;
    if(cs_pc286_keyboard_config_begin(&s,3)!=CS_286_CFG_WAIT)return 2;
    if(cs_pc286_keyboard_config_step(&s,2)!=CS_286_CFG_WAIT ||
       cs_pc286_keyboard_config_step(&s,0)!=CS_286_CFG_SEND ||
       cs_pc286_keyboard_config_step(&s,0)!=CS_286_CFG_WAIT ||
       cs_pc286_keyboard_config_step(&s,1)!=CS_286_CFG_READ ||
       cs_pc286_keyboard_config_reply(&s,0x40)!=CS_286_CFG_READY || s.value!=0x40 ||
       cs_pc286_keyboard_config_step(&s,1)!=CS_286_CFG_READY)return 3;
    for(i=0;i<256u;++i) {
        int expected=(i&0x50u)==0x40u?CS_286_CFG_READY:CS_286_CFG_INCOMPATIBLE;
        if(cs_pc286_keyboard_config_begin(&s,1)!=CS_286_CFG_WAIT ||
           cs_pc286_keyboard_config_step(&s,0)!=CS_286_CFG_SEND ||
           cs_pc286_keyboard_config_step(&s,1)!=CS_286_CFG_READ ||
           cs_pc286_keyboard_config_reply(&s,(uint8_t)i)!=expected || s.value!=(uint8_t)i)return 4;
    }
    if(cs_pc286_keyboard_config_begin(&s,3)!=CS_286_CFG_WAIT ||
       cs_pc286_keyboard_config_step(&s,1)!=CS_286_CFG_PENDING ||
       cs_pc286_keyboard_config_step(&s,0)!=CS_286_CFG_PENDING)return 5;
    if(cs_pc286_keyboard_config_begin(&s,2)!=CS_286_CFG_WAIT ||
       cs_pc286_keyboard_config_step(&s,2)!=CS_286_CFG_WAIT ||
       cs_pc286_keyboard_config_step(&s,2)!=CS_286_CFG_WAIT ||
       cs_pc286_keyboard_config_step(&s,0)!=CS_286_CFG_TIMEOUT)return 6;
    if(cs_pc286_keyboard_config_begin(&s,2)!=CS_286_CFG_WAIT ||
       cs_pc286_keyboard_config_step(&s,0)!=CS_286_CFG_SEND ||
       cs_pc286_keyboard_config_step(&s,0)!=CS_286_CFG_WAIT ||
       cs_pc286_keyboard_config_step(&s,0)!=CS_286_CFG_WAIT ||
       cs_pc286_keyboard_config_step(&s,1)!=CS_286_CFG_TIMEOUT)return 7;
    if(cs_pc286_keyboard_config_begin(&s,2)!=CS_286_CFG_WAIT ||
       cs_pc286_keyboard_config_step(&s,0x80)!=CS_286_CFG_STATUS)return 8;
    if(cs_pc286_keyboard_config_begin(&s,2)!=CS_286_CFG_WAIT ||
       cs_pc286_keyboard_config_step(&s,0)!=CS_286_CFG_SEND ||
       cs_pc286_keyboard_config_step(&s,0x21)!=CS_286_CFG_STATUS)return 9;
    if(cs_pc286_keyboard_config_begin(&s,2)!=CS_286_CFG_WAIT ||
       cs_pc286_keyboard_config_step(&s,0)!=CS_286_CFG_SEND ||
       cs_pc286_keyboard_config_step(&s,1)!=CS_286_CFG_READ ||
       cs_pc286_keyboard_config_step(&s,1)!=CS_286_CFG_STATE ||
       cs_pc286_keyboard_config_reply(&s,0x40)!=CS_286_CFG_STATE)return 10;
    s.phase=0;s.result=CS_286_CFG_READY;before=s;
    if(cs_pc286_keyboard_config_step(&s,0)!=CS_286_CFG_STATE || memcmp(&s,&before,sizeof(s)) ||
       cs_pc286_keyboard_config_reply(NULL,0)!=CS_286_CFG_STATE)return 11;
    puts("PASS 256 configurations, pending/status/refusal and bounded timeout checks");return 0;
}
