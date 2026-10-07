# ADR-0007 — Hosted contracts and measurable native boot

- Date: 2026-10-07
- Status: accepted
- Sources: SRC-0001, SRC-0005; independent milestone design

## Context

A Windows hosted process accelerates testing but is not a native OS boot. A UEFI
program drawing while firmware provides services does not prove kernel ownership.
Macintosh 128K startup and size budgets remain unverified.

## Decision

Start with bounded graphics surfaces/headless tests and a Windows adapter. Stage
memory/events for B1, then a native x86-64 UEFI handoff with own framebuffer,
timing/input and repeatable evidence for B2. Macintosh 128K is a separate required
research/size/startup gate, followed by native 68000 evidence. ARM64 native core
execution is required before the release portability claim.

## Alternatives

Starting with Finder visuals has weak test leverage. Macintosh-first hardware
work would entangle startup uncertainty with core contracts. Calling hosted or
pre-handoff firmware output a native boot would misstate progress.

## Consequences and verification

Native B2 does not need MFS/Toolbox/68000 translation. Post-ExitBootServices
progress and an owned input driver are required. Early VM keyboard support can
be narrower than modern PC USB support; document the exact tested machine.

## Review conditions

Target order may change with hardware/tooling availability. Do not silently drop
128K or inflate its memory requirement when size measurements arrive.
