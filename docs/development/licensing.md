# Approved license and decision comparison

Status: **GPL-3.0-or-later explicitly approved by the owner on 2026-10-07** for
the project's own code, documentation and assets. The standard GPL v3 text is in
[LICENSE](../../LICENSE); [COPYRIGHT.md](../../COPYRIGHT.md) states the version-3-or-later
grant and scope. This record compares terms; it does not certify the legal
sufficiency of a clean-room process.

Public hosting alone is not a general permission to reuse or redistribute project
work. GitHub's terms allow viewing/forking on the service; broader rights depend
on a selected license. [GitHub's licensing guidance](https://docs.github.com/en/repositories/managing-your-repositorys-settings-and-features/customizing-your-repository/licensing-a-repository)
supports leaving the decision explicit (SRC-0012).

| Option | Adoption and obligations | Patent / reciprocity implication | Fit to consider |
| --- | --- | --- | --- |
| [BSD-2-Clause](https://opensource.org/license/bsd-2-clause) | Broad reuse, including proprietary distributions, with copyright/license notices | No express patent grant in its short text; no source-sharing requirement | Simple permissive systems-software policy |
| [MIT](https://opensource.org/license/mit) | Broad reuse, including proprietary uses, with notice retention | No express patent grant in its short text; no source-sharing requirement | Familiar minimal permissive alternative |
| [Apache-2.0](https://www.apache.org/licenses/LICENSE-2.0) | Permissive reuse with license/notice requirements and change notices | Express contributor patent grant with patent-litigation termination; no general source-sharing requirement | Permissive adoption with explicit patent terms |
| [MPL-2.0](https://www.mozilla.org/MPL/2.0/) | Distributed covered source files/modifications retain MPL availability; larger works can use other terms | File-level reciprocity and contributor patent terms | Sharing improvements to project files while allowing broader integration |
| [GPL-3.0-only or GPL-3.0-or-later](https://www.gnu.org/licenses/gpl-3.0.html) | Distribution of covered combined/modified works requires corresponding source and GPL terms | Strong reciprocity and patent provisions; some product distributions have installation-information obligations | Prioritize continued freedom of covered derivatives; choose version policy explicitly |

Sources: SRC-0008 through SRC-0012 and SRC-0015. These options were compared before
the owner selected GPL-3.0-or-later; the others are not adopted. All listed licenses
allow commercial use; none is an Apple permission grant
or a remedy for prohibited source/asset derivation. The license cannot cover rights
the project does not own. No license gives users rights to redistribute proprietary
applications merely because the system can execute them.

## Recorded owner decision

After reviewing the comparison, the owner selected the offered
**GPL-3.0-or-later** option, including the same scope for code, documentation and
original assets. The selected version policy permits version 3 or later versions;
no noncommercial restriction, dual licensing, copyright assignment, or separate
CLA is introduced.

The standard publisher license text, accurate project notices, source/script SPDX
identifiers, README, matching inbound contribution terms, provenance and ADR-0008
are updated only after that explicit approval. Contributors retain their rights
and offer intentional submissions under the project's approved terms.

Third-party tools keep their own notices. Any future shipped runtimes/helpers/assets
require an auditable license and provenance inventory compatible with distribution.
The clean-room policy remains mandatory even for GPL-compatible contributions.
