# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Read an original MAR1 subset at its verified spin; no guest access."""
import argparse,json
from pathlib import Path

def decode(r,spin):
    names=['d'+str(i) for i in range(8)]+['a'+str(i) for i in range(7)]+['pc']
    if any(type(r.get(k)) is not int or not 0<=r[k]<=0xFFFFFFFF for k in names):
        raise ValueError('All MAR1 registers and PC must be uint32 integers')
    if type(spin) is not int or spin%2 or not 0x400104<=spin<0x410000:
        raise ValueError('Expected own-ROM spin required')
    if r['d0']!=0x4D415231 or r['pc']!=spin:raise ValueError('Not the MAR1 checkpoint')
    keys=('pairs pb_x_changes pb_y_changes dcd_a_changes dcd_b_changes button_changes '
          'nonzero_queued buttononly_queued losses first_previous first_s0 first_s1 '
          'first_reason first_service').split()
    values=[r['d'+str(i)] for i in range(1,8)]+[r['a'+str(i)] for i in range(7)]
    d=dict(zip(keys,values))
    if any(d[k]>d['pairs'] for k in keys[1:9]):raise ValueError('Count exceeds pairs')
    v=d['first_previous']
    if v&0x7F000000 or (v and not v&0x80000000):raise ValueError('Invalid prior endpoint')
    if d['first_s0']>0xFFFFFF or d['first_s1']>0xFFFFFF or d['first_reason']>6:
        raise ValueError('Invalid loss fields')
    if d['first_reason']==0:
        if any(d[k] for k in keys[9:]):raise ValueError('Incomplete first loss')
    elif not 1<=d['first_service']<=d['pairs'] or not d['losses']:
        raise ValueError('Invalid first-loss index/count')
    d.update(format='MAR1',pc=spin,omitted_fields=['tag','version','bytes','stable_pairs',
        'last_observed','previous_accepted'],limitation='Register subset only;not a full MWA1 record or root-cause proof.')
    return d

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--file',required=True)
    p.add_argument('--spin',required=True,type=lambda s:int(s,0));a=p.parse_args()
    try:print(json.dumps(decode(json.loads(Path(a.file).read_text()),a.spin),indent=2))
    except (OSError,ValueError,TypeError) as e:p.exit(2,'refused: '+str(e)+'\n')
