# SPEC-0056 - Original Windows hosted window preview

Accepted2026-10-21;SRC-0332..0335;ADR-0064.
Separate classick_window_preview executable;existing surface-demo unchanged.
Shared native C owner,window,cursor,product backdrop and counter remain unchanged.
This adapter is an engineering test/preview interface,not a Macintosh ABI.

Fresh caller-owned cs_win_window state owns a38,400-byte Mono1 MSB640x480 frame,
1,228,800-byte BGRX presentation scratch and48-byte cursor payload. These are
Windows host costs,not a Macintosh128K budget. Original7x7 black plus cursor,
hotspot3,3;fixed window80,70,224x144,title18;product97/inputs0/progress16/phases6/
success1. Initial LEFT snapshot is explicit;held-at-start suppresses until release.
No copy,move or reopen of live session. InvalidNULL/visible/buttons refuses before
initialization. Partial initialization is cleaned up;close is quiescent/idempotent.

Logical512x342 sits at64,69 within640x480. Larger clients center the frame at
floor((client-frame)/2),smaller clients clip from0,0;no scaling. A local saved-DC
wrapper fills outer client white,sets the frame viewport/clipping,then calls the
unchanged legacy presenter with fixed640x480 dimensions. Its legacy scaling and
center-cropping behavior must not be used for this preview. Restore DC state.
WM_PAINT and WM_PRINTCLIENT use this identical path;paints count only success.

Decode signed16-bit client message coordinates;subtract frame origin plus64,69,
then difference against current logical pointer. Inside canvas or own capture,
send exactly one sample to shared owner;captured outside motion clamps there.
Mousemove reads MK_LBUTTON;button messages force the named state. Right/middle
are outside this adapter's scope. An outside uncaptured press cancels without
motion/count;an observed outside release also delivers a zero-motion release so
a later fresh canvas press is accepted. Capture is acquired/verified before an
accepted down sample;release follows successful up under a reentrancy guard.
An intentional release must not trigger cancellation via WM_CAPTURECHANGED.

Focus/capture loss,cancel mode and live resize cancel the gesture;preserve count,
window/pattern and require observed release. Mouseleave cancels only without
capture. Track in-canvas mouse moves;do not stop a captured drag on mouseleave.
Hide the OS cursor in the client;the original logical cursor remains clipped to
the canvas. Keep live0 through window creation/initial show to avoid constructor
size messages inventing cancellation. WM_NCCREATE stores state;register only own
window procedure. WM_DESTROY clears live/window and queues WM_QUIT.

Owner refusal/fault or presentation/capture/tracking failure sets sticky failed,
stops further input and requests close. Never automatically repair,rebegin or
reset accepted input. Close destroys the window with input disabled,retire cursor
while storage lives,then free host buffers. Main opts into per-monitor DPI
awareness,reads initial physical LEFT,shows the window and dispatches its queue;
Escape closes. Native Win32 calls are host adaptation,not historical evidence.

TEST-0067 uses synthetic SendMessage calls against actual hidden windows plus
GDI client paints. Whole-frame oracle predates worker inspection and shares the
previously verified product backdrop;window/cursor pixels are direct reference
writes. Large1280x960/default640x480/small400x300 client images check1:1 mapping,
white margins and clipping. Synthetic results do not qualify host-delivered GUI,
human input,firmware,Mac hardware or release readiness.
[Executed results](../development/hosted-window-preview-evidence.json).


Owner-requested evidence extension:separate original capture observer exports
actual hidden-client paints for stills and drag demonstration. It checks native
frame/GDI equality and logical drag/count at each step. Encoder uses existing
Pillow only outside normal build/CI;every decoded GIF/PNG pixel must equal its
capture. Store2 PNGs and one specifically reviewed49-frame GIF with source pins,
rights,captions,hashes and explicit synthetic limitations. Animation timing is
chosen presentation timing,not a benchmark. General GIFs remain excluded.


Hidden-window test/capture fixtures must request dimensions independently of
monitor tracking-size limits;this changes fixture setup,not product constraints.
A controlled800x600 tracking ceiling reproduces the original CI assertion before
the fixture fix. Retain1280x960 client/pixel checks;never skip or reduce them.


Compiler audit follow-up:five existing Clang EFI-ABI object audits request stable
COFF metadata through the actual supported -Xclang
-mno-incremental-linker-compatible control. Separate twins by1.1s and retain full
byte/hash equality. Do not normalize headers or ignore bytes. Runtime image
source lists/compile inputs and released artifacts remain unchanged.
