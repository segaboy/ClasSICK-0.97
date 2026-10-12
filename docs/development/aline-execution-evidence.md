# Original A-line instruction checkpoint - 2026-10-12

SPEC-0021 / ADR-0024 / IMPL-0025 / TEST-0029; SRC-0092-0095.

**The original instruction probe now reaches shared services on both ports.**
Three native PC cold starts interpret the same twelve original 68000 bytes that
two own-ROM Snow 128K cold starts execute through actual vector-10 entry and
RTE. Each portable probe completes six instructions, three calls and eight checks,
zero errors/signature `414c4e31`. Each Mac also passes three hardware A-line
returns, all preserved register sentinels and SR checks, normal scene completion,
one old TRAP return, the expected ROM seal and zero faults.

The PC interpreter supports MOVEQ, NOP, RTS and the two intrinsic A-line services
only, in supervisor mode without tracing. It uses a collapsed immediate service
exception/return model, with checked fetch/stack buffers and a completed six-byte
frame. It does not fetch a general vector or interpret RTE. Mac uses the actual
68000 frame and RTE in its own bridge. These paths have distinct CPU claims.
The common fixture is a bare original probe, **not a legacy Macintosh application**.
Symbolic service tokens remain non-executable identities; arbitrary patches,
Toolbox/callback/user ABI, most instructions and the complete SPEC-0017 gateway
are unimplemented. No January 1984 or complete OS compatibility claim.

[Sanitized evidence](aline-execution-evidence.json) contains owned serial/register
observations, 64 diagnostic bytes per Mac run and original image/build identities.
Private paths, VM identities/inventory, logs/screenshots, manuals and emulator
binaries remain outside Git. No Apple ROM/OS implementation/assets, firmware
internals or emulator source inspected; no UART source/test body changes/review.

## Verification

Compiled source/observer `f29113b13775102f294d76e9c3deb1691228b1c8`.
[CI 38284131256](https://github.com/segaboy/ClasSICK-0.97/actions/runs/38284131256)
passes Windows and 68000 jobs. All six EFI O0/O2 fingerprints, both Mac ROM
profiles and every ROM input fingerprint match local. The downloaded own Os ROM
matches the admitted trial image. CI builds/tests; these native trials are local.

1,002 CTest checks pass in nine configurations:
124/124/124/97/124/109/109/109/82. New independent suites exhaust 2,048 MOVEQ
words, unsupported 16-bit words and 4,096 A-line formats, preserve registers/SR,
and cover fetch/span/stack/RTS boundaries, failure stability and independent tables.
Clang/GCC twins, actual i686 and scoped x64 sanitizers pass. Twenty CPU object
audits add to the existing twenty service audits across widths/optimizations and
ARM64 LE/BE compile-only profiles. Eight observer corruption/missing-data controls
reject; existing Mac 24 hosted suites/four zero-divisor controls pass.

Normal/UD/GP EFI each have four deterministic images, with 27/28/28 corruption
rejections and fourteen omitted-object rejections per profile; injections refuse
normal classification. The new exact terminal helper bytes/target are audited.
Existing nine media writers, thirty media corruptions and three writer refusals
pass. Only the normal new image is executed; earlier fault results stay attached
to their original dated images.

Four Mac builds/twins each reject eight corruptions, four resealed vector/bridge/
fixture mutations and eleven omitted objects. Exact original entry/save/restore/
callback/RTE bytes and common program bytes are checked. No target libc/libgcc
archive or startup runtime is linked; the reviewed original helper module remains.
Os/O0 ROM occupancy is 7,248/11,328 bytes, data/BSS 2,140. The existing 0x2000
upper bound and 8-KiB stack hold. Frame sums plus 192 bytes for assembly/exception
and margin are 1,676/1,460, **not measured stack high-water marks**.

| Admitted image | SHA-256 |
| --- | --- |
| Normal O2 EFI, 48,640 bytes, loaded extent 65,536 | `6e2ea128e819923e6f88fd933bf40c7a7c9c7871d18d59c08315976a610269a7` |
| Own 67,108,864-byte raw disk | `d178b720ee2a17db1aaa933b6652f18bf868e493728033d442717cf7fc66ba40` |
| Own Os 65,536-byte ROM, seal ad3b6bcb | `20175281806971dd9b21f50f8f726947b162da38fe0b10571311515c0f313b22` |
| Own O0 ROM, seal 2ccb7293 | `578f8a727de9f63d735988cc655739c6f68fd8ba8ad18540702bf56c8dce92d6` |

## Native observations

One fresh VirtualBox 7.2.16r174877 machine retains the previously reviewed
isolation: EFI64, one CPU, owned fixed VDI, no network/audio/USB/shares/clipboard/
drag/drop/remote display, Secure Boot off. Exact executable hash matches SRC-0059.
All three cold starts reach successful original keyboard/qualification/second-60
serial completion before any register query. Two delayed observations agree:
own terminal RIP/ordinary-stack RSP; RCX=bundle+503808+2088; R9=8/R10=0,
R11=`414c4e31`, R12=6/R13=3, R14=`43534131`. Nine selected registers only;
no memory/CR2/full dumps. No operator input proof or calibrated timing claim.
Final VM is off, VDI unchanged, raw roundtrip matches, inventory adds only this
identity. Earlier machines/media remain retained.

Both fresh Snow profiles use the exact cleared v1.5.0 binary, Early128K,
RelativeHw, own ROM only, audio off, normal speed, no disks/PRAM/extensions/
bridge/shared folder. Both reach stage4/ticks3610/presses0, one existing TRAP
return, seal ad3b6bcb, fault0, old services10/0/53564331 and new probe8/0/414c4e31,
steps6/calls3/native calls3. The native service slot is reset to unsupported after
the interpreter so hardware Set must install it itself. The assembly wrapper
compares D1-D7/A1-A6 sentinels, final D0/A0/SR and returns before the scene starts.
This proves bounded instruction/exception entry and return on the emulated CPU.

Computer Use operates host menus/address field only, observing own scene and
RAM. The initial viewer shows our copied vectors; B00 navigation also displays
adjacent own directory header. Only B00-B3F's 64 diagnostic bytes are retained.
No ROM/code/disassembly viewer or intentionally generated guest input. Both
windows close through normal Alt-F4, verified by absence and loop-exit logs.
Process exit codes were not retained; no zero-exit-code claim.

## Failure history and next work

GCC caught misleading indentation in the new test main after five passing Clang
configurations; corrected formatting and a fresh full chain pass. A local catalog
update refused UTF-8 under default encoding; corrected without altering old facts.
Publisher PDF downloads refused; eligible web-parsed manuals supplied the prose
and encodings, with no local hash/successful visual-page review claimed. Initial
Mac builds are retained; slot reset/exact bridge/assembly allowance were finalized
before any trial. All five admitted native starts pass.

Next is broader checked guest memory/instruction coverage and the user-call/
executable-stub/patch boundary, then individually specified managers. Full CPU,
historical SPEC-0017, physical Macintosh/PC, Mini vMac qualification, full budgets,
parity and three full editions remain open. B2 criterion6 firmware-image identity
is unchanged; platform/container hashes are not a firmware image fingerprint.
Human provenance review remains pending; no legally certified clean-room claim.
