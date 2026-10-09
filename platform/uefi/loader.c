/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "loader.h"

static uint32_t read32(const unsigned char *p)
{
    return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);
}
static uint64_t read64(const unsigned char *p)
{
    return (uint64_t)read32(p)|((uint64_t)read32(p+4)<<32);
}
uint32_t cs_uefi_table_crc(const void *table,size_t size)
{
    const unsigned char *p=table;
    uint32_t crc=UINT32_MAX;
    for(size_t i=0;i<size;++i) {
        crc^=(i>=16 && i<20)?0:p[i];
        for(unsigned bit=0;bit<8;++bit)
            crc=(crc>>1)^((crc&1u)?UINT32_C(0xEDB88320):0);
    }
    return ~crc;
}
static int table_valid(const cs_efi_header *h,uint64_t signature,size_t size)
{
    return h!=NULL && h->signature==signature && h->revision>=UINT32_C(0x20000)
        && (h->revision>>16)==2 && h->size==size && h->reserved==0
        && h->crc==cs_uefi_table_crc(h,size);
}
static int image_bounds(uint64_t base,uint64_t size,uint64_t *rounded)
{
    if(base==0 || base%4096!=0 || size==0 || size>UINT64_C(16777216)) return 0;
    *rounded=(size+4095)&~UINT64_C(4095);
    return base<(UINT64_C(1)<<47) && *rounded<=(UINT64_C(1)<<47)-base;
}
cs_uefi_result cs_uefi_image_spans(const unsigned char *map,size_t length,size_t stride,
    uint32_t version,uint64_t image_base,uint64_t image_size,cs_uefi_owned_span *out,size_t *count)
{
    cs_uefi_owned_span pending[7]; size_t descriptors,used=0; uint64_t size,covered=0;
    if(out==NULL || count==NULL) return CS_UEFI_ARGUMENT;
    if(!image_bounds(image_base,image_size,&size)) return CS_UEFI_LIMIT;
    cs_uefi_result status=cs_uefi_check_map(map,length,stride,version,&descriptors);
    if(status!=CS_UEFI_OK) return status;
    for(size_t i=0;i<descriptors;++i) {
        const unsigned char *p=map+i*stride;
        uint64_t base=read64(p+8),end=base+(read64(p+24)<<12);
        if(base>=image_base+size || end<=image_base) continue;
        uint32_t kind=read32(p);
        if((kind!=1 && kind!=2) || (read64(p+32)&(UINT64_C(1)<<63))!=0) return CS_UEFI_OWNERSHIP;
        if(used==7) return CS_UEFI_LIMIT;
        uint64_t start=base>image_base?base:image_base,finish=end<image_base+size?end:image_base+size;
        pending[used].base=start; pending[used].size=finish-start; pending[used].kind=kind;
        covered+=finish-start; ++used;
    }
    if(covered!=size) return CS_UEFI_OWNERSHIP;
    for(size_t i=0;i<used;++i) out[i]=pending[i];
    *count=used; return CS_UEFI_OK;
}
uint64_t cs_uefi_acpi20_rsdp(size_t count,const void *table)
{
    /* ACPI 6.6 5.2.5.2: 8868e871-e4f1-11d3-bc22-0080c73c8881, stored per UEFI GUID layout. */
    static const unsigned char guid[16]={0x71,0xE8,0x68,0x88,0xF1,0xE4,0xD3,0x11,
        0xBC,0x22,0x00,0x80,0xC7,0x3C,0x88,0x81};
    const unsigned char *p=table;
    if(p==NULL || count>256) return 0;
    for(size_t i=0;i<count;++i,p+=24) {
        unsigned j=0;
        while(j<16 && p[j]==guid[j]) ++j;
        if(j==16) return read64(p+16);
    }
    return 0;
}
/* SPEC-0016: bounded copy of the UCS-2 FirmwareVendor string, read bytewise. */
static uint32_t vendor_copy(const unsigned char *vendor,unsigned char *out)
{
    uint32_t units;
    if(vendor==NULL) return CS_UEFI_VENDOR_ABSENT;
    for(units=0;units<CS_UEFI_VENDOR_UNITS;++units) {
        uint32_t unit=(uint32_t)vendor[units*2u]|((uint32_t)vendor[units*2u+1u]<<8);
        if(unit==0) return CS_UEFI_VENDOR_COMPLETE;
        out[units]=(unsigned char)(unit>=0x20u && unit<0x7Fu?unit:0x3Fu);
    }
    return (vendor[2u*CS_UEFI_VENDOR_UNITS]|vendor[2u*CS_UEFI_VENDOR_UNITS+1u])==0
        ?CS_UEFI_VENDOR_COMPLETE:CS_UEFI_VENDOR_TRUNCATED;
}
static cs_efi_status finish(cs_uefi_load_result *out,cs_efi_status status)
{
    out->status=status; return status;
}
cs_efi_status cs_uefi_loader_run(void *image,cs_efi_system *system,cs_uefi_load_result *out)
{
    const cs_efi_guid image_guid={0x5B1B31A1,0x9562,0x11D2,{0x8E,0x3F,0,0xA0,0xC9,0x69,0x72,0x3B}};
    const cs_efi_guid gop_guid={0x9042A9DE,0x23DC,0x4A38,{0x96,0xFB,0x7A,0xDE,0xD0,0x80,0x51,0x6A}};
    cs_efi_status status; void *protocol=NULL; cs_efi_image *loaded; cs_efi_gop *gop;
    cs_uefi_framebuffer f; cs_uefi_framebuffer_layout layout; uint64_t image_base,image_size,address;
    uint64_t rsdp;
    if(out==NULL) return CS_EFI_ERROR(2);
    out->status=CS_EFI_ERROR(2); out->attempts=0; out->exited=0; out->handoff=NULL;
    if(image==NULL || system==NULL) return out->status;
    if(!table_valid(&system->header,UINT64_C(0x5453595320494249),sizeof(*system))) return finish(out,CS_EFI_ERROR(27));
    cs_efi_boot *boot=system->boot;
    if(boot==NULL || !table_valid(&boot->header,UINT64_C(0x56524553544F4F42),sizeof(*boot))) return finish(out,CS_EFI_ERROR(27));
    if(boot->allocate==NULL || boot->free_pages==NULL || boot->get_map==NULL
            || boot->exit_boot==NULL || boot->open_protocol==NULL || boot->watchdog==NULL
            || boot->locate_protocol==NULL) return finish(out,CS_EFI_ERROR(3));
    rsdp=cs_uefi_acpi20_rsdp(system->configuration_count,system->configuration);
    cs_efi_map get_map=boot->get_map; cs_efi_exit exit_boot=boot->exit_boot;
    cs_efi_free release=boot->free_pages;
    status=boot->watchdog(0,0,0,NULL);
    if(status!=0 && status!=CS_EFI_ERROR(3)) return finish(out,status);
    status=boot->open_protocol(image,&image_guid,&protocol,image,NULL,2);
    if(status!=0) return finish(out,status);
    loaded=protocol;
    if(loaded==NULL || loaded->revision<0x1000 || loaded->system!=system
            || loaded->code_type!=1 || loaded->data_type!=2
            || !image_bounds((uint64_t)(uintptr_t)loaded->base,loaded->size,&image_size)) return finish(out,CS_EFI_ERROR(1));
    image_base=(uint64_t)(uintptr_t)loaded->base;
    protocol=NULL; status=boot->locate_protocol(&gop_guid,NULL,&protocol);
    if(status!=0) return finish(out,status);
    gop=protocol;
    if(gop==NULL || gop->mode==NULL || gop->mode->info==NULL || gop->mode->max_mode==0
            || gop->mode->mode>=gop->mode->max_mode || gop->mode->info_size<36
            || gop->mode->info_size>256 || gop->mode->info->version!=0) return finish(out,CS_EFI_ERROR(3));
    f.base=gop->mode->framebuffer; f.size=gop->mode->framebuffer_size;
    f.width=gop->mode->info->width; f.height=gop->mode->info->height;
    f.pitch=gop->mode->info->pitch; f.format=gop->mode->info->format;
    if(cs_uefi_check_framebuffer(&f,&layout)!=CS_UEFI_OK) return finish(out,CS_EFI_ERROR(3));
    address=(UINT64_C(1)<<47)-1;
    status=boot->allocate(1,2,CS_LOADER_BUNDLE/4096,&address);
    if(status!=0) return finish(out,status);
    if(address==0 || address%4096!=0 || address>=(UINT64_C(1)<<47)
            || CS_LOADER_BUNDLE>(UINT64_C(1)<<47)-address
            || (address<image_base+image_size && image_base<address+CS_LOADER_BUNDLE)
            || (address<f.base+f.size && f.base<address+CS_LOADER_BUNDLE)) return finish(out,CS_EFI_ERROR(33));
    volatile unsigned char *zero=(volatile unsigned char *)(uintptr_t)address;
    for(size_t i=0;i<CS_LOADER_BUNDLE;++i) zero[i]=0;
    cs_uefi_handoff *h=(cs_uefi_handoff *)(uintptr_t)address;
    h->magic=UINT64_C(0x4353303937484F46); h->version=CS_UEFI_HANDOFF_VERSION; h->size=(uint32_t)sizeof(*h);
    h->image_base=image_base; h->image_size=image_size; h->bundle_base=address; h->bundle_size=CS_LOADER_BUNDLE;
    h->framebuffer=f; h->map_base=address+CS_LOADER_MAP_OFFSET;
    h->stack_top=address+CS_LOADER_STACK_OFFSET+CS_LOADER_STACK_BYTES;
    h->fault_top=address+CS_LOADER_ARENA_OFFSET;
    h->arena_base=address+CS_LOADER_ARENA_OFFSET; h->arena_size=CS_LOADER_ARENA_BYTES;
    h->trace_base=address+CS_LOADER_TRACE_OFFSET; h->trace_size=CS_LOADER_TRACE_BYTES;
    h->rsdp=rsdp;
    h->firmware_revision=system->firmware_revision;
    h->firmware_vendor_state=vendor_copy((const unsigned char *)system->vendor,h->firmware_vendor);
    cs_uefi_exit_state state; (void)cs_uefi_exit_init(&state);
    for(;;) {
        size_t length=CS_LOADER_MAP_BYTES,stride=0,key=0,span_count=0; uint32_t version=0;
        status=get_map(&length,(void *)(uintptr_t)h->map_base,&key,&stride,&version);
        uint32_t reason=1;
        if(status==0) {
            if(length>CS_LOADER_MAP_BYTES) status=CS_EFI_ERROR(33);
            else if(cs_uefi_image_spans((const unsigned char *)(uintptr_t)h->map_base,length,stride,version,
                        image_base,image_size,h->spans,&span_count)!=CS_UEFI_OK) status=CS_EFI_ERROR(33);
            else {
                h->spans[span_count].base=address; h->spans[span_count].size=CS_LOADER_BUNDLE; h->spans[span_count].kind=2;
                ++span_count;
                if(cs_uefi_check_owned((const unsigned char *)(uintptr_t)h->map_base,length,stride,version,
                            h->spans,span_count,&f)!=CS_UEFI_OK) status=CS_EFI_ERROR(33);
            }
        }
        if(status==0) {
            h->map_size=length; h->map_stride=stride; h->map_version=version; h->map_key=key; h->span_count=(uint32_t)span_count;
            (void)cs_uefi_exit_snapshot(&state,key);
            status=exit_boot(image,key); reason=2;
            (void)cs_uefi_exit_observe(&state,status==0?0u:(status==CS_EFI_ERROR(2)?1u:2u));
        }
        h->exit_attempts=state.attempts; h->stage=state.phase;
        h->firmware_status=status; h->reason=status==0?0:reason;
        if(status!=0 && state.attempts==0) {
            cs_efi_status free_status=release(address,CS_LOADER_BUNDLE/4096);
            return finish(out,free_status==0?status:free_status);
        }
        if(status!=0 && state.phase==CS_UEFI_RETRY && reason==2) continue;
        out->attempts=state.attempts; out->exited=state.phase==CS_UEFI_EXITED?1u:0u; out->handoff=h;
        return finish(out,status);
    }
}
