/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* SPEC-0014 hosted conformance. The oracle below re-reads the image from the
   published field tables and recomputes CRCs with a separate table method. */
#include "../platform/pc/media.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned failures;
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); ++failures; } } while(0)
#define SECTOR(lba) (img+(size_t)(lba)*512u)

static uint32_t table[256];
static uint32_t oracle_crc(const unsigned char *p,size_t n)
{
    if(table[1]==0) for(uint32_t i=0;i<256;++i) {
        uint32_t c=i; for(int k=0;k<8;++k) c=(c&1u)?(c>>1)^0xEDB88320u:c>>1; table[i]=c;
    }
    uint32_t c=0xFFFFFFFFu;
    for(size_t i=0;i<n;++i) c=table[(c^p[i])&0xFFu]^(c>>8);
    return c^0xFFFFFFFFu;
}
static uint32_t g16(const unsigned char *p) { return (uint32_t)p[0]|((uint32_t)p[1]<<8); }
static uint32_t g32(const unsigned char *p) { return g16(p)|(g16(p+2)<<16); }
static uint64_t g64(const unsigned char *p) { return (uint64_t)g32(p)|((uint64_t)g32(p+4)<<32); }
static int zero(const unsigned char *p,size_t n) { for(size_t i=0;i<n;++i) if(p[i]) return 0; return 1; }

typedef struct { uint32_t vol,reserved,fatsz,data,clusters; } fat_view;
static uint32_t next(const unsigned char *img,const fat_view *v,uint32_t n,uint32_t fat)
{
    return g32(SECTOR(v->vol+v->reserved+fat*v->fatsz)+4u*n)&0x0FFFFFFFu;
}
static const unsigned char *clus(const unsigned char *img,const fat_view *v,uint32_t n)
{
    return SECTOR(v->vol+v->data+(n-2u));
}
static const unsigned char *find(const unsigned char *dir,const char *name,unsigned attr)
{
    for(unsigned i=0;i<16;++i) {
        const unsigned char *d=dir+32u*i;
        if(d[0]==0) return NULL;
        if(memcmp(d,name,11)==0 && d[11]==attr) return d;
    }
    return NULL;
}
static uint32_t first(const unsigned char *d) { return (g16(d+20)<<16)|g16(d+26); }

/* Returns the number of failed checks found in one image. */
static unsigned verify(const unsigned char *img,const unsigned char *payload,size_t size)
{
    unsigned before=failures;
    static const unsigned char esp[16]={0x28,0x73,0x2A,0xC1,0x1F,0xF8,0xD2,0x11,0xBA,0x4B,0x00,0xA0,0xC9,0x3E,0xC9,0x3B};
    const unsigned char *mbr=SECTOR(0), *rec=mbr+446;
    CHECK(mbr[510]==0x55 && mbr[511]==0xAA && zero(mbr,446) && zero(mbr+462,48));
    CHECK(rec[0]==0 && rec[1]==0 && rec[2]==2 && rec[3]==0 && rec[4]==0xEE);
    CHECK(rec[5]==0xFF && rec[6]==0xFF && rec[7]==0xFF && g32(rec+8)==1 && g32(rec+12)==131071u);
    uint64_t total=CS_MEDIA_BYTES/512u;
    for(int b=0;b<2;++b) {
        const unsigned char *h=SECTOR(b?total-1u:1u);
        unsigned char copy[92]; memcpy(copy,h,92); memset(copy+16,0,4);
        CHECK(memcmp(h,"EFI PART",8)==0 && g32(h+8)==0x00010000u && g32(h+12)==92u);
        CHECK(g32(h+16)==oracle_crc(copy,92) && g32(h+20)==0 && zero(h+92,420));
        CHECK(g64(h+24)==(b?total-1u:1u) && g64(h+32)==(b?1u:total-1u));
        uint64_t array=g64(h+72), entries=g32(h+80), esz=g32(h+84);
        CHECK(entries*esz>=16384u && (esz==128u||esz==256u||esz==512u));
        CHECK(g64(h+40)>=34u && g64(h+48)<total-1u-(entries*esz)/512u);
        CHECK(b ? array==g64(h+48)+1u : array==2u);
        CHECK(g32(h+88)==oracle_crc(SECTOR(array),(size_t)(entries*esz)));
        CHECK(memcmp(h+56,SECTOR(1)+56,16)==0);
        const unsigned char *e=SECTOR(array);
        CHECK(memcmp(e,esp,16)==0 && !zero(e+16,16) && g64(e+48)==0);
        CHECK(g64(e+32)%2048u==0 && g64(e+32)>=g64(h+40) && g64(e+40)<=g64(h+48) && g64(e+40)>=g64(e+32));
        CHECK(zero(e+128,(size_t)(entries*esz)-128u));
    }
    const unsigned char *e=SECTOR(2);
    fat_view v; v.vol=(uint32_t)g64(e+32);
    uint32_t partsz=(uint32_t)(g64(e+40)-g64(e+32)+1u);
    const unsigned char *bs=SECTOR(v.vol);
    CHECK((bs[0]==0xEB && bs[2]==0x90) || bs[0]==0xE9);
    CHECK(g16(bs+11)==512u && bs[13]==1 && g16(bs+14)!=0 && bs[16]==2 && g16(bs+17)==0 && g16(bs+19)==0);
    CHECK(bs[21]==0xF8 && g16(bs+22)==0 && g32(bs+28)==v.vol && g32(bs+32)==partsz);
    CHECK(g16(bs+40)==0 && g16(bs+42)==0 && g32(bs+44)==2 && g16(bs+48)==1 && g16(bs+50)==6 && zero(bs+52,12));
    CHECK(bs[66]==0x29 && memcmp(bs+71,"NO NAME    ",11)==0 && memcmp(bs+82,"FAT32   ",8)==0);
    CHECK(bs[510]==0x55 && bs[511]==0xAA);
    v.reserved=g16(bs+14); v.fatsz=g32(bs+36); v.data=v.reserved+2u*v.fatsz;
    v.clusters=(partsz-v.data)/1u;
    CHECK(v.clusters>=65525u+16u && (uint64_t)(v.clusters+2u)*4u<=(uint64_t)v.fatsz*512u);
    CHECK((v.vol+v.data)%2048u==0);
    const unsigned char *fsi=SECTOR(v.vol+1u);
    CHECK(g32(fsi)==0x41615252u && g32(fsi+484)==0x61417272u && g32(fsi+508)==0xAA550000u);
    CHECK(zero(fsi+4,480) && zero(fsi+496,12));
    CHECK(memcmp(SECTOR(v.vol+6u),bs,512)==0 && memcmp(SECTOR(v.vol+7u),fsi,512)==0);
    CHECK(memcmp(SECTOR(v.vol+v.reserved),SECTOR(v.vol+v.reserved+v.fatsz),(size_t)v.fatsz*512u)==0);
    const unsigned char *fat=SECTOR(v.vol+v.reserved);
    CHECK(g32(fat)==0x0FFFFFF8u && g32(fat+4)==0x0FFFFFFFu);
    CHECK(zero(fat+4u*(v.clusters+2u),(size_t)v.fatsz*512u-4u*(v.clusters+2u)));
    /* Walk \EFI\BOOT\BOOTX64.EFI exactly as a reader would. */
    uint32_t root=g32(bs+44); CHECK(next(img,&v,root,0)>=0x0FFFFFF8u);
    const unsigned char *efi=find(clus(img,&v,root),"EFI        ",0x10); CHECK(efi!=NULL);
    if(efi==NULL) return failures-before;
    uint32_t efic=first(efi); CHECK(g32(efi+28)==0 && next(img,&v,efic,0)>=0x0FFFFFF8u);
    const unsigned char *edir=clus(img,&v,efic);
    CHECK(memcmp(edir,".          ",11)==0 && first(edir)==efic && memcmp(edir+32,"..         ",11)==0 && first(edir+32)==0);
    const unsigned char *boot=find(edir,"BOOT       ",0x10); CHECK(boot!=NULL);
    if(boot==NULL) return failures-before;
    uint32_t bootc=first(boot); const unsigned char *bdir=clus(img,&v,bootc);
    CHECK(first(bdir)==bootc && first(bdir+32)==efic && next(img,&v,bootc,0)>=0x0FFFFFF8u);
    const unsigned char *file=find(bdir,"BOOTX64 EFI",0x20); CHECK(file!=NULL);
    if(file==NULL) return failures-before;
    CHECK(g32(file+28)==size);
    uint32_t c=first(file), used=0; size_t at=0;
    while(c>=2u && c<v.clusters+2u && at<size) {
        size_t n=size-at<512u?size-at:512u;
        CHECK(memcmp(clus(img,&v,c),payload+at,n)==0 && zero(clus(img,&v,c)+n,512u-n));
        at+=n; ++used; uint32_t f=next(img,&v,c,0);
        if(f>=0x0FFFFFF8u) break;
        CHECK(f==c+1u); c=f;
    }
    CHECK(at==size && used==(size+511u)/512u);
    uint32_t allocated=0;
    for(uint32_t n=2;n<v.clusters+2u;++n) if(next(img,&v,n,0)!=0) ++allocated;
    CHECK(allocated==3u+used && g32(fsi+488)==v.clusters-allocated && g32(fsi+492)==2u+allocated);
    /* Nothing outside the described structures is nonzero. */
    uint32_t extra=0;
    for(uint64_t s=0;s<total;++s) {
        int known=s<=2u || s==total-1u || s==g64(SECTOR(total-1u)+72) ||
            s==v.vol || s==v.vol+1u || s==v.vol+6u || s==v.vol+7u ||
            (s>=v.vol+v.reserved && s<v.vol+v.data) ||
            (s>=v.vol+v.data && s<v.vol+v.data+3u+used);
        if(!known && !zero(SECTOR(s),512)) ++extra;
    }
    CHECK(extra==0);
    return failures-before;
}

static unsigned char *make(size_t size,unsigned seed)
{
    unsigned char *p=malloc(size?size:1u); if(p==NULL) exit(2);
    for(size_t i=0;i<size;++i) p[i]=(unsigned char)((i*131u+seed*7u+(i>>9))&0xFFu);
    if(size>1) { p[0]='M'; p[1]='Z'; }
    return p;
}

static int crc_suite(void)
{
    CHECK(cs_media_crc32((const unsigned char *)"123456789",9)==0xCBF43926u);
    CHECK(cs_media_crc32((const unsigned char *)"",0)==0);
    unsigned char buf[1031]; for(unsigned i=0;i<sizeof buf;++i) buf[i]=(unsigned char)(i*37u+11u);
    for(size_t n=0;n<=sizeof buf;n+=103) CHECK(cs_media_crc32(buf,n)==oracle_crc(buf,n));
    printf("Media CRC: check value and table oracle PASS\n"); return failures!=0;
}

static int layout_suite(void)
{
    /* Microsoft FAT32 sizing with 32 reserved sectors, then reserve more so the
       data region starts on a 1-MiB disk boundary; FAT size may be larger than needed. */
    uint32_t total=CS_MEDIA_PART_SECTORS, tmp1=total-32u, tmp2=(256u*1u+2u)/2u;
    uint32_t fatsz=(tmp1+tmp2-1u)/tmp2;
    CHECK(fatsz==CS_MEDIA_FAT_SECTORS);
    uint32_t reserved=32u; while((CS_MEDIA_PART_FIRST+reserved+2u*fatsz)%2048u!=0) ++reserved;
    CHECK(reserved==CS_MEDIA_RESERVED && reserved+2u*fatsz==CS_MEDIA_DATA_FIRST);
    uint32_t clusters=total-CS_MEDIA_DATA_FIRST;
    CHECK(clusters==CS_MEDIA_CLUSTERS && clusters>=65525u+16u && (clusters+2u)*4u<=fatsz*512u);
    CHECK(66600u<total && total<=532480u);
    CHECK(CS_MEDIA_PART_FIRST+CS_MEDIA_PART_SECTORS-1u<=CS_MEDIA_LAST_USABLE);
    CHECK(CS_MEDIA_LAST_USABLE+1u==CS_MEDIA_BACKUP_ENTRIES && CS_MEDIA_BACKUP_ENTRIES+32u==CS_MEDIA_BACKUP_HEADER);
    CHECK(CS_MEDIA_BACKUP_HEADER+1u==CS_MEDIA_DISK_SECTORS && CS_MEDIA_MAX_PAYLOAD_CLUSTERS==clusters-3u);
    printf("Media layout: FAT32 sizing, alignment and GPT bounds PASS\n"); return failures!=0;
}

static int structure_suite(void)
{
    unsigned char *img=malloc(CS_MEDIA_BYTES); unsigned char *p=make(37888,1); cs_media_layout l;
    if(img==NULL) return 2;
    CHECK(cs_media_build(p,37888,img,CS_MEDIA_BYTES,&l)==CS_MEDIA_OK);
    CHECK(verify(img,p,37888)==0);
    CHECK(l.payload_clusters==74 && l.free_clusters==CS_MEDIA_CLUSTERS-77u && l.next_free==79);
    CHECK(l.header_crc==g32(img+512+16) && l.entries_crc==g32(img+512+88));
    CHECK(l.backup_crc==g32(SECTOR(CS_MEDIA_BACKUP_HEADER)+16));
    free(img); free(p);
    printf("Media structure: protective MBR, twin GPT, FAT32 volume and path PASS\n"); return failures!=0;
}

static int payload_suite(void)
{
    static const size_t sizes[]={1,2,511,512,513,1024,37888,65537};
    unsigned char *img=malloc(CS_MEDIA_BYTES); cs_media_layout l;
    if(img==NULL) return 2;
    for(unsigned i=0;i<sizeof sizes/sizeof sizes[0];++i) {
        unsigned char *p=make(sizes[i],i+2u);
        CHECK(cs_media_build(p,sizes[i],img,CS_MEDIA_BYTES,&l)==CS_MEDIA_OK);
        CHECK(verify(img,p,sizes[i])==0); free(p);
    }
    unsigned char *big=make(CS_MEDIA_MAX_PAYLOAD+1u,9);
    CHECK(cs_media_build(big,CS_MEDIA_MAX_PAYLOAD,img,CS_MEDIA_BYTES,&l)==CS_MEDIA_OK);
    CHECK(verify(img,big,CS_MEDIA_MAX_PAYLOAD)==0 && l.free_clusters==0);
    memset(img,0x5A,CS_MEDIA_BYTES); l.free_clusters=77;
    CHECK(cs_media_build(big,CS_MEDIA_MAX_PAYLOAD+1u,img,CS_MEDIA_BYTES,&l)==CS_MEDIA_CAPACITY);
    CHECK(img[0]==0x5A && img[CS_MEDIA_BYTES-1u]==0x5A && l.free_clusters==77);
    free(big); free(img);
    printf("Media payload: nine sizes including full capacity, and overflow rejection PASS\n"); return failures!=0;
}

static int arguments_suite(void)
{
    unsigned char *img=malloc(CS_MEDIA_BYTES+1024u); unsigned char p[4]={'M','Z',1,2}; cs_media_layout l;
    if(img==NULL) return 2;
    memset(img,0xA5,CS_MEDIA_BYTES+1024u); memset(&l,0xC3,sizeof l);
    CHECK(cs_media_build(NULL,4,img,CS_MEDIA_BYTES,&l)==CS_MEDIA_ARGUMENT);
    CHECK(cs_media_build(p,0,img,CS_MEDIA_BYTES,&l)==CS_MEDIA_ARGUMENT);
    CHECK(cs_media_build(p,4,NULL,CS_MEDIA_BYTES,&l)==CS_MEDIA_ARGUMENT);
    CHECK(cs_media_build(p,4,img,CS_MEDIA_BYTES,NULL)==CS_MEDIA_ARGUMENT);
    CHECK(cs_media_build(p,4,img,CS_MEDIA_BYTES-1u,&l)==CS_MEDIA_ARGUMENT);
    CHECK(cs_media_build(p,4,img,CS_MEDIA_BYTES+1u,&l)==CS_MEDIA_ARGUMENT);
    CHECK(cs_media_build(img+100,4,img,CS_MEDIA_BYTES,&l)==CS_MEDIA_ARGUMENT);
    CHECK(cs_media_build(img+CS_MEDIA_BYTES-2u,4,img,CS_MEDIA_BYTES,&l)==CS_MEDIA_ARGUMENT);
    CHECK(cs_media_build(img+6,4,img+8,CS_MEDIA_BYTES,&l)==CS_MEDIA_ARGUMENT);
    unsigned untouched=1; unsigned char *raw=(unsigned char *)&l;
    for(size_t i=0;i<CS_MEDIA_BYTES+1024u;++i) if(img[i]!=0xA5) untouched=0;
    for(size_t i=0;i<sizeof l;++i) if(raw[i]!=0xC3) untouched=0;
    CHECK(untouched);
    /* Adjacent, non-overlapping payload storage is accepted. */
    memcpy(img+CS_MEDIA_BYTES,p,4);
    CHECK(cs_media_build(img+CS_MEDIA_BYTES,4,img,CS_MEDIA_BYTES,&l)==CS_MEDIA_OK);
    CHECK(verify(img,p,4)==0);
    free(img);
    printf("Media arguments: nine rejections leave storage untouched PASS\n"); return failures!=0;
}

static int determinism_suite(void)
{
    unsigned char *a=malloc(CS_MEDIA_BYTES), *b=malloc(CS_MEDIA_BYTES), *p=make(37888,1); cs_media_layout la,lb;
    if(a==NULL || b==NULL) return 2;
    memset(a,0x11,CS_MEDIA_BYTES); memset(b,0xEE,CS_MEDIA_BYTES);
    CHECK(cs_media_build(p,37888,a,CS_MEDIA_BYTES,&la)==CS_MEDIA_OK);
    CHECK(cs_media_build(p,37888,b,CS_MEDIA_BYTES,&lb)==CS_MEDIA_OK);
    CHECK(memcmp(a,b,CS_MEDIA_BYTES)==0 && memcmp(&la,&lb,sizeof la)==0);
    p[37887]^=1u;
    CHECK(cs_media_build(p,37888,b,CS_MEDIA_BYTES,&lb)==CS_MEDIA_OK);
    CHECK(memcmp(a,b,CS_MEDIA_BYTES)!=0 && la.header_crc==lb.header_crc);
    free(a); free(b); free(p);
    printf("Media determinism: identical rebuild from dirty storage PASS\n"); return failures!=0;
}

int main(int argc,char **argv)
{
    if(argc!=2) return 2;
    if(strcmp(argv[1],"crc")==0) return crc_suite();
    if(strcmp(argv[1],"layout")==0) return layout_suite();
    if(strcmp(argv[1],"structure")==0) return structure_suite();
    if(strcmp(argv[1],"payload")==0) return payload_suite();
    if(strcmp(argv[1],"arguments")==0) return arguments_suite();
    if(strcmp(argv[1],"determinism")==0) return determinism_suite();
    return 2;
}
