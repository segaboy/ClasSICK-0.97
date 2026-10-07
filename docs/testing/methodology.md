# Testing and compatibility methodology

Status: tests exercise SPEC-0001 and SPEC-0002 Windows presentation alongside tooling and
repository controls. No historical System 1 behavior has been observed or validated.

Presentation tests inspect real GDI DIB pixels and a hidden native-window message
path. Original geometric fixtures/captures test our adapter; owner visible review
and historical equivalence remain separate evidence classes.

## Evidence classes

| Class | Input / oracle | Claim permitted |
| --- | --- | --- |
| Unit/core contract | Independently specified inputs and expected outputs | Our contract holds for tested cases |
| Historical API conformance | Approved versioned documentation and original tests | Documented behavior covered by named tests |
| Reference black-box | Authorized exact-version system, original probe, sanitized results | Named observed behavior under stated conditions |
| Format interoperability | Original synthetic disks/forks; private authorized counterpart if needed | Named format operations and error cases |
| Graphics | Original geometric scenes/expected bytes; private reference observations if authorized | Primitive behavior; no blanket visual equivalence |
| Trap/guest runtime | Original assembly/instruction probes and ABI fixtures | Named instruction/trap/callback coverage |
| Portability/parity | Same contracts and trace across adapters/architectures | Tested targets behave consistently for the measured subset |
| Real hardware | Named hardware, own image, safe diagnostics | That native configuration passes the stated gate |

A spec lists input, preconditions, outputs, state transitions, errors, ownership,
observable side effects, timing tolerances if relevant, and source applicability.
Each test has an ID, spec link, independent oracle, target/profile, result, source
revision, and limitations. Compilation, screenshots, and synthetic fixtures are
not reference-system evidence.

## Black-box procedure

Establish lawful access and observation/export boundaries first. Record exact
System/Finder/ROM versions privately where necessary, publish only safe identifiers,
and identify the profile. Run an independently authored probe repeatedly with
controlled inputs. Report results and uncertainty without inspecting/disassembling
implementation. Translate findings into an independent behavioral spec, then tests.
Conflicts between versions stay visible; do not average them into invented behavior.

No private binaries/images/screenshots are uploaded to GitHub or ordinary CI.
Public test fixtures are independently authored and carry rights/provenance records.
Reference-dependent tests are an explicitly opt-in local suite with missing-input
skip reporting, never a false pass. Golden captures of original assets remain
private if allowed at all; safe synthetic expected results are preferred.

## Failure and portability focus

Cover malformed/truncated forks, bad offsets, resource sizes, MFS corruption,
integer overflow, allocation exhaustion, handle lifetime, event-queue overflow,
callback reentrancy, guest alignment/address boundaries, trap variants, and errors.
Begin storage read-only; writes require corruption/crash and round-trip tests.

Fuzz isolated decoders with independently generated corpora. Add sanitizer presets
only after target availability is exercised. Keep compiler/platform imports out of
core and compare big/little-endian byte vectors, 32/64-bit host sizes, unaligned
buffers, unusual screen stride, tiny RAM, and non-floppy block devices. Cross-compile
checks supplement runtime tests; they cannot replace m68k/ARM64 execution evidence.

## Continuous integration

Bootstrap CI runs on Windows with the same pinned setup, source guard, fresh build,
and CTest probes; it never fetches a reference system. Use read-only repository
permissions and no untrusted PR secrets or `pull_request_target` execution. Future
jobs add core contracts, alternate compilers, sanitizers, safe fuzzing, architecture
link/import audits, and named native boot tests as tooling is reviewed.

Test tiers will separate fast per-change contracts from slower fuzz/boot suites.
Compatibility reports distinguish documented, observed, partial, unsupported,
unverified, and intentionally extended behavior. Publish failures and gaps beside
passes. Do not promote a milestone without its acceptance evidence.
