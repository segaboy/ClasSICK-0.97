#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Qualify original DBcc/MOVEM fixtures on hosts, native286 and native68000.

Only original probe programs and generated diagnostic media are accepted. Native
286 inspections use bounded fresh deterministic replays; no owner media enters
this verifier. Reuses the project's admitted loader/toolchain identity helpers.
"""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
BUS = ROOT / 'core/cpu/m68k-bus.c'
CORE = [BUS, ROOT / 'core/cpu/m68k.c', ROOT / 'core/memory/guest.c', ROOT / 'core/services/traps.c']
PROBE = ROOT / 'probes/cpu/m68k-transfer.c'
PC_EXTRA = [ROOT / ('platform/pc286/' + name) for name in
            ('banked-guest.c', 'm68k-banked.c', 'm68k-transfer.c')]
PC_ENTRY = ROOT / 'probes/pc286/boot-m68k-transfer.c'
MAC_ENTRY = ROOT / 'probes/mac128k/m68k-transfer.c'
EMULATOR_SHA = '7adb8a74cef009b7c532df24458c3a5db6b77725356fdb31e5ac4966d7ed971a'
WARNINGS = ['-Wall', '-Wextra', '-Werror', '-Wconversion', '-Wsign-conversion']
PC_PHYSICAL = {
    0x49fdf: bytes([0x96]) + bytes(range(1, 32)),
    0x52000: bytes([32]),
    0x53000: bytes(range(33, 65)) + bytes([0x69]),
}
MUTATIONS = {
    'dbf-condition': ('if(c!=1u && condition(c,t->frame.sr))',
                      'if(condition(c,t->frame.sr))', 0xe1),
    'mask-order': ('unsigned index=pre?15u-i:i;', 'unsigned index=i;', 0xe2),
    'predecrement-base': ('value=index<8u?t->before->d[index]:t->before->a[index-8u];',
                         'value=index<8u?t->before->d[index]:t->before->a[index-8u];\n'
                         '            if(pre && index==reg+8u)value-=width;', 0xe3),
    'early-frame': ('        if(t.store_block)r->memory=t.transfer->write_bytes(t.backend->context,t.store_address,t.store_bytes,t.data);',
                    '        if(t.store_block)copy_frame(f,&t.frame);\n'
                    '        if(t.store_block)r->memory=t.transfer->write_bytes(t.backend->context,t.store_address,t.store_bytes,t.data);', 0xe5),
}


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


def omf_symbols(metadata, kind):
    sections = re.findall(r'(?:static )?' + kind + r'\([^\n]*\n(.*?)(?=\n(?:static )?[A-Z][A-Z0-9]+\(|\Z)',
                          metadata, re.S)
    return {value for section in sections for value in re.findall(r"'([^']+)'", section)}


def inspection_groups(requests):
    groups, current, size = [], [], 0
    for request in requests:
        if not 1 <= request[1] <= 4096:
            raise ValueError('inspection range outside original bounded profile')
        if current and (len(current) == 16 or size + request[1] > 16384):
            groups.append(current)
            current, size = [], 0
        current.append(request)
        size += request[1]
    if current:
        groups.append(current)
    return groups


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('cc', 'tool-root', 'm68k-bin', 'emulator', 'output'):
        parser.add_argument('--' + name, required=True, type=Path)
    args = parser.parse_args()
    out = None
    progress = {'status': 'incomplete', 'commands': [], 'hosted': [], 'c16': [], 'pc286': [], 'mac128k': []}
    try:
        cc = args.cc.resolve(strict=True)
        tool = args.tool_root.resolve(strict=True)
        emulator = args.emulator.resolve(strict=True)
        out = args.output.resolve()
        if out == ROOT or ROOT in out.parents:
            raise ValueError('output must be outside the source repository')
        if out.exists():
            raise ValueError('output must be a fresh directory')
        if sha(emulator) != EMULATOR_SHA:
            raise ValueError('emulator does not match the admitted original-fixture executable')
        lock_path = ROOT / 'scripts/pc286-toolchain-lock.json'
        lock = json.loads(lock_path.read_text(encoding='utf-8'))
        tool_paths = {'host_cc': cc, 'emulator': emulator, 'python': Path(sys.executable).resolve(strict=True)}
        for item in lock['selected_members']:
            path = tool / item['path']
            if path.stat().st_size != item['bytes'] or sha(path) != item['sha256']:
                raise ValueError('pinned C16 compiler/header mismatch: ' + item['path'])
            tool_paths['pc286:' + item['path']] = path
        native = module('transfer_m68k_tools', ROOT / 'scripts/m68k_windows_toolchain.py')
        m68k, identity = native.inspect(args.m68k_bin)
        tool_paths.update({'m68k:' + name: path for name, path in m68k.items()})
        loader = module('transfer_pc_loader', ROOT / 'scripts/pc286_loader.py')
        inputs = [*CORE, PROBE, *PC_EXTRA, PC_ENTRY, MAC_ENTRY, *[ROOT / name for name in (
            'core/cpu/m68k-bus.h', 'core/cpu/m68k-backend.h', 'core/cpu/m68k-guard.h',
            'core/cpu/m68k-transfer.h', 'core/cpu/m68k.h', 'core/memory/guest.h', 'core/services/traps.h',
            'platform/pc286/banked-guest.h', 'platform/pc286/m68k-banked.h', 'platform/pc286/m68k-transfer.h',
            'probes/cpu/m68k-transfer.h', 'tests/m68k-transfer.c', 'tests/m68k-startup.c',
            'tests/m68k-bus.c', 'tests/m68k-guard.c', 'apps/memory-probe/probe.c',
            'apps/memory-probe/probe.h', 'apps/memory-probe/program.h', 'apps/aline-probe/program.h',
            'docs/specifications/SPEC-0123-m68k-loop-register-transfer.md',
            'docs/specifications/SPEC-0120-banked-m68k-execution.md',
            'docs/specifications/SPEC-0118-m68k-address-stack.md',
            'docs/specifications/SPEC-0117-pc286-banked-guest-memory.md',
            'docs/specifications/SPEC-0022-guest-memory-execution.md',
            'docs/specifications/SPEC-0026-bounded-guest-calls.md',
            'docs/specifications/SPEC-0034-owned-image-execution.md',
            'provenance/records/SRC-0616-m68k-address-stack-instructions.md',
            'provenance/records/SRC-0618-movem-instruction.md',
            'arch/i286/c16.h', 'arch/m68k/mac-reset.S', 'arch/m68k/mac-rom.ld',
            'scripts/m68k_windows_toolchain.py', 'scripts/pc286_loader.py',
            'scripts/Test-M68kStartup.py', 'scripts/Test-BankedM68k.py')], lock_path, Path(__file__).resolve()]
        source_hashes = {path.relative_to(ROOT).as_posix(): sha(path) for path in inputs}
        tool_hashes = {name: sha(path) for name, path in tool_paths.items()}
        progress.update(source_sha256=source_hashes, tool_sha256=tool_hashes, m68k_identity=identity)
        out.mkdir(parents=True, exist_ok=False)
        env = os.environ.copy()
        env['PATH'] = str(cc.parent) + os.pathsep + str(m68k['gcc'].parent) + os.pathsep + env.get('PATH', '')
        for key in ('WCC', 'WATCOM', 'INCLUDE', 'LIB', 'WCL', 'WLINK'):
            env.pop(key, None)

        def run(command, label, timeout=45):
            command = [str(item) for item in command]
            progress['commands'].append(command)
            progress['stage'] = label
            result = subprocess.run(command, cwd=out, env=env, capture_output=True, timeout=timeout,
                                    creationflags=getattr(subprocess, 'CREATE_NO_WINDOW', 0))
            (out / (label + '.stdout')).write_bytes(result.stdout)
            (out / (label + '.stderr')).write_bytes(result.stderr)
            if result.returncode:
                raise ValueError(label + ' failed with exit ' + str(result.returncode) + '; retained stdout/stderr')
            return result.stdout

        suites = {'transfer': [''], 'startup': [''],
                  'bus': ['registers', 'operands', 'branches', 'failures', 'services', 'probe'],
                  'guard': ['events', 'atomicity', 'control', 'services', 'aliases']}
        transfer_counts = []
        for profile, language, flags in (
                ('release', 'c11', ['-O3', '-DNDEBUG']),
                ('sanitized', 'c11', ['-O1', '-g', '-fsanitize=address,undefined',
                                     '-fno-sanitize-recover=all', '-fno-omit-frame-pointer']),
                ('c99', 'c99', ['-O2'])):
            for name, cases in suites.items():
                binary = out / (profile + '-' + name + '.exe')
                extra = [ROOT / 'apps/memory-probe/probe.c'] if name == 'bus' else []
                run([cc, '-std=' + language, *WARNINGS, *flags, *CORE, *extra,
                     ROOT / ('tests/m68k-' + name + '.c'), '-o', binary], profile + '-' + name + '-build')
                for case in cases:
                    result = run([binary, *([case] if case else [])], profile + '-' + name + '-' + (case or 'all')).decode().strip()
                    row = {'profile': profile, 'suite': name, 'case': case, 'result': result or 'exit 0', 'sha256': sha(binary)}
                    if name == 'transfer':
                        count = re.fullmatch(r'M68K transfer: ([1-9][0-9]*) checks passed', result)
                        if count is None:
                            raise ValueError('missing complete transfer host test report')
                        row['checks'] = int(count.group(1))
                        transfer_counts.append(row['checks'])
                    progress['hosted'].append(row)
        if len(set(transfer_counts)) != 1:
            raise ValueError('host transfer profiles executed different assertion counts')

        source = BUS.read_text(encoding='utf-8')
        variants = [('baseline', BUS, 0xba)]
        mutation_hashes = {}
        for name, (old, new, marker) in MUTATIONS.items():
            if source.count(old) != 1:
                raise ValueError('changed mutation anchor: ' + name)
            path = out / ('mutant-' + name + '.c')
            path.write_text(source.replace(old, new), encoding='utf-8')
            variants.append((name, path, marker))
            mutation_hashes[name] = sha(path)
        progress['mutation_sha256'] = mutation_hashes
        cflags = ['-zq', '-2', '-zl', '-s', '-os', '-zastd=c99', '-we',
                  '-dCS_C16_REQUIRE_16BIT', '-i=' + str(tool / 'h'), '-i=' + str(BUS.parent)]
        allowed = {
            'm68k-bus.c': {'cs_guest_validate_', 'cs_guest_read_', 'cs_guest_window_', 'cs_m68k_aline_'},
            'm68k.c': {'cs_trap_call_'}, 'guest.c': set(), 'traps.c': set(),
            'probe': {'cs_guest_validate_', 'cs_guest_window_', 'cs_guest_read_', 'cs_guest_write_', 'cs_m68k_bus_step_transfer_'},
            'banked-guest.c': set(),
            'm68k-banked.c': {'cs_pc286_guest_validate_', 'cs_pc286_guest_span_', 'cs_pc286_guest_read_', 'cs_pc286_guest_write_'},
            'adapter': {'cs_pc286_m68k_backend_', 'cs_pc286_guest_write_bytes_'},
            'entry': {'cs_m68k_transfer_probe_', 'cs_pc286_guest_bind_', 'cs_pc286_guest_read_',
                      'cs_pc286_guest_write_', 'cs_pc286_guest_write_bytes_', 'cs_pc286_guest_retire_',
                      'cs_pc286_m68k_transfer_backend_', 'cs_m68k_bus_step_transfer_'},
        }

        def audit_object(obj, label, permitted):
            metadata = run([tool / 'binnt64/dmpobj.exe', obj], label + '-metadata').decode('utf-8', errors='replace').replace('\r\n', '\n')
            if '80286' not in metadata or 'USE16' not in metadata:
                raise ValueError(label + ': object is not native80286 USE16')
            definitions = omf_symbols(metadata, 'L?PUBDEF')
            imports = omf_symbols(metadata, 'L?EXTDEF') - definitions
            if not imports <= permitted:
                raise ValueError(label + ': unexpected C16 imports ' + repr(sorted(imports - permitted)))
            lengths = [int(value, 16) for value in re.findall(r'USE16 Length ([0-9a-fA-F]+)h', metadata)]
            if not lengths:
                raise ValueError(label + ': no USE16 segment lengths')
            return {'sha256': sha(obj), 'imports': sorted(imports), 'definitions': sorted(definitions),
                    'use16_segment_bytes': lengths}

        baseline_objects = {}
        for model in ('ms', 'mc'):
            objects, rows = [], []
            for index, path in enumerate([*CORE, PROBE]):
                label = model + '-' + str(index)
                obj = out / (label + '.obj')
                run([tool / 'binnt64/wcc.exe', *cflags, '-' + model, '-fo=' + str(obj), path], label + '-compile')
                row = audit_object(obj, label, allowed['probe' if path == PROBE else path.name])
                row['source'] = path.relative_to(ROOT).as_posix()
                rows.append(row)
                objects.append(obj)
            progress['c16'].append({'model': model, 'objects': rows, 'scope': 'Object/import qualification; native execution uses small model.'})
            baseline_objects[model] = objects
        pc_extra_objects, pc_extra_rows = [], []
        for index, path in enumerate([*PC_EXTRA, PC_ENTRY]):
            label = 'pc-extra-' + str(index)
            obj = out / (label + '.obj')
            run([tool / 'binnt64/wcc.exe', *cflags, '-ms', '-fo=' + str(obj), path], label + '-compile')
            permitted = allowed['entry' if path == PC_ENTRY else 'adapter' if path.name == 'm68k-transfer.c' else path.name]
            row = audit_object(obj, label, permitted)
            row['source'] = path.relative_to(ROOT).as_posix()
            pc_extra_objects.append(obj)
            pc_extra_rows.append(row)
        progress['pc286_native_objects'] = pc_extra_rows

        for name, path, marker in variants:
            objects = [*baseline_objects['ms'], *pc_extra_objects]
            mutation_object = None
            if name != 'baseline':
                obj = out / ('pc-' + name + '.obj')
                run([tool / 'binnt64/wcc.exe', *cflags, '-ms', '-fo=' + str(obj), path], 'pc-' + name + '-compile')
                mutation_object = audit_object(obj, 'pc-' + name, allowed['m68k-bus.c'])
                objects[0] = obj
            binary, mapfile = out / ('pc-' + name + '.bin'), out / ('pc-' + name + '.map')
            run([tool / 'binnt64/wlink.exe', 'format', 'raw', 'bin', 'option', 'start=start_', 'option', 'nodefaultlibs',
                 'option', 'map=' + str(mapfile), 'name', binary, 'file', ','.join(str(item) for item in objects)], 'pc-' + name + '-link')
            mapping = mapfile.read_text(encoding='utf-8', errors='replace')
            entry = re.search(r'Entry point address:\s*([0-9a-fA-F]+)', mapping)
            bss = re.search(r'^_BSS\s+BSS\s+DGROUP\s+([0-9a-fA-F]+)\s+([0-9a-fA-F]+)', mapping, re.M)
            if entry is None or bss is None or '.lib' in mapping.lower():
                raise ValueError('286 missing entry/BSS or unexpected runtime library: ' + name)
            bss_start, bss_size = (int(value, 16) for value in bss.groups())
            payload = binary.read_bytes()
            image = out / ('pc-' + name + '.img')
            image.write_bytes(loader.floppy(payload, int(entry.group(1), 16), bss_end=bss_start + bss_size,
                                            poison_bss=True, verify_payload=True))
            image_hash = sha(image)
            requests = [(0x10000 + at, min(4096, len(payload) - at)) for at in range(0, len(payload), 4096)]
            payload_ranges = len(requests)
            if name == 'baseline':
                requests += [(at, len(value)) for at, value in PC_PHYSICAL.items()]
            memories, replays = [], []
            for replay, group in enumerate(inspection_groups(requests)):
                command = [emulator, '--machine', 'pc286', '--floppy', image, '--cycles', '60000000',
                           '--timeout-ms', '25000', '--until-post', hex(marker)]
                for at, size in group:
                    command += ['--inspect', hex(at) + ':' + str(size)]
                result = json.loads(run(command, 'pc-' + name + '-guest-' + str(replay)))
                ranges = result.pop('memory_ranges', None)
                single = result.pop('memory', None)
                ranges = ranges if ranges is not None else [single or {}]
                if (result.get('status') != 'condition_met' or result.get('post') != marker or result.get('cs') != 0x1000):
                    raise ValueError('286 unexpected stop: ' + name)
                if replays and result != replays[0]:
                    raise ValueError('286 fresh replay reported different machine state: ' + name)
                if len(ranges) != len(group) or any(row.get('address') != at or
                        len(bytes.fromhex(row.get('bytes', ''))) != size for row, (at, size) in zip(ranges, group)):
                    raise ValueError('286 malformed bounded inspection: ' + name)
                memories.extend(ranges)
                replays.append(result)
                if sha(image) != image_hash:
                    raise ValueError('286 generated media changed: ' + name)
            loaded = b''.join(bytes.fromhex(row['bytes']) for row in memories[:payload_ranges])
            if loaded != payload:
                raise ValueError('286 complete loaded program differs: ' + name)
            physical = {row['address']: bytes.fromhex(row['bytes']) for row in memories[payload_ranges:]}
            if name == 'baseline' and physical != PC_PHYSICAL:
                raise ValueError('286 final physical bank/guard bytes differ')
            progress['pc286'].append({'case': name, 'expected_post': marker, 'mutation_object': mutation_object,
                'payload_bytes': len(payload), 'payload_sha256': sha(binary), 'bss_start': bss_start, 'bss_bytes': bss_size,
                'linked_near_end': bss_start + bss_size, 'initial_stack_top': 65534, 'runtime_modules': [],
                'floppy_sha256': image_hash, 'loader_poison_bss': True, 'loader_clear_bss': True,
                'loader_payload_checksum': True, 'loaded_payload_matches': True,
                'physical_ram': {hex(at): value.hex() for at, value in physical.items()},
                'inspection_groups': inspection_groups(requests), 'replays': replays, 'input_unchanged': True})

        mac_cases = [('O0', BUS, 0xba, '-O0'), ('Os', BUS, 0xba, '-Os')]
        mac_cases += [(name, path, marker, '-Os') for name, path, marker in variants[1:]]
        for name, path, marker, optimization in mac_cases:
            objects, frames = [], []
            sources = [ROOT / 'arch/m68k/mac-reset.S', MAC_ENTRY, PROBE, path, *CORE[1:]]
            for index, source_path in enumerate(sources):
                obj = out / ('mac-' + name + '-' + str(index) + '.o')
                flags = ['-mcpu=68000', '-ffreestanding', '-fno-builtin', '-nostdlib', optimization, '-I', BUS.parent]
                if source_path.suffix == '.c':
                    flags += ['-std=c99', *WARNINGS, '-fstack-usage']
                run([m68k['gcc'], *flags, '-c', source_path, '-o', obj], 'mac-' + name + '-' + str(index) + '-compile')
                objects.append(obj)
                if source_path.suffix == '.c':
                    frames += obj.with_suffix('.su').read_text(encoding='utf-8').splitlines()
            elf, raw, rom = (out / ('mac-' + name + suffix) for suffix in ('.elf', '.raw', '.rom'))
            run([m68k['gcc'], '-mcpu=68000', '-nostdlib', '-Wl,--build-id=none',
                 '-Wl,-T,' + str(ROOT / 'arch/m68k/mac-rom.ld'), *objects, '-o', elf], 'mac-' + name + '-link')
            if run([m68k['nm'], '-u', elf], 'mac-' + name + '-imports').strip():
                raise ValueError('Mac unresolved import: ' + name)
            symbols = run([m68k['nm'], '-n', '-S', elf], 'mac-' + name + '-symbols').decode().replace('\r\n', '\n')
            diag = re.search(r'^([0-9a-fA-F]+)\s+[0-9a-fA-F]+\s+[BbDd]\s+cs_mac_diag\s*$', symbols, re.M)
            boundaries = {}
            for symbol in ('__data_start', '__data_end', '__bss_start', '__bss_end'):
                match = re.search(r'^([0-9a-fA-F]+)\s+(?:[0-9a-fA-F]+\s+)?[A-Za-z]\s+' + symbol + r'\s*$', symbols, re.M)
                if match is None:
                    raise ValueError('missing Mac memory-boundary symbol: ' + symbol)
                boundaries[symbol] = int(match.group(1), 16)
            if diag is None:
                raise ValueError('missing Mac diagnostic symbol')
            address = int(diag.group(1), 16)
            if not 0xb00 <= address <= 0x2000 - 28 or not 0xb00 <= boundaries['__data_start'] <= boundaries['__bss_end'] <= 0x2000:
                raise ValueError('Mac probe violates unchanged reset RAM budget')
            run([m68k['objcopy'], '-O', 'binary', elf, raw], 'mac-' + name + '-binary')
            payload = raw.read_bytes()
            if not 260 <= len(payload) <= 65536:
                raise ValueError('Mac original probe does not fit ROM')
            rom.write_bytes(payload + bytes([255]) * (65536 - len(payload)))
            rom_hash = sha(rom)
            result = json.loads(run([emulator, '--machine', 'mac128k', '--rom', rom, '--cycles', '50000000',
                '--timeout-ms', '20000', '--until', hex(address) + ':4:' + hex(marker),
                '--inspect', hex(address) + ':28'], 'mac-' + name + '-guest'))
            memory = result.pop('memory', {})
            actual = bytes.fromhex(memory.get('bytes', ''))
            if (result.get('status') != 'condition_met' or memory.get('address') != address or len(actual) != 28 or
                    int.from_bytes(actual[:4], 'big') != marker or actual[24:] != bytes(4) or sha(rom) != rom_hash):
                raise ValueError('Mac stop/diagnostic/input check failed: ' + name)
            sizes = {function: int(size, 16) for _, size, function in re.findall(
                r'^([0-9a-fA-F]+)\s+([0-9a-fA-F]+)\s+[Tt]\s+([A-Za-z_][A-Za-z0-9_]*)\s*$', symbols, re.M)}
            progress['mac128k'].append({'case': name, 'expected_marker': marker, 'result': result,
                'rom_sha256': rom_hash, 'raw_bytes': len(payload), 'diag_address': address, 'diag_hex': actual.hex(),
                'memory_boundaries': boundaries, 'data_bss_limit': 0x2000, 'stack_bottom': 0x2000,
                'initial_stack_top': 0x4000, 'local_compiler_frames': frames, 'function_bytes': sizes,
                'runtime_modules': [], 'rom_artifact_unchanged': True,
                'loaded_rom_inspection': 'Not attempted; this emulator inspection API admits RAM/video only.'})

        _, after_identity = native.inspect(args.m68k_bin)
        if after_identity != identity:
            raise ValueError('m68k target/compiler identity changed during verification')
        if {path.relative_to(ROOT).as_posix(): sha(path) for path in inputs} != source_hashes:
            raise ValueError('declared source/spec/helper changed during qualification')
        if {name: sha(path) for name, path in tool_paths.items()} != tool_hashes:
            raise ValueError('declared compiler/header/emulator changed during qualification')
        for name, path, _ in variants[1:]:
            if sha(path) != mutation_hashes[name]:
                raise ValueError('generated original mutation changed: ' + name)
        progress.update(status='pass', stage='complete', inputs_unchanged=True,
            limits='Original bounded DBcc/MOVEM interpreter qualification. Native286 executes the small model, including '
                   '128KiB guest backing and 64byte writes across three physical banks; compact model is object/import '
                   'qualification only. Mac tests execute original68000 ROM probes with unchanged reset/linker memory bounds. '
                   'No protected program, complete CPU, Resource Manager, System startup, persistence or physical hardware '
                   'claim. Local compiler frames are not cumulative stack proof. Modeled cycles are not hardware timing. '
                   'Only the declared tool/source closure is pinned; no wider transitive toolchain claim.')
        (out / 'report.json').write_text(json.dumps(progress, indent=2) + '\n', encoding='utf-8')
        print(json.dumps({'status': 'pass', 'host_suites': len(progress['hosted']), 'transfer_checks': transfer_counts[0],
            'pc286_cases': len(progress['pc286']), 'mac128k_cases': len(progress['mac128k']),
            'report': str(out / 'report.json')}, indent=2))
        return 0
    except (OSError, ValueError, KeyError, subprocess.TimeoutExpired) as exc:
        progress.update(status='failed', error=str(exc))
        if out is not None and out.is_dir() and progress.get('source_sha256'):
            (out / 'failure.json').write_text(json.dumps(progress, indent=2) + '\n', encoding='utf-8')
        print('M68K transfer verification failed: ' + str(exc), file=sys.stderr)
        return 1


if __name__ == '__main__':
    sys.exit(main())
