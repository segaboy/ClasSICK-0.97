/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original resource/CODE drawing fixture. */
#include "probe.h"
const uint8_t cs_drawing_app_program[40]={
    0x20,0x4D,0x21,0x3C,0,0x38,0,0x78,0x21,0x3C,0,8,0,8,
    0x2F,8,0xA8,0xA2,0x20,0x4D,0x21,0x3C,0,0x30,0,0x70,
    0x21,0x3C,0,0x10,0,0x10,0x2F,8,0xA8,0xA3,0x70,0x61,0x4E,0x75};
/* CODE0 at260, CODE1 at288, program292..331, map332..393.
 * Native byte construction is original; no foreign application/resource. */
static const uint8_t source[394]={
    [2]=1,[6]=1,[7]=0x4C,[11]=0x4C,[15]=0x3E,
    [259]=24,[263]=40,[267]=16,[271]=8,[275]=32,
    [277]=4,[278]=0x3F,[279]=0x3C,[281]=1,[282]=0xA9,[283]=0xF0,
    [287]=44,[291]=1,
    [357]=28,[359]=62,[362]='C',[363]='O',[364]='D',[365]='E',
    [367]=1,[369]=10,[372]=0xFF,[373]=0xFF,
    [383]=1,[384]=0xFF,[385]=0xFF,[389]=28};
int cs_drawing_app_prepare(cs_drawing_app_workspace *w,cs_trap_gateway *gateway,
    const uint8_t *program,size_t length)
{
    if(!w || !gateway || !program || !length || length>40)return 0;
    uint8_t fork[sizeof source];for(size_t i=0;i<sizeof fork;++i)fork[i]=source[i];
    for(size_t i=0;i<40;++i)fork[292u+i]=i<length?program[i]:(i&1u?0x71u:0x4Eu);
    return cs_loaded_app_prepare_source(&w->app,gateway,fork,sizeof fork);
}
int cs_drawing_app_probe(cs_drawing_app_workspace *w,cs_trap_gateway *gateway,
    cs_loaded_app_result *result)
{
    if(!w || !gateway || !result || w->app.image.tag)return 0;
    result->success=0;result->phase=0;result->value=0;result->pattern=0;
    result->steps=0;result->generation=0;result->retired=0;
    if(!cs_drawing_app_prepare(w,gateway,cs_drawing_app_program,sizeof cs_drawing_app_program)){
        result->phase=w->app.prepared_phase;return 0;}
    cs_loaded_app_workspace *a=&w->app;result->phase=3;result->generation=a->image.generation;
    cs_image_run_result ran=cs_image_run_rectangles(&a->image,&a->run_request,
        &a->caller,&a->run_report,&w->port);
    result->steps=a->run_report.steps;result->value=a->run_report.guest.d[0];
    if(ran!=CS_IMAGE_RUN_RETURNED || a->run_report.entered!=1 || result->steps!=13)return 0;
    result->phase=4;
    uint32_t hash=UINT32_C(2166136261),black=0;
    for(size_t i=0;i<sizeof w->port.pixels;++i){uint8_t v=w->port.pixels[i];
        hash=(hash^v)*UINT32_C(16777619);for(unsigned bit=0;bit<8;++bit)black+=(uint32_t)((v>>bit)&1u);}
    result->pattern=hash;
    if(result->value!=97 || w->port.paints!=1 || w->port.erases!=1 || black!=2304)return 0;
    result->phase=5;
    if(cs_image_run_retire(&a->image,&a->run_request,&a->image_workspace,sizeof a->image_workspace,
        &a->description,&a->image_report)!=CS_IMAGE_OK)return 0;
    result->retired=a->image_report.retired;
    if(a->image.state!=CS_IMAGE_EMPTY || result->retired!=1 ||
        cs_image_external_admit(&a->image)!=CS_IMAGE_OK)return 0;
    result->phase=6;result->success=1;return 1;
}
