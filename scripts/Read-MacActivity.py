# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Strict reader for original, coherently paused MWA1 v1 bytes; no guest access."""
import argparse, json, struct
from pathlib import Path

FIELDS = ('tag version bytes pairs pb_x_changes pb_y_changes dcd_a_changes '
          'dcd_b_changes button_changes stable_pairs nonzero_queued '
          'buttononly_queued losses last_observed previous_accepted '
          'first_previous first_s0 first_s1 first_reason first_service').split()

def endpoint(word):
    return {'valid':bool(word & 0x80000000),'PB':(word>>16)&255,
            'RR0A':(word>>8)&255,'RR0B':word&255}

def decode(raw):
    if len(raw)!=80: raise ValueError('MWA1 requires exactly80 bytes')
    d=dict(zip(FIELDS,struct.unpack('>20I',raw)))
    if (d['tag'],d['version'],d['bytes'])!=(0x4D574131,1,80):
        raise ValueError('Expected MWA1 v1/80;old or unknown format refused')
    for name in ('last_observed','previous_accepted','first_previous'):
        v=d[name]
        if v&0x7F000000 or (not v&0x80000000 and v):
            raise ValueError('Invalid packed endpoint/valid marker')
    if d['first_s0']>0xFFFFFF or d['first_s1']>0xFFFFFF or d['first_reason']>6:
        raise ValueError('Invalid first-loss fields')
    if any(d[k]>d['pairs'] for k in FIELDS[4:13]):
        raise ValueError('Activity count exceeds observed pairs')
    if d['first_reason']==0:
        if any(d[k] for k in ('first_previous','first_s0','first_s1','first_service')):
            raise ValueError('Incomplete first-loss record')
    elif not 1<=d['first_service']<=d['pairs']:
        raise ValueError('Invalid first-loss pair index')
    d['format']='MWA1'
    d['endpoints']={k:endpoint(d[k]) for k in ('last_observed','previous_accepted','first_previous')}
    d['limitation']='Observed raw levels/admission only;no missed-edge,physical-coherence,frequency or root-cause proof.'
    return d

def main():
    p=argparse.ArgumentParser(description=__doc__)
    g=p.add_mutually_exclusive_group(required=True);g.add_argument('--file');g.add_argument('--hex')
    a=p.parse_args()
    try: out=decode(Path(a.file).read_bytes() if a.file else bytes.fromhex(a.hex))
    except (OSError,ValueError) as e: p.exit(2,'refused: '+str(e)+'\n')
    print(json.dumps(out,indent=2))
if __name__=='__main__': main()
