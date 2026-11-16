# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Execute original register-preservation fixtures through the real veneers.

TRAP supplies a native exception frame; it does not simulate physical IRQ timing.
All generated source, objects, images, commands and observations stay outside Git.
"""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import struct
import subprocess

ROOT = Path(__file__).resolve().parents[1]
D = [0x12340000 + i * 0x10203 for i in range(8)]
A = [0xA1200000 + i * 0x20304 for i in range(7)] + [0x7000]
NAMES = ('cs_mac_window_via_irq', 'cs_mac_window_scc_irq', 'cs_mac_window_combined_irq')


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def boot_source(source, sr):
    lines = ['.section .vectors,"a"', '.long 0x7000, _start',
             '.rept 62', '.long cs_mac_fault', '.endr', '.text', '.globl _start', '_start:',
             'move.w #0x2700,%sr', 'move.b #0,0xEFFFFE', 'move.b #0x10,0xEFE7FE']
    lines += [f'move.l #{name},{0x80 + i*4}' for i, name in enumerate(NAMES)]
    lines += [f'move.l #{v},%d{i}' for i, v in enumerate(D)]
    lines += [f'movea.l #{v},%a{i}' for i, v in enumerate(A)]
    lines += [f'move.w #{sr},%sr', f'trap #{source-1}',
              '.globl fixture_done', 'fixture_done:', 'bra.s fixture_done',
              '.globl cs_mac_fault', 'cs_mac_fault:', 'bra.s cs_mac_fault',
              '.bss', '.even', '.globl fixture_seen', 'fixture_seen:', '.space 4',
              '.globl fixture_sr', 'fixture_sr:', '.space 2',
              '.section .note.GNU-stack,"",@progbits']
    return '\n'.join(lines)+'\n'


def handler_source():
    # Explicit clobbers make the compiler save its chosen nonvolatile registers.
    # They are unrelated sentinel values, not an expected-result implementation.
    # A6 is exercised by the forced compiler frame, not an inline-asm clobber.
    registers = [f'd{i}' for i in range(8)] + [f'a{i}' for i in range(6)]
    instructions = '; '.join(f'move.l #0x55aa{i:04x},%%{reg}' for i, reg in enumerate(registers))
    clobbers = ','.join('"'+reg+'"' for reg in registers + ['cc', 'memory'])
    return ('typedef unsigned int u32;\nextern volatile u32 fixture_seen;\nextern volatile unsigned short fixture_sr;\n'
            'void cs_mac_window_interrupt(u32 source) {\n'
            ' unsigned short sr; volatile u32 held=source;\n'
            ' fixture_seen=held;\n'
            ' __asm__ volatile("move.w %%sr,%0":"=d"(sr));\n'
            ' fixture_sr=sr;\n'
            ' __asm__ volatile("'+instructions+'":::'+clobbers+');\n}\n')


def mismatch(result, done, source, sr):
    wrong = []
    if result.get('status') != 'condition_met':
        return ['stop-condition']
    cpu = result.get('cpu', {})
    for name, expected in (('d', D), ('a', A), ('sr', sr), ('pc', done)):
        if cpu.get(name) != expected:
            wrong.append(name)
    memory = bytes.fromhex(result['memory']['bytes'])
    seen, mask = struct.unpack('>IH', memory)
    if seen != source:
        wrong.append('source-argument')
    if (mask & 0x2700) != 0x2700:
        wrong.append('active-mask')
    return wrong


def main():
    p = argparse.ArgumentParser(description=__doc__)
    for name in ('tool-bin', 'emulator', 'output'):
        p.add_argument('--'+name, type=Path, required=True)
    a = p.parse_args()
    out = a.output.resolve(); exe = a.emulator.resolve(strict=True)
    if out == ROOT or ROOT in out.parents:
        p.error('output must be outside repository')
    spec = importlib.util.spec_from_file_location('native_tools', ROOT/'scripts/m68k_windows_toolchain.py')
    native = importlib.util.module_from_spec(spec); spec.loader.exec_module(native)
    tools, tool_record = native.inspect(a.tool_bin.resolve(), 'm68k-unknown-elf-')
    out.mkdir(parents=True, exist_ok=False)
    veneer = ROOT/'arch/m68k/window-irqs.S'
    identities = {'emulator': sha(exe), 'veneer': sha(veneer), 'script': sha(Path(__file__))}
    (out/'window-irqs.S').write_bytes(veneer.read_bytes())
    linker = out/'fixture.ld'
    linker.write_text('ENTRY(_start)\nSECTIONS { . = 0x400000; .vectors : { *(.vectors) } .text : { *(.text*) } .bss 0xb00 (NOLOAD) : { *(.bss*) } /DISCARD/ : { *(.comment) *(.note*) *(.eh_frame*) } }\n')
    handler = out/'handler.c'; handler.write_text(handler_source())
    bad = out/'bad-callee.S'
    bad.write_text('.text\n.globl cs_mac_window_interrupt\ncs_mac_window_interrupt:\nmove.l 4(%sp),0xb00\nmove.w %sr,0xb04\nmove.l #0xbad,%d7\nrts\n')
    commands = []; cases = []; controls = []
    env = os.environ.copy(); env['PATH'] = str(a.tool_bin)+os.pathsep+env.get('PATH', '')

    def run(cmd):
        cmd = [str(x) for x in cmd]; commands.append(cmd)
        r = subprocess.run(cmd, capture_output=True, text=True, timeout=30, env=env,
                           creationflags=getattr(subprocess, 'CREATE_NO_WINDOW', 0))
        if r.returncode:
            raise RuntimeError('command failed: '+repr(cmd)+'\n'+r.stdout+r.stderr)
        return r.stdout

    def observe(name, image, symbols, source, sr, collection, expected_bad):
        rom = out/(name+'.rom'); rom.write_bytes(image + bytes(65536-len(image)))
        cmd = [exe, '--machine', 'mac128k', '--rom', rom, '--cycles', '50000', '--timeout-ms', '2000',
               '--until-pc', str(symbols['fixture_done']), '--inspect', '0xb00:6', '--mac-cpu-state', '1']
        cmd = [str(x) for x in cmd]; commands.append(cmd)
        r = subprocess.run(cmd, capture_output=True, text=True, timeout=5,
                           creationflags=getattr(subprocess, 'CREATE_NO_WINDOW', 0))
        (out/(name+'.log')).write_text(r.stdout+r.stderr)
        result = json.loads(r.stdout); differences = mismatch(result, symbols['fixture_done'], source, sr)
        if bool(differences) != expected_bad:
            raise ValueError(name+': unexpected preservation result '+repr(differences))
        collection.append({'name': name, 'source': source, 'sr': sr, 'rom_sha256': sha(rom),
                           'observed': result, 'differences': differences, 'returncode': r.returncode})

    for optimization in ('Os', 'O2'):
        obj = out/(optimization+'-handler.o')
        run([tools['gcc'], '-mcpu=68000', '-'+optimization, '-fno-omit-frame-pointer', '-ffreestanding',
             '-fno-builtin', '-fno-pic', '-fno-pie', '-Wall', '-Wextra', '-Werror', '-c', handler, '-o', obj])
        (out/(optimization+'-handler.dis')).write_text(run([tools['objdump'], '-d', obj]))
        for abi in (0, 1):
            vobj = out/f'veneer-{abi}.o'
            run([tools['gcc'], '-mcpu=68000', '-DCLASSICK_MAC_C_ABI_IRQ='+str(abi), '-c', out/'window-irqs.S', '-o', vobj])
            for source in (1, 2, 3):
                for sr in (0x2000, 0x231F):
                    name = f'{optimization}-abi{abi}-source{source}-sr{sr:04x}'
                    boot = out/(name+'.S'); boot.write_text(boot_source(source, sr))
                    elf = out/(name+'.elf'); raw = out/(name+'.bin')
                    run([tools['gcc'], '-mcpu=68000', '-nostdlib', '-no-pie', '-Wl,--build-id=none', '-Wl,-T,'+str(linker), boot, vobj, obj, '-o', elf])
                    symbols = {line.split()[-1]: int(line.split()[0], 16)
                               for line in run([tools['nm'], '--defined-only', elf]).splitlines() if len(line.split()) == 3}
                    run([tools['objcopy'], '-O', 'binary', elf, raw]); image = raw.read_bytes()
                    if len(image) > 65536:
                        raise ValueError('fixture exceeds ROM')
                    observe(name, image, symbols, source, sr, cases, False)
                    if optimization == 'Os' and source == 1 and sr == 0x2000:
                        offset = symbols[NAMES[0]]-0x400000
                        for mutation in ('missing-d1', 'return', 'mask'):
                            changed = bytearray(image)
                            if mutation == 'missing-d1':
                                old, new = ('c0c0', '80c0') if abi else ('fffe', 'bffe')
                                assert changed[offset+6:offset+8] == bytes.fromhex(old)
                                changed[offset+6:offset+8] = bytes.fromhex(new)
                                old, new = ('0303', '0301') if abi else ('7fff', '7ffd')
                                assert changed[offset+24:offset+26] == bytes.fromhex(old)
                                changed[offset+24:offset+26] = bytes.fromhex(new)
                            elif mutation == 'return':
                                assert changed[offset+26:offset+28] == bytes.fromhex('4e73')
                                changed[offset+26:offset+28] = bytes.fromhex('4e75')
                            else:
                                changed[offset+2:offset+4] = bytes(2)
                            observe(name+'-'+mutation, changed, symbols, source, sr, controls, True)
                        bad_elf = out/(name+'-bad-callee.elf'); bad_raw = out/(name+'-bad-callee.bin')
                        run([tools['gcc'], '-mcpu=68000', '-nostdlib', '-no-pie', '-Wl,--build-id=none', '-Wl,-T,'+str(linker), boot, vobj, bad, '-o', bad_elf])
                        run([tools['objcopy'], '-O', 'binary', bad_elf, bad_raw])
                        observe(name+'-bad-callee', bad_raw.read_bytes(), symbols, source, sr, controls, bool(abi))
    if sha(exe) != identities['emulator'] or sha(veneer) != identities['veneer']:
        raise ValueError('input changed during test')
    report = {'status': 'pass', 'identities': identities, 'toolchain': tool_record,
              'commands': commands, 'cases': cases, 'controls': controls,
              'limits': 'Original TRAP/C fixtures in BootHush; no physical IRQ or production call-graph proof. Bad-callee control demonstrates the ABI dependency.'}
    (out/'report.json').write_text(json.dumps(report, indent=2)+'\n')
    print(json.dumps({'status': 'pass', 'cases': len(cases), 'controls': len(controls), 'report': str(out/'report.json')}))


if __name__ == '__main__':
    main()
