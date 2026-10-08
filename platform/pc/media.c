/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "media.h"

/* SPEC-0014. Original writer for the fixed profile; every multi-byte field is
   stored little-endian byte by byte so the result does not depend on the host. */
enum { PART_LAST=CS_MEDIA_PART_FIRST+CS_MEDIA_PART_SECTORS-1, DATE=0x5D48,
    ROOT=2, EFI_DIR=3, BOOT_DIR=4 };
#define EOC UINT32_C(0x0FFFFFFF)
#define VOLUME_ID UINT32_C(0x20261008)
/* EFI_GUID byte order: first three fields little-endian (UEFI Appendix A). */
static const unsigned char esp_type[16]={0x28,0x73,0x2A,0xC1,0x1F,0xF8,0xD2,0x11,
    0xBA,0x4B,0x00,0xA0,0xC9,0x3E,0xC9,0x3B};
/* Fixed project GUIDs 5AD001EF-55C4-45C1-91E6-B46E536C16DD and
   2A9F2871-A46A-48FB-844C-5429E306796A keep the image reproducible. */
static const unsigned char disk_guid[16]={0xEF,0x01,0xD0,0x5A,0xC4,0x55,0xC1,0x45,
    0x91,0xE6,0xB4,0x6E,0x53,0x6C,0x16,0xDD};
static const unsigned char part_guid[16]={0x71,0x28,0x9F,0x2A,0x6A,0xA4,0xFB,0x48,
    0x84,0x4C,0x54,0x29,0xE3,0x06,0x79,0x6A};

static void put16(unsigned char *p,uint32_t v) { p[0]=(unsigned char)v; p[1]=(unsigned char)(v>>8); }
static void put32(unsigned char *p,uint32_t v) { put16(p,v&0xFFFFu); put16(p+2,v>>16); }
static void put64(unsigned char *p,uint64_t v) { put32(p,(uint32_t)v); put32(p+4,(uint32_t)(v>>32)); }
static void copy(unsigned char *to,const unsigned char *from,size_t n) { for(size_t i=0;i<n;++i) to[i]=from[i]; }
static unsigned char *sector(unsigned char *image,uint32_t lba) { return image+(size_t)lba*CS_MEDIA_SECTOR; }
static unsigned char *volume(unsigned char *image,uint32_t lba) { return sector(image,CS_MEDIA_PART_FIRST+lba); }
static unsigned char *cluster(unsigned char *image,uint32_t n)
{
    return volume(image,CS_MEDIA_DATA_FIRST+(n-2u));
}

uint32_t cs_media_crc32(const unsigned char *data,size_t size)
{
    uint32_t crc=UINT32_MAX;
    for(size_t i=0;i<size;++i) {
        crc^=data[i];
        for(unsigned bit=0;bit<8;++bit) crc=(crc>>1)^((crc&1u)?UINT32_C(0xEDB88320):0u);
    }
    return ~crc;
}

static void protective_mbr(unsigned char *s)
{
    unsigned char *record=s+446;
    record[2]=0x02; record[4]=0xEE; record[5]=0xFF; record[6]=0xFF; record[7]=0xFF;
    put32(record+8,1); put32(record+12,CS_MEDIA_DISK_SECTORS-1u);
    s[510]=0x55; s[511]=0xAA;
}

static void partition_entry(unsigned char *e)
{
    static const char name[]="ClasSICK ESP";
    copy(e,esp_type,16); copy(e+16,part_guid,16);
    put64(e+32,CS_MEDIA_PART_FIRST); put64(e+40,PART_LAST);
    for(unsigned i=0;name[i]!=0;++i) put16(e+56+2u*i,(unsigned char)name[i]);
}

static uint32_t gpt_header(unsigned char *s,uint32_t mine,uint32_t other,uint32_t entries,uint32_t array_crc)
{
    copy(s,(const unsigned char *)"EFI PART",8); put32(s+8,0x00010000u); put32(s+12,92);
    put64(s+24,mine); put64(s+32,other);
    put64(s+40,CS_MEDIA_FIRST_USABLE); put64(s+48,CS_MEDIA_LAST_USABLE);
    copy(s+56,disk_guid,16); put64(s+72,entries);
    put32(s+80,CS_MEDIA_ENTRIES); put32(s+84,CS_MEDIA_ENTRY_SIZE); put32(s+88,array_crc);
    uint32_t crc=cs_media_crc32(s,92); put32(s+16,crc);
    return crc;
}

static void boot_sector(unsigned char *s)
{
    static const unsigned char jump[3]={0xEB,0x58,0x90};
    /* Original legacy stub at 90: CLI, HLT, JMP back to HLT. UEFI never runs it. */
    static const unsigned char stub[4]={0xFA,0xF4,0xEB,0xFD};
    copy(s,jump,3); copy(s+3,(const unsigned char *)"MSWIN4.1",8);
    put16(s+11,CS_MEDIA_SECTOR); s[13]=1; put16(s+14,CS_MEDIA_RESERVED); s[16]=CS_MEDIA_FATS;
    s[21]=0xF8; put32(s+28,CS_MEDIA_PART_FIRST); put32(s+32,CS_MEDIA_PART_SECTORS);
    put32(s+36,CS_MEDIA_FAT_SECTORS); put32(s+44,ROOT); put16(s+48,1); put16(s+50,6);
    s[64]=0x80; s[66]=0x29; put32(s+67,VOLUME_ID);
    copy(s+71,(const unsigned char *)"NO NAME    ",11); copy(s+82,(const unsigned char *)"FAT32   ",8);
    copy(s+90,stub,4); s[510]=0x55; s[511]=0xAA;
}

static void fsinfo(unsigned char *s,uint32_t free_clusters,uint32_t next_free)
{
    put32(s,0x41615252u); put32(s+484,0x61417272u);
    put32(s+488,free_clusters); put32(s+492,next_free); put32(s+508,0xAA550000u);
}

static void entry(unsigned char *d,const char *name,unsigned char attr,uint32_t first,uint32_t size)
{
    copy(d,(const unsigned char *)name,11); d[11]=attr;
    put16(d+16,DATE); put16(d+18,DATE); put16(d+20,first>>16);
    put16(d+24,DATE); put16(d+26,first&0xFFFFu); put32(d+28,size);
}

static void fat_entry(unsigned char *image,uint32_t n,uint32_t value)
{
    for(uint32_t copy_index=0;copy_index<CS_MEDIA_FATS;++copy_index) {
        uint32_t offset=n*4u;
        unsigned char *fat=volume(image,CS_MEDIA_RESERVED+copy_index*CS_MEDIA_FAT_SECTORS);
        put32(fat+offset,value);
    }
}

cs_media_result cs_media_build(const unsigned char *payload,size_t payload_size,
    unsigned char *image,size_t image_size,cs_media_layout *out)
{
    if(payload==NULL || image==NULL || out==NULL || payload_size==0 || image_size!=CS_MEDIA_BYTES)
        return CS_MEDIA_ARGUMENT;
    if(payload_size>CS_MEDIA_MAX_PAYLOAD) return CS_MEDIA_CAPACITY;
    uintptr_t p=(uintptr_t)payload, i=(uintptr_t)image;
    if(p<i+image_size && i<p+payload_size) return CS_MEDIA_ARGUMENT;
    uint32_t clusters=(uint32_t)((payload_size+CS_MEDIA_SECTOR-1u)/CS_MEDIA_SECTOR);
    uint32_t next_free=CS_MEDIA_FIRST_PAYLOAD_CLUSTER+clusters;
    uint32_t free_clusters=CS_MEDIA_CLUSTERS-3u-clusters;
    for(size_t k=0;k<image_size;++k) image[k]=0;

    unsigned char *array=sector(image,2);
    partition_entry(array);
    uint32_t array_crc=cs_media_crc32(array,(size_t)CS_MEDIA_ENTRIES*CS_MEDIA_ENTRY_SIZE);
    copy(sector(image,CS_MEDIA_BACKUP_ENTRIES),array,(size_t)CS_MEDIA_ENTRIES*CS_MEDIA_ENTRY_SIZE);
    protective_mbr(sector(image,0));
    uint32_t header_crc=gpt_header(sector(image,1),1,CS_MEDIA_BACKUP_HEADER,2,array_crc);
    uint32_t backup_crc=gpt_header(sector(image,CS_MEDIA_BACKUP_HEADER),CS_MEDIA_BACKUP_HEADER,1,
        CS_MEDIA_BACKUP_ENTRIES,array_crc);

    boot_sector(volume(image,0)); fsinfo(volume(image,1),free_clusters,next_free);
    copy(volume(image,6),volume(image,0),CS_MEDIA_SECTOR);
    copy(volume(image,7),volume(image,1),CS_MEDIA_SECTOR);

    fat_entry(image,0,0x0FFFFFF8u); fat_entry(image,1,EOC);
    fat_entry(image,ROOT,EOC); fat_entry(image,EFI_DIR,EOC); fat_entry(image,BOOT_DIR,EOC);
    for(uint32_t n=0;n<clusters;++n) {
        uint32_t here=CS_MEDIA_FIRST_PAYLOAD_CLUSTER+n;
        fat_entry(image,here,n+1u==clusters?EOC:here+1u);
    }
    entry(cluster(image,ROOT),"EFI        ",0x10,EFI_DIR,0);
    entry(cluster(image,EFI_DIR),".          ",0x10,EFI_DIR,0);
    entry(cluster(image,EFI_DIR)+32,"..         ",0x10,0,0);
    entry(cluster(image,EFI_DIR)+64,"BOOT       ",0x10,BOOT_DIR,0);
    entry(cluster(image,BOOT_DIR),".          ",0x10,BOOT_DIR,0);
    entry(cluster(image,BOOT_DIR)+32,"..         ",0x10,EFI_DIR,0);
    entry(cluster(image,BOOT_DIR)+64,"BOOTX64 EFI",0x20,CS_MEDIA_FIRST_PAYLOAD_CLUSTER,(uint32_t)payload_size);
    copy(cluster(image,CS_MEDIA_FIRST_PAYLOAD_CLUSTER),payload,payload_size);

    out->payload_clusters=clusters; out->free_clusters=free_clusters; out->next_free=next_free;
    out->header_crc=header_crc; out->backup_crc=backup_crc; out->entries_crc=array_crc;
    return CS_MEDIA_OK;
}
