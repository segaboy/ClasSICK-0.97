/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original directed/fault-injection tests. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../apps/window-scene/scene.h"
static unsigned mode,checks;
#define CHECK(x) do { ++checks; if (!(x)) { fprintf(stderr,"line%d:%s\n",__LINE__,#x); exit(1); } } while(0)
int cs_fault_scene_compose(cs_cursor_state*,cs_cursor_token,const cs_cursor_art*,const cs_product_state*,const cs_window*,const cs_window*,uint32_t,int32_t,int32_t,cs_window_scene_effect*);
cs_cursor_result cs_fault_hide(cs_cursor_state *s,cs_cursor_token t) { return mode==1u?CS_CURSOR_ERR_SURFACE:cs_cursor_hide(s,t); }
cs_cursor_result cs_fault_advance(cs_cursor_state *s,cs_cursor_token t,cs_cursor_token *o) { return mode==2u?CS_CURSOR_ERR_EXHAUSTED:cs_cursor_advance(s,t,o); }
cs_cursor_result cs_fault_show(cs_cursor_state *s,cs_cursor_token t,const cs_cursor_art *a,int32_t x,int32_t y) { return mode==3u?CS_CURSOR_ERR_ART:cs_cursor_show(s,t,a,x,y); }
int cs_fault_product(const cs_surface *s,int32_t w,int32_t h,int32_t top,const cs_product_state *p)
{ if(mode==4u){s->storage[0]^=1u;return 0;}return cs_product_scene(s,w,h,top,p); }
int main(void)
{
    uint8_t pixels[128u*96u/8u],copy[sizeof(pixels)],payload[48],payload_copy[48];
    cs_surface surface;
    cs_cursor_state cursor,before_cursor;
    cs_cursor_token token;
    cs_cursor_bits bits={{0},{0}};
    cs_cursor_art art={&bits,2u,2u,0u,0u};
    cs_window window,old;
    const cs_window_config cfg={16,16,80,64,8};
    cs_product_state product={97u,0u,16u,6u,1u,NULL};
    cs_window_scene_effect effect,before_effect;
    bits.mask[0]=bits.mask[2]=0xC0u;bits.image[0]=0x80u;bits.image[2]=0x40u;
    for(unsigned scenario=0u;scenario<5u;++scenario){
        memset(pixels,0xA5,sizeof(pixels));memset(payload,0x5A,sizeof(payload));mode=0u;
        CHECK(cs_surface_init(&surface,pixels,sizeof(pixels),128,96,16u,CS_SURFACE_MONO1_MSB)==CS_SURFACE_OK);
        CHECK(cs_cursor_init(&cursor)==CS_CURSOR_OK);CHECK(cs_cursor_bind(&cursor,&surface,payload,sizeof(payload),&token)==CS_CURSOR_OK);
        CHECK(cs_window_begin(&window,&cfg)==CS_WINDOW_OK);
        CHECK(cs_window_scene_compose(&cursor,token,&art,&product,NULL,&window,0u,30,30,&effect)==CS_WINDOW_SCENE_OK);
        token=effect.token;old=window;window.x=24;
        before_cursor=cursor;memcpy(copy,pixels,sizeof(copy));memcpy(payload_copy,payload,sizeof(payload_copy));
        memset(&effect,0xA5,sizeof(effect));before_effect=effect;
        if(scenario==0u){
            cs_cursor_token stale=token;stale.revision--;
            CHECK(cs_window_scene_compose(&cursor,stale,&art,&product,&old,&window,0u,40,40,&effect)==CS_WINDOW_SCENE_ERR_STATE);
            CHECK(memcmp(&effect,&before_effect,sizeof(effect))==0);
            CHECK(memcmp(&cursor,&before_cursor,sizeof(cursor))==0 && memcmp(pixels,copy,sizeof(copy))==0);
            CHECK(memcmp(payload,payload_copy,sizeof(payload))==0);
            bits.image[31]=1u;
            CHECK(cs_window_scene_compose(&cursor,token,&art,&product,&old,&window,0u,40,40,&effect)==CS_WINDOW_SCENE_ERR_ARGUMENT);
            bits.image[31]=0u;
            window.x=-1;CHECK(cs_window_scene_compose(&cursor,token,&art,&product,&old,&window,0u,40,40,&effect)==CS_WINDOW_SCENE_ERR_ARGUMENT);window.x=24;
            CHECK(cs_window_scene_compose(&cursor,token,&art,&product,&old,&window,0u,512,40,&effect)==CS_WINDOW_SCENE_ERR_ARGUMENT);
            CHECK(cs_window_scene_compose(&cursor,token,&art,&product,&old,&window,0u,40,40,(cs_window_scene_effect*)(void*)pixels)==CS_WINDOW_SCENE_ERR_ARGUMENT);
            CHECK(cs_window_scene_compose(&cursor,token,&art,&product,&old,&window,0u,40,40,(cs_window_scene_effect*)(void*)&cursor)==CS_WINDOW_SCENE_ERR_ARGUMENT);
            CHECK(cs_window_scene_compose(&cursor,token,&art,&product,NULL,&window,0u,40,40,&effect)==CS_WINDOW_SCENE_ERR_STATE);
            CHECK(memcmp(&effect,&before_effect,sizeof(effect))==0 && memcmp(pixels,copy,sizeof(copy))==0 && memcmp(payload,payload_copy,sizeof(payload))==0);
            /* Canonical synthetic exhaustion state;never wraps or writes. */
            cursor.revision=UINT32_MAX;token.revision=UINT32_MAX;
            CHECK(cs_window_scene_compose(&cursor,token,&art,&product,&old,&window,0u,40,40,&effect)==CS_WINDOW_SCENE_ERR_EXHAUSTED);
            CHECK(memcmp(pixels,copy,sizeof(copy))==0 && memcmp(&effect,&before_effect,sizeof(effect))==0);
        }else{
            mode=scenario;
            CHECK(cs_fault_scene_compose(&cursor,token,&art,&product,&old,&window,0u,40,40,&effect)==(scenario==4u?CS_WINDOW_SCENE_ERR_PAINT:CS_WINDOW_SCENE_ERR_CURSOR));
            if(scenario==1u){
                CHECK(memcmp(&effect,&before_effect,sizeof(effect))==0 && memcmp(pixels,copy,sizeof(copy))==0 && memcmp(&cursor,&before_cursor,sizeof(cursor))==0);
                CHECK(memcmp(payload,payload_copy,sizeof(payload))==0);
            }else{
                CHECK(cursor.phase==CS_CURSOR_HIDDEN && window.x==24 && old.x==16);
                CHECK(effect.stage==(scenario==2u?CS_WINDOW_SCENE_PAINTED:scenario==3u?CS_WINDOW_SCENE_ADVANCED:CS_WINDOW_SCENE_PAINTING));
                CHECK(effect.cause!=0 && effect.token.binding==token.binding);
                CHECK(effect.token.revision==token.revision+(scenario==3u?1u:0u));
                CHECK(effect.dirty.top==16 && effect.dirty.bottom==80);
                mode=0u;token=effect.token;
                CHECK(cs_window_scene_compose(&cursor,token,&art,&product,&old,&window,0u,40,40,&effect)==CS_WINDOW_SCENE_ERR_STATE);
                CHECK(cs_window_scene_compose(&cursor,token,&art,&product,NULL,&window,0u,40,40,&effect)==CS_WINDOW_SCENE_OK);
                token=effect.token;
            }
        }
        mode=0u;CHECK(cs_cursor_hide(&cursor,token)==CS_CURSOR_OK);CHECK(cs_cursor_unbind(&cursor,token)==CS_CURSOR_OK);
    }
    printf("Window composition fault/refusal PASS:%u checks\n",checks);return 0;
}
