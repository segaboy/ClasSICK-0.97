<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# IMPL-0174 - Diagnostic Macintosh service terminals

[SPEC-0144](../../docs/specifications/SPEC-0144-startup-service-labels.md)
uses a link-time wrapper for the developer ROM loader and the independent
assembly gateway in `probes/mac128k/startup-service-label.S`. The catalog
selects an immutable ROM terminal after hardware vector entry. No guest RAM
report or implementation of the named service is required. The isolated
builder and replay modes are in `scripts/Test-StartupServiceLabels.py`.
