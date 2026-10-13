# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Check reviewed guest marshalling metadata, including refusal controls."""
import copy
import json
from pathlib import Path
EXPECTED={'GetTrapAddress':('A146',[('D0',2)]),'SetTrapAddress':('A047',[('D0',2),('A0',4)]),
          'NewPtr':('A11E',[('D0',4)]),'DisposPtr':('A01F',[('A0',4)]),
          'GetPtrSize':('A021',[('A0',4)]),'SetPtrSize':('A020',[('A0',4),('D0',4)]),
          'NewHandle':('A122',[('D0',4)]),'DisposHandle':('A023',[('A0',4)]),
          'GetHandleSize':('A025',[('A0',4)]),'SetHandleSize':('A024',[('A0',4),('D0',4)]),
          'HLock':('A029',[('A0',4)]),'HUnlock':('A02A',[('A0',4)]),
          'BlockMove':('A02E',[('A0',4),('A1',4),('D0',4)])}

def check(d):
    if d['schema_version']!=1 or d['historical_1984_confirmed'] is not False:raise ValueError('Version/provenance')
    names=[r['name'] for r in d['routines']]
    if len(names)!=13 or set(names)!=set(EXPECTED):raise ValueError('Routine membership/duplicates')
    for r in d['routines']:
        word,parameters=EXPECTED[r['name']]
        if r['trap_word']!=word or r['number']!=int(word,16)%256:raise ValueError('Trap identity')
        if [(p['register'],p['bytes']) for p in r['parameters']]!=parameters:raise ValueError('Marshalling width/location')
        if r['introduced_release'] is not None or r['first_documented']!='1985':raise ValueError('Unverified release')
        if r['convention']!='os-register-v1' or not r['source_id'] or not r['locator']:raise ValueError('Unreviewed signature')
        allowed=['CLEAR','SYS'] if r['name'] in ('NewPtr','NewHandle') else []
        if r['flags']!=allowed or r['result_code_global'] is not None:raise ValueError('Flags/Pascal result-global scope')
        for p in r['parameters']:
            if p['type'] not in d['types'] or p['bytes']!=d['types'][p['type']]['bytes']:raise ValueError('Parameter type')
            if p['class'] not in ('value','object','code'):raise ValueError('Parameter class')
            if p['class']=='code' and p.get('signature')!='routine-specific':raise ValueError('Code signature')
    c=d['calling_conventions']['os-register-v1']
    if c['stack_arguments'] or c['stack_cleanup_bytes']!=0 or 'A5' not in c['preserved'] or 'A7' not in c['preserved']:raise ValueError('Stack/register contract')
    master=d['guest_records'][0]
    if master['bytes']!=4 or master['endian']!='big' or master['address_bits']!=[0,23] or master['flags_in_byte']!={'lock':7,'purge':6,'resource':5}:raise ValueError('Guest master cell')

def main():
    p=Path(__file__).resolve().parents[1]/'docs/interfaces/system1-routines.json'
    d=json.loads(p.read_text());check(d)
    for n in range(8):
        bad=copy.deepcopy(d)
        if n==0:bad['routines'][2]['trap_word']='A01E'
        elif n==1:bad['routines'][2]['parameters'][0]['bytes']=2
        elif n==2:bad['routines'][3]['flags']=['SYS']
        elif n==3:bad['routines'][0]['introduced_release']='1984'
        elif n==4:bad['routines'][1]['parameters'][1]['signature']='native-function'
        elif n==5:bad['guest_records'][0]['endian']='little'
        elif n==6:bad['routines'].append(copy.deepcopy(bad['routines'][0]))
        else:bad['calling_conventions']['os-register-v1']['stack_cleanup_bytes']=4
        try:check(bad)
        except ValueError:pass
        else:raise ValueError('Corrupt metadata accepted')
    print('Guest interface registry PASS: 13 reviewed records and 8 refusal controls; no native headers.')

if __name__=='__main__':main()
