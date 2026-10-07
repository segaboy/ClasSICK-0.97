# Contributing to ClasSICK 0.97

Read [the clean-room policy](docs/clean-room/POLICY.md) before opening an issue,
PR, experiment, or AI-assisted task. It applies to all public surfaces, including
attachments, commit messages, branch history, and CI artifacts.

## Licensing gate

The owner has not selected a project license. Outside implementation and asset
contributions must not be merged until the owner approves licensing and inbound
terms. The owner-authorized founding documentation and build probes may proceed.
Do not attach copyrighted implementations while asking a licensing question.

## Contribution sequence

1. Identify the scope, relevant ADRs, behavioral specification, and source IDs.
2. For new historical behavior, provide an independently written specification
   first: inputs, outputs, state changes, errors, and observable side effects.
3. Record exact document edition and section or an authorized black-box experiment.
   A modern API manual is not proof of System 1.0 behavior.
4. Write original implementation from the approved specification. Keep platform
   dependencies out of the core. Record any extension separately.
5. Add meaningful behavior and boundary tests using independently authored fixtures.
6. Supply a provenance record and disclosure of third-party material and AI use.
7. Run the repository guard and relevant checks. Review **all staged bytes and
   every new commit**, not just the final diff, before publishing a branch.
8. Request maintainer review. Compatibility-affecting PRs need specification,
   provenance, and test review; architectural changes need an ADR.

## PR expectations

Describe the trigger/problem, observable result, specification/source IDs,
independent design, compatibility profile, test evidence, and remaining gaps.
State explicitly whether any third-party code/assets were used. If AI helped,
record the model/tool, task, permitted inputs, human review, and limitations;
never send prohibited inputs to an AI system. AI output is not provenance proof.

Keep changes small enough to audit. Do not claim compatibility based only on a
successful compile or a similar-looking screen. A maintainer cannot waive the
foundational exclusion of Apple executable code or prohibited derived sources.

## Reporting a problem

Report behavior, inputs that you authored, source citations, and sanitized results.
Never attach ROMs, original System/Finder binaries, proprietary applications,
disassembly, decompiled code, or protected reference disk images. Do not publicly
post credentials or suspected prohibited material; privately contact the owner
through their GitHub profile about the location and nature of the incident,
without forwarding the material.

Commit identity is configured per developer. Never commit tokens, local account
details, or workstation-specific settings. Maintainer review remains necessary:
automated checks reduce accidents but cannot establish clean-room provenance.
