/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original first loaded ROM application probe. */
#include "probe.h"
#define APP_OWNER 93u
#define APP_TOKEN 0x500000u
#define APP_TYPE0 ((uint8_t)(CS_CODE_TYPE>>24))
#define APP_TYPE1 ((uint8_t)(CS_CODE_TYPE>>16))
#define APP_TYPE2 ((uint8_t)(CS_CODE_TYPE>>8))
#define APP_TYPE3 ((uint8_t)CS_CODE_TYPE)
/* Four NOPs: the predeclared RX return token. */
static const uint8_t app_token[8]={0x4E,0x71,0x4E,0x71,0x4E,0x71,0x4E,0x71};
/* Original resource fork: data at 256 (48 bytes), map at 304 (62 bytes).
 * CODE0 length@256 data@260; CODE1 length@284 data@288. No names. */
static const uint8_t app_source[366]={
    /* header: data offset, map offset, data length, map length */
    [2]=0x01,[6]=0x01,[7]=0x30,[11]=0x30,[15]=0x3E,
    /* CODE0: 24 bytes */
    [259]=24,
    [263]=40,            /* above A5 */
    [267]=16,            /* below A5 */
    [271]=8,             /* jump table length */
    [275]=32,            /* jump table offset */
    [277]=0x04,[278]=0x3F,[279]=0x3C,[281]=0x01,[282]=0xA9,[283]=0xF0,
    /* CODE1: 16 bytes, first entry 0, count 1, then program */
    [287]=16,
    [291]=1,
    [292]=0x20,[293]=0x4D,                         /* MOVEA.L A5,A0 */
    [294]=0x21,[295]=0x3C,[296]=0x5A,[297]=0x5A,
    [298]=0x5A,[299]=0x5A,                         /* MOVE.L #$5A5A5A5A,-(A0) */
    [300]=0x70,[301]=0x61,                         /* MOVEQ #97,D0 */
    [302]=0x4E,[303]=0x75,                         /* RTS */
    /* map: type list offset 28, name list offset 62 (empty) */
    [329]=28,[331]=62,
    [334]=APP_TYPE0,[335]=APP_TYPE1,[336]=APP_TYPE2,[337]=APP_TYPE3,
    [339]=1,[341]=10,    /* two CODE resources, reference list at +10 */
    /* reference CODE0: id 0, no name, offset 0 */
    [344]=0xFF,[345]=0xFF,
    /* reference CODE1: id 1, no name, offset 28 */
    [355]=1,[356]=0xFF,[357]=0xFF,[361]=28,
};
static void app_clear_request(cs_image_request *q,cs_image_operation operation,uint32_t generation,uint32_t epoch)
{
    q->operation=operation;q->generation=generation;q->epoch=epoch;
    q->source=0;q->length=0;q->staging=0;q->staging_capacity=0;
}
static void context_copy(cs_image_context *to,const cs_image_context *from)
{
    uint8_t *a=(uint8_t *)to;const uint8_t *b=(const uint8_t *)from;
    for(size_t i=0;i<sizeof *to;++i)a[i]=b[i];
}
static int prepare(cs_loaded_app_workspace *w,cs_trap_gateway *gateway,const uint8_t *source,size_t length)
{
    if(!w || !gateway)return 0;
    if(!w->image.tag){
    w->regions[0].bytes=w->low;w->regions[0].writable=w->low;w->regions[0].base=0;
    w->regions[0].size=sizeof w->low;w->regions[0].flags=3;
    w->regions[1].bytes=w->ram;w->regions[1].writable=w->ram;w->regions[1].base=0x1000;
    w->regions[1].size=sizeof w->ram;w->regions[1].flags=3;
    w->regions[2].bytes=app_token;w->regions[2].writable=0;w->regions[2].base=APP_TOKEN;
    w->regions[2].size=sizeof app_token;w->regions[2].flags=CS_GUEST_READ|CS_GUEST_EXECUTE;
    w->map.regions=w->regions;w->map.count=3;
    cs_mm_config config;
    config.system.identity=0x400;config.system.begin=0x1000;config.system.end=0x1800;
    config.application.identity=0x402;config.application.begin=0x2000;config.application.end=0x6000;
    if(cs_mm_init(&w->manager,&w->map,&config)!=CS_MM_OK)return 0;
    if(cs_life_init(&w->lifecycle,&w->manager)!=CS_LIFE_OK)return 0;
    cs_image_profile profile;profile.maximum_source=sizeof app_source;
    profile.maximum_staging=sizeof w->staging;profile.maximum_world=4096;profile.owner=APP_OWNER;
    if(cs_image_init(&w->image,&w->lifecycle,&profile)!=CS_IMAGE_OK)return 0;
    w->inventory.kind=CS_LIFE_FUTURE;w->inventory.action=CS_LIFE_REFUSE;w->inventory.owner=APP_OWNER;
    w->inventory.address=0;w->inventory.bytes=sizeof w->image;w->inventory.value=0;w->inventory.native=&w->image;
    w->prepared_phase=1;

    }
    if(w->image.state!=CS_IMAGE_EMPTY || cs_image_external_admit(&w->image)!=CS_IMAGE_OK)return 0;
    cs_image_request *q=&w->image_request;
    q->context.context=CS_LIFE_ORDINARY;q->context.active_root=0;q->context.active_callback=0;
    q->context.active_patch=0;q->context.active_manager=0;q->context.session=0;
    q->context.inventory=&w->inventory;q->context.count=1;q->context.resources=0;q->context.resource_count=0;
    q->operation=CS_IMAGE_PREPARE;q->b=0x8000;q->s=512;q->h_min=0;
    q->source=source;q->length=length;
    q->staging=w->staging;q->staging_capacity=sizeof w->staging;
    if(cs_image_execute(&w->image,q,&w->image_workspace,sizeof w->image_workspace,
        &w->description,&w->image_report)!=CS_IMAGE_OK)return 0;
    if(w->image.state!=CS_IMAGE_PREPARED)return 0;
    uint32_t generation=w->image.generation,epoch=w->image.epoch;
    w->prepared_phase=2;

    app_clear_request(q,CS_IMAGE_LOAD,generation,epoch);
    if(cs_image_execute(&w->image,q,&w->image_workspace,sizeof w->image_workspace,
        &w->description,&w->image_report)!=CS_IMAGE_OK)return 0;
    if(w->image.state!=CS_IMAGE_LOADED)return 0;
    w->prepared_phase=3;

    cs_image_run_request *r=&w->run_request;
    context_copy(&r->context,&q->context);r->gateway=gateway;r->generation=generation;r->epoch=epoch;
    r->token=APP_TOKEN;r->budget=32;
    return 1;
}
int cs_loaded_app_prepare(cs_loaded_app_workspace *w,cs_trap_gateway *gateway)
{return prepare(w,gateway,app_source,sizeof app_source);}
int cs_loaded_app_prepare_program(cs_loaded_app_workspace *w,cs_trap_gateway *gateway,
    const uint8_t *program,size_t length)
{
    if(!program || !length || length>12)return 0;
    uint8_t source[sizeof app_source];for(size_t i=0;i<sizeof source;++i)source[i]=app_source[i];
    for(size_t i=0;i<12;++i)source[292u+i]=i<length?program[i]:(i&1u?0x71u:0x4Eu);
    return prepare(w,gateway,source,sizeof source);
}
int cs_loaded_app_probe(cs_loaded_app_workspace *w,cs_trap_gateway *gateway,cs_loaded_app_result *result)
{
    if(!w || !gateway || !result || w->image.tag)return 0;
    result->success=0;result->phase=0;result->value=0;result->pattern=0;
    result->steps=0;result->generation=0;result->retired=0;
    int loaded=cs_loaded_app_prepare(w,gateway);result->phase=w->prepared_phase;
    if(!loaded)return 0;
    result->generation=w->image.generation;
    cs_image_run_request *r=&w->run_request;
    cs_image_run_result ran=cs_image_run(&w->image,r,&w->caller,&w->run_report);
    result->steps=w->run_report.steps;result->value=w->run_report.guest.d[0];
    if(ran!=CS_IMAGE_RUN_RETURNED || w->run_report.status!=CS_IMAGE_RUN_RETURNED)return 0;
    if(w->run_report.entered!=1u || w->run_report.steps!=5u)return 0;
    result->phase=4;

    uint32_t pattern=0;
    if(cs_guest_read(&w->map,w->image.layout.a5-4u,4,CS_GUEST_READ,&pattern)!=CS_GUEST_OK)return 0;
    result->pattern=pattern;
    if(result->value!=97u || pattern!=0x5A5A5A5Au)return 0;
    result->phase=5;

    if(cs_image_run_retire(&w->image,r,&w->image_workspace,sizeof w->image_workspace,
        &w->description,&w->image_report)!=CS_IMAGE_OK)return 0;
    result->retired=w->image_report.retired;
    if(w->image.state!=CS_IMAGE_EMPTY)return 0;
    if(cs_image_external_admit(&w->image)!=CS_IMAGE_OK)return 0;
    if(result->retired!=1u)return 0;
    result->phase=6;
    result->success=1;
    return 1;
}
