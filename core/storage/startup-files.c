/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "startup-files.h"
static void clear_bytes(void *object,size_t count)
{uint8_t *p=object;size_t i;for(i=0;i<count;++i)p[i]=0;}
static void copy_bytes(void *to,const void *from,size_t count)
{uint8_t *d=to;const uint8_t *s=from;size_t i;for(i=0;i<count;++i)d[i]=s[i];}
static cs_startup_result refuse(cs_startup_report *report,
    cs_startup_domain domain,unsigned status)
{report->domain=domain;report->status=status;return CS_STARTUP_REFUSED;}

cs_startup_result cs_startup_files_open(const cs_block_device *device,
    cs_startup_workspace *scratch,cs_startup_files *out,cs_startup_report *report)
{
    cs_startup_files *candidate;cs_mac_boot_result boot;cs_mfsv_result mfs;
    cs_rfs_result resource;cs_rfs_source source;cs_rfs_report detail;unsigned i;
    if(device==NULL || scratch==NULL || out==NULL || report==NULL ||
        device->read==NULL || device->blocks==0 ||
        (const void *)device==(const void *)scratch ||
        (const void *)device==(const void *)out ||
        (const void *)device==(const void *)report ||
        (const void *)scratch==(const void *)out ||
        (const void *)scratch==(const void *)report ||
        (const void *)out==(const void *)report)return CS_STARTUP_ARGUMENT;
    clear_bytes(report,sizeof *report);report->file=CS_STARTUP_NO_FILE;
    report->stage=CS_STARTUP_HEADER;candidate=&scratch->candidate;
    clear_bytes(candidate,sizeof *candidate);
    boot=cs_mac_boot_header_read(device,&scratch->boot,&candidate->header);
    if(boot!=CS_MAC_BOOT_OK)return refuse(report,CS_STARTUP_BOOT,(unsigned)boot);
    report->stage=CS_STARTUP_MOUNT;
    mfs=cs_mfsv_mount(device,&scratch->disk,&candidate->volume);
    if(mfs!=CS_MFSV_OK)return refuse(report,CS_STARTUP_MFS,(unsigned)mfs);
    for(i=0;i<CS_STARTUP_FILES;++i) {
        const cs_mac_boot_name *name=&candidate->header.names[i];
        report->file=i;report->stage=CS_STARTUP_FIND;
        mfs=cs_mfsv_find(&candidate->volume,&scratch->disk,
            name->bytes,name->length,&candidate->files[i]);
        if(mfs!=CS_MFSV_OK)return refuse(report,CS_STARTUP_MFS,(unsigned)mfs);
        report->stage=CS_STARTUP_BIND;
        mfs=cs_rfs_mfs_bind(&candidate->volume,&candidate->files[i],
            &scratch->disk,&candidate->adapters[i],&source);
        if(mfs!=CS_MFSV_OK)return refuse(report,CS_STARTUP_MFS,(unsigned)mfs);
        report->stage=CS_STARTUP_OPEN;
        resource=cs_rfs_open(&source,&scratch->resource,&candidate->resources[i],&detail);
        if(resource!=CS_RFS_OK) {
            report->field=detail.field;report->position=detail.position;
            return refuse(report,CS_STARTUP_RESOURCE,(unsigned)resource);
        }
        /* Check diagnostic additions before counting this completed open. */
        if(detail.types>UINT32_MAX-report->types ||
            detail.references>UINT32_MAX-report->references)
            return refuse(report,CS_STARTUP_RESOURCE,(unsigned)CS_RFS_OVERFLOW);
        ++report->opened;report->types+=detail.types;
        report->references+=detail.references;report->warnings|=detail.warnings;
    }
    /* No fallible calls follow publication. Pointers into the construction
     * object are rebased to the stable output; the disk workspace is borrowed. */
    copy_bytes(out,candidate,sizeof *out);
    for(i=0;i<CS_STARTUP_FILES;++i) {
        out->adapters[i].volume=&out->volume;
        out->adapters[i].entry=&out->files[i];
        out->resources[i].source.context=&out->adapters[i];
    }
    report->stage=CS_STARTUP_COMPLETE;report->file=CS_STARTUP_NO_FILE;
    return CS_STARTUP_OK;
}
