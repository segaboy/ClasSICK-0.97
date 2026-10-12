# Shared trap-address services

SPEC-0020 / ADR-0023 / IMPL-0024 / TEST-0028. A portable native C directory
implements checked GetTrapAddress/SetTrapAddress behavior under an explicitly
adopted later-manual engineering profile. Scalar guest addresses never become
host pointers. Unknown patch targets request guest execution rather than return
success. Toolbox formats are decoded but their stack ABI is not implemented.

Both original boot demonstrators link the same core and original ten-check
probe. This is a service foundation, not the complete SPEC-0017 Line-A gateway,
an interpreter, guest-readable stubs, or January 1984 compatibility. No runtime
library, Apple code/assets or third-party implementation enters this module.
