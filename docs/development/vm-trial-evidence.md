# First native-PC VM observations — 2026-10-08

SPEC-0015 / ADR-0019 / IMPL-0019 / TEST-0022; owner SRC-0060. The unchanged
ClasSICK image now starts in an isolated VirtualBox machine, displays our original
scene, completes its 60-second loop and emits readable serial diagnostics. The
owner confirmed two live Space presses and visible scene changes in cold-04.
This is the first observed native execution of our image. **Formal B2 remains
partial**, for the specific qualification gaps below; no OS edition is certified.

The [sanitized manifest](vm-trial-evidence.json) retains own serial lines, host
observations, capture hashes, declared settings, command argument sequences and
failures. Original-scene PNGs and complete management logs remain local; the
public repository's text-only fixture policy is unchanged. No firmware pixels,
pre-banner serial bytes, binaries, NVRAM contents or owner VM inventory are
exported. This is our VM output, not a historical Macintosh reference observation.

## Identity and admission

| Item | Recorded identity/result |
| --- | --- |
| Checkout before trial | `052260b264c3b56ada60de1c63e697e6728d0b2f` |
| Compiled packaging/test source | `78b00e51c401d1ee967618d7d2df316912cf917d` |
| Native EFI source | `3b44cb9510a400397a33cd1cc0802f45c36c8110`; native code unchanged |
| Payload | 37,888 bytes; SHA-256 `8e25d6947677b538d84bc17f98ff44d056dcd3d7769d1389676072286ad831f2` |
| Raw GPT/FAT32 disk | 67,108,864 bytes; SHA-256 `370b7d4e7fc4200b77c367e09afa0c4944f534c2623d7f5f68db159c1da46078` |
| Platform | Oracle VirtualBox `7.2.16r174877`, EFI64, owner-cleared black-box use only |
| Preparation | Fixed VDI conversion and conversion back to RAW reproduce the original raw hash |
| Configuration | 46 public settings checked before launch; one CPU/512 MiB, PIIX3/ACPI, VBoxVGA/16 MiB, PS/2, UART1 file, one SATA disk, isolation/integration restrictions per SPEC-0015 |
| Fresh Windows verification | 846 CTest checks across nine configurations, x64 sanitizers and existing audits; 64 comparison labels/58 distinct fingerprints; all 59 prior UART labels unchanged |
| Media controls | All nine writer images identical; independent checker, 30 corruption controls and three tool refusals pass |

The executable and installed producer-container hashes are recorded in the
manifest and SRC-0035. **The producer-container fingerprint is not a firmware
image fingerprint.** The latter is unavailable: SRC-0059 forbids extraction and
the installed image/source correspondence remains uncertified. No claim bridges
that gap. Public CLI/manual contracts (SRC-0061) are the only external management
inputs; no implementation bodies were read.

## Retained cold starts

| Run | Input | Observed termination |
| --- | --- | --- |
| cold-01, headless | One synthetic Space scan-code sequence | Guest seconds `3C` (60), Space count 1, `result=00000000 kbd=00000001` |
| cold-02, headless | None | Guest seconds 60, Space count 0, success; timed initial/intermediate/final original-scene captures |
| cold-03, GUI | No received input; owner could not see window | Guest seconds 60, Space count 0, success; does not count as operator acceptance |
| cold-04, GUI brought forward | Two owner-operated Space presses; no generated keys | Guest seconds 60, Space count 2, success; owner: “I tapped Space twice and saw the scene change” |
| cold-05, headless | None | Guest seconds 60, success; ready-to-terminal line arrivals separated by 59.972246 host seconds |
| cold-06, headless | None; intended injection did not occur | Guest seconds 60, success; 59.971268 host seconds; retained observer-selection failure |
| cold-07, headless | Synthetic Escape after readiness | Expected early-stop result `0000000C`, keyboard ready; separate from duration acceptance |

Each run began from powered-off state with a fresh serial path, the same admitted
project UUID and only the same SPEC-0014 disk. No saved-state/resume is counted.
All six uninterrupted runs reached guest second 60 and normal completion.

cold-02/03/04 record UTC and monotonic host samples at scheduled 0/10/30/60/90/120
seconds. The 60-second host sample still shows an incomplete bar because VM
startup precedes keyboard READY; later captures show completion. cold-05/06 poll
own serial lines nominally every 100 ms and retain actual previous/current poll
brackets. Arrival differences agree with approximately 60 seconds; they include
serial delivery, buffering, poll overhead and host scheduling, and are not exact
guest timer calibration or a proof of an electrical UART rate. The manifest gives
the measured uncertainty rather than rounding it into an exact host-duration claim.

## Qualification and limits

The own banner/ready/frame/result sequence follows the reviewed SPEC-0013 observer
gate, which requires successful exit, owned descriptor READY, validated map/ACPI/
8042/framebuffer/arena and timer inputs. The existing entry/transition and unloaded
image audits tie that sequence to owned native execution. A post-scene CPU register
query in cold-01 placed RIP in our loaded image; no code or firmware memory was
read. These are scoped control-flow/runtime observations, not independent dumps of
every gate's RAM record or a native exception-delivery test.

The successful startup path accepted controller self-test and negotiated input.
Live Space and synthetic Escape demonstrate the selected driver path. They do
not independently instrument A20/required machine-state preservation across
self-test. Own RAM diagnostic headers were not exported; serial output supplies
the observed bounded diagnostics. Physical UART hardware/clock/baud, other PCs,
USB input and fault delivery remain unqualified.

Formal B2 cannot yet be marked passed against all of its existing evidence
requirements: the exact firmware image fingerprint is unavailable and explicit
self-test/machine-state qualification remains incomplete. Record the successful
native runtime/input observations without weakening these requirements. Future
qualification must use reviewed interfaces and our own outputs within SRC-0059;
no firmware extraction or new driver work is implied by this report.

All three full OS editions remain `not-started`, parity `not-tested`; System 0.97
APIs, Macintosh 128K feasibility, independent replacement ROM/startup, real
Macintosh and mini vMac boots and physical hardware gates remain required.
Human provenance review is pending. CI verifies hosted/unloaded/media checks,
not these local VM or operator results. The observed publication result is
recorded below.

Publication verification: [Windows CI 37858136145](https://github.com/segaboy/ClasSICK-0.97/actions/runs/37858136145)
passed at `b8696da3fb63e3b9f4a2892fb29eed2b40a77055`: source guard, pinned
setup, fresh bootstrap builds and the full hosted/unloaded/media chain. The
chain's nine configuration counts match local 846-check admission. All 64
comparison labels / 58 distinct fingerprints agree with the retained local log;
the raw image and native payload identities are unchanged. This CI did not run
VirtualBox or generate operator input. This follow-up changes records only.
