<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# TEST-0208 - Host and native guarded address/frame execution

2026-11-18; SPEC-0118/IMPL-0148. `tests/m68k-startup.c`, original shared
probe/native wrappers and `scripts/Test-M68kStartup.py` pass the final `-c`
qualification. Thirty host sub-suites include 27,589 new assertions in each
Release/sanitized/C99 startup build. C16 small/compact import audits pass.
Four native286 and five nativeMac baseline/mutation outcomes have the expected
markers and unchanged inputs. Exact report/source identities, earlier harness
setup failures, measurements and limits are in the
[canonical receipt](../../docs/development/m68k-address-stack-20261118.md).
Original fixtures only; no protected-program, physical or System boot claim.
