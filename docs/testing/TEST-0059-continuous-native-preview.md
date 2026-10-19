# TEST-0059 - Continuous native preview session

2026-10-19; SPEC-0050 / IMPL-0053; SRC-0298..0300.
Actual native-loop fixtures cover nominal60/1 seconds,late mouse press/held/
release,Space,Escape,Space after Escape,exact noncounter bytes,padding/guards,
unchanged argument/startup refusals and an injected timer-value fault after65
seconds. The same fixture linked to legacy input must fail,proving it detects
the old behavior. Existing native-keyboard/native-pointer suites retain old
deadline/Escape expectations. Compiler flag refusals,strict/sanitized profiles,
native EFI twins/media and CI/target observations are reported only after actual
execution in the [receipt](../development/preview-session-evidence.json).
No test proposal or synthetic event establishes ordinary human reliability.
