/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "iwm-write.h"
static const uint8_t alphabet[64]={
    0x96,0x97,0x9a,0x9b,0x9d,0x9e,0x9f,0xa6,
    0xa7,0xab,0xac,0xad,0xae,0xaf,0xb2,0xb3,
    0xb4,0xb5,0xb6,0xb7,0xb9,0xba,0xbb,0xbc,
    0xbd,0xbe,0xbf,0xcb,0xcd,0xce,0xcf,0xd3,
    0xd6,0xd7,0xd9,0xda,0xdb,0xdc,0xdd,0xde,
    0xdf,0xe5,0xe6,0xe7,0xe9,0xea,0xeb,0xec,
    0xed,0xee,0xef,0xf2,0xf3,0xf4,0xf5,0xf6,
    0xf7,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff
};
static void pack(const uint8_t *input,unsigned count,uint8_t *wire)
{
    unsigned i,top=0;
    for(i=0;i<3u;++i) top|=((unsigned)input[i]>>6)<<(4u-2u*i);
    wire[0]=alphabet[top];
    for(i=0;i<count;++i) wire[i+1u]=alphabet[input[i]&63u];
}
int cs_mac_iwm_encode_sector(uint16_t sector,const uint8_t *tags,
    const uint8_t *block,uint8_t *wire)
{
    unsigned produced=0,position=1;
    uint8_t sums[3]={0,0,0};
    if(sector>=12u || tags==NULL || block==NULL || wire==NULL) return 0;
    wire[0]=alphabet[sector];
    while(produced<CS_MAC_IWM_DECODED_BYTES) {
        unsigned count=CS_MAC_IWM_DECODED_BYTES-produced;
        unsigned carry=(unsigned)sums[2]>>7,i;
        uint8_t encoded[3];
        if(count>3u)count=3u;
        sums[2]=(uint8_t)(((unsigned)sums[2]<<1)|carry);
        for(i=0;i<count;++i) {
            uint8_t value=produced<12u?tags[produced]:block[produced-12u];
            unsigned previous=i==0u?2u:i-1u;
            unsigned total=(unsigned)sums[i]+value+carry;
            encoded[i]=(uint8_t)(value^sums[previous]);
            sums[i]=(uint8_t)total;carry=total>>8;++produced;
        }
        /* The short final group omits the third low symbol, but its two high
         * bits still encode the implicit zero XOR the updated second sum. */
        if(count==2u)encoded[2]=sums[1];
        pack(encoded,count,wire+position);position+=count+1u;
    }
    pack(sums,3u,wire+position);
    wire[704]=0xdeu;wire[705]=0xaau;
    return 1;
}
cs_mac_iwm_write_result cs_mac_iwm_write_one(cs_mac_iwm *provider,uint32_t lba,
    const uint8_t *block,uint8_t *scratch)
{
    uint16_t track=0;
    uint32_t relative=lba;
    int sent;
    if(provider==NULL || block==NULL || scratch==NULL || block==scratch ||
        lba>=CS_MAC_IWM_BLOCKS) return CS_MAC_IWM_WRITE_ARGUMENT;
    if(cs_mac_iwm_read_one(provider,lba,scratch)!=CS_BLOCK_IO_OK)
        return CS_MAC_IWM_WRITE_READ;
    while(relative>=(uint32_t)(12u-track/16u)) {
        relative-=(uint32_t)(12u-track/16u);++track;
    }
    if(!cs_mac_iwm_encode_sector((uint16_t)relative,provider->decoded,block,provider->wire))
        return CS_MAC_IWM_WRITE_ARGUMENT;
    sent=cs_mac_iwm_send(provider->wire);
    return sent==1?CS_MAC_IWM_WRITE_OK:
        sent==0?CS_MAC_IWM_WRITE_TIMEOUT:CS_MAC_IWM_WRITE_REJECTED;
}
