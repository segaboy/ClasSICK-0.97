# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Render guest-only paired evidence and capture a bounded continuous Mac clip.

Requires Pillow and FFmpeg on the host. Neither is part of a guest image.
"""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
from PIL import Image,ImageDraw,ImageFont
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ('parity','qualification','emulator','rom','ffmpeg','output'):p.add_argument('--'+n,type=Path,required=True)
    a=p.parse_args();out=a.output.resolve();out.mkdir(parents=True,exist_ok=False)
    report=json.loads(a.parity.read_text());q=json.loads((a.qualification/'report.json').read_text());pc=Path(q['session'])
    if report['status']!='pass' or q['status']!='pass' or report['mac_rom_sha256']!=sha(a.rom) or report['boothush_sha256']!=sha(a.emulator):
        raise ValueError('candidate identity changed')
    frames=[];font=ImageFont.load_default(size=18)
    for i,row in enumerate(report['checkpoints']):
        name=row['name'];mac=a.parity.parent/(name+'.pbm');ega=pc/(name+'.ppm')
        if sha(mac)!=row['mac_pbm_sha256'] or sha(ega)!=row['pc_ega_ppm_sha256']:raise ValueError('capture changed')
        canvas=Image.new('RGB',(1200,420),'#e9e8e2');draw=ImageDraw.Draw(canvas)
        draw.text((16,12),'Macintosh 128K / native 68000',font=font,fill='black')
        draw.text((552,12),'PC/AT / native 286 / EGA',font=font,fill='black')
        canvas.paste(Image.open(mac).convert('RGB'),(16,44));canvas.paste(Image.open(ega),(552,40))
        draw.text((16,398),f'{i+1}/8  {name}  |  Matching 512 x 342 canvas; BootHush scheduled input',font=font,fill='black')
        canvas.save(out/(name+'.png'));frames.append(canvas)
    frames[0].save(out/'paired-checkpoints.gif',save_all=True,append_images=frames[1:],duration=1000,loop=0,disposal=2)
    for name in ('prompt','install','boot-crc','occupied-end'):
        item=next(r for r in q['observations'] if r['name']==name);source=a.qualification/(name+'.ppm')
        if sha(source)!=item['frame_sha256']:raise ValueError('installer capture changed')
        Image.open(source).save(out/('installer-'+name+'.png'))
    cmd=[str(a.emulator),'--machine','mac128k','--rom',str(a.rom),'--cycles','202000000','--timeout-ms','20000',
         '--mouse-dcd-invert','3','--mouse-edge-cycles',str(report['requested_edge_cycles']),
         '--mouse-vbl-guard',str(report['requested_vbl_guard']),'--video',str(out/'mac-window.y4m'),
         '--video-start','100000000','--video-fps','10','--video-frames','140','--frame',str(out/'video-final.pbm')]
    for event in report['events']:cmd+=['--window-mouse',event]
    r=subprocess.run(cmd,capture_output=True,text=True,timeout=25,creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
    (out/'video-run.json').write_text(r.stdout);(out/'video-command.json').write_text(json.dumps(cmd,indent=2)+'\n')
    data=json.loads(r.stdout)
    if r.returncode or data['status']!='cycle_limit' or sha(out/'video-final.pbm')!=report['checkpoints'][-1]['mac_pbm_sha256']:
        raise ValueError('video final frame differs from qualification')
    ff=[str(a.ffmpeg),'-nostdin','-hide_banner','-loglevel','error','-i',str(out/'mac-window.y4m'),
        '-an','-c:v','libx264','-crf','18','-pix_fmt','yuv420p','-movflags','+faststart',str(out/'mac-window.mp4')]
    subprocess.run(ff,check=True,timeout=45,creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
    record={'status':'pass','parity_report_sha256':sha(a.parity),'qualification_report_sha256':sha(a.qualification/'report.json'),
            'emulator_sha256':sha(a.emulator),'rom_sha256':sha(a.rom),'ffmpeg_sha256':sha(a.ffmpeg),
            'script_sha256':sha(Path(__file__)),'video_result':data.get('video'),'ffmpeg_command':ff,
            'presentation':'GIF is eight one-second checkpoints, not continuous motion. MP4 is continuous sampled guest time, 10 fps, no audio.',
            'rights':'Original ClasSICK artwork and project-owned guest captures; no reference assets or desktop chrome.',
            'files':{f.name:sha(f) for f in out.iterdir() if f.is_file() and f.suffix in ('.png','.gif','.mp4')}}
    (out/'capture.json').write_text(json.dumps(record,indent=2)+'\n');print(json.dumps(record,indent=2))
if __name__=='__main__':main()
