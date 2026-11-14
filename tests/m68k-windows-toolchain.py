#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Exercise Windows toolchain identity intake with harmless host fixtures."""
import argparse
import importlib.util
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('toolchain',ROOT/'scripts/m68k_windows_toolchain.py')
m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)


def main():
    p=argparse.ArgumentParser();p.add_argument('--cc',required=True,type=Path);a=p.parse_args()
    cc=a.cc.resolve(strict=True)
    with tempfile.TemporaryDirectory(prefix='classick-m68k-intake-') as temporary:
        root=Path(temporary);source=root/'fixture.c';binary=root/'fixture.exe'
        source.write_text('#include <stdio.h>\n#include <string.h>\nint main(int n,char **v) {'
                          'if(n!=2)return 1; if(!strcmp(v[1],"-dumpmachine"))puts("m68k-unknown-elf");'
                          'else if(!strcmp(v[1],"--version"))puts("synthetic fixture");else return 2;return 0;}\n',encoding='utf-8')
        subprocess.run([str(cc),str(source),'-o',str(binary)],check=True,timeout=30,
                       creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
        for name in m.TOOLS:shutil.copyfile(binary,root/('m68k-unknown-elf-'+name+'.exe'))
        paths,report=m.inspect(root)
        assert report['target']=='m68k-unknown-elf' and report['compiler_version']=='synthetic fixture'
        assert set(paths)==set(m.TOOLS) and all(report['tools'][x]['sha256']==m.sha(binary) for x in m.TOOLS)
        # Claiming the target is not enough to produce a successful ROM: the
        # build must stop when this harmless fixture cannot compile anything.
        output=root/'failed-build'
        command=[sys.executable,str(ROOT/'scripts/Build-LoadedAppROM.py'),
                 '--tool-root',str(root),'--windows-native','--window-drawing','--output',str(output)]
        attempted=subprocess.run(command,capture_output=True,text=True,timeout=30,
                                 creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
        assert attempted.returncode!=0 and not (output/'manifest.json').exists()
        # A valid host executable reporting the wrong target must be refused.
        source.write_text(source.read_text(encoding='utf-8').replace('m68k-unknown-elf','x86_64-w64-mingw32'),encoding='utf-8')
        subprocess.run([str(cc),str(source),'-o',str(paths['gcc'])],check=True,timeout=30,
                       creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
        try:m.inspect(root)
        except ValueError as exc:assert 'not an m68k ELF compiler' in str(exc),str(exc)
        else:raise AssertionError('host compiler was admitted')
        output=root/'wrong-target-build';command[-1]=str(output)
        attempted=subprocess.run(command,capture_output=True,text=True,timeout=30,
                                 creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
        assert attempted.returncode!=0 and 'not an m68k ELF compiler' in attempted.stderr
        assert not output.exists()
        paths['gcc'].write_bytes(b'not a PE')
        try:m.inspect(root)
        except ValueError as exc:assert 'not a PE executable' in str(exc)
        else:raise AssertionError('non-PE file was admitted')
        paths['gcc'].unlink()
        try:m.inspect(root)
        except ValueError as exc:assert 'missing native tool' in str(exc)
        else:raise AssertionError('missing compiler was admitted')
    print('PASS synthetic identity, wrong target, invalid PE and missing compiler controls')


if __name__=='__main__':main()
