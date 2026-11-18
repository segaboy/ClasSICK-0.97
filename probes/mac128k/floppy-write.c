/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../../platform/mac128k/iwm-write.h"
#ifndef PROBE_CASE
#define PROBE_CASE 0
#endif
#ifndef EXPECT_TAGS
#define EXPECT_TAGS 0
#endif
volatile uint32_t cs_mac_diag[8];
static cs_mac_iwm provider;
static cs_block_device device;
static uint8_t block[514],scratch[514];
static const uint16_t lbas[]={0,191,192,367,368,527,528,671,672,799,2};
static uint8_t old_byte(uint32_t lba,unsigned offset)
{return (uint8_t)(lba*37u+offset*29u+(offset>>3));}
static uint8_t new_byte(uint32_t lba,unsigned offset)
{return (uint8_t)(lba*19u+offset*13u+(offset>>2)+0x5bu);}
static int probe(void)
{
    unsigned i,j;
    if(!cs_mac_iwm_bind(&provider,&device))return 0;
    for(j=0;j<514;++j){block[j]=0xac;scratch[j]=0xbd;}
    if(cs_mac_iwm_write_one(&provider,800,block+1,scratch+1)!=CS_MAC_IWM_WRITE_ARGUMENT || provider.initialized)return 0;
    for(i=0;i<sizeof lbas/sizeof lbas[0];++i) {
        uint32_t lba=lbas[i];
        cs_mac_diag[1]=i+1u;
        for(j=0;j<512;++j)block[j+1]=new_byte(lba,j);
#if PROBE_CASE == 0 || PROBE_CASE == 2
        {
            cs_mac_iwm_write_result result=cs_mac_iwm_write_one(&provider,lba,block+1,scratch+1);
            cs_mac_diag[2]=(uint32_t)result;
            if(result!=(PROBE_CASE==2?CS_MAC_IWM_WRITE_REJECTED:CS_MAC_IWM_WRITE_OK))return 0;
        }
#elif PROBE_CASE == 3
        if(cs_mac_iwm_read_one(&provider,lba,scratch+1)!=CS_BLOCK_IO_OK)return 0;
        if(!cs_mac_iwm_encode_sector(0,provider.decoded,block+1,provider.wire))return 0;
        provider.wire[701]=provider.wire[701]==0x96u?0x97u:0x96u;
        if(cs_mac_iwm_send(provider.wire)!=-1)return 0;
#endif
        if(cs_mac_iwm_read_one(&provider,lba,scratch+1)!=CS_BLOCK_IO_OK)return 0;
        for(j=0;j<512;++j) {
            uint8_t expected=(PROBE_CASE==2 || PROBE_CASE==3)?old_byte(lba,j):new_byte(lba,j);
            if(scratch[j+1]!=expected || block[j+1]!=new_byte(lba,j))return 0;
        }
        for(j=0;j<12;++j) {
            uint8_t expected=EXPECT_TAGS?(uint8_t)(lba*11u+j*7u+3u):0;
            if(provider.decoded[j]!=expected)return 0;
        }
        if(block[0]!=0xac || block[513]!=0xac || scratch[0]!=0xbd || scratch[513]!=0xbd)return 0;
        ++cs_mac_diag[3];
#if PROBE_CASE == 3
        break;
#endif
    }
    return 1;
}
void cs_mac_main(void)
{
    int success=probe();cs_mac_diag[4]=provider.reads;cs_mac_diag[5]=provider.track;
    cs_mac_diag[6]=(uint32_t)provider.error;cs_mac_diag[0]=success?0xb8u:0xf8u;
    for(;;){}
}
void cs_mac_line_a_handler(void){cs_mac_diag[7]=0xf002u;for(;;){}}
