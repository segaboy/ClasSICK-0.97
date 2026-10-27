#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Endpoint-only versus full-cadence polled-phase sampling study.

Host-only research tool. It compiles an original C99 adapter against the
unchanged polled-phase model and writes results only below a new --output
directory. No network, VM, emulator or target execution. Results count
emitted DCD units of the software model, not physical edges, pixels or time.
Endpoint agreement is never proof that no physical transitions were missed.
"""
import argparse
import csv
import hashlib
import io
import json
import os
import subprocess
import sys

INPUTS = {
    'platform/mac128k/polled-phase.h':
        '889aa6a1314e32114722f8ef822de14b100f197927fc77b9768561ac1566428f',
    'platform/mac128k/polled-phase.c':
        '537d6d62bf3177e16153c86ba4d42fee90a90192bf17e103fc635f5bf1518766',
    'core/input/pointer.h':
        'd0a53cdadbe728195ce2c35bc4ef0b2a1687351912c759eda1bf6b4c38abdc38',
}
RC = {0: 'SAMPLE', 1: 'EMPTY', 2: 'BASELINED', 3: 'LOST',
      4: 'ARGUMENT', 5: 'STATE'}
COLUMNS = ['axis', 'start', 'direction', 'steps', 'invert', 'button',
           'rc', 'dx', 'dy', 'full_dx', 'full_dy']
TRACE_COLUMNS = ['prior_pb', 'baseline_rc', 'rc', 'dx', 'dy', 'buttons']
EXPECTED_CASES = 2 * 4 * 2 * 13 * 4 * 2  # 1664
COMPILE_TIMEOUT = 120
RUN_TIMEOUT = 60

ADAPTER = r'''/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Research adapter: host-only, public begin/step APIs only. */
#include <stdio.h>
#include <string.h>
#include "platform/mac128k/polled-phase.h"

static const unsigned gray[4] = { 0u, 1u, 3u, 2u };

static unsigned pos_of(unsigned ph)
{
    unsigned i;
    for (i = 0; i < 4u; i++)
        if (gray[i] == ph)
            return i;
    return 0u;
}

/* Numeric phase = DCD(bit3) + 2*PB4 (X) or PB5 (Y); other axis phase 0. */
static void raw(int axis, unsigned ph, int button, uint32_t *pb,
    uint32_t *a, uint32_t *b)
{
    *pb = button ? 0x00u : 0x08u; /* PB3 active low */
    *a = *b = 0u;
    if (axis == 0) {
        if (ph & 1u) *a = 0x08u;
        if (ph & 2u) *pb |= 0x10u;
    } else {
        if (ph & 1u) *b = 0x08u;
        if (ph & 2u) *pb |= 0x20u;
    }
}

static int fail(const char *what, int rc)
{
    fprintf(stderr, "FAIL %s rc=%d\n", what, rc);
    return 2;
}

static int sweep(void)
{
    int axis, start, di, steps, inv, btn, k, rc;
    printf("axis,start,direction,steps,invert,button,rc,dx,dy,full_dx,full_dy\n");
    for (axis = 0; axis < 2; axis++)
    for (start = 0; start < 4; start++)
    for (di = 0; di < 2; di++)
    for (steps = 0; steps <= 12; steps++)
    for (inv = 0; inv < 4; inv++)
    for (btn = 0; btn < 2; btn++) {
        cs_mac_polled_phase sf, se;
        cs_pointer_sample o;
        uint32_t pb, a, b;
        unsigned p;
        long fx = 0, fy = 0;
        int dir = di ? -1 : 1;
        memset(&sf, 0, sizeof sf);
        memset(&se, 0, sizeof se);
        memset(&o, 0, sizeof o);
        if ((rc = cs_mac_polled_begin(&sf, (uint32_t)inv)) != CS_MAC_POLLED_EMPTY)
            return fail("begin full", rc);
        if ((rc = cs_mac_polled_begin(&se, (uint32_t)inv)) != CS_MAC_POLLED_EMPTY)
            return fail("begin endpoint", rc);
        raw(axis, (unsigned)start, btn, &pb, &a, &b);
        if ((rc = cs_mac_polled_step(&sf, pb, a, b, &o)) != CS_MAC_POLLED_BASELINED)
            return fail("baseline full", rc);
        memset(&o, 0, sizeof o);
        if ((rc = cs_mac_polled_step(&se, pb, a, b, &o)) != CS_MAC_POLLED_BASELINED)
            return fail("baseline endpoint", rc);
        p = pos_of((unsigned)start);
        for (k = 0; k < steps; k++) {
            p = (p + (dir > 0 ? 1u : 3u)) & 3u;
            raw(axis, gray[p], btn, &pb, &a, &b);
            memset(&o, 0, sizeof o);
            rc = cs_mac_polled_step(&sf, pb, a, b, &o);
            if (rc != CS_MAC_POLLED_SAMPLE && rc != CS_MAC_POLLED_EMPTY)
                return fail("full-cadence step", rc);
            if (o.buttons != (uint32_t)btn)
                return fail("full-cadence button", rc);
            fx += o.dx;
            fy += o.dy;
        }
        raw(axis, gray[p], btn, &pb, &a, &b);
        memset(&o, 0, sizeof o);
        rc = cs_mac_polled_step(&se, pb, a, b, &o);
        printf("%d,%d,%d,%d,%d,%d,%d,%ld,%ld,%ld,%ld\n", axis, start, dir,
            steps, inv, btn, rc, (long)o.dx, (long)o.dy, fx, fy);
    }
    return 0;
}

/* Recorded trace: prior PB4/PB5 not retained; enumerate them, DCD=0. */
static int trace(void)
{
    static const uint32_t prior[4] = { 0x8Fu, 0x9Fu, 0xAFu, 0xBFu };
    int i, rc0, rc;
    printf("prior_pb,baseline_rc,rc,dx,dy,buttons\n");
    for (i = 0; i < 4; i++) {
        cs_mac_polled_phase s;
        cs_pointer_sample o;
        memset(&s, 0, sizeof s);
        memset(&o, 0, sizeof o);
        if ((rc = cs_mac_polled_begin(&s, 0u)) != CS_MAC_POLLED_EMPTY)
            return fail("trace begin", rc);
        rc0 = cs_mac_polled_step(&s, prior[i], 0u, 0u, &o);
        if (rc0 != CS_MAC_POLLED_BASELINED)
            return fail("trace baseline", rc0);
        memset(&o, 0, sizeof o);
        rc = cs_mac_polled_step(&s, 0xAFu, 0x4Cu, 0x4Cu, &o);
        printf("%u,%d,%d,%ld,%ld,%lu\n", (unsigned)prior[i], rc0, rc,
            (long)o.dx, (long)o.dy, (unsigned long)o.buttons);
    }
    return 0;
}

int main(int argc, char **argv)
{
    if (argc == 2 && strcmp(argv[1], "sweep") == 0)
        return sweep();
    if (argc == 2 && strcmp(argv[1], "trace") == 0)
        return trace();
    fprintf(stderr, "usage: adapter sweep|trace\n");
    return 1;
}
'''


def lf_sha256(path):
    with open(path, 'rb') as f:
        data = f.read()
    return hashlib.sha256(data.replace(b'\r\n', b'\n')).hexdigest()


def classify(r):
    if r['rc'] == 3:
        return 'whole_sample_loss'
    e = (r['dx'], r['dy'])
    f = (r['full_dx'], r['full_dy'])
    if e == f:
        return 'exact_emitted_unit_agreement'
    if e == (0, 0):
        return 'hidden_nonzero_full_cadence_units'
    if any(x * y < 0 for x, y in zip(e, f)):
        return 'opposite_sign_output'
    return 'other_mismatch'


def run(cmd, timeout, cwd):
    try:
        return subprocess.run(cmd, cwd=cwd, capture_output=True, text=True,
                              timeout=timeout, check=False)
    except subprocess.TimeoutExpired:
        sys.exit('timeout after %ds: %s' % (timeout, ' '.join(cmd)))
    except OSError as exc:
        sys.exit('cannot run %s: %s' % (cmd[0], exc))


def parse(text, columns, what):
    rows = list(csv.DictReader(io.StringIO(text)))
    if not rows or list(rows[0].keys()) != columns:
        sys.exit('unexpected %s header' % what)
    return [{k: int(v) for k, v in d.items()} for d in rows]


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument('--repo', required=True)
    ap.add_argument('--compiler', required=True,
                    help='gcc/clang-compatible C99 compiler driver')
    ap.add_argument('--output', required=True,
                    help='new directory outside the repo; must not exist')
    a = ap.parse_args()
    repo = os.path.realpath(a.repo)
    out = os.path.realpath(a.output)
    if os.path.commonpath([repo, out]) == repo:
        sys.exit('output must be outside the repository')
    if os.path.lexists(out):
        sys.exit('output already exists: ' + out)
    for rel, want in INPUTS.items():
        try:
            got = lf_sha256(os.path.join(repo, *rel.split('/')))
        except OSError as exc:
            sys.exit('cannot read %s: %s' % (rel, exc))
        if got != want:
            sys.exit('input hash mismatch %s: %s' % (rel, got))
    os.makedirs(out)
    src = os.path.join(out, 'adapter.c')
    exe = os.path.join(out, 'adapter' + ('.exe' if os.name == 'nt' else ''))
    with open(src, 'w', newline='\n') as f:
        f.write(ADAPTER)
    cmd = [a.compiler, '-std=c99', '-pedantic', '-Wall', '-Wextra',
           '-Werror', '-Wconversion', '-Wsign-conversion',
           '-I', repo, src,
           os.path.join(repo, 'platform', 'mac128k', 'polled-phase.c'),
           '-o', exe]
    r = run(cmd, COMPILE_TIMEOUT, out)
    if r.returncode:
        sys.exit('compile failed:\n' + r.stdout + r.stderr)
    diagnostics = (r.stdout + r.stderr).strip()
    outputs = {}
    for mode in ('sweep', 'trace'):
        r = run([exe, mode], RUN_TIMEOUT, out)
        if r.returncode:
            sys.exit('adapter %s failed rc=%d:\n%s%s'
                     % (mode, r.returncode, r.stdout, r.stderr))
        outputs[mode] = r.stdout
    rows = parse(outputs['sweep'], COLUMNS, 'sweep')
    if len(rows) != EXPECTED_CASES:
        sys.exit('expected %d cases, got %d' % (EXPECTED_CASES, len(rows)))
    keys = {(r['axis'], r['start'], r['direction'], r['steps'], r['invert'], r['button']) for r in rows}
    expected = {(a, p, d, n, i, b) for a in range(2) for p in range(4) for d in (-1, 1) for n in range(13) for i in range(4) for b in range(2)}
    if keys != expected or len(keys) != len(rows) or any(r['rc'] not in (0, 1, 3) for r in rows):
        sys.exit('sweep coverage or return-code mismatch')
    with open(os.path.join(out, 'sweep.csv'), 'w', newline='\n') as f:
        f.write(outputs['sweep'])
    classes, by_steps, rcs = {}, {}, {}
    for row in rows:
        c = classify(row)
        classes[c] = classes.get(c, 0) + 1
        s = by_steps.setdefault(str(row['steps']), {})
        s[c] = s.get(c, 0) + 1
        name = RC.get(row['rc'], str(row['rc']))
        rcs[name] = rcs.get(name, 0) + 1
    trace = parse(outputs['trace'], TRACE_COLUMNS, 'trace')
    if len(trace) != 4 or {t['prior_pb'] for t in trace} != {0x8F, 0x9F, 0xAF, 0xBF} or any(t['baseline_rc'] != 2 or t['rc'] not in (0, 3) for t in trace):
        sys.exit('trace coverage or return-code mismatch')
    for t in trace:
        t['prior_pb'] = '%02X' % t['prior_pb']
        t['prior_pb4'] = int(t['prior_pb'], 16) >> 4 & 1
        t['prior_pb5'] = int(t['prior_pb'], 16) >> 5 & 1
        t['rc_name'] = RC.get(t['rc'], str(t['rc']))
    summary = {
        'study': 'endpoint-only vs full-cadence polled-phase sampling',
        'units': 'emitted DCD units of the software model; not physical '
                 'edges, pixels or elapsed time',
        'caveat': 'endpoint agreement is not proof that no physical '
                  'transitions were missed; inversion cannot restore '
                  'information absent from endpoints',
        'cases': len(rows),
        'classes': classes,
        'classes_by_steps': by_steps,
        'endpoint_rc': rcs,
        'trace_candidates': trace,
        'trace_note': 'current PB=AF RR0A=RR0B=4C, invert 0; prior PB4/PB5 '
                      'were not retained. Four logical candidates, not '
                      'probabilities or proof of which axis moved. Model '
                      'LOST(3) is distinct from wrapper loss_reason 6.',
        'compiler_command': cmd,
        'compiler_diagnostics': diagnostics,
    }
    with open(os.path.join(out, 'summary.json'), 'w', newline='\n') as f:
        json.dump(summary, f, indent=2, sort_keys=True)
        f.write('\n')
    print(json.dumps({'cases': len(rows), 'classes': classes}, sort_keys=True))
    return 0


if __name__ == '__main__':
    sys.exit(main())
