# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
import importlib.util
import pathlib
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parent.parent
SPEC = importlib.util.spec_from_file_location('scene_guard', ROOT / 'scripts' / 'Verify-SceneImportContracts.py')
guard = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(guard)

GOOD = ['cs_a', 'cs_b', 'cs_native_draw']
SCENE = 'apps/boot-scene/scene.c'


def drawing(imports, extra=''):
    return "$modules=@(\n    @('x/a.c','a','q'),\n    @('" + SCENE + "','boot','" + ','.join(imports) + "'))\n" + extra


def native(imports):
    return "$portable=@(@('p/f.c','cs_a'),\n    @('" + SCENE + "','" + ','.join(imports) + "'))\n"


def uefi(imports, before=2, other='cs_other'):
    srcs = ['s/%d.c' % i for i in range(before)] + [SCENE, 's/tail.c']
    allowed = {before: ','.join(imports), before + 1: other, before + 5: 'cs_native_draw'}
    body = ';'.join('%d=\'%s\'' % kv for kv in allowed.items())
    return '$sources=@(' + ','.join("'" + s + "'" for s in srcs) + ')\n$allowedImports=@{' + body + '}\n'


def cmake(imports, target='classick_boot_scene_freestanding', name='scene.freestanding', decoy=''):
    return (decoy + '    add_test(NAME other.freestanding COMMAND ${CMAKE_COMMAND}\n        "-DALLOWED=cs_zzz"\n'
            '        "-DOBJECT=$<TARGET_OBJECTS:classick_other>" -P x.cmake)\n'
            '    add_test(NAME ' + name + ' COMMAND ${CMAKE_COMMAND}\n        "-DNM=${NM}"\n'
            '        "-DALLOWED=' + ','.join(imports) + '"\n'
            '        "-DOBJECT=$<TARGET_OBJECTS:' + target + '>"\n        -P x.cmake)\n')


def tree(root, imports=GOOD, **over):
    files = {
        'scripts/Verify-DrawingAppObjects.ps1': drawing(imports),
        'scripts/Verify-NativeFramebuffer.ps1': native(imports),
        'scripts/Verify-UEFIImage.ps1': uefi(imports),
        'CMakeLists.txt': cmake(imports),
    }
    files.update(over)
    for rel, text in files.items():
        if text is None:
            continue
        path = pathlib.Path(root) / rel
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(text.encode('utf-8'))
    return pathlib.Path(root)


class SceneImportContracts(unittest.TestCase):
    def run_audit(self, **kw):
        imports = kw.pop('imports', GOOD)
        with tempfile.TemporaryDirectory() as d:
            return guard.audit(tree(d, imports, **kw))

    def test_agreeing_fixture_passes(self):
        self.assertEqual([], self.run_audit())

    def test_stale_probe_rejected_everywhere(self):
        imports = GOOD + ['cs_drawing_app_probe']
        self.assertTrue(self.run_audit(imports=imports))

    def test_stale_probe_in_one_consumer_rejected(self):
        self.assertTrue(self.run_audit(**{'scripts/Verify-NativeFramebuffer.ps1': native(GOOD + ['cs_drawing_app_probe'])}))

    def test_missing_native_call_rejected_when_all_agree(self):
        self.assertTrue(self.run_audit(imports=['cs_a', 'cs_b']))

    def test_extra_import_in_cmake_rejected(self):
        self.assertTrue(self.run_audit(**{'CMakeLists.txt': cmake(GOOD + ['cs_extra'])}))

    def test_duplicate_token_rejected(self):
        self.assertTrue(self.run_audit(**{'scripts/Verify-UEFIImage.ps1': uefi(GOOD + ['cs_a'])}))

    def test_empty_token_rejected(self):
        self.assertTrue(self.run_audit(**{'scripts/Verify-NativeFramebuffer.ps1': native(['cs_a', '', 'cs_native_draw'])}))

    def test_duplicate_selected_record_rejected(self):
        twice = drawing(GOOD) + drawing(GOOD)
        self.assertTrue(self.run_audit(**{'scripts/Verify-DrawingAppObjects.ps1': twice}))

    def test_missing_file_fails_closed(self):
        self.assertTrue(self.run_audit(**{'scripts/Verify-UEFIImage.ps1': None}))

    def test_malformed_file_fails_closed(self):
        self.assertTrue(self.run_audit(**{'scripts/Verify-NativeFramebuffer.ps1': '@(@('}))

    def test_uefi_index_movement_follows_scene(self):
        for before in (0, 5, 12):
            with self.subTest(before=before):
                self.assertEqual([], self.run_audit(**{'scripts/Verify-UEFIImage.ps1': uefi(GOOD, before)}))

    def test_uefi_wrong_index_entry_rejected(self):
        text = uefi(GOOD, 2).replace("2='" + ','.join(GOOD) + "'", "2='cs_other'")
        self.assertTrue(self.run_audit(**{'scripts/Verify-UEFIImage.ps1': text}))

    def test_cmake_wrong_block_rejected(self):
        self.assertTrue(self.run_audit(**{'CMakeLists.txt': cmake(GOOD, name='other.freestanding')}))

    def test_cmake_wrong_target_rejected(self):
        self.assertTrue(self.run_audit(**{'CMakeLists.txt': cmake(GOOD, target='classick_other')}))

    def test_cmake_name_suffix_is_not_the_selected_test(self):
        self.assertTrue(self.run_audit(**{'CMakeLists.txt': cmake(GOOD, name='scene.freestanding.other')}))

    def test_commented_decoy_ignored(self):
        decoy = '    # add_test(NAME scene.freestanding COMMAND x "-DALLOWED=cs_bad")\n'
        text = decoy + cmake(GOOD)
        ps = '# @(\'' + SCENE + "','boot','cs_bad')\n" + drawing(GOOD)
        self.assertEqual([], self.run_audit(**{'CMakeLists.txt': text, 'scripts/Verify-DrawingAppObjects.ps1': ps}))

    def test_order_whitespace_crlf_bom_tolerated(self):
        shuffled = ['cs_native_draw', 'cs_b', 'cs_a']
        spaced = ' , '.join(shuffled)
        crlf = lambda t: '\ufeff' + t.replace('\n', '\r\n')
        self.assertEqual([], self.run_audit(**{
            'scripts/Verify-NativeFramebuffer.ps1': crlf(native(shuffled)),
            'CMakeLists.txt': crlf(cmake([spaced])),
        }))

    def test_diagnostics_deterministic(self):
        kw = {'CMakeLists.txt': cmake(GOOD + ['cs_extra'])}
        self.assertEqual(self.run_audit(**kw), self.run_audit(**kw))


if __name__ == '__main__':
    unittest.main()
