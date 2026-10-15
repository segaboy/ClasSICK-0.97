# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Reuse actual bus closure audit; no emulator or application execution."""
import argparse,hashlib,json,sys
from pathlib import Path
from importlib.util import spec_from_file_location,module_from_spec

def main():
    p=argparse.ArgumentParser();p.add_argument('--tool-root',required=True);p.add_argument('--output',required=True)
    a=p.parse_args();repo=Path(__file__).resolve().parents[1]
    spec=spec_from_file_location('guest_audit',repo/'scripts/Verify-GuestM68k.py');h=module_from_spec(spec)
    sys.dont_write_bytecode=True;spec.loader.exec_module(h);h.main()
    output=Path(a.output).resolve()/'manifest.json';d=json.loads(output.read_text())
    d.update(specification='SPEC-0034',dependency='X-1',base_specification='SPEC-0022',application_executed=False,
        image_session_authority=False)
    for name in ('core/cpu/m68k-guard.h','tests/m68k-guard.c','scripts/Verify-GuardedBusM68k.py',
                 'scripts/Verify-GuardedBus.ps1','docs/specifications/SPEC-0034-owned-image-execution.md'):
        d['inputs'][name]=hashlib.sha256((repo/name).read_bytes().replace(b'\r\n',b'\n')).hexdigest()
    output.write_text(json.dumps(d,indent=2)+'\n')
    print('SPEC-0034 X-1 guarded bus closure PASS; no application execution.')

if __name__=='__main__':main()
