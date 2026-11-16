/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Included after the native or test SCC/VIA accessors. */
void cs_mac_native_collect(void *cookie,uint32_t out[4])
{
#if defined(__m68k__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    /* Store bytes directly in aligned packed words. Explicit byte stores
     * prevent the compiler from combining an odd-address sample pair. */
    volatile uint8_t *v=(volatile uint8_t *)out;
    uint8_t r=scc_read(cookie,CS_MAC_SCC_A,3u);
    out[0]=r;
    if(r&(uint8_t)~(CS_MAC_SCC_EXT_A|CS_MAC_SCC_EXT_B))return;
    v[4]=0;v[5]=motion_pb(cookie);v[6]=scc_read(cookie,CS_MAC_SCC_A,0u);
    v[7]=scc_read(cookie,CS_MAC_SCC_B,0u);
    v[8]=0;v[9]=motion_pb(cookie);
#if CLASSICK_MAC_SHORT_COLLECT
    v[10]=(r&CS_MAC_SCC_EXT_A)?v[6]:scc_read(cookie,CS_MAC_SCC_A,0u);
    v[11]=(r&CS_MAC_SCC_EXT_B)?v[7]:scc_read(cookie,CS_MAC_SCC_B,0u);
#else
    v[10]=scc_read(cookie,CS_MAC_SCC_A,0u);
    v[11]=scc_read(cookie,CS_MAC_SCC_B,0u);
#endif
    if(r&CS_MAC_SCC_EXT_A)scc_write(cookie,CS_MAC_SCC_A,0u,CS_MAC_SCC_RESET_EXT);
    if(r&CS_MAC_SCC_EXT_B)scc_write(cookie,CS_MAC_SCC_B,0u,CS_MAC_SCC_RESET_EXT);
    scc_write(cookie,CS_MAC_SCC_A,0u,CS_MAC_SCC_RESET_IUS);
    v[12]=0;v[13]=motion_pb(cookie);v[14]=scc_read(cookie,CS_MAC_SCC_A,0u);
    v[15]=scc_read(cookie,CS_MAC_SCC_B,0u);
#else
    uint8_t r=scc_read(cookie,CS_MAC_SCC_A,3u);
    uint32_t p,a,b;
    out[0]=r;
    if(r&(uint8_t)~(CS_MAC_SCC_EXT_A|CS_MAC_SCC_EXT_B))return;
    p=motion_pb(cookie); a=scc_read(cookie,CS_MAC_SCC_A,0u);
    b=scc_read(cookie,CS_MAC_SCC_B,0u);
    out[1]=p<<16|a<<8|b;
    p=motion_pb(cookie);
#if CLASSICK_MAC_SHORT_COLLECT
    /* Pending DCD stays latched; copied unrelated bits are not new reads. */
    if(!(r&CS_MAC_SCC_EXT_A))a=scc_read(cookie,CS_MAC_SCC_A,0u);
    if(!(r&CS_MAC_SCC_EXT_B))b=scc_read(cookie,CS_MAC_SCC_B,0u);
#else
    a=scc_read(cookie,CS_MAC_SCC_A,0u);
    b=scc_read(cookie,CS_MAC_SCC_B,0u);
#endif
    out[2]=p<<16|a<<8|b;
    if(r&CS_MAC_SCC_EXT_A)scc_write(cookie,CS_MAC_SCC_A,0u,CS_MAC_SCC_RESET_EXT);
    if(r&CS_MAC_SCC_EXT_B)scc_write(cookie,CS_MAC_SCC_B,0u,CS_MAC_SCC_RESET_EXT);
    scc_write(cookie,CS_MAC_SCC_A,0u,CS_MAC_SCC_RESET_IUS);
    p=motion_pb(cookie); a=scc_read(cookie,CS_MAC_SCC_A,0u);
    b=scc_read(cookie,CS_MAC_SCC_B,0u);
    out[3]=p<<16|a<<8|b;
#endif
}
