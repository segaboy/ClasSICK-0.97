# Trying ClasSICK ROM 0.3.0-alpha.1

This is an experimental preview. It draws an original test scene. It is not System 1 or a Mac desktop, and it tells you nothing about real hardware.

The qualified Windows build is Snow v1.5.0-317c188 (v1.5.0), executable SHA-256 `32cf5285582c0f509e4f1ac38d86bfb90440bae9da43b7823bfb0841dea588e6`. Get it separately from the Snow project. No emulator is included. You do not need an Apple ROM or disk.

## 1. Check the files

Unzip `classick-rom-0.3.0-alpha.1-demo.zip`. Compare the hashes with SHA256SUMS.txt. In PowerShell use `Get-FileHash -Algorithm SHA256 classick-rom-0.3.0-alpha.1-mac128k.rom`; on Linux use `sha256sum -c SHA256SUMS.txt`.

## 2. Set up Snow

1. Start Snow with a fresh workspace that has never been used, so no old settings carry over.
2. Open the ROM loader and choose `classick-rom-0.3.0-alpha.1-mac128k.rom`.
3. Snow may report the ROM as unknown, because it is not an Apple ROM. When Snow validates the ROM, allow a custom ROM.
4. Choose the Early Macintosh 128K model with 128 KB of memory.
5. Do not add a PRAM file, extension, floppy or disk image, shared directory or bridge. None of these are needed.
6. Set the mouse to Relative hardware mouse at normal speed.

## 3. Run it

Start the emulated machine. You should see a black-and-white 512x342 screen with a ring and rails. It goes through six phases, then shows the result 0097 and a counter.

Click twice inside the emulated screen, pausing between the clicks. The counter should go up by one for each click.

The ROM has no quit command. When you are done, close the Snow window.

## Known limitations

- Snow may keep preferences between workspaces. Isolation from those preferences is not qualified.
- A person checked only Snow's Early Macintosh 128K model. Full 128K behaviour, mini vMac and real Macintosh hardware are not qualified.

## More information

- Release notes: RELEASE.txt in this ZIP.
- Source: https://github.com/segaboy/ClasSICK-0.97/tree/c631b824b2a264c6d81eb9832f17b7b5918aa9ff
- Evidence: https://github.com/segaboy/ClasSICK-0.97/blob/87e95de40cfb0f06b6398a3cfa762e9c62b87375/docs/development/preview-session-evidence.json
- License: GPL-3.0-or-later (see COPYING).
