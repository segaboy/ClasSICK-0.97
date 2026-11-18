/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "mfs-transport.h"
static cs_rfs_io read_mfs(void *context,uint32_t offset,uint16_t count,uint8_t *destination)
{
    cs_rfs_mfs_adapter *a=context;cs_mfsv_result result;
    if(a==NULL || destination==NULL || count==0 || count>CS_RFS_CHUNK)return CS_RFS_IO_PROVIDER;
    result=cs_mfsv_fork_read(a->volume,a->scratch,a->entry,CS_MFSV_RESOURCE,
        offset,count,destination,count,a->staging,sizeof a->staging);
    if(result==CS_MFSV_OK)return CS_RFS_IO_OK;
    if(result==CS_MFSV_UNAVAILABLE)return CS_RFS_IO_UNAVAILABLE;
    if(result==CS_MFSV_IO)return CS_RFS_IO_ERROR;
    return CS_RFS_IO_PROVIDER;
}
cs_mfsv_result cs_rfs_mfs_bind(const cs_mfsv_volume *volume,const cs_mfs_entry *entry,
    cs_mfsv_workspace *scratch,cs_rfs_mfs_adapter *adapter,cs_rfs_source *source)
{
    cs_mfsv_result result;
    if(volume==NULL || entry==NULL || scratch==NULL || adapter==NULL || source==NULL)
        return CS_MFSV_ARGUMENT;
    result=cs_mfsv_fork_validate(volume,entry,CS_MFSV_RESOURCE);
    if(result!=CS_MFSV_OK)return result;
    adapter->volume=volume;adapter->entry=entry;adapter->scratch=scratch;
    source->length=entry->resource_logical;source->read=read_mfs;source->context=adapter;
    return CS_MFSV_OK;
}
