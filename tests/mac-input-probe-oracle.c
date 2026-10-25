/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Frozen before worker artifact review. */
#include "platform/mac128k/input-probe.h"
#include <stdio.h>
int main(void)
{
    static const uint32_t cases[][4]={
        {0,0,0,0},{1,0,0,0},{4999,0,0,0},{5000,0,0,1},
        {5001,0,0,1},{UINT32_MAX,0,0,1},{0,1,0,1},{0,0,1,1},
        {4999,1,0,1},{4999,0,1,1},{0,UINT32_MAX,0,1},
        {0,0,UINT32_MAX,1},{0,1,1,1},{5000,UINT32_MAX,UINT32_MAX,1}};
    unsigned errors=0;
    for(unsigned i=0;i<sizeof cases/sizeof cases[0];++i){
        int got=cs_mac_first_input_ready(cases[i][0],cases[i][1],cases[i][2]);
        if(got!=(int)cases[i][3]){++errors;fprintf(stderr,"FAIL literal trigger %u\n",i);}
    }
    printf("14 frozen trigger cases, %u failures\n",errors);
    return errors?1:0;
}
