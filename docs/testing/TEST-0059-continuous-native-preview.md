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

Executed result: local596 CTests/2 profiles;CI3150/11;2 valid/6 invalid compiler configurations;EFI twins,27 corruption/22 omission controls,matching original disk writers;one successful headless guest boot after GUI preboot failure.38 mouse+5 Space events accepted through guest second185;8 static and2 whole-canvas Mac comparisons pass. Escape injections did not stop later input. Host pointer capture/human reliability unqualified;old VM inventory restored.
