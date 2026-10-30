# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Argument-fence tests only;never compiles or executes a guest."""
from contextlib import redirect_stderr
from importlib.util import module_from_spec,spec_from_file_location
from io import StringIO
from pathlib import Path
import sys

root=Path(__file__).resolve().parents[1]
spec=spec_from_file_location('loaded_builder',root/'scripts/Build-LoadedAppROM.py')
builder=module_from_spec(spec);spec.loader.exec_module(builder)
prefix=['--window-drawing','--window-timer-poll-probe','--window-timer-refresh-probe']
live=prefix+['--window-timer-live-pair']
bad=[['--window-timer-live-pair'],['--window-drawing','--window-timer-live-pair'],
     ['--window-drawing','--window-timer-poll-probe','--window-timer-live-pair']]
bad += [live+[flag] for flag in ['--window-idle-probe','--window-timer-activity-probe',
        '--window-timer-recovery-probe','--window-first-input-probe',
        '--window-pending-defer-probe','--window-phase-idle-probe','--window-timer-pair-probe']]
bad += [live+['--window-timer-divisor',str(n)] for n in (2,8,32)]
bad += [prefix+['--window-timer-divisor',str(n)] for n in (4,16)]
bad += [['--window-timer-live-pair','--window-timer-divisor','4'],
        ['--window-drawing','--window-timer-live-pair','--window-timer-divisor','16']]
bad += [live+[flag,'--window-timer-divisor','4'] for flag in ['--window-idle-probe',
        '--window-timer-activity-probe','--window-timer-recovery-probe','--window-first-input-probe',
        '--window-pending-defer-probe','--window-phase-idle-probe','--window-timer-pair-probe']]
bad += [prefix+['--window-timer-pair-probe'],prefix+['--window-timer-recovery-probe']]
good=[live,live+['--window-timer-divisor','1'],live+['--window-timer-divisor','4'],live+['--window-timer-divisor','16'],prefix+['--window-idle-probe','--window-timer-activity-probe',
                  '--window-timer-recovery-probe','--window-timer-pair-probe']]

phase = live+['--window-timer-divisor','4','--window-timer-phase-witness']
bad += [['--window-timer-phase-witness'],prefix+['--window-timer-phase-witness'],
        prefix+['--window-timer-divisor','4','--window-timer-phase-witness'],
        live+['--window-timer-phase-witness'],
        live+['--window-timer-divisor','16','--window-timer-phase-witness']]
bad += [phase+[flag] for flag in ['--window-idle-probe','--window-timer-activity-probe',
        '--window-timer-recovery-probe','--window-first-input-probe',
        '--window-pending-defer-probe','--window-phase-idle-probe','--window-timer-pair-probe']]
good += [phase]

class BeforeOutput(Exception):
    pass

original_mkdir=Path.mkdir
original_argv=sys.argv
try:
    def refuse_output(*args,**kwargs):
        raise BeforeOutput()
    Path.mkdir=refuse_output
    for args in bad+good:
        sys.argv=['builder','--tool-root','never-read-tool-root','--output','never-created',*args]
        with redirect_stderr(StringIO()):
            try:
                builder.main()
            except SystemExit as exc:
                assert args in bad and exc.code==2,(args,exc.code)
            except BeforeOutput:
                assert args in good,args
            else:
                raise AssertionError(('Unexpected build/effect path',args))
finally:
    Path.mkdir=original_mkdir
    sys.argv=original_argv
print('PASS:',len(bad),'refused compositions;',len(good),'accepted before first output;no build/run.')
