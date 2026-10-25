/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include <stdio.h>
#include "../platform/mac128k/input-probe.h"

struct tc{uint32_t s,l,d;int want;};
static const struct tc cases[]={
    {0u,0u,0u,0},{1u,0u,0u,0},{499u,0u,0u,0},{500u,0u,0u,0},
    {4999u,0u,0u,0},{5000u,0u,0u,1},{5001u,0u,0u,1},
    {UINT32_MAX,0u,0u,1},
    {0u,1u,0u,1},{0u,0u,1u,1},{0u,1u,1u,1},
    {4999u,1u,0u,1},{4999u,0u,1u,1},{4999u,1u,1u,1},
    {5000u,1u,1u,1},{UINT32_MAX,1u,1u,1},
    {0u,UINT32_MAX,0u,1},{0u,0u,UINT32_MAX,1},
    {0u,UINT32_MAX,UINT32_MAX,1},
    {UINT32_MAX,UINT32_MAX,UINT32_MAX,1},
};

int main(void)
{
    unsigned i,bad=0;
    if(CS_MAC_FIRST_INPUT_LIMIT!=5000u){puts("FAIL limit");return 1;}
    for(i=0;i<sizeof cases/sizeof cases[0];i++){
        int got=cs_mac_first_input_ready(cases[i].s,cases[i].l,cases[i].d)!=0;
        if(got!=cases[i].want){
            printf("FAIL case %u s=%lu l=%lu d=%lu want=%d got=%d\n",i,
                (unsigned long)cases[i].s,(unsigned long)cases[i].l,
                (unsigned long)cases[i].d,cases[i].want,got);
            bad++;
        }
    }
    if(bad)return 1;
    puts("mac-input-probe: ok");
    return 0;
}
