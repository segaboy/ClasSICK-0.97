# Paired preview input check instructions

Current SPEC-0050 continuous outline candidate no longer stops normally at60 seconds or Escape; end it through the host. The dated diagnostic procedure below applies to older images. Capture setup must be separated from the measured click baseline. [Current instructions](../../releases/pc/TRYING-paired-preview-1.md); [verified session](preview-session-evidence.md).

SPEC-0049 / TEST-0058; procedure clarified2026-10-19 after the failed PC human check.
These are instructions for a future explicitly scoped trial,not a new pass or run.
The owner has cleared automated testing without requiring human participation.

For a human check,first identify the running target and record its starting count.
On the Mac,click twice inside the scene and verify two increments with the ring,
rails and other pixels intact. On the PC,click inside VirtualBox and accept its
capture prompt if shown; treat that first click separately as capture setup and
record the count again. Pointer integration is distinct from pointer capture.
Right Ctrl releases the mouse to Windows.

During an explicitly confirmed active interval,hold the left button for two
seconds,release for one second,then repeat. Expect one increment per distinct
hold,none for holding continuously. Tap Space once: expect one more increment.
Record the actual count and every missed input; do not reinterpret many clicks
as a two-click pass. Longer holds are a basic delivery check,not a remedy or proof
of short-click reliability. Ordinary short clicks need a separate controlled check.

The current PC carrier stops consuming input at diagnostic second60 while leaving
the scene visible. A static screenshot cannot establish that input is still active.
The test controller must track serial state and reject post-completion input as
a live-input test. Do not ask the operator to interact with a paused or completed
guest. The2026-10-19 automated matrix accepted21/21 presses and one Space before
completion; after completion,input produced no change. Human PC acceptance still
failed. [Actual observations](paired-preview-boot-trial.md).
