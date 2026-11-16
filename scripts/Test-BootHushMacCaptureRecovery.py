# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Overrun the raw capture FIFO, then verify release and fresh movement."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess

ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ('emulator','rom','manifest','output'):p.add_argument('--'+n,type=Path,required=True)
    a=p.parse_args();exe=a.emulator.resolve(strict=True);rom=a.rom.resolve(strict=True);out=a.output.resolve()
    b=json.loads(a.manifest.read_text());profile=b['profiles'][0];s=profile['symbols']
    if not b.get('buffered_collect') or not b['twins_equal'] or profile['image_sha256']!=sha(rom):
        raise ValueError('buffered ROM identity mismatch')
    for n,digest in b['inputs'].items():
        if hashlib.sha256((ROOT/n).read_bytes().replace(b'\r\n',b'\n')).hexdigest()!=digest:
            raise ValueError('source changed: '+n)
    if out==ROOT or ROOT in out.parents:raise ValueError('output must be outside repository')
    out.mkdir(parents=True,exist_ok=False)
    start=min(s['cs_mac_trace'],s['cs_mac_diag'],s['cs_mac_capture_trace'])
    end=max(s['cs_mac_trace']+128,s['cs_mac_diag']+28,s['cs_mac_capture_trace']+16)
    cmd=[str(exe),'--machine','mac128k','--rom',str(rom),'--cycles','170000000','--timeout-ms','20000',
         '--mouse-dcd-invert','3','--mouse-edge-cycles','2048','--mouse-vbl-guard','4096',
         '--mouse-motion','100000000:0:-96','--mouse-button','112000000:1',
         '--mouse-motion','120000000:5000:0','--mouse-button','148000000:0',
         '--mouse-motion','160000000:-16:0','--capture-series',str(out/'frames.pbmseq'),
         '--capture-cycle','154000000','--capture-cycle','170000000',
         '--capture-memory',f'{start}:{end-start}']
    r=subprocess.run(cmd,capture_output=True,text=True,timeout=25,
                     creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
    (out/'run.log').write_text(r.stdout+'\n--- stderr ---\n'+r.stderr)
    d=json.loads(r.stdout)
    if r.returncode or d['status']!='cycle_limit' or d['mouse']['pending']:
        raise ValueError('run did not complete')
    f=d['frame_series']
    if (f['processed'],f['pending'],f['dropped'])!=(2,0,0):raise ValueError('missing capture')
    observations=[]
    for v in f['samples']:
        raw=bytes.fromhex(v['memory']['bytes']);word=lambda n:int.from_bytes(raw[n-start:n-start+4],'big')
        t=s['cs_mac_trace'];q=s['cs_mac_capture_trace'];diag=s['cs_mac_diag']
        row={'cycle':v['cycle'],'pointer':[word(t+52),word(t+56)],'window':[word(t+60),word(t+64)],
             'held':word(t+88),'losses':word(t+36),'first_reason':word(t+100),
             'capture_queue':[word(q+i*4) for i in range(4)],'fault':word(diag+24),'presses':word(diag+12)}
        if (row['held'] or row['fault'] or row['first_reason']!=5 or row['losses']<1 or
            row['capture_queue'][0:3]!=[0x4d435131,0,128] or row['capture_queue'][3]<1):
            raise ValueError('overrun did not cancel and drain: '+str(row))
        observations.append(row)
    before,after=observations
    if (after['pointer']!=[max(0,before['pointer'][0]-16),before['pointer'][1]] or
        after['window']!=before['window'] or after['losses']!=before['losses'] or
        after['presses']!=before['presses']):raise ValueError('fresh movement after release failed')
    report={'status':'pass','command':cmd,'observations':observations,
            'emulator_sha256':sha(exe),'rom_sha256':sha(rom),'manifest_sha256':sha(a.manifest),
            'frames_sha256':sha(out/'frames.pbmseq'),'scope':'modeled excessive burst recovery; no lossless or physical claim'}
    (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps({'status':'pass','report':str(out/'report.json')}))
if __name__=='__main__':main()
