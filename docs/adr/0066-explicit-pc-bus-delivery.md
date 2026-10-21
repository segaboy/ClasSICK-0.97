# ADR-0066 - Report events directly from the existing controller poll

Accepted2026-10-21;SPEC-0058;SRC-0338..0340.

Expose one explicit receipt at the existing decode/loss decision. Preserve the
legacy API and state layout through a shared internal poll with optional output;
the public delivery API requires output storage. Cumulative diagnostic counters
cannot establish freshness after saturation. Avoid adding another controller
consumer or decoder. Publish accepted reports and known loss only;never infer
unobserved hardware history. Keep the shared UI owner as the only UI counter.

Use one qualified Copilot primary for the bounded independent transcript and
window-integration tests from frozen public interfaces/fixtures. Codex owns the
small driver change,integration and actual verification. No second worker or
new qualification. The single test attempt reaches360s with transport retries
and no artifact;Codex completes the original tests directly from the frozen
contract/fixtures. No retry or second provider. This does not authorize boot
binding or a target trial.
