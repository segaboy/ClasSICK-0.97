# SPEC-0061 - Firmware-neutral PC window session

2026-10-22;original engineering leaf of SPEC-0059, using SPEC-0055/0057/0058/0060.
Native API: [window-session.h](../../platform/pc/window-session.h).
No UEFI, boot binding, allocation, clock hardware, UART or persistent binary ABI.

The caller supplies fresh/quiescent aligned session storage and a truthful readable
configuration, ordinary writable canvas/payload/row buffers and an independently
mapped device framebuffer. It owns all mappings and synchronous exclusive access;
callbacks cannot reenter, mutate the session graph or invalidate these resources.
The opaque I/O context and callback lifetime remain caller obligations. The config
is copied and need only live through start. All retained objects/buffers remain
live at fixed addresses until explicit quiescence and cursor detach. Terminal
state does not release a live cursor binding. Stop then hide/unbind, or use the
existing explicit cursor abandonment protocol; no automatic repair/rebegin.

Start preflight checks nonnull/numeric nonwrapping aligned fixed session/config
objects before config access; nonnull I/O functions; full nonwrapping declared
buffer/device spans; canvas>=21888, payload>=48 and <=UINT32_MAX; row>=4*width;
target validity through cs_fb_init and width>=512,height>=342. Reject every
overlap between writable spans (session, full declared canvas/payload/row/device)
and each other or the readonly config. No reads of pixel bytes, caller writes or
I/O before complete preflight. All refusals return ARGUMENT and preserve every
caller byte. Numeric validation does not prove native accessibility. Extra canvas,
payload and row capacity remains borrowed, not available for another live object.

After preflight initialize the embedded descriptors, queue of8, original7x7 plus
cursor/hotspot3,3, hidden cursor binding, window{80,70,224,144,18}, and product
{97,0,16,6,1,NULL}. Begin with software buttons0; cancel once. Both must succeed
before one initial full-target publication through cs_pc_mono_present(initial1).
Only after successful publication call cs_ps2_bus_begin. No controller reads or
writes during start. This software0 is not a device observation: cancellation
keeps LEFT suppressed until an actual released packet. No optional pixel buffer.
ram_frames counts1 after successful begin+cancel; published_frames counts1 only
after the complete first publication. Startup failure retains its reached prefix,
sets FAULTED/STARTUP (or PRESENT for presentation failure), raw cause and stage.

Poll requires the magic and RUNNING phase; null/invalid magic returns ARGUMENT
without effects. STOPPED returns OK and FAULTED returns its retained result, both
without effects. Each RUNNING turn first clears delivery to NONE/zero, consumed0
and drained0. clock_ok!=1 or backwards ticks is terminal CLOCK before any I/O or
owner operation. First valid tick establishes the baseline. Equal ticks increase
stalled, with CLOCK on the1000000th consecutive equal tick; advancing ticks clear
stalled. This is a bounded poll-count detector, not elapsed wall-time measurement.

Call cs_ps2_bus_poll_delivery once. Save its result; >READY is terminal CONTROLLER
before consuming delivery. Otherwise pop at most8 keyboard records, stop at EMPTY,
increment drained per successful pop; any other queue result is terminal QUEUE
before mouse dispatch. Space/Escape records are drained without UI or stop effects.
PACKET calls cs_pc_window_packet once; LOST calls cs_window_input_cancel once;
NONE does neither and publishes nothing. Invalid receipt kind is terminal
CONTROLLER/CS_PS2_STATE. Set consumed1 immediately before the owner call. Only OK
or CHANGED is accepted: increment ram_frames then publish(initial0), including
OK0 results. Increment published_frames only for COMPLETE success. All diagnostic
counters saturate at UINT32_MAX; none controls report freshness or dispatch.

Owner refusal is terminal OWNER with its raw cause; no success publication. A
presentation refusal is terminal PRESENT with raw bridge result and its retained
report (including raw row-presenter cause). Fatal clock/controller/queue paths do
not cancel or publish. Earlier RAM/owner/queue/device effects remain; no rollback,
replay, hidden retry, fabricated release or recovery from native/MMIO exceptions.
stage identifies the last attempted operation; cause0 on successful operations.
Self-detected clock failures have cause0; the session result and TIME stage
identify them without inventing an underlying clock API result.
Normal polls set stage POLL, DRAIN, DISPATCH or PUBLISH as applicable; diagnostics
are native inspectable fields, not installed UEFI or UART trace serialization.

Stop changes RUNNING to STOPPED/OK only; repeated STOPPED is inert. FAULTED retains
its cause/result. It never polls, cancels, publishes, resets or shuts hardware down.
This leaf and its native storage sizes do not prove whole-image/128K budget or any
TEST-0070 full boot-binding case. Those cases stay planned until the binding exists.
Local software tests are not hardware/emulator, human-input or historical evidence.
