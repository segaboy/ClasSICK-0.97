# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Compare exact preview images at corner, clamp and repeated-click checkpoints."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import sys
from importlib.util import spec_from_file_location,module_from_spec
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def module(name,path):
    s=spec_from_file_location(name,path);m=module_from_spec(s);s.loader.exec_module(m);return m
def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ('emulator','rom','mac-manifest','pc-session','parity','host-cc','output'):p.add_argument('--'+n,type=Path,required=True)
    a=p.parse_args();out=a.output.resolve();pc=a.pc_session.resolve();exe=a.emulator.resolve();rom=a.rom.resolve()
    if out==ROOT or ROOT in out.parents:raise ValueError('output must be external')
    pair=json.loads(a.parity.read_text());mac=json.loads(a.mac_manifest.read_text());prior=json.loads((pc/'report.json').read_text())
    if (pair['status']!='pass' or pair['mac_rom_sha256']!=sha(rom) or pair['boothush_sha256']!=sha(exe)
        or pair['pc_media_sha256']!=sha(pc/'session.img') or pair['mac_manifest_sha256']!=sha(a.mac_manifest)):
        raise ValueError('paired identities changed')
    for name,d in mac['inputs'].items():
        if hashlib.sha256((ROOT/name).read_bytes().replace(b'\r\n',b'\n')).hexdigest()!=d:raise ValueError(name)
    for name,d in prior['source_sha256'].items():
        if sha(ROOT/name)!=d:raise ValueError(name)
    if sha(a.host_cc)!=prior['host_compiler_sha256']:raise ValueError('host compiler changed')
    out.mkdir(parents=True,exist_ok=False)
    def run(cmd,name,timeout=65):
        cmd=list(map(str,cmd));(out/(name+'-command.json')).write_text(json.dumps(cmd,indent=2)+'\n')
        r=subprocess.run(cmd,capture_output=True,text=True,timeout=timeout,creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
        (out/(name+'.log')).write_text(r.stdout+'\n--- stderr ---\n'+r.stderr)
        if r.returncode:raise ValueError(name+': '+r.stderr)
        return r.stdout
    helpers=module('session_helpers',ROOT/'scripts/Test-BootHushPC286SharedSession.py')
    oracle=module('pixel_oracle',ROOT/'scripts/Test-BootHushPC286SharedCompositor.py')
    host=out/'host.exe';fixture=ROOT/'probes/pc286/window-host-edges.c'
    run([a.host_cc,'-std=c11','-O2','-Wall','-Wextra','-Werror','-DCLASSICK_PRODUCT_ROM_CACHE=0',fixture,
         *[ROOT/n for n in helpers.SOURCES],'-o',host],'host-build')
    # An independent scalar state model supplies expected geometry and counts.
    steps=[];checks=[];x,y,wx,wy,pattern,held,presses=256,171,80,70,0,0,0
    drag=False;gx=gy=0;rawx,rawy=256,171
    def step(dx,dy,button):
        nonlocal x,y,wx,wy,pattern,held,presses,drag,gx,gy,rawx,rawy
        oldx,oldy=x,y;x=max(0,min(511,x+dx));y=max(0,min(341,y+dy))
        rawx+=dx;rawy+=dy
        if drag and (x!=oldx or y!=oldy):wx=max(0,min(288,x-gx));wy=max(0,min(198,y-gy))
        if button and not held:
            presses+=1
            if wx<=x<wx+224 and wy<=y<wy+144:
                if y<wy+18:drag=True;gx=x-wx;gy=y-wy
                else:pattern^=1
        if not button:drag=False
        flag=1 if button and not held else 3 if held and not button else 2 if held else 0
        held=button
        code=0x74 if dx>0 else 0x6b if dx<0 else 0x72 if dy>0 else 0x75 if dy<0 else 0x29
        steps.append((dx,dy,button,code,rawx,rawy,flag))
    def move(dx,dy,n):
        for _ in range(n):step(dx,dy,held)
    def check(name):checks.append((name,len(steps),[x,y,wx,wy,pattern,held,presses],int(drag)))
    move(0,-16,6);step(0,0,1);check('title-held')
    move(-16,0,20);move(0,-16,10);check('drag-top-left')
    move(16,0,34);check('drag-top-right')
    move(0,16,23);check('drag-bottom-right')
    move(-16,0,34);check('drag-bottom-left')
    step(0,0,0);check('corner-released')
    move(0,-16,22);check('cursor-top-left')
    move(16,0,1);move(0,16,14);check('content-hover')
    for i in range(3):
        step(0,0,1);check('content-down-'+str(i+1));step(0,0,0);check('content-up-'+str(i+1))
    # Each arrow is made and broken; each space make is deliberately repeated.
    interval=5000000;pc_start=12000000;mac_start=100000000
    symbols=mac['profiles'][0]['symbols'];start=0xc00;size=256
    cmd=[exe,'--machine','mac128k','--rom',rom,'--cycles',mac_start+len(steps)*interval,
         '--timeout-ms','60000','--mouse-dcd-invert','3','--mouse-edge-cycles','2048','--mouse-vbl-guard','4096',
         '--capture-series',out/'mac.pbmseq','--capture-memory',f'{hex(start)}:{size}']
    for i,(dx,dy,b,*_) in enumerate(steps):
        when=mac_start+i*interval
        cmd+=['--mouse-motion',f'{when}:{dx}:{dy}'] if dx or dy else ['--mouse-button',f'{when}:{b}']
    for _,n,_,_ in checks:cmd+=['--capture-cycle',str(mac_start+n*interval-100000)]
    data=json.loads(run(cmd,'mac'));series=data['frame_series'];blob=(out/'mac.pbmseq').read_bytes()
    if data['status']!='cycle_limit' or series['processed']!=len(checks) or series['dropped'] or series['pending']:
        raise ValueError('Mac capture incomplete')
    rows=[]
    for (name,n,state,dragged),sample in zip(checks,series['samples']):
        memory=bytes.fromhex(sample['memory']['bytes'])
        def word(address):return int.from_bytes(memory[address-start:address-start+4],'big')
        t=symbols['cs_mac_trace'];d=symbols['cs_mac_diag'];loss=symbols['cs_mac_loss_trace'];drop=symbols['cs_mac_motion_drop_trace']
        actual=[word(t+52),word(t+56),word(t+60),word(t+64),word(t+68),word(t+88),word(d+12)]
        if actual!=state or word(d+4)!=16 or word(d+24) or word(drop+4) or any(word(loss+i*4) for i in range(1,5)):
            raise ValueError(name+': Mac state '+str(actual)+' expected '+str(state))
        pbm=out/(name+'-mac.pbm');pbm.write_bytes(blob[sample['offset']:sample['offset']+sample['length']])
        sequence=out/(name+'.txt');sequence.write_text(''.join(f'{dx} {dy} {b}\n' for dx,dy,b,*_ in steps[:n]))
        host_frame=out/(name+'-host.pbm');hstate=list(map(int,run([host,sequence,host_frame],name+'-host').split()))
        if hstate!=state or pbm.read_bytes()!=host_frame.read_bytes():raise ValueError(name+': host state/pixels differ')
        ppm=out/(name+'-pc.ppm')
        cmd=[exe,'--machine','pc286','--hard-disk',pc/'session.img','--cycles',pc_start+n*interval,
             '--timeout-ms','60000','--inspect','0x60000:17','--frame',ppm]
        for i,(dx,dy,b,code,*_) in enumerate(steps[:n]):
            when=pc_start+i*interval
            values=[0xe0,code,0xe0,0xf0,code] if dx or dy else [code,code,code] if b else [0xf0,code]
            for j,v in enumerate(values):cmd+=['--scancode',f'{when+j*10000}:{v}']
        pcdata=json.loads(run(cmd,name+'-pc'))
        expected=struct.pack('<4sBBHHHHBBB',b'S286',n&255,state[5],*state[:4],state[4],dragged,state[6])
        if pcdata['status']!='cycle_limit' or bytes.fromhex(pcdata['memory']['bytes'])!=expected:
            raise ValueError(name+': PC state differs '+str(pcdata['memory']))
        oracle.compare(pbm.read_bytes(),ppm.read_bytes(),margin_white=True)
        rows.append({'name':name,'samples':n,'state':state,'mac_frame_sha256':sha(pbm),
                     'pc_frame_sha256':sha(ppm),'canvas_pixels':175104,'white_margin_pixels':48896})
        print(name+': pass',flush=True)
    report={'status':'pass','invocation':[sys.executable,*sys.argv],'mac_rom_sha256':sha(rom),
            'pc_disk_sha256':sha(pc/'session.img'),'emulator_sha256':sha(exe),'prior_parity_sha256':sha(a.parity),
            'source_sha256':{str(p.relative_to(ROOT)).replace('\\','/'):sha(p) for p in (Path(__file__),fixture)},
            'checkpoints':rows,'steps':len(steps),'edge_cycles':2048,
            'limits':'Scheduled E2 input plus hosted oracle; no host GUI/human/physical qualification. No startup-held or cancellation claim.'}
    (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print('All '+str(len(rows))+' edge checkpoints passed.')
if __name__=='__main__':main()
