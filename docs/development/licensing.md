# Licensing decision brief

Status: **owner decision pending**. No project license file or license notice is
adopted by this document. This brief compares published license terms; it does not
certify the legal sufficiency of a clean-room process.

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

Sources: SRC-0008 through SRC-0012 and SRC-0015. These are options, not adopted
terms. All listed licenses allow commercial use; none is an Apple permission grant
or a remedy for prohibited source/asset derivation. The license cannot cover rights
the project does not own. No license gives users rights to redistribute proprietary
applications merely because the system can execute them.

## Questions for the owner's decision

Choose whether to prioritize broad permissive adoption, minimal terms, file-level
improvement sharing, or stronger derivative reciprocity. Decide whether explicit
patent terms are important. Specify `only` versus `or-later` if choosing GPL.
Do not impose a noncommercial restriction while claiming a standard open-source
license. A multi-license or dual-license policy adds ownership/inbound complexity
and is not needed to bootstrap.

After approval, add the exact standard license and accurate copyright notices,
update README/contribution terms and ADR-0008, record the approval date, and review
inbound contributions/dependencies against it. Do not require copyright assignment
or a broad CLA without a separate owner decision. Documentation/assets can share
the project license or use distinct approved terms; specify that scope rather than
silently adding another license.

Until then, defer external implementation/asset merging. The owner's expressly
requested bootstrap proceeds without pretending public visibility is an open-source
grant. Tools keep their own notices outside the project license decision; future
shipped runtimes/helpers/assets require an auditable dependency license inventory.
