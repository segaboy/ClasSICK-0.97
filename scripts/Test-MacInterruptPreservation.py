# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Re-run exact traced production IRQ boundaries and compare stopped registers."""
import argparse
import copy
import hashlib
import json
from pathlib import Path
import struct
import subprocess


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def preserved(before, after):
    a, b = before['cpu'], after['cpu']
    if a['d'] != b['d'] or a['a'][:7] != b['a'][:7]:
        raise ValueError('data/address registers changed across interrupt')
    if b['a'][7] != a['a'][7] + 6:
        raise ValueError('exception stack not balanced')
    memory=before['memory']; offset=a['a'][7]-memory['address']
    data=bytes.fromhex(memory['bytes'])
    if offset<0 or offset+6>len(data): raise ValueError('exception frame not captured')
    saved_sr,saved_pc=struct.unpack_from('>HI',data,offset)
    if saved_sr & 0x0700 or a['sr'] != (saved_sr | 0x0700):
        raise ValueError('unexpected entry status or interrupted mask')
    if b['sr'] != saved_sr or b['pc'] != saved_pc:
        raise ValueError('RTE did not restore the saved SR/PC')
    if before['status'] != 'cycle_limit' or after['status'] != 'cycle_limit':
        raise ValueError('unexpected stop')


def controls():
    a = {'status':'cycle_limit','cpu':{'d':[0]*8,'a':[0]*7+[100],'sr':0x2715,'pc':0x400104},
         'memory':{'address':100,'bytes':'201500401234'}}
    b = copy.deepcopy(a); b['cpu']['a'][7] = 106; b['cpu']['sr'] = 0x2015; b['cpu']['pc']=0x401234
    preserved(a,b)
    for name, index in [('d',i) for i in range(8)]+[('a',i) for i in range(8)]+[('sr',None),('pc',None)]:
        bad = copy.deepcopy(b)
        if index is None: bad['cpu'][name] ^= 1
        else: bad['cpu'][name][index] ^= 1
        try: preserved(a,bad)
        except ValueError: pass
        else: raise ValueError('synthetic negative control missed '+str((name,index)))


def main():
    controls()
    p = argparse.ArgumentParser(description=__doc__)
    for name in ('emulator','probe-report','manifest','output'):
        p.add_argument('--'+name,type=Path,required=True)
    a = p.parse_args(); exe=a.emulator.resolve(strict=True); report=a.probe_report.resolve(strict=True)
    out=a.output.resolve(); root=Path(__file__).resolve().parents[1]
    if out==root or root in out.parents: p.error('output must be outside repository')
    probe=json.loads(report.read_text()); manifest=json.loads(a.manifest.read_text())
    identity=probe['identities']; current_exe=sha(exe)
    if identity['emulator'] != current_exe: raise ValueError('probe executable differs')
    profile=next(x for x in manifest['profiles'] if x['image_sha256']==identity['rom'])
    irq=profile['symbols']['cs_mac_window_scc_irq']
    stack_top=profile['native_stack_top']; stack_start=stack_top-4096
    out.mkdir(parents=True,exist_ok=False); observations=[]; hashes={}
    for case in probe['cases']:
        original=case['commands'][-1]
        if not case['traced_matches_plain']: raise ValueError('trace changed guest result')
        rom=Path(original[original.index('--rom')+1]); trace=Path(original[original.index('--mouse-witness')+1])
        if sha(rom)!=identity['rom'] or sha(trace)!=case['witness']['sha256']:
            raise ValueError('probe input or trace changed')
        hashes[str(rom)]=sha(rom);hashes[str(trace)]=sha(trace)
        header, fields, *lines=trace.read_text(encoding='ascii').splitlines()
        h=header.split()
        if h[0]!='BHMACMOUSE2' or h[5:8]!=['0','1','1'] or len(lines)!=int(h[4]):
            raise ValueError('incomplete or dropped witness')
        rows=[dict(zip(fields.split(),line.split())) for line in lines]
        masks=[r for r in rows if r['kind']=='cpu_mask']
        if any(x['value']!=y['reg'] for x,y in zip(masks,masks[1:])):
            raise ValueError('broken mask chain')
        spans=[(int(x['cycle']),int(y['cycle'])) for x,y in zip(masks,masks[1:])
               if int(x['address'])==irq and x['reg']!='8' and x['value']=='7' and y['value']=='0']
        if not spans: raise ValueError('no complete SCC return to foreground')
        selected=sorted({spans[0],max(spans,key=lambda pair:pair[1]-pair[0])})
        options=[]
        for flag in ('--mouse-motion','--mouse-dcd-invert','--mouse-edge-cycles','--mouse-vbl-guard'):
            index=original.index(flag); options.extend((flag,original[index+1]))
        for start,end in selected:
            states=[];commands=[]
            for cycle in (start,end):
                cmd=[str(exe),'--machine','mac128k','--rom',str(rom),'--cycles',str(cycle),
                     '--timeout-ms','20000',*options,'--mac-cpu-state','1',
                     '--inspect',f'{stack_start}:4096']
                r=subprocess.run(cmd,capture_output=True,text=True,timeout=25,
                                 creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
                (out/f'{case["edge_cycles"]}-{cycle}.log').write_text(r.stdout+r.stderr)
                state=json.loads(r.stdout)
                if r.returncode or state['cycles']!=cycle: raise ValueError('boundary was not reproduced exactly')
                states.append(state);commands.append(cmd)
            if states[0]['cpu']['pc']!=irq+4: raise ValueError('entry is not just after veneer mask')
            preserved(*states)
            observations.append({'edge_cycles':case['edge_cycles'],'start':start,'end':end,
                                 'before':states[0],'after':states[1],'commands':commands})
    if sha(exe)!=current_exe or any(sha(Path(path))!=digest for path,digest in hashes.items()):
        raise ValueError('input changed during checks')
    result={'status':'pass','observations':observations,'negative_controls':18,
            'identities':{'emulator':current_exe,'probe':sha(report),'manifest':sha(a.manifest),'script':sha(Path(__file__)),**hashes},
            'limits':'First and longest complete SCC spans in the supplied +X schedules; not all paths, interrupts, physical hardware or release qualification.'}
    (out/'report.json').write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps({'status':'pass','spans':len(observations),'negative_controls':18,'report':str(out/'report.json')}))


if __name__=='__main__': main()
