# ADR-0027 - Original bounded allocation before full heap services

Accepted 2026-10-12, SPEC-0023 / SRC-0101-0103.

Use a caller-owned side table and fixed guest-visible master-cell bands over the
checked map. These original structures keep allocation bounded and auditable,
preserve stable handles and let guest flag changes affect locking without native
pointer casts. They deliberately differ from the published internal Zone/header/
list structures. Record their RAM cost; host storage is not free on a 128K Mac.

Plan relocation before committing any bytes. Downward moves in source order
avoid a whole-zone temporary buffer; resize keeps its source fixed until the
destination is known. This conservative profile can refuse requests that a
more complete compactor might satisfy. Add purging/growth/callbacks only through
later contracts. Word/long results and register preservation have explicit
per-routine metadata rather than a guessed universal C ABI.

Keep the semantic core and register adapter separate from the existing directory
and CPU. The new routines are verified hosted/standalone first. Native image
integration and any VM run remain separate, owner-approved milestones. The
owner's native-application gate remains closed; own native core/test tools are
permitted. [Interface registry](../interfaces/system1-routines.json) contains
guest facts only, with unknown January1984 introduction explicitly preserved.
