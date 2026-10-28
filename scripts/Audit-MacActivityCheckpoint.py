# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Verify the exact original MAR1 mirror; never accesses a guest or device."""
import struct

def audit(image,syms,enabled):
    entry=syms.get('cs_mac_activity_checkpoint');spin=syms.get('cs_mac_activity_spin')
    if not enabled:
        if entry is not None or spin is not None:raise ValueError('Activity checkpoint in default ROM')
        return None
    trace=syms.get('cs_mac_activity_trace',0)
    if trace%4 or not 0x400<=trace<=0x2100-80:raise ValueError('Activity checkpoint own-RAM operand')
    expected=bytes.fromhex('007c07002c7c')+struct.pack('>I',trace)+bytes.fromhex('203c4d415231')
    for reg,offset in enumerate((12,16,20,24,28,32,40),1):expected+=struct.pack('>HH',0x202e|(reg<<9),offset)
    for reg,offset in enumerate((44,48,60,64,68,72,76)):expected+=struct.pack('>HH',0x206e|(reg<<9),offset)
    expected+=bytes.fromhex('60fe')
    if len(expected)!=74 or entry is None or spin!=entry+72 or entry%2 or not 260<=entry-0x400000<=len(image)-74:
        raise ValueError('Activity checkpoint native span/spin')
    if image[entry-0x400000:entry-0x400000+74]!=expected:raise ValueError('Activity checkpoint encoding')
    return {'format':'MAR1','entry':entry,'spin':spin,'bytes':74,'trace_address':trace,
        'exact_opcodes_verified':True,'guest_memory_writes':0,'MMIO_accesses':0,
        'additional_RAM_bytes':0,'semantic_session_shutdown':False}
