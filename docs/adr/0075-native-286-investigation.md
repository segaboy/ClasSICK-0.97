# ADR-0075 - Investigate a separate native 286 PC build

2026-10-23;SRC-0375..0382. Accept the owner's research goal:broad PC/AT-compatible
286 boot/run coverage plus unchanged68000 Mac128K applications through our
original interpreter. Investigation accepted;architecture/toolchain/minRAM/ABI
and implementation remain proposals. Keep existing Mac68000/native x64 parity
and immediate paired release work intact. A286 build is separate from the x64
binary;it does not weaken or replace the one64-bit PC payload/distribution goal.

Candidate:standalone real-mode native286 OS services,reviewed BIOS-first boot,
true16-bit compiler,near hot data/far code and checked segmented guest backend.
No DOS/Windows or Apple ROM/System dependency. Guest24-bit addresses remain
logical,not host physical pointers. Prefer measured compiler/portability proof
before a full port. 640KiB profile/576KiB cap are unmeasured;512KiB is a stretch,
not a requirement decision or support claim. Diagnostic boot,unscaled GUI and
named guest-app compatibility are distinct;no fleet percentage or speed claim.

Preserve512x342 unscaled.CGA200-row modes cannot meet that GUI criterion;
bitmap adapter/monitor candidates require later admission/tests. No tool
install,compiler build,fixture,code/header/runtime/boot-input change,target
trial,release or gate expansion. Native external-app ABI stays closed.
[Study](../development/pc286-feasibility.md),[lead/source review](../research/pc286-source-review.md).
