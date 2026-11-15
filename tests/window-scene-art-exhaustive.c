/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original cursor-art validation oracle. */
#include <stdio.h>
#include <string.h>

/* Include the implementation so this test can exercise its private validator
 * without adding a public ABI solely for testing. */
#include "../apps/window-scene/scene.c"

static int reference(const cs_cursor_art *a)
{
    unsigned row, col;
    if (!a || !a->bits || a->width < 1u || a->width > 16u ||
        a->height < 1u || a->height > 16u ||
        a->hot_x >= a->width || a->hot_y >= a->height) return 0;
    for (row=0; row<16u; ++row) {
        for (col=0; col<16u; ++col) {
            unsigned i=row*2u+col/8u, bit=0x80u>>(col%8u);
            int mask=(a->bits->mask[i]&bit)!=0, image=(a->bits->image[i]&bit)!=0;
            if ((row>=a->height || col>=a->width) && (mask || image)) return 0;
            if (image && !mask) return 0;
        }
    }
    return 1;
}

int main(void)
{
    cs_cursor_bits bits;
    cs_cursor_art art;
    unsigned width,height,index,bit,kind;
    unsigned long checks=0;
    memset(&bits,0,sizeof bits);art.bits=&bits;art.hot_x=art.hot_y=0;
    for (width=1;width<=16u;++width) for (height=1;height<=16u;++height) {
        art.width=width;art.height=height;
        if (art_ok(&art)!=reference(&art)) return 1;
        for (index=0;index<32u;++index) for (bit=0;bit<8u;++bit) for (kind=0;kind<3u;++kind) {
            memset(&bits,0,sizeof bits);
            if (kind!=1u) bits.mask[index]=(uint8_t)(1u<<bit);
            if (kind!=0u) bits.image[index]=(uint8_t)(1u<<bit);
            if (art_ok(&art)!=reference(&art)) {
                fprintf(stderr,"width=%u height=%u byte=%u bit=%u kind=%u\n",width,height,index,bit,kind);
                return 1;
            }
            ++checks;
        }
        memset(&bits,0,sizeof bits);
        for (index=0;index<32u;++index) for(bit=0;bit<8u;++bit) {
            unsigned row=index/2u,col=(index%2u)*8u+bit;
            if(row<height && col<width) {
                bits.mask[index]|=(uint8_t)(0x80u>>bit);
                if((row+col)%2u)bits.image[index]|=(uint8_t)(0x80u>>bit);
            }
        }
        if (!art_ok(&art) || !reference(&art)) return 1;
        art.hot_x=width;
        if (art_ok(&art)!=reference(&art)) return 1;
        art.hot_x=0;art.hot_y=height;
        if (art_ok(&art)!=reference(&art)) return 1;
        art.hot_y=0;memset(&bits,0,sizeof bits);
    }
    art.width=0;if (art_ok(&art)!=reference(&art)) return 1;
    art.width=17;if (art_ok(&art)!=reference(&art)) return 1;
    art.width=1;art.height=0;if (art_ok(&art)!=reference(&art)) return 1;
    art.height=17;if (art_ok(&art)!=reference(&art)) return 1;
    art.height=1;art.bits=NULL;if (art_ok(&art)!=reference(&art)) return 1;
    printf("cursor-art exhaustive PASS: %lu single-bit cases\n",checks);
    return 0;
}
