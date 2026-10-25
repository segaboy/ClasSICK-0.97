# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Independent original replay-data CLI controls;never runs a guest."""
from pathlib import Path
import json,subprocess,sys,hashlib,argparse
p=argparse.ArgumentParser();p.add_argument('--output',required=True);a=p.parse_args()
r=Path(__file__).resolve().parents[1];script=r/'scripts/New-SnowMouseReplay.py'
root=Path(a.output).resolve();root.mkdir(exist_ok=False);checks=[]
def run(out,*args):return subprocess.run([sys.executable,'-B','-X','utf8',str(script),'--output',str(out),*map(str,args)],capture_output=True,text=True,timeout=15)
def legacy(out,start,gap,case):return run(out,'--start',start,'--gap',gap,'--case',case)
def expected(times):return [[t,{'MouseUpdateRelative':{'relx':1,'rely':0,'btn':None}}] for t in times]
base_keys={'case','start','gap','event_count','last_timestamp','sha256','bytes','original_data','timestamp_semantics_verified','tests_executed'}
def good(out,z,times,extra=None):
    assert z.returncode==0,z.stderr
    b=out.read_bytes();assert b==(json.dumps(expected(times),separators=(',',':'))+'\n').encode()
    q=json.loads(z.stdout);assert set(q)==base_keys|set(extra or {})
    assert q['sha256']==hashlib.sha256(b).hexdigest() and q['bytes']==len(b) and q['event_count']==len(times) and q['last_timestamp']==times[-1] and q['start']==times[0]
    assert q['original_data'] and q['timestamp_semantics_verified'] is False and q['tests_executed']==[]
    for k,v in (extra or {}).items():assert q[k]==v,(k,q[k],v)
for case,times in [('single-x',[2000000]),('slow-x',[2000000,2500000,3000000,3500000,4000000,4500000,5000000,5500000])]:
    out=root/(case+'.snowr');good(out,legacy(out,2000000,500000,case),times);checks.append(case+' literal bytes/legacy receipt')
    before=out.read_bytes();z=legacy(out,2,3,case);assert z.returncode!=0 and out.read_bytes()==before;checks.append(case+' overwrite refusal')
for i,(s,g,c) in enumerate([(-1,1,'single-x'),(0,0,'single-x'),(0,-1,'single-x'),(2**63,1,'single-x'),(2**63-7,1,'slow-x'),('1.5',1,'single-x'),(0,'1.5','slow-x'),(0,1,'bad-case')]):
    out=root/f'bad-legacy-{i}.snowr';z=legacy(out,s,g,c);assert z.returncode!=0 and not out.exists();checks.append('legacy refusal '+str(i))
out=root/'missing/none.snowr';z=legacy(out,1,1,'single-x');assert z.returncode!=0 and not out.parent.exists();checks.append('legacy missing parent')
out=root/'maximum.snowr';good(out,legacy(out,2**63-1,1,'single-x'),[2**63-1]);checks.append('legacy max timestamp')
# Original synthetic opaque markers;not observed emulator keyboard data.
marker=root/'marker.json';raw=b'[[42,{"ClockMarker":{"down":true}}],[42,{"ClockMarker":{"down":false}}]]';marker.write_bytes(raw)
def clock(out,source=marker,offset=8,case='single-x',gap=3):return run(out,'--paused-recording',source,'--offset',offset,'--case',case,'--gap',gap)
extra={'paused_clock_anchor':42,'paused_recording_sha256':hashlib.sha256(raw).hexdigest(),'paused_marker_count':2,'offset':8}
for case,times in [('single-x',[50]),('slow-x',[50,53,56,59,62,65,68,71])]:
    out=root/('clock-'+case+'.snowr');good(out,clock(out,case=case),times,extra);checks.append('clock '+case+' literal bytes/anchor/hash')
out=root/'clock-single-x.snowr';old=out.read_bytes();z=clock(out);assert z.returncode!=0 and out.read_bytes()==old;checks.append('clock output preserved')
z=clock(marker);assert z.returncode!=0 and marker.read_bytes()==raw;checks.append('clock source alias preserved')
invalid=[b'[]',b'{}',b'not JSON',b'\xff',raw+b' '*4097,b'[[42,{"MouseUpdateRelative":{}}]]',b'[[42,{"mOuSeUp":null}]]',b'[[true,{"ClockMarker":null}]]',b'[[42.0,{"ClockMarker":null}]]',b'[[-1,{"ClockMarker":null}]]',b'[[42,{"ClockMarker":null}],[43,{"ClockMarker":null}]]',b'[[42,{}]]',b'[[42,{"A":0,"B":1}]]',b'[[42,[]]]',b'[[42,{"":null}]]',b'[[42,{"A":0},0]]']
invalid.extend([b'[[42,{"ClockMarker":NaN}]]',b'[[42,{"ClockMarker":Infinity}]]',b'[[42,{"ClockMarker":-Infinity}]]'])
for i,b in enumerate(invalid):
    source=root/f'invalid-{i}.json';source.write_bytes(b);out=root/f'bad-clock-{i}.snowr';z=clock(out,source);assert z.returncode!=0 and not out.exists(),(i,z.stdout,z.stderr);assert source.read_bytes()==b;checks.append('clock invalid source '+str(i))
for i,opts in enumerate([['--paused-recording',marker,'--case','single-x','--gap',1],['--paused-recording',marker,'--offset',0,'--case','single-x','--gap',1],['--paused-recording',marker,'--offset',-1,'--case','single-x','--gap',1],['--paused-recording',marker,'--offset','1.5','--case','single-x','--gap',1],['--paused-recording',marker,'--start',1,'--offset',1,'--case','single-x','--gap',1],['--case','single-x','--gap',1],['--start',1,'--offset',1,'--case','single-x','--gap',1],['--paused-recording',root/'not-found.json','--offset',1,'--case','single-x','--gap',1]]):
    out=root/f'bad-mode-{i}.snowr';z=run(out,*opts);assert z.returncode!=0 and not out.exists();checks.append('origin/offset refusal '+str(i))
source=root/'boundary-marker.json';source.write_text(json.dumps([[2**63-2,{'ClockMarker':None}]]),encoding='utf-8');b=source.read_bytes()
out=root/'clock-boundary.snowr';good(out,clock(out,source,1),[2**63-1],{'paused_clock_anchor':2**63-2,'paused_recording_sha256':hashlib.sha256(b).hexdigest(),'paused_marker_count':1,'offset':1});checks.append('clock signed boundary accepted')
out=root/'clock-overflow.snowr';z=clock(out,source,2);assert z.returncode!=0 and not out.exists();checks.append('clock anchor addition overflow')
out=root/'clock-last-overflow.snowr';z=clock(out,source,1,'slow-x',1);assert z.returncode!=0 and not out.exists();checks.append('clock final-event overflow')
source=root/'limit-marker.json';b=b'[[42,{"ClockMarker":null}]]';b+=b' '*(4096-len(b));source.write_bytes(b)
out=root/'clock-limit.snowr';good(out,clock(out,source),[50],{'paused_clock_anchor':42,'paused_recording_sha256':hashlib.sha256(b).hexdigest(),'paused_marker_count':1,'offset':8});checks.append('4096-byte input boundary')
result={'schema_version':1,'passed':len(checks),'checks':checks,'script_sha256':hashlib.sha256(script.read_bytes()).hexdigest(),'input_markers_are_original_synthetic_data':True,'paused_clock_or_native_replay_qualified':False}
(root/'verification.json').write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8');print('PASS:',len(checks),'independent literal/refusal checks;no target qualification')
