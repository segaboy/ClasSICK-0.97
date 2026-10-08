/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* SPEC-0014 host tool: wrap one owned EFI image in the fixed boot-media profile. */
#include "../../platform/pc/media.h"
#include <stdio.h>
#include <stdlib.h>

static int fail(const char *message)
{
    fprintf(stderr,"classick_boot_media: %s\n",message);
    return 1;
}

int main(int argc,char **argv)
{
    if(argc!=3) return fail("usage: classick_boot_media BOOTX64.EFI output.img");
    FILE *existing=fopen(argv[2],"rb");
    if(existing!=NULL) { fclose(existing); return fail("output exists; use a fresh path"); }
    FILE *in=fopen(argv[1],"rb");
    if(in==NULL) return fail("cannot open payload");
    unsigned char *payload=malloc(CS_MEDIA_MAX_PAYLOAD+1u);
    unsigned char *image=malloc(CS_MEDIA_BYTES);
    if(payload==NULL || image==NULL) { fclose(in); free(payload); free(image); return fail("out of memory"); }
    size_t size=fread(payload,1,CS_MEDIA_MAX_PAYLOAD+1u,in);
    int read_error=ferror(in); fclose(in);
    int status=0; cs_media_layout layout;
    if(read_error) status=fail("payload read failed");
    else if(size<2 || payload[0]!='M' || payload[1]!='Z') status=fail("payload is not an MZ image");
    else if(cs_media_build(payload,size,image,CS_MEDIA_BYTES,&layout)!=CS_MEDIA_OK)
        status=fail("payload does not fit the fixed profile");
    else {
        FILE *out=fopen(argv[2],"wb");
        if(out==NULL) status=fail("cannot create output");
        else {
            size_t written=fwrite(image,1,CS_MEDIA_BYTES,out);
            if(fclose(out)!=0 || written!=CS_MEDIA_BYTES) status=fail("output write failed");
        }
    }
    if(status==0)
        printf("Boot media: %u-byte payload, %u clusters, %u free; GPT CRC %08X/%08X, entries %08X\n",
            (unsigned)size,(unsigned)layout.payload_clusters,(unsigned)layout.free_clusters,
            (unsigned)layout.header_crc,(unsigned)layout.backup_crc,(unsigned)layout.entries_crc);
    free(payload); free(image);
    return status;
}
