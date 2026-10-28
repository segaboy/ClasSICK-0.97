/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "activity-window.h"
int cs_mac_activity_window_begin(cs_mac_activity_window *s)
{
    if(!s)return 1;
    s->valid=CS_MAC_ACTIVITY_WINDOW_VALID;
    s->armed=s->first_service=s->last_service=0;
    return 0;
}
int cs_mac_activity_window_step(cs_mac_activity_window *s,uint32_t services,uint32_t activity)
{
    if(!s||activity>1u)return CS_MAC_AW_ARGUMENT;
    if(s->valid!=CS_MAC_ACTIVITY_WINDOW_VALID||s->armed>1u||
       s->first_service>s->last_service||(!s->armed&&s->first_service))
        return CS_MAC_AW_STATE;
    if(services<s->last_service)return CS_MAC_AW_ARGUMENT;
    if(activity&&!s->armed){s->armed=1;s->first_service=services;}
    s->last_service=services;
    if(services>=CS_MAC_ACTIVITY_WINDOW_MINIMUM&&s->armed&&services-s->first_service>=CS_MAC_ACTIVITY_WINDOW_DELAY)
        return CS_MAC_AW_COMPLETE;
    return services>=CS_MAC_ACTIVITY_WINDOW_FALLBACK?CS_MAC_AW_FALLBACK:CS_MAC_AW_WAIT;
}
