<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# TEST-0222 - Installed startup with a separate read-only floppy

SPEC-0131 / IMPL-0161. The verifier installs onto a disposable fixed disk and
checks the complete installed image. Repeated boots inspect only the native
payload and aggregate startup state. See the [cases and identities](../../docs/development/installed-startup-20261119.md).
