/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include <stdio.h>
#include <string.h>
#include "../platform/mac128k/activity-window.h"
static unsigned checks,failures;
#define K(c) do{++checks;if(!(c)){++failures;printf("FAIL:%d:%s\n",__LINE__,#c);}}while(0)
int main(int argc,char **argv)
{
    cs_mac_activity_window s,before;
    uint32_t delay,fallback,divisor;
    if(argc!=2)return 2;
    if(!strcmp(argv[1],"4")){delay=2048;fallback=200000;divisor=4;}
    else if(!strcmp(argv[1],"16")){delay=8192;fallback=800000;divisor=16;}
    else return 2;
    K(CLASSICK_MAC_TIMER_DIVISOR==divisor);
    K(CS_MAC_ACTIVITY_WINDOW_DELAY==delay&&CS_MAC_ACTIVITY_WINDOW_FALLBACK==fallback);
    K(cs_mac_activity_window_begin(&s)==0);
    K(cs_mac_activity_window_step(&s,1000,1)==CS_MAC_AW_WAIT);
    K(cs_mac_activity_window_step(&s,1000+delay-1u,1)==CS_MAC_AW_WAIT);
    K(s.first_service==1000u);
    K(cs_mac_activity_window_step(&s,1000+delay,0)==CS_MAC_AW_COMPLETE);
    K(cs_mac_activity_window_begin(&s)==0);
    K(cs_mac_activity_window_step(&s,fallback-1u,0)==CS_MAC_AW_WAIT);
    K(cs_mac_activity_window_step(&s,fallback,0)==CS_MAC_AW_FALLBACK);
    K(cs_mac_activity_window_begin(&s)==0);
    K(cs_mac_activity_window_step(&s,fallback-delay,1)==CS_MAC_AW_WAIT);
    K(cs_mac_activity_window_step(&s,fallback,1)==CS_MAC_AW_COMPLETE);
    K(cs_mac_activity_window_begin(&s)==0);
    K(cs_mac_activity_window_step(&s,fallback-1u,1)==CS_MAC_AW_WAIT);
    K(cs_mac_activity_window_step(&s,fallback,1)==CS_MAC_AW_FALLBACK);
    K(cs_mac_activity_window_step(&s,UINT32_MAX,1)==CS_MAC_AW_COMPLETE);
    memcpy(&before,&s,sizeof s);
    K(cs_mac_activity_window_step(&s,0,1)==CS_MAC_AW_ARGUMENT);
    K(!memcmp(&before,&s,sizeof s));
    printf("reload divisor%u:%u checks,%u failures\n",(unsigned)divisor,checks,failures);
    return failures?1:0;
}
