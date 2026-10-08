/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "state.h"

static void put(unsigned char *p,uint64_t value,unsigned bytes)
{
    for(unsigned i=0;i<bytes;++i) { p[i]=(unsigned char)value; value>>=8; }
}
cs_x64_result cs_x64_tables_init(void *storage,size_t capacity,const cs_x64_layout *layout)
{
    const uint64_t limit=UINT64_C(1)<<47;
    uint64_t base[7],size[7];
    if(storage==NULL || layout==NULL) return CS_X64_ARGUMENT;
    if(capacity<CS_X64_STATE_BYTES) return CS_X64_LIMIT;
    base[0]=layout->state_base; size[0]=CS_X64_STATE_BYTES;
    base[1]=layout->vector_base; size[1]=CS_X64_VECTOR_BYTES;
    if(base[0]%16!=0 || base[1]%32!=0) return CS_X64_LIMIT;
    for(unsigned i=0;i<5;++i) {
        uint64_t top=i==0?layout->stack_top:layout->ist_top[i-1];
        size[i+2]=i==0?CS_X64_KERNEL_STACK_BYTES:CS_X64_IST_BYTES;
        if(top>limit || top%4096!=0 || top<=size[i+2]) return CS_X64_LIMIT;
        base[i+2]=top-size[i+2];
    }
    for(unsigned i=0;i<7;++i) {
        if(base[i]==0 || base[i]>=limit || size[i]>limit-base[i]) return CS_X64_LIMIT;
        for(unsigned j=0;j<i;++j)
            if(base[i]<base[j]+size[j] && base[j]<base[i]+size[i]) return CS_X64_OVERLAP;
    }
    unsigned char *p=storage;
    /* Volatile byte loop also prevents a freestanding optimizer's memset helper. */
    volatile unsigned char *zero=storage;
    for(size_t i=0;i<CS_X64_STATE_BYTES;++i) zero[i]=0;
    put(p+8,UINT64_C(0x00AF9B000000FFFF),8);
    put(p+16,UINT64_C(0x00CF93000000FFFF),8);
    uint64_t tss=base[0]+CS_X64_TSS;
    put(p+24,103,2); put(p+26,tss,3); p[29]=0x89;
    p[31]=(unsigned char)(tss>>24); put(p+32,tss>>32,4);
    put(p+CS_X64_GDTR,39,2); put(p+CS_X64_GDTR+2,base[0],8);
    put(p+CS_X64_IDTR,4095,2); put(p+CS_X64_IDTR+2,base[0]+CS_X64_IDT,8);
    put(p+CS_X64_TSS+4,layout->stack_top,8);
    for(unsigned i=0;i<4;++i) put(p+CS_X64_TSS+36+i*8,layout->ist_top[i],8);
    put(p+CS_X64_TSS+102,104,2);
    for(unsigned vector=0;vector<256;++vector) {
        unsigned char *gate=p+CS_X64_IDT+vector*16;
        uint64_t address=base[1]+vector*UINT64_C(32);
        put(gate,address,2); put(gate+2,8,2);
        gate[4]=(unsigned char)(vector==2?2:(vector==8?3:(vector==18?4:1)));
        gate[5]=0x8E; put(gate+6,address>>16,2); put(gate+8,address>>32,4);
    }
    return CS_X64_OK;
}
