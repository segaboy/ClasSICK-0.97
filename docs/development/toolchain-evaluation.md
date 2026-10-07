# Language and build-system evaluation

Status: selected bootstrap direction; revisit with native m68k and bare-metal
evidence before claiming long-term toolchain sufficiency (ADR-0003 and ADR-0004).

## Implementation language

| Choice | Benefits | Costs / gate | Decision |
| --- | --- | --- | --- |
| Conservative freestanding C11 | Explicit low-level layout/lifetimes, established m68k GCC option, native modern CPU toolchains, small runtime surface | Undefined behavior, bounds/lifetime bugs, compiler-generated helpers; require tests/import audits | Primary planning and first core language |
| C++ subset | Stronger type abstractions possible | ABI/runtime/initialization policy and m68k footprint must be proved; exceptions/RTTI not assumed | Defer unless a specific benefit justifies review |
| Rust | Safer ownership abstractions for suitable components | Original 68000 compiler/target/runtime availability, bootstrap reproducibility and size need independent proof | Re-evaluate after target/toolchain proof; not a prerequisite |
| Assembly | Reset/exception entry and special instructions | Architecture lock-in and audit cost | Only minimal architecture-required operations |

Avoid mandatory thread/atomic/TLS/FPU support in portable core. Native C ABI is not
the historical application ABI. Do not globally enable 16-bit `int` to emulate
guest interfaces; marshal explicitly (SRC-0006). C selection can be superseded
through an ADR if measurable safety/portability/size evidence supports a change.

## Windows compiler choices

| Choice | Benefit | Limitation | Decision |
| --- | --- | --- | --- |
| Pinned portable LLVM-MinGW | Self-contained hosted compiler/linker plus Windows headers/imports; unpack setup; native x64 process | Third-party distribution; helper/runtime license review; not an m68k or bare-metal SDK | Select for first hosted probes |
| MSVC Build Tools + Windows SDK | Established Windows integration and debugging | Larger installed environment; separate non-Windows/m68k tools remain necessary | Candidate secondary Windows compiler; not required now |
| Standalone LLVM using MSVC SDK | Clang tooling with Windows SDK integration | LLVM alone does not provide the complete hosted Windows headers/libraries setup | Alternative after explicit SDK provisioning |
| MSYS2 / GCC | Package-managed GNU toolchain | Additional environment/package reproducibility and update control | Candidate later secondary compiler or cross-toolchain host |

No option is assumed preinstalled. The first toolchain does not define OS
architecture. Every native target gets an explicit startup, linker/image, helper
runtime, and execution-evidence plan.

## Build system

Choose CMake with Ninja and CTest: per-target libraries and toolchain files make
dependency/host isolation explicit, shared presets make developer and CI commands
consistent, and tests do not require a third-party test framework at bootstrap.
Upstream documentation supports project/user preset separation (SRC-0003).

Plain PowerShell compiler commands would suffice for one probe but grow fragile
across native targets. Make would add another Windows dependency. MSBuild ties the
primary workflow to one platform. Meson is a possible alternative but introduces
a Python provisioning requirement. Keep scripts as orchestration rather than a
custom dependency engine.

Bootstrap tests demonstrate the chosen Windows flow only. Validate a reviewed
m68k GCC toolchain, x86-64 freestanding link/image generation and ARM64 execution
before freezing cross-target build assumptions. Do not advertise universal targets
based on the compiler executable's presence.
