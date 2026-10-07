# Clean-room policy

Status: **binding project policy**, established 2026-10-07 before implementation.

## Compatibility boundary

ClasSICK 0.97 reproduces documented and externally observable behavior. It must
contain zero original Apple executable code, zero Apple ROM code, and zero code
derived from prohibited source, disassembly, or decompilation. Original Apple
algorithms and data structures are not a reconstruction target.

## Eligible knowledge

- Historically published programming documentation and public API specifications.
- Public hardware specifications and legitimately published technical manuals.
- Publicly documented file formats and interfaces.
- Independently authored behavioral specifications and tests.
- Authorized black-box observations on legitimately possessed reference systems.

Eligibility is assessed at the **specific source and section**, not merely by
hosting domain or public availability. Record author, title, edition/date,
section/page, canonical URL, access date, purpose, and rights/provenance notes.
Link to copyrighted reference manuals rather than republishing them. Do not use
sample code from a manual as replacement implementation without separate rights
and derivation review; prefer prose contracts and independent algorithms.

## Ineligible implementation inputs

- Apple source, leaked source, or reconstructed Apple source.
- Apple ROM, System, Finder, or other Apple executable code.
- Disassembly, decompiled output, translations, or structural reconstructions of
  Apple machine code, whether hosted publicly or privately.
- Third-party implementations of uncertain origin or incorporating excluded code.
- AI prompts/output generated from any of the above prohibited implementation inputs.
- Copied Apple fonts, icons, sounds, cursors, or other copyrighted assets.

Do not fetch or study disassembly/leaked-source repositories for implementation.
Public availability does not make material eligible. Stop using uncertain material
as an implementation source and record only a sanitized eligibility concern.

## Specification and implementation work

Reference/specification work describes inputs, outputs, state transitions, errors,
data formats, timing if observable, and side effects. It does not describe Apple's
internal instruction sequence, implementation structures, or inferred algorithms.

Implementation work begins from reviewed specifications and approved interfaces.
Its algorithms, data structures, allocation, rendering, and scheduling are original.
Link each subsystem to source IDs, spec IDs, experiments, and tests. Conceptual
separation and an exposure log are required even with one contributor. This is a
documented project process, not a claim of an independently staffed or legally
certified two-team clean-room procedure.

## Private reference handling

Legitimately possessed reference systems and applications may be used for authorized
black-box testing. Possession alone does not grant redistribution or every form of
use. Keep them in a separate directory outside the checkout, tool cache, and public
CI. Never stage, upload, or temporarily commit them. Public experiments contain
original probe source and safe specifications/results, not protected binaries or
copied reference pixels/assets. Public graphics goldens must be independently made.

The owner-designated `C:\ClasSICK` workspace is for generated local builds/tests.
It is **not automatically a rights-cleared reference library**. A private reference
location, lawful access, and observation/export permissions must be established
before any reference-system experiment.

## Publication controls

Before publication review staged content, new history, source eligibility, and
fixture rights. `.gitignore` is an accident barrier, not permission to store
prohibited content in the checkout. The repository guard checks common binary,
secret, and payload mistakes; it cannot prove originality or detect every secret.
PRs require provenance disclosure and maintainer review. Public CI uses only
independently authored, rights-cleared inputs and least-privilege credentials.

## Contamination response

Stop affected work, do not reuse or redistribute the material, and notify the
owner privately with a sanitized description. Record affected paths/commits and
exposure scope without reproducing prohibited content. The owner determines
removal, hosting support, credential rotation if relevant, and whether an unexposed
implementer must restart from an independent specification. Deleting a file in a
later commit is insufficient when prohibited content was already published.

No contribution, task, deadline, or compatibility goal overrides these exclusions.
