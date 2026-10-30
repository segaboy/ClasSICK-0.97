/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Short CSPSEQ01 build: halts after CLASSICK_PC_INPUT_SEQUENCE_LENGTH=3 bytes;
 * unused record bytes and flags are zero. */
#include "../platform/pc/input-probe.h"
#include <setjmp.h>
#include <stdio.h>
#include <string.h>
#if (CLASSICK_PC_INPUT_PROBE_SEQUENCE+0)!=1 || (CLASSICK_PC_INPUT_SEQUENCE_LENGTH+0)!=3
#error build with CLASSICK_PC_INPUT_PROBE_SEQUENCE=1 CLASSICK_PC_INPUT_SEQUENCE_LENGTH=3
#endif
static jmp_buf done;
static uint64_t actual[8];
static unsigned checks;
static cs_pc_window_session s;
static cs_pc_input_sequence rec;
#define C(x) do {++checks;if(!(x)){printf("sequence short FAIL:%d:%s\n",__LINE__,#x);return 1;}}while(0)
__attribute__((noreturn)) void cs_pc_input_probe_halt(const uint64_t values[8])
{memcpy(actual,values,sizeof(actual));longjmp(done,1);}
int main(void)
{
    static unsigned i, halted;
    C(sizeof rec.bytes==3u);
    C(cs_pc_input_sequence_begin(&rec)==0);
    s.bus.phase=CS_PS2_BUS_READY; s.bus.result=CS_PS2_READY;
    C(cs_pc_input_probe_observe(&rec,0,&s)==0);
    if(setjmp(done)==0){
        for(i=0;i<3u;++i){
            s.bus.last_byte=0xC8u+i; ++s.bus.auxiliary; if(i==1u) ++s.bus.errors;
            C(cs_pc_input_probe_observe(&rec,0,&s)==0);
        }
    } else halted=1;
    C(halted && i==2u);
    C(actual[0]==UINT64_C(0x4353505345513031));
    C(actual[1]==(UINT64_C(2)|(UINT64_C(3)<<56)));
    C(actual[2]==UINT64_C(0x0000000000CAC9C8));
    C(actual[3]==0 && actual[4]==0 && actual[5]==0 && actual[6]==0 && actual[7]==0);
    printf("sequence short:%u assertions\n",checks);
    return 0;
}
