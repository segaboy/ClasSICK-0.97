/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright 2026 Dean Howell */

	.text
	.globl	cs_mac_activity_checkpoint
	.globl	cs_mac_activity_spin

cs_mac_activity_checkpoint:
	ori.w	#0x0700,%sr
	movea.l	#cs_mac_activity_trace,%a6
	move.l	#0x4D415231,%d0
	move.l	12(%a6),%d1
	move.l	16(%a6),%d2
	move.l	20(%a6),%d3
	move.l	24(%a6),%d4
	move.l	28(%a6),%d5
	move.l	32(%a6),%d6
	move.l	40(%a6),%d7
	movea.l	44(%a6),%a0
	movea.l	48(%a6),%a1
	movea.l	60(%a6),%a2
	movea.l	64(%a6),%a3
	movea.l	68(%a6),%a4
	movea.l	72(%a6),%a5
	movea.l	76(%a6),%a6
cs_mac_activity_spin:
	bra.s	cs_mac_activity_spin
