/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../tools/core-link/probe.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    const size_t size=CS_CORE_LINK_STORAGE+32u;
    /* Allocated storage permits native typed objects, beyond alignment alone. */
    unsigned char *first=malloc(size), *second=malloc(size), *before=malloc(size);
    int result=1;
    if (first==NULL || second==NULL || before==NULL) goto cleanup;
    memset(first,0x71,size);
    memset(second,0x83,size);
    memcpy(before,first,size);
    if (cs_core_link_probe(NULL,CS_CORE_LINK_STORAGE)!=1
            || cs_core_link_probe(first+16,CS_CORE_LINK_STORAGE-1)!=1
            || cs_core_link_probe(first+16,CS_CORE_LINK_STORAGE+1)!=1
            || memcmp(first,before,size)!=0) goto cleanup;
    if (cs_core_link_probe(first+16,CS_CORE_LINK_STORAGE)!=0) goto cleanup;
    for (size_t i=0;i<size;++i) if (second[i]!=0x83) goto cleanup;
    if (cs_link_entry(second+16,CS_CORE_LINK_STORAGE)!=0) goto cleanup;
    for (size_t i=0;i<16;++i) {
        if (first[i]!=0x71 || first[CS_CORE_LINK_STORAGE+16+i]!=0x71
                || second[i]!=0x83 || second[CS_CORE_LINK_STORAGE+16+i]!=0x83) goto cleanup;
    }
    puts("Core link probe behavior/ownership: PASS");
    result=0;
cleanup:
    free(first); free(second); free(before);
    return result;
}
