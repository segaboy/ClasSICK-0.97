/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../platform/uefi/loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do { if(!(x)){fprintf(stderr,"Loader failure line %d\n",__LINE__);return 1;} } while(0)
typedef struct {
    cs_efi_system system; cs_efi_boot boot; cs_efi_image image;
    cs_efi_gop gop; cs_efi_gop_mode mode; cs_efi_gop_info info;
    unsigned char *storage; uint64_t bundle; unsigned sequence,fault,step,maps,exits,frees,errors;
    unsigned char vendor[80];
    char calls[32]; size_t calls_count;
} fixture;
static fixture *active;
static uint32_t oracle_crc(const unsigned char *p,size_t n)
{
    uint32_t remainder=UINT32_MAX,result=0;
    for(size_t i=0;i<n;++i) {
        unsigned char byte=(i>=16 && i<20)?0:p[i];
        for(unsigned bit=0;bit<8;++bit) {
            uint32_t feedback=((remainder>>31)^((uint32_t)byte>>bit))&1u;
            remainder<<=1;
            if(feedback) remainder^=UINT32_C(0x04C11DB7);
        }
    }
    remainder=~remainder;
    for(unsigned bit=0;bit<32;++bit) { result=(result<<1)|(remainder&1u); remainder>>=1; }
    return result;
}
static void header(cs_efi_header *h,uint64_t signature,size_t n)
{
    h->signature=signature; h->revision=0x20064; h->size=(uint32_t)n; h->reserved=0;
    h->crc=oracle_crc((const unsigned char *)h,n);
}
static void call(char name)
{
    if(active->calls_count>=sizeof(active->calls)-1) { ++active->errors; return; }
    active->calls[active->calls_count++]=name; active->calls[active->calls_count]=0;
}
static cs_efi_status CS_EFIAPI watchdog(size_t timeout,uint64_t code,size_t size,const uint16_t *data)
{
    call('W'); if(timeout || code || size || data) ++active->errors;
    return active->fault==1?CS_EFI_ERROR(7):(active->fault==2?CS_EFI_ERROR(3):0);
}
static cs_efi_status CS_EFIAPI open_protocol(void *image,const cs_efi_guid *guid,void **out,
    void *agent,void *controller,uint32_t attributes)
{
    const cs_efi_guid expected={0x5B1B31A1,0x9562,0x11D2,{0x8E,0x3F,0,0xA0,0xC9,0x69,0x72,0x3B}};
    call('O');
    if(image!=active || agent!=active || controller!=NULL || attributes!=2
            || memcmp(guid,&expected,sizeof(expected))) ++active->errors;
    if(active->fault==3) return CS_EFI_ERROR(14);
    *out=&active->image; return 0;
}
static cs_efi_status CS_EFIAPI locate(const cs_efi_guid *guid,void *registration,void **out)
{
    const cs_efi_guid expected={0x9042A9DE,0x23DC,0x4A38,{0x96,0xFB,0x7A,0xDE,0xD0,0x80,0x51,0x6A}};
    call('L'); if(registration!=NULL || memcmp(guid,&expected,sizeof(expected))) ++active->errors;
    if(active->fault==4) return CS_EFI_ERROR(14);
    *out=&active->gop; return 0;
}
static cs_efi_status CS_EFIAPI allocate(uint32_t type,uint32_t kind,size_t pages,uint64_t *address)
{
    call('A'); if(type!=1 || kind!=2 || pages!=128 || *address!=(UINT64_C(1)<<47)-1) ++active->errors;
    if(active->fault==5) return CS_EFI_ERROR(9);
    *address=active->fault==6?active->bundle+1:active->bundle;
    return 0;
}
static cs_efi_status CS_EFIAPI release(uint64_t address,size_t pages)
{
    call('F'); ++active->frees;
    if(address!=active->bundle || pages!=128 || active->exits) ++active->errors;
    return active->fault==7?CS_EFI_ERROR(7):0;
}
static void put(unsigned char *p,uint64_t n,unsigned width)
{
    for(unsigned i=0;i<width;++i) p[i]=(unsigned char)((n>>(8*i))&255u);
}
static void descriptor(unsigned char *p,uint32_t kind,uint64_t base,uint64_t pages,uint64_t attr)
{
    memset(p,0xA5,48); put(p,kind,4); put(p+8,base,8); put(p+16,0,8); put(p+24,pages,8); put(p+32,attr,8);
}
static cs_efi_status CS_EFIAPI get_map(size_t *length,void *bytes,size_t *key,size_t *stride,uint32_t *version)
{
    call('M'); ++active->maps;
    if(*length!=CS_LOADER_MAP_BYTES || (uint64_t)(uintptr_t)bytes!=active->bundle+CS_LOADER_MAP_OFFSET) ++active->errors;
    if(active->fault==8 || active->fault==7 || (active->fault==9 && active->maps==2)) { *length=CS_LOADER_MAP_BYTES+48; return CS_EFI_ERROR(5); }
    descriptor(bytes,2,active->bundle,128,0);
    descriptor((unsigned char *)bytes+48,1,0x400000,2,0);
    descriptor((unsigned char *)bytes+96,2,0x402000,2,0);
    *length=144; *stride=48; *version=1; *key=active->maps==1?0:SIZE_MAX-active->maps;
    if(active->fault==10) *length=CS_LOADER_MAP_BYTES+1;
    if(active->fault==11) *stride=41;
    if(active->fault==12) *version=2;
    if(active->fault==13) put((unsigned char *)bytes+48,7,4);
    if(active->fault==14) put((unsigned char *)bytes+48+32,UINT64_C(1)<<63,8);
    if(active->fault==15) put(bytes,7,4);
    if(active->fault==16) put((unsigned char *)bytes+96+8,0x403000,8);
    return 0;
}
static cs_efi_status CS_EFIAPI exit_boot(void *image,size_t key)
{
    call('E'); ++active->exits;
    if(image!=active || key!=(active->maps==1?0:SIZE_MAX-active->maps)) ++active->errors;
    unsigned outcome=active->sequence%3; active->sequence/=3;
    /* Destroy firmware/protocol fields after the first attempt; cached callbacks
       and owned metadata must sustain retries. This is an original mock only. */
    active->system.boot=NULL; active->boot.get_map=NULL; active->boot.exit_boot=NULL;
    memset(active->vendor,'X',sizeof active->vendor); active->system.firmware_revision=0xDEADu;
    active->boot.free_pages=NULL; active->info.format=99; active->image.base=NULL;
    return outcome==0?0:(outcome==1?CS_EFI_ERROR(2):CS_EFI_ERROR(7));
}
static int init(fixture *f)
{
    memset(f,0,sizeof(*f));
    f->storage=malloc(CS_LOADER_BUNDLE+8192);
    if(f->storage==NULL) return 1;
    memset(f->storage,0xA5,CS_LOADER_BUNDLE+8192);
    f->bundle=((uint64_t)(uintptr_t)f->storage+4095)&~UINT64_C(4095);
    f->system.boot=&f->boot;
    f->boot.allocate=allocate; f->boot.free_pages=release; f->boot.get_map=get_map;
    f->boot.open_protocol=open_protocol; f->boot.locate_protocol=locate;
    f->boot.watchdog=watchdog; f->boot.exit_boot=exit_boot;
    f->image.revision=0x1000; f->image.system=&f->system; f->image.base=(void *)(uintptr_t)0x400000;
    f->image.size=16383; f->image.code_type=1; f->image.data_type=2;
    f->gop.mode=&f->mode; f->mode.max_mode=1; f->mode.info=&f->info; f->mode.info_size=36;
    f->mode.framebuffer=0x10000000; f->mode.framebuffer_size=4096;
    f->info.width=9; f->info.height=2; f->info.pitch=12; f->info.format=1;
    /* SPEC-0016: an original UCS-2 vendor string, little-endian code units. */
    { const char *name="Original test firmware";
      for(size_t i=0;name[i]!=0;++i) f->vendor[2*i]=(unsigned char)name[i]; }
    f->system.vendor=f->vendor; f->system.firmware_revision=0x0001ABCDu;
    header(&f->boot.header,UINT64_C(0x56524553544F4F42),sizeof(f->boot));
    header(&f->system.header,UINT64_C(0x5453595320494249),sizeof(f->system));
    active=f; return 0;
}
static int guards(fixture *f)
{
    unsigned char *start=(unsigned char *)(uintptr_t)f->bundle;
    for(unsigned char *p=f->storage;p<start;++p) if(*p!=0xA5) return 0;
    for(unsigned char *p=start+CS_LOADER_BUNDLE;p<f->storage+CS_LOADER_BUNDLE+8192;++p) if(*p!=0xA5) return 0;
    return 1;
}
static int crc_tests(void)
{
    const unsigned char vector[]="123456789";
    CHECK(cs_uefi_table_crc(vector,9)==UINT32_C(0xCBF43926));
    unsigned char table[376]; size_t cases=0;
    for(size_t n=24;n<=376;n+=8) for(unsigned seed=0;seed<8;++seed) {
        for(size_t i=0;i<n;++i) table[i]=(unsigned char)((i*17+seed*31)&255u);
        CHECK(cs_uefi_table_crc(table,n)==oracle_crc(table,n)); ++cases;
    }
    printf("Loader CRC: %zu independent polynomial cases PASS\n",cases); return 0;
}
static int transactions(void)
{
    for(unsigned sequence=0;sequence<27;++sequence) {
        fixture f; CHECK(init(&f)==0); f.sequence=sequence;
        unsigned input=sequence,attempts=0,exited=0;
        char expected[32]="WOLA"; size_t position=4;
        for(unsigned i=0;i<3;++i) {
            unsigned outcome=input%3; input/=3; ++attempts;
            expected[position++]='M'; expected[position++]='E';
            if(outcome==0) { exited=1; break; }
            if(outcome==2) break;
        }
        expected[position]=0;
        cs_uefi_load_result result; cs_efi_status status=cs_uefi_loader_run(&f,&f.system,&result);
        CHECK(result.attempts==attempts && result.exited==exited && ((status==0)==(exited!=0)));
        CHECK(strcmp(f.calls,expected)==0 && f.maps==attempts && f.exits==attempts && f.frees==0 && f.errors==0);
        CHECK(result.handoff==(cs_uefi_handoff *)(uintptr_t)f.bundle);
        cs_uefi_handoff *h=result.handoff;
        CHECK(h->version==2 && h->size==sizeof(*h) && h->image_base==0x400000 && h->image_size==16384 && h->span_count==3);
        CHECK(h->firmware_revision==0x0001ABCDu && h->firmware_vendor_state==CS_UEFI_VENDOR_COMPLETE);
        CHECK(memcmp(h->firmware_vendor,"Original test firmware",23)==0);
        for(size_t i=23;i<32;++i) CHECK(h->firmware_vendor[i]==0);
        CHECK(h->map_size==144 && h->map_stride==48 && h->map_version==1 && h->exit_attempts==attempts);
        CHECK(h->framebuffer.format==1 && h->native_entered==0 && h->stack_top==f.bundle+331776);
        CHECK(h->spans[0].base==0x400000 && h->spans[0].size==8192 && h->spans[0].kind==1);
        CHECK(h->spans[1].base==0x402000 && h->spans[1].size==8192 && h->spans[1].kind==2);
        CHECK(h->spans[2].base==f.bundle && h->spans[2].size==CS_LOADER_BUNDLE && guards(&f));
        free(f.storage);
    }
    fixture a,b; CHECK(init(&a)==0); cs_uefi_load_result first,second;
    CHECK(cs_uefi_loader_run(&a,&a.system,&first)==0); uint64_t saved=first.handoff->stack_top;
    CHECK(init(&b)==0); CHECK(cs_uefi_loader_run(&b,&b.system,&second)==0);
    CHECK(first.handoff!=second.handoff && first.handoff->stack_top==saved);
    free(a.storage); free(b.storage);
    puts("Loader transactions: 27 finite traces/cached ABI callbacks and independent bundles PASS"); return 0;
}
static int rejection(void)
{
    for(unsigned fault=1;fault<=16;++fault) {
        fixture f; CHECK(init(&f)==0); f.fault=fault; if(fault==9) f.sequence=1;
        cs_uefi_load_result result; cs_efi_status status=cs_uefi_loader_run(&f,&f.system,&result);
        if(fault==2) CHECK(status==0 && result.exited==1);
        else CHECK(status!=0 && result.exited==0);
        CHECK(f.errors==0 && guards(&f));
        if(fault==9) CHECK(strcmp(f.calls,"WOLAMEM")==0 && result.attempts==1 && result.handoff!=NULL && f.frees==0);
        else if(fault>=7) CHECK(strcmp(f.calls,"WOLAMF")==0 && result.attempts==0 && result.handoff==NULL && f.exits==0 && f.frees==1);
        if(fault==6) {
            CHECK(strcmp(f.calls,"WOLA")==0 && f.frees==0);
            unsigned char *bundle=(unsigned char *)(uintptr_t)f.bundle;
            for(size_t i=0;i<CS_LOADER_BUNDLE;++i) CHECK(bundle[i]==0xA5);
        }
        free(f.storage);
    }
    for(unsigned fault=0;fault<15;++fault) {
        fixture f; CHECK(init(&f)==0);
        if(fault==0) ++f.system.header.crc;
        if(fault==1) ++f.boot.header.crc;
        if(fault==2) f.system.header.size=128;
        if(fault==3) f.system.header.reserved=1;
        if(fault==4) f.system.header.revision=0x30000;
        if(fault==5) { f.boot.get_map=NULL; header(&f.boot.header,UINT64_C(0x56524553544F4F42),sizeof(f.boot)); }
        if(fault==6) f.image.size=0;
        if(fault==7) f.image.base=(void *)(uintptr_t)0x400001;
        if(fault==8) f.image.code_type=7;
        if(fault==9) f.mode.info=NULL;
        if(fault==10) f.mode.info_size=35;
        if(fault==11) f.info.version=1;
        if(fault==12) f.mode.mode=1;
        if(fault==13) f.info.format=2;
        if(fault==14) f.info.pitch=8;
        cs_uefi_load_result result;
        CHECK(cs_uefi_loader_run(&f,&f.system,&result)!=0 && result.attempts==0 && result.handoff==NULL);
        CHECK(f.maps==0 && f.exits==0 && f.frees==0 && f.errors==0 && guards(&f));
        free(f.storage);
    }
    CHECK(cs_uefi_loader_run(NULL,NULL,NULL)==CS_EFI_ERROR(2));
    cs_uefi_load_result result; CHECK(cs_uefi_loader_run(NULL,NULL,&result)==CS_EFI_ERROR(2) && result.attempts==0);
    puts("Loader malformed data/cleanup/partial-exit rejections PASS"); return 0;
}
static int ownership(void)
{
    unsigned char map[8*48]; cs_uefi_owned_span out[7],saved[7]; size_t count=99;
    memset(out,0xA5,sizeof(out)); memcpy(saved,out,sizeof(out));
    for(size_t i=0;i<8;++i) descriptor(map+i*48,i%2?1u:2u,0x400000+i*4096,1,0);
    CHECK(cs_uefi_image_spans(map,sizeof(map),48,1,0x400000,8*4096,out,&count)==CS_UEFI_LIMIT);
    CHECK(count==99 && memcmp(out,saved,sizeof(out))==0);
    CHECK(cs_uefi_image_spans(map,7*48,48,1,0x400000,7*4096,out,&count)==CS_UEFI_OK && count==7);
    for(size_t i=0;i<7;++i) CHECK(out[i].base==0x400000+i*4096 && out[i].size==4096 && out[i].kind==(i%2?1u:2u));
    memcpy(out,saved,sizeof(out)); count=99;
    CHECK(cs_uefi_image_spans(map,7*48,48,1,0x400000,8*4096,out,&count)==CS_UEFI_OWNERSHIP);
    CHECK(cs_uefi_image_spans(map,7*48,48,1,0x400001,4096,out,&count)==CS_UEFI_LIMIT);
    CHECK(count==99 && memcmp(out,saved,sizeof(out))==0);
    puts("Loader image coverage/seven-span bound/output stability PASS"); return 0;
}
/* SPEC-0010: ACPI 2.0+ GUID bytes written independently from the ACPI 6.6 text. */
static void entry(unsigned char *p,int acpi20,uint64_t pointer)
{
    static const unsigned char v20[16]={0x71,0xE8,0x68,0x88,0xF1,0xE4,0xD3,0x11,0xBC,0x22,0,0x80,0xC7,0x3C,0x88,0x81};
    static const unsigned char v10[16]={0x30,0x2D,0x9D,0xEB,0x88,0x2D,0xD3,0x11,0x9A,0x16,0,0x90,0x27,0x3F,0xC1,0x4D};
    memcpy(p,acpi20?v20:v10,16); put(p+16,pointer,8);
}
static int configuration(void)
{
    unsigned char table[257*24];
    memset(table,0x3C,sizeof table);
    CHECK(cs_uefi_acpi20_rsdp(0,table)==0 && cs_uefi_acpi20_rsdp(3,NULL)==0);
    entry(table,0,0x1000); entry(table+24,0,0x2000);
    CHECK(cs_uefi_acpi20_rsdp(2,table)==0);
    entry(table+48,1,0x7FF00100u); entry(table+72,1,0x9000);
    CHECK(cs_uefi_acpi20_rsdp(4,table)==0x7FF00100u && cs_uefi_acpi20_rsdp(3,table)==0x7FF00100u);
    CHECK(cs_uefi_acpi20_rsdp(2,table)==0);
    table[48+15]^=1; CHECK(cs_uefi_acpi20_rsdp(4,table)==0x9000); table[48+15]^=1;
    entry(table+255*24,1,0xABC000); memset(table,0,48*2);
    for(unsigned i=0;i<255;++i) memset(table+i*24,0,24);
    CHECK(cs_uefi_acpi20_rsdp(256,table)==0xABC000 && cs_uefi_acpi20_rsdp(257,table)==0);
    CHECK(cs_uefi_acpi20_rsdp(255,table)==0);
    /* End to end: the handoff carries the pointer; absent tables leave zero. */
    fixture f; cs_uefi_load_result result;
    CHECK(init(&f)==0); entry(table,1,0x7FF00100u);
    f.system.configuration_count=1; f.system.configuration=table;
    header(&f.system.header,UINT64_C(0x5453595320494249),sizeof(f.system));
    CHECK(cs_uefi_loader_run(&f,&f.system,&result)==0 && result.handoff->rsdp==0x7FF00100u);
    free(f.storage);
    CHECK(init(&f)==0);
    CHECK(cs_uefi_loader_run(&f,&f.system,&result)==0 && result.handoff->rsdp==0);
    free(f.storage);
    CHECK(init(&f)==0); f.system.configuration_count=300; f.system.configuration=table;
    header(&f.system.header,UINT64_C(0x5453595320494249),sizeof(f.system));
    CHECK(cs_uefi_loader_run(&f,&f.system,&result)==0 && result.handoff->rsdp==0);
    free(f.storage);
    puts("Loader configuration: ACPI 2.0 GUID selection, bounds and handoff capture PASS"); return 0;
}
/* SPEC-0016: bounded printable copy of FirmwareVendor, taken before exit. */
static int vendor(void)
{
    static const struct { unsigned units,nul; uint32_t state; } cases[]={
        {0,1,CS_UEFI_VENDOR_COMPLETE},{5,1,CS_UEFI_VENDOR_COMPLETE},{30,1,CS_UEFI_VENDOR_COMPLETE},
        {31,1,CS_UEFI_VENDOR_COMPLETE},{32,1,CS_UEFI_VENDOR_TRUNCATED},{39,1,CS_UEFI_VENDOR_TRUNCATED},
        {39,0,CS_UEFI_VENDOR_TRUNCATED}};
    for(size_t c=0;c<sizeof cases/sizeof cases[0];++c) {
        fixture f; cs_uefi_load_result result; unsigned char expected[32];
        CHECK(init(&f)==0); memset(f.vendor,0,sizeof f.vendor); memset(expected,0,sizeof expected);
        for(unsigned i=0;i<cases[c].units;++i) {
            /* ASCII, then a control unit, a non-ASCII unit and DEL, each replaced by '?'. */
            unsigned unit=i%7==3?0x09u:(i%7==5?0x263Au:(i%7==6?0x7Fu:0x41u+i%26u));
            f.vendor[2*i]=(unsigned char)unit; f.vendor[2*i+1]=(unsigned char)(unit>>8);
            if(i<31) expected[i]=(unsigned char)(i%7==3 || i%7>=5?0x3F:unit);
        }
        if(!cases[c].nul) memset(f.vendor+2*cases[c].units,0x41,sizeof f.vendor-2*cases[c].units);
        header(&f.system.header,UINT64_C(0x5453595320494249),sizeof(f.system));
        CHECK(cs_uefi_loader_run(&f,&f.system,&result)==0);
        CHECK(result.handoff->firmware_vendor_state==cases[c].state);
        CHECK(memcmp(result.handoff->firmware_vendor,expected,32)==0 && result.handoff->firmware_vendor[31]==0);
        free(f.storage);
    }
    fixture f; cs_uefi_load_result result; CHECK(init(&f)==0);
    f.system.vendor=NULL; f.system.firmware_revision=0; header(&f.system.header,UINT64_C(0x5453595320494249),sizeof(f.system));
    CHECK(cs_uefi_loader_run(&f,&f.system,&result)==0);
    CHECK(result.handoff->firmware_vendor_state==CS_UEFI_VENDOR_ABSENT && result.handoff->firmware_revision==0);
    for(size_t i=0;i<32;++i) CHECK(result.handoff->firmware_vendor[i]==0);
    free(f.storage);
    puts("Loader firmware identity: bounded printable vendor copy, truncation and absence PASS"); return 0;
}
int main(int argc,char **argv)
{
    if(argc!=2) return 2;
    if(strcmp(argv[1],"crc")==0) return crc_tests();
    if(strcmp(argv[1],"transactions")==0) return transactions();
    if(strcmp(argv[1],"rejection")==0) return rejection();
    if(strcmp(argv[1],"ownership")==0) return ownership();
    if(strcmp(argv[1],"configuration")==0) return configuration();
    if(strcmp(argv[1],"vendor")==0) return vendor();
    return 2;
}
