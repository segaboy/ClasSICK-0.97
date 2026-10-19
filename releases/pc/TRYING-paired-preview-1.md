# Trying the continuous PC drawing preview

Unreleased DRAFT,2026-10-19; SPEC-0050. No physical-media or general-hardware
qualification. This is the original x86-64 OS preview,not a68k OS running in an
interpreter. The same original native drawing workload also builds for68000.

Use the matching original EFI/disk from this candidate in the documented isolated
VirtualBox EFI64/PS2 profile. Keep its source/checksum/instructions together.
The512x342 monochrome scene is centered in the available framebuffer. It shows
an outline ring,two rails,six phase marks,result0097 and the lower-right input count.

For a mouse check,click inside VirtualBox and accept its capture prompt if shown.
Record the counter after capture setup; that first click may itself count.
Click twice with distinct press/releases: expect baseline+2. Tap Space once:
expect baseline+3. The ring,rails and other scene pixels must stay intact. Record
any missed event rather than treating many clicks as a two-click pass.

VirtualBox pointer integration and pointer capture are separate. This prototype
does not provide Guest Additions or general pointer motion. Right Ctrl releases
the mouse to Windows. The current continuous outline preview has no normal
60-second stop; Escape does not quit it. Close/power off through the host to end
the session. Older timed diagnostic builds keep their60-second/Escape behavior.

The automated current-image test accepted38 mouse presses and5 Spaces through
guest second185. This tests injected device input; ordinary human capture/focus
reliability remains unqualified after the earlier intermittent-click report.
One GUI start failed before boot; the same VM booted headlessly without settings
changes. The runtime test passed there; this is not a fix for that host error.
[Actual evidence](../../docs/development/preview-session-evidence.md).
