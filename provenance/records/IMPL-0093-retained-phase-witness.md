# IMPL-0093 - Opt-in retained decoder-phase witness

2026-10-30;SPEC-0084,SRC-0553/0554. Original optional encoding captures the earlier decoder phases before a sample or queue overflow can overwrite them. Version3 strict reader refuses legacy/contradictory data instead of guessing missing PB bits. No persistent fields or hardware operations added;default0 remains. Native proof/trial pending.

[Canonical inputs,actual results,worker corrections and limitations](../../docs/development/mac-phase-witness-evidence.json).
