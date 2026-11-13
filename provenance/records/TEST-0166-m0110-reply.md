# TEST-0166 - Experimental M0110 inquiry-reply parser

2026-11-13; SRC-0606 / SPEC-0099 / IMPL-0119. The original test checks all
256 reply bytes, four independently observed transition replies and unchanged
outputs on refusals. Clang CTest, Clang ASan/UBSan and separate GCC tests
passed; 68000 compilation and twin pinned 16-bit BootHush diagnostic runs
passed. Two wrong-expectation controls reached their intended failure markers.
The parser has no native keyboard binding or physical-input qualification.
[Evidence](../../docs/development/tilt-keyboard-intake-20261113.md).
