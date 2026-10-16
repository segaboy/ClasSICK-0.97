# Trying ClasSICK ROM 0.1.0-alpha.1

This experimental firmware runs an embedded original 68k test application. It
does not boot a System disk or provide a desktop. No Apple ROM or System disk
is needed for this demonstration. Real Macintosh operation is unverified.

## Download and check

Download `classick-rom-0.1.0-alpha.1-demo.zip` and `SHA256SUMS.txt` from the
`rom-v0.1.0-alpha.1` release. Check the ZIP's SHA-256 before extracting it.
Windows PowerShell can calculate it with:

```powershell
Get-FileHash .\classick-rom-0.1.0-alpha.1-demo.zip -Algorithm SHA256
```

The extracted `classick-rom-0.1.0-alpha.1-mac128k.rom` is exactly 65,536 bytes.
Its SHA-256 is:

```text
43c0296f808b85366fdbe55c9bcfd42e2daf9ef5fae77dfecfb85e5f80db0942
```

The same ROM is also offered as a standalone download. Keep an existing
emulator workspace separate; create a fresh workspace for this demo.

## Snow settings and expected result

The observed platform was [Snow v1.5.0](https://github.com/twvd/snow/releases/tag/v1.5.0),
Windows x86-64, reporting v1.5.0-317c188. The tested `snowemu.exe` SHA-256 is
`32cf5285582c0f509e4f1ac38d86bfb90440bae9da43b7823bfb0841dea588e6`.
Snow is downloaded separately and is not included or modified by this release.
Other versions and emulators have not been qualified for this image.

1. Create a fresh Snow workspace and load the extracted ClasSICK `.rom` file
   through its ROM loader. Allow the custom-ROM validation override for this
   image. Select Macintosh 128K (`Early128K`) with 128 KB RAM.
2. Use normal speed and **Relative (hardware emulation)** mouse mode. Leave
   persistent PRAM, extension ROM, 800K drive override and fast-forward off.
   Attach no disks, shared directory or serial bridge; start from reset rather
   than a saved machine state.
3. Start the machine. Drawing can take time. The result shows `APP`, `097`,
   the guest-written `5A5A5A5A` pattern and six completed phase marks.
4. The native kernel continues running. Click inside the display to exercise
   its mouse counter. Some automated presses were not credited in our trial;
   reliable input delivery and a human-input pass are still unqualified for
   this candidate. Counter updates should preserve the rest of the scene.
5. Close the fresh workspace when finished. No install or disk writing is needed.

If the ROM loader does not respond, first verify that the intended Snow window
has focus. Our earlier automation had inconsistent button mapping; a particular
button workaround is not a guaranteed fix. Record the Snow version, settings,
ROM checksum and visible result when reporting a problem. Attach no Apple ROM,
System disk, proprietary application or reference screenshot.

These are instructions based on the recorded trial; no new emulator run was
performed for packaging. Original System 1 boot and physical hardware remain
future compatibility targets.

## Source and rebuilding

Download `classick-rom-0.1.0-alpha.1-source.zip` alongside the demo. It contains
the complete tracked original project source at compiled revision
`2ea01129377d248ef48dd81d6f8873e948456ce6`, including the license, build scripts,
linker scripts, tests and hash-pinned toolchain definitions. It excludes local
tools, Git history, private manuals and untracked research originals.

The recorded compiler host is Ubuntu 24.04 x86-64. With Python 3 and `dpkg-deb`
available, run from the extracted source directory, choosing new output paths:

```sh
python3 scripts/Setup-M68k.py --output /tmp/classick-m68k
python3 scripts/Build-LoadedAppROM.py --tool-root /tmp/classick-m68k/root --output /tmp/classick-rom
```

Setup downloads the hash-pinned compiler packages; the build script does not
run an emulator. The resulting `Os-a/classick-loaded-app.rom` should have the
checksum above. Compare the input fingerprints and image hash in the supplied
`build-manifest.json`; stack-frame list ordering can vary by filesystem.
The original project is GPL-3.0-or-later; see `LICENSE` and `COPYRIGHT.md`.
The compiler and Snow have their own licenses and are not distributed here.
