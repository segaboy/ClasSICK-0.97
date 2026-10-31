/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Included after the native or test SCC/VIA accessors. */
void cs_mac_native_collect(void *cookie,uint8_t out[10])
{
    uint8_t r=scc_read(cookie,CS_MAC_SCC_A,3u);
    out[0]=r;
    if(r&(uint8_t)~(CS_MAC_SCC_EXT_A|CS_MAC_SCC_EXT_B))return;
    out[1]=motion_pb(cookie); out[2]=scc_read(cookie,CS_MAC_SCC_A,0u);
    out[3]=scc_read(cookie,CS_MAC_SCC_B,0u);
    out[4]=motion_pb(cookie); out[5]=scc_read(cookie,CS_MAC_SCC_A,0u);
    out[6]=scc_read(cookie,CS_MAC_SCC_B,0u);
    if(r&CS_MAC_SCC_EXT_A)scc_write(cookie,CS_MAC_SCC_A,0u,CS_MAC_SCC_RESET_EXT);
    if(r&CS_MAC_SCC_EXT_B)scc_write(cookie,CS_MAC_SCC_B,0u,CS_MAC_SCC_RESET_EXT);
    scc_write(cookie,CS_MAC_SCC_A,0u,CS_MAC_SCC_RESET_IUS);
    out[7]=motion_pb(cookie); out[8]=scc_read(cookie,CS_MAC_SCC_A,0u);
    out[9]=scc_read(cookie,CS_MAC_SCC_B,0u);
}
