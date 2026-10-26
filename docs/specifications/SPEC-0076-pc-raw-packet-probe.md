# SPEC-0076 - Diagnostic raw PC packet observation

Engineering-only;SRC-0488/0489,IMPL-0084/TEST-0097. No new input policy.
CLASSICK_PC_RAW_PACKET_PROBE defaults0 with numeric0/1 preprocessor/C99 typedef
validation. Default cs_ps2_mouse_packet remains12B and decoder state remains12B.
Flag1 appends three uint32 fields raw_header,raw_x,raw_y (output24B). Every
translation unit must use the same flag;no mixed-layout library linkage.

Only successful PACKET commits raw fields from the actual saved header/X locals
and currentY byte. MORE/DROPPED/ARGUMENT/STATE preserve all output bytes. Existing
framing,signs,buttons,overflow/corruption/reset policy remains. No IO,globals,
retained pointers or packet reconstruction. These are bytes delivered to this
decoder,not proof of physical origin,upstream framing or host-tool correctness.

The motion observer requires MOTION=1 with RAW=1. Its magic isCSPRAW01
(0x4353505241573031),v2 high32=raw_header,low32=(raw_x<<8)|raw_y. All other
mailbox words/stopping rules stay as inSPEC-0073. Mode raw-motion stops on first
nonzero decoded packet;raw-motion-later additionally skips total packet count<=1,
not a guaranteed exact later host gesture. Default CSPMOT01 keeps its existing
post-reset parser-field meaning;it is not silently upgraded to a raw capture.

Native builder/packager require explicit WindowDrawing/WindowInputProbe and
raw-motion or raw-motion-later. Macro1 applies to all C units in those profiles.
Normal images retain macro0. Native compile/source/image/default equality and
guarded original-register observation precede any actual raw-byte claim.
Synthetic {2A,FF,01}->255,-255,buttons2 is an original fixture only. It does not
count as a native observation or settle the GUI dragging blocker.
