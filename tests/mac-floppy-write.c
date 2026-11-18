/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../platform/mac128k/iwm-write.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do { ++checks; if(!(x)){fprintf(stderr,"line%d\n",__LINE__);return 1;} } while(0)
static unsigned checks;
int cs_mac_iwm_capture(uint8_t *a,uint8_t *w,const uint8_t *e)
{(void)a;(void)w;(void)e;return 0;}
int cs_mac_iwm_send(const uint8_t *w){(void)w;return 0;}
int main(void)
{
    static const uint8_t symbols[12]={0x96,0x97,0x9a,0x9b,0x9d,0x9e,0x9f,0xa6,0xa7,0xab,0xac,0xad};
    uint8_t block[512],tags[12],wire[708],decoded[526],address[7];
    unsigned seed,sector,i;
    memset(wire,0xac,sizeof wire);
    CHECK(!cs_mac_iwm_encode_sector(12,tags,block,wire+1));
    CHECK(!cs_mac_iwm_encode_sector(0,NULL,block,wire+1));
    CHECK(!cs_mac_iwm_encode_sector(0,tags,NULL,wire+1));
    CHECK(!cs_mac_iwm_encode_sector(0,tags,block,NULL));
    for(i=0;i<sizeof wire;++i)CHECK(wire[i]==0xac);
    memset(block,0,sizeof block);memset(tags,0,sizeof tags);
    CHECK(cs_mac_iwm_encode_sector(0,tags,block,wire+1));
    for(i=1;i<=704;++i)CHECK(wire[i]==0x96);
    CHECK(wire[705]==0xde && wire[706]==0xaa);
    /* Independent final-group vector: preceding zero groups leave sums zero.
     * Last data80/80 produces encoded80/00, implicit third high bits10,
     * and final sums80/80/00. A reader ignoring padding would miss this. */
    block[510]=0x80;block[511]=0x80;
    CHECK(cs_mac_iwm_encode_sector(0,tags,block,wire+1));
    CHECK(wire[698]==0xd9 && wire[699]==0x96 && wire[700]==0x96);
    CHECK(wire[701]==0xdf && wire[702]==0x96 && wire[703]==0x96 && wire[704]==0x96);
    for(seed=0;seed<256;++seed)for(sector=0;sector<12;++sector) {
        for(i=0;i<512;++i)block[i]=(uint8_t)(seed+i*29u+(i>>3));
        for(i=0;i<12;++i)tags[i]=(uint8_t)(seed*11u+i*7u+3u);
        memset(decoded,0xbd,sizeof decoded);
        CHECK(cs_mac_iwm_encode_sector((uint16_t)sector,tags,block,wire+1));
        address[0]=0x96;address[1]=symbols[sector];address[2]=0x96;address[3]=0x9a;
        address[4]=symbols[sector^2u];address[5]=0xde;address[6]=0xaa;
        CHECK(cs_mac_iwm_decode(0,(uint16_t)sector,address,wire+1,decoded+1)==CS_MAC_IWM_OK);
        CHECK(memcmp(tags,decoded+1,12)==0 && memcmp(block,decoded+13,512)==0);
        CHECK(decoded[0]==0xbd && decoded[525]==0xbd && wire[0]==0xac && wire[707]==0xac);
    }
    CHECK(cs_mac_iwm_write_one(NULL,0,block,decoded)==CS_MAC_IWM_WRITE_ARGUMENT);
    printf("Mac write encoder: %u checks passed\n",checks);return 0;
}
