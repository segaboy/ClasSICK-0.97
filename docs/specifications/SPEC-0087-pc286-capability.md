<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0087 - 286 capability and borrowed-span experiment

2026-10-30; SRC-0574 / IMPL-0098 / TEST-0122. Adopt the [capability contract](../development/pc286-span-cap-contract.md) as an isolated model over SPEC-0069 and SPEC-0082, using SPEC-0086's validated binding. A capability holds its registry identity, lifetime, generation and rights; advancing or rebinding invalidates earlier capabilities. A borrowed run is bounded by the request, the bank and the native span, and writable only when allowed. Failed operations follow the contract's refusal order and preservation rules.

The caller must supply truthful, stable and disjoint metadata and actual backing storage; registry and owner storage must not be recycled while a capability can be used. Returned native pointers have the short lifetime in the contract, and never survive a callout, rebind or owner change. These checks do not establish physical ownership or protect against forged native pointers. The four-byte data-pointer guard is required on the 16-bit compiler. This model is not a production provider, guest-memory binding, target boot, application compatibility or performance claim.
