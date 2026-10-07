# Founding workstation inventory

Date: 2026-10-07. This is a sanitized capability snapshot, not an exhaustive
installed-software audit. No credentials, usernames, serials, or private user paths
are published. `Inventory-Windows.ps1` reproduces PATH/common-location probes.

The owner identifies the workstation as Windows 11. Built-in version probes
reported `10.0.26300.0`, display version `26H2`, x64 process/architecture, and
registry product label `Windows 10 Home`. Record these observations without
treating the legacy registry label as proof of a different marketed OS. Hardware
CPU model/RAM details were not obtained: CIM access was unavailable in the task
environment and was not required for bootstrap.

| Capability before setup | Observation | Interpretation |
| --- | --- | --- |
| Git | 2.53.0.windows.3; supplied through task runtime PATH | Usable here; an ordinary developer still needs Git installed |
| GitHub CLI | 2.97.0, authenticated GitHub access verified | Used for repository administration; not a build prerequisite |
| Windows PowerShell | 5.1.26100.9549 | Built-in bootstrap runtime verified |
| PowerShell 7 | 7.6.5 through task runtime | Available here; not required by setup |
| WinGet | Launcher found | No package installed and no readiness claim needed |
| Node.js | Task-runtime executable found | Not used or required |
| Python/python3 | WindowsApps launch aliases | No usable interpreter established |
| WSL | Launcher found | No distribution/toolchain established |
| MSVC / GCC / Clang | Not found on PATH; no checked common VS/LLVM/MSYS2 install | Provision a compiler explicitly; absence outside checked paths is not proved |
| CMake / Ninja / Make | Not found on PATH; common CMake location absent | Provision CMake/Ninja explicitly |
| Rust / Cargo | Not found on PATH | No dependency assumed |
| QEMU / NASM / m68k compiler / Docker | Not found on PATH | Deferred, not installed by bootstrap |

GitHub repository creation and authenticated publication succeeded after permitted
network access. No token or secret is stored in the repository. Commit identity is
configured locally using the owner's public GitHub no-reply convention; global Git
settings were not changed.

## After explicit portable setup

Verified package preparation: LLVM-MinGW 20260908 / Clang 23.1.1, CMake 4.4.4, and
Ninja 1.13.2, all from the pinned publisher archives with matching SHA-256 digests.
They reside only in ignored local tool directories. The machine PATH/registry was
not modified. LLDB executable presence is observed; its interactive workflow has
not been tested. Native build/run evidence is recorded separately.
