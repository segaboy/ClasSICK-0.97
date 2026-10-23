#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Boundary equivalence for a proposed C16 surface span proof; no core edit."""
import itertools

def original(width,height,stride,storage,rgba):
    row=width*4 if rgba else width//8+(width%8!=0)
    if row>65535:return 'overflow'
    if stride<row:return 'stride'
    if height>65535//stride:return 'overflow'
    span=stride*height
    if span>32767:return 'overflow'
    return 'ok' if storage>=span else 'storage'

def narrowed(width,height,stride,storage,rgba):
    if rgba:
        if width>65535//4:return 'overflow'
        row=width*4
    else:
        row=(width>>3)+(width%8!=0)
        if row>65535:return 'overflow'
    if stride<row:return 'stride'
    # stride>=row>=1. Replacing the two span limits preserves their same error.
    if height>32767//stride:return 'overflow'
    span=stride*height
    return 'ok' if storage>=span else 'storage'

if __name__=='__main__':
    widths=[1,7,8,9,512,8191,8192,16383,16384,262136,262144,524280,524281,2147483647]
    heights=[1,2,3,342,511,512,32767,32768,65535,65536,2147483647]
    strides=[1,2,4,63,64,65,32767,32768,65535]
    stores=[0,1,21887,21888,32766,32767,32768,65535]
    count=0
    for args in itertools.product(widths,heights,strides,stores,[False,True]):
        assert original(*args)==narrowed(*args),args
        count+=1
    assert original(512,342,64,21888,False)=='ok'
    assert original(512,512,64,65535,False)=='overflow'
    print(f'C16 width proof: {count} boundary combinations agree; abstract model, no production edit.')
