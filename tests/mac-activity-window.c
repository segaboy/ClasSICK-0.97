/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include <stdio.h>
#include <string.h>
#include "../platform/mac128k/activity-window.h"
static unsigned checks,failures;
#define K(c) do {++checks;if(!(c)){++failures;printf("FAIL:%d:%s\n",__LINE__,#c);}} while(0)
int main(void)
{
    cs_mac_activity_window s,before;
    K(cs_mac_activity_window_begin(NULL)==1);
    memset(&s,0xA5,sizeof s);K(cs_mac_activity_window_begin(&s)==0);
    K(s.valid==CS_MAC_ACTIVITY_WINDOW_VALID&&!s.armed&&!s.first_service&&!s.last_service);
    K(cs_mac_activity_window_step(&s,49999,0)==CS_MAC_AW_WAIT);
    K(cs_mac_activity_window_step(&s,50000,0)==CS_MAC_AW_FALLBACK);
    K(!s.armed&&!s.first_service);
    K(cs_mac_activity_window_step(&s,UINT32_MAX,0)==CS_MAC_AW_FALLBACK);
    K(cs_mac_activity_window_begin(&s)==0);
    K(cs_mac_activity_window_step(&s,17817,1)==CS_MAC_AW_WAIT);
    K(s.first_service==17817u&&s.armed==1u);
    K(cs_mac_activity_window_step(&s,18328,1)==CS_MAC_AW_WAIT);
    K(s.first_service==17817u); /* Repeated activity cannot postpone completion. */
    K(cs_mac_activity_window_step(&s,18329,0)==CS_MAC_AW_COMPLETE);
    K(cs_mac_activity_window_step(&s,18329,1)==CS_MAC_AW_COMPLETE);
    K(cs_mac_activity_window_begin(&s)==0);
    K(cs_mac_activity_window_step(&s,0,1)==CS_MAC_AW_WAIT);
    K(cs_mac_activity_window_step(&s,499,1)==CS_MAC_AW_WAIT);
    K(cs_mac_activity_window_step(&s,511,0)==CS_MAC_AW_WAIT);
    K(cs_mac_activity_window_step(&s,512,0)==CS_MAC_AW_COMPLETE);
    K(cs_mac_activity_window_begin(&s)==0);
    K(cs_mac_activity_window_step(&s,49488,1)==CS_MAC_AW_WAIT);
    K(cs_mac_activity_window_step(&s,50000,1)==CS_MAC_AW_COMPLETE);
    K(cs_mac_activity_window_begin(&s)==0);
    K(cs_mac_activity_window_step(&s,49999,1)==CS_MAC_AW_WAIT);
    K(cs_mac_activity_window_step(&s,50000,1)==CS_MAC_AW_FALLBACK);
    K(cs_mac_activity_window_begin(&s)==0);
    K(cs_mac_activity_window_step(&s,UINT32_MAX-1u,1)==CS_MAC_AW_FALLBACK);
    K(cs_mac_activity_window_step(&s,UINT32_MAX,1)==CS_MAC_AW_FALLBACK);
    memcpy(&before,&s,sizeof s);
    K(cs_mac_activity_window_step(&s,0,1)==CS_MAC_AW_ARGUMENT);
    K(!memcmp(&s,&before,sizeof s));
    K(cs_mac_activity_window_step(&s,UINT32_MAX,2)==CS_MAC_AW_ARGUMENT);
    K(!memcmp(&s,&before,sizeof s));
    K(cs_mac_activity_window_step(NULL,0,0)==CS_MAC_AW_ARGUMENT);
    s.valid=0;memcpy(&before,&s,sizeof s);
    K(cs_mac_activity_window_step(&s,UINT32_MAX,0)==CS_MAC_AW_STATE);
    K(!memcmp(&s,&before,sizeof s));
    K(cs_mac_activity_window_begin(&s)==0);s.armed=2;
    K(cs_mac_activity_window_step(&s,0,0)==CS_MAC_AW_STATE);
    K(cs_mac_activity_window_begin(&s)==0);s.first_service=1;
    K(cs_mac_activity_window_step(&s,1,0)==CS_MAC_AW_STATE);
    K(cs_mac_activity_window_begin(&s)==0);s.armed=1;s.first_service=2;s.last_service=1;
    K(cs_mac_activity_window_step(&s,2,0)==CS_MAC_AW_STATE);
    printf("activity observation window:%u checks,%u failures\n",checks,failures);
    return failures?1:0;
}
