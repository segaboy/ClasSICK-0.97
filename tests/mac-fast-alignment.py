# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Controls for the fast service's narrow frame-alignment inspection."""
from importlib.util import module_from_spec, spec_from_file_location
from pathlib import Path

path=Path(__file__).resolve().parents[1]/'scripts'/'Build-LoadedAppROM.py'
spec=spec_from_file_location('loaded_builder',path)
builder=module_from_spec(spec)
spec.loader.exec_module(builder)

def listing(instruction):
    return '00401160 <cs_mac_motion_service>:\n  4011d4:  0000  '+instruction+'\n00401200 <other>:\n'

for bad in ('movew %fp@(-9),%fp@(-19)', 'movel %d0,%fp@(3)'):
    try:
        builder.audit_fast_frame_alignment(listing(bad))
    except ValueError:
        pass
    else:
        raise AssertionError('accepted odd frame access: '+bad)
for good in ('moveb %fp@(-9),%d0', 'movew %fp@(-10),%fp@(-20)'):
    builder.audit_fast_frame_alignment(listing(good))
try:
    builder.audit_fast_frame_alignment('')
except ValueError:
    pass
else:
    raise AssertionError('accepted missing service')
