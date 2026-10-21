#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Encode original Win32 client captures;Pillow required only for this explicit tool."""
import argparse
import hashlib
import json
from pathlib import Path
from PIL import Image

def main():
    ap=argparse.ArgumentParser();ap.add_argument('--raw',required=True,type=Path);ap.add_argument('--output',required=True,type=Path)
    a=ap.parse_args();a.output.mkdir(parents=True,exist_ok=True)
    frames=[];hashes=[]
    palette=Image.new('P',(1,1));palette.putpalette([255,255,255,0,0,0]+[255]*762)
    for n in range(49):
        im=Image.open(a.raw/('drag-%02d.ppm'%n)).convert('RGB')
        colors=im.getcolors(maxcolors=3)
        assert im.size==(640,480) and colors is not None and {c[1] for c in colors}=={(0,0,0),(255,255,255)}
        frames.append(im.quantize(palette=palette,dither=Image.Dither.NONE));hashes.append(hashlib.sha256(im.tobytes()).hexdigest())
    durations=[480 if n in (0,24,48) else 60 for n in range(49)]
    gif=a.output/'hosted-window-drag.gif';assert not gif.exists()
    frames[0].save(gif,save_all=True,append_images=frames[1:],duration=durations,loop=0,disposal=2,optimize=False)
    decoded=Image.open(gif);assert decoded.n_frames==49 and decoded.size==(640,480) and decoded.info['loop']==0
    total=0
    for n,expected in enumerate(hashes):
        decoded.seek(n);assert hashlib.sha256(decoded.convert('RGB').tobytes()).hexdigest()==expected
        assert decoded.info['duration']==durations[n];total+=decoded.info['duration']
    stills=[]
    for name in ('hosted-window-dragged','hosted-window-large'):
        source=Image.open(a.raw/(name+'.ppm')).convert('RGB');dest=a.output/(name+'.png');assert not dest.exists()
        source.save(dest,optimize=True);assert Image.open(dest).convert('RGB').tobytes()==source.tobytes();stills.append(dict(path=dest.name,width=source.width,height=source.height,bytes=dest.stat().st_size,sha256=hashlib.sha256(dest.read_bytes()).hexdigest()))
    audit=dict(frames=49,duration_ms=total,loop=0,decoded_pixels_exact=True,rgb_frame_sha256=hashes,gif=dict(path=gif.name,width=640,height=480,bytes=gif.stat().st_size,sha256=hashlib.sha256(gif.read_bytes()).hexdigest()),stills=stills)
    (a.raw/'media-audit.json').write_text(json.dumps(audit,indent=2)+'\n');print(json.dumps({k:v for k,v in audit.items() if k!='rgb_frame_sha256'},indent=2))
if __name__=='__main__':main()
