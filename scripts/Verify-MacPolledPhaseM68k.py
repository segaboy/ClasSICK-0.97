#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Compile only the isolated phase model;no MMIO,boot,link or target run."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import struct
import subprocess

def main():
    p=argparse.ArgumentParser()
    p.add_argument('--tool-root',required=True,type=Path)
    p.add_argument('--output',required=True,type=Path)
    a=p.parse_args();r=Path(__file__).resolve().parents[1]
    a.output.mkdir(parents=True,exist_ok=False)
    bins=a.tool_root/'usr/bin';cc=bins/'m68k-linux-gnu-gcc-13';nm=bins/'m68k-linux-gnu-nm'
    env=os.environ.copy();env['LD_LIBRARY_PATH']=str(a.tool_root/'usr/lib/x86_64-linux-gnu')
    def run(args):
        result=subprocess.run([str(x) for x in args],env=env,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,timeout=60)
        if result.returncode:raise RuntimeError('Original phase compilation failed:\n'+result.stdout)
        return result.stdout
    flags=['-mcpu=68000','-std=c99','-pedantic-errors','-ffreestanding','-fno-builtin',
           '-fno-stack-protector','-fno-ident','-fno-pic','-fno-pie',
           '-Wall','-Wextra','-Werror','-Wconversion','-Wsign-conversion',
           '--sysroot='+str(a.tool_root),'-I'+str(r),
           '-ffile-prefix-map='+str(r)+'=.','-ffile-prefix-map='+str(a.output)+'=.']
    layout=a.output/'layout.c'
    layout.write_text('#include "platform/mac128k/polled-phase.h"\n'
        'const unsigned char cs_polled_state_size[sizeof(cs_mac_polled_phase)]={0};\n'
        'const unsigned char cs_polled_sample_size[sizeof(cs_pointer_sample)]={0};\n')
    lo=a.output/'layout.o';run([cc,*flags,'-Os','-c',layout,'-o',lo])
    symbols=run([nm,'-S','--defined-only','--format=posix',lo])
    for name,size in [('cs_polled_state_size',40),('cs_polled_sample_size',12)]:
        m=re.search(r'^'+name+r' R \S+ (\S+)$',symbols,re.M)
        if not m or int(m[1],16)!=size:raise ValueError('Model layout: '+name)
    rows=[]
    for opt in ('0','2','s'):
        twins=[];frames=[]
        for twin in ('a','b'):
            folder=a.output/('O'+opt+'-'+twin);folder.mkdir();obj=folder/'phase.o'
            run([cc,*flags,'-O'+opt,'-fstack-usage','-Wframe-larger-than=128',
                 '-c',r/'platform/mac128k/polled-phase.c','-o',obj])
            b=obj.read_bytes()
            if b[:7]!=b'\x7fELF\x01\x02\x01' or struct.unpack_from('>HH',b,16)!=(1,4):
                raise ValueError('Not a big-endian MC68000-family relocatable object')
            if run([nm,'-u',obj]).strip():raise ValueError('Unexpected external helper')
            defined=run([nm,'--defined-only','--format=posix',obj])
            if re.search(r'^\S+ [BbCcDdGgSsVv] ',defined,re.M):raise ValueError('Mutable global data')
            for name in ('cs_mac_polled_begin','cs_mac_polled_step'):
                if not re.search(r'^'+name+r' T ',defined,re.M):raise ValueError('Missing model API')
            for line in obj.with_suffix('.su').read_text().splitlines():
                name,n,kind=line.rsplit('\t',2)
                if kind not in ('static','dynamic,bounded') or int(n)>128:raise ValueError('Model stack frame')
                if twin=='a':frames.append({'function':name.split(':')[-1],'bytes':int(n)})
            twins.append(hashlib.sha256(b).hexdigest())
        if twins[0]!=twins[1]:raise ValueError('Object twins differ')
        rows.append({'optimization':'O'+opt,'twins_equal':True,'object_sha256':twins[0],'imports':[],
                     'mutable_globals':0,'frames':frames})
    files=['platform/mac128k/polled-phase.c','platform/mac128k/polled-phase.h','core/input/pointer.h',
           'scripts/Verify-MacPolledPhaseM68k.py']
    report={'specification':'SPEC-0074','cpu':'68000','compiler':run([cc,'--version']).splitlines()[0],'object_compilations':7,'state_bytes':40,
            'sample_bytes':12,'records':rows,'inputs':{f:hashlib.sha256((r/f).read_bytes().replace(b'\r\n',b'\n')).hexdigest() for f in files},
            'linked':False,'target_executed':False,'ROM_inputs_changed':False,
            'hardware_sampling_or_whole_stack_qualified':False}
    (a.output/'manifest.json').write_text(json.dumps(report,indent=2)+'\n')
    print('Polled phase68000:7 objects,40B state,matched O0/O2/Os twins,no imports/mutable globals;not linked or run.')

if __name__=='__main__':main()
