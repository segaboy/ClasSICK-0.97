/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original SPEC-0042/0043/0044 selection owner. */
#ifndef CLASSICK_SYSTEM1_CODE_SELECT_H
#define CLASSICK_SYSTEM1_CODE_SELECT_H
#include "code-run.h"
#include "quickdraw-select.h"
/* Profile prepared-port-owner-v1. cs_image_select_state/_phase/_cause and the
 * image member `select` live in code-image.h (constructor-zeroed scalars). */
#define CS_IMAGE_SELECT_PROFILE UINT32_C(0x50524631)
#define CS_IMAGE_SELECT_DRAW_PROFILE UINT32_C(0x50524632)
#define CS_IMAGE_SELECT_RESERVATION UINT32_C(206)
#define CS_IMAGE_SELECT_BELOW_G UINT32_C(202)
#define CS_IMAGE_SELECT_DEST_BYTES UINT32_C(4)
#define CS_IMAGE_SELECT_PORT_BYTES UINT32_C(1032)
#define CS_IMAGE_SELECT_SET_PORT 0xA873u
#define CS_IMAGE_SELECT_GET_PORT 0xA874u
typedef struct {
    cs_image_run_request run;          /* unchanged legacy request */
    const cs_qd_selection *selection;  /* candidate; g, A5, 1..2 bindings */
} cs_image_select_request;
/* Private guard context; addressed only through guard.context. */
typedef struct {
    cs_image_service *image;
    const cs_image_select_request *request;
    uint32_t root,token;
} cs_image_select_policy;
/* Single caller-owned workspace holding every driver local. Bounded: no
 * full-image, heap or pixel snapshot. capacity >= sizeof(this), bytes.
 * selection/views are owner-made complete control snapshots taken under the
 * exclusive admission; caller contents on entry grant no authority and are
 * overwritten. View pointers and guest windows are root-local borrows.
 * Lifetime: borrowed from entry until proven synchronous quiescence.
 * Resolved safe returns clear this entire workspace; unsafe native ARGUMENT
 * preserves it byte-for-byte. After return the caller may
 * reinitialize/reuse it only if report.borrow_unresolved==0; otherwise it and
 * every capability, view and port must stay live and unchanged. */
typedef struct {
    cs_qd_selection selection;
    cs_qd_view views[CS_QD_SELECT_CAPACITY];
    cs_qd_select_plan plan;
    cs_guest_view seal,cell,parameter,destination,scratch;
    cs_trap_frame guest,next;
    cs_m68k_bus_report step;
    cs_m68k_stack_limits limits;
    cs_m68k_guard guard;
    cs_image_select_policy policy;
    uint32_t word,parameter_value,current,staged_value;
    uint8_t scratch_bytes[6],value_bytes[4],rect_bytes[8];
} cs_image_select_workspace;
/* Reports never contain a native view/port pointer. run.guest is the actual
 * stop frame; first_* are copies of the image's immutable first-stop and
 * first-invariant-fault scalars. Cleanup fields are actual counts/phases. */
typedef struct {
    cs_image_run_report run;
    cs_image_select_cause cause;
    cs_image_select_phase phase;
    uint32_t profile,generation,consumed_generation,g,current;
    uint32_t services,last_word,binding_count;
    uint32_t first_stop_valid,first_stop_status,first_stop_pc,first_stop_a7;
    uint32_t first_fault_valid;
    cs_image_select_cause first_fault_cause;
    cs_image_select_phase first_fault_phase;
    uint32_t first_fault_pc,first_fault_word;
    uint32_t borrow_unresolved,quarantined;
    uint32_t scrubbed_bytes,anchor_bytes,handback_started,handback_result,retired;
    cs_image_report release;   /* original RELEASE report, valid once started */
} cs_image_select_report;
/* One synchronous root of a LOADED one-entry/one-segment image whose truthful
 * CODE0 below-A5 contains the 206-byte reservation [g-202,g+4), 1..2 disjoint
 * 108-byte records and any GetPort destination. Never auto-grows.
 * Admission (one transaction): unchanged SPEC-0034/0035 zero-header world
 * checks; SPEC-0043 table with current=0; cross-disjointness of views, ports,
 * capability, controls, workspace, reports, caller and map backings; pinned
 * (A5), g..g+3, root token and frame; then claim/consume generation, install
 * g, current=0, push token. Nothing after the claim can fail.
 * Only canonical A873/A874 (P=4) are serviced internally; both directory
 * slots must read unsupported at admission and every service. All other
 * A-lines, Paint/Erase, patches, JSR equivalence and GetTrapAddress refuse.
 * Every service re-reads (A5)==g and the current cell, revalidates every
 * record extent and every binding snapshot, then pins scratch S-6..S,
 * parameter S..S+4 (S+4<=root) and owned RX PC+2 before committing scratch
 * SR/PC, then cell (Set; NIL refused) or dest (Get; even, below A5, disjoint),
 * then A7=S+4,PC+2. Bad guest arguments: CPU/policy stop, no effect.
 * Trusted mismatch after admission: quarantine, effects/frame retained.
 * Caller frame is never written. Unsafe native-envelope refusal returns
 * ARGUMENT and preserves every output byte, including report and workspace.
 * Unknown native liveness is a truthful-host precondition, not detected. */
cs_image_run_result cs_image_run_selection(cs_image_service *image,
    const cs_image_select_request *request,cs_trap_frame *caller,
    cs_image_select_workspace *workspace,size_t capacity,
    cs_image_select_report *report);
/* SPEC-0046 opt-in PRF2: selection plus canonical PaintRect/EraseRect through
 * freshly selected native view. All four slots unsupported; owned8-byte Rect
 * copied before helper/scratch writes. Existing PRF1 entry remains unchanged. */
cs_image_run_result cs_image_run_selection_draw(cs_image_service *image,
    const cs_image_select_request *request,cs_trap_frame *caller,
    cs_image_select_workspace *workspace,size_t capacity,
    cs_image_select_report *report);
/* One-way retirement of a selection generation stopped CLEAN or GUEST-FAULT
 * with proven quiescence, no unresolved borrow and no quarantine. Uses only
 * image-owned scalars and current owned map/header integrity, never former
 * views. Current cell may hold any ID. Pins whole below-A5 and header, then
 * zeroes below-A5, then (A5)..A5+3, holds RETIRING/consumed protection and
 * hands back to the unchanged original RELEASE. RETIRED only on its success;
 * any failure quarantines, keeping tombstone, phase counts and reservations.
 * workspace is the existing cs_image_workspace (capacity in bytes). */
cs_image_result cs_image_run_selection_retire(cs_image_service *image,
    const cs_image_run_request *request,cs_image_workspace *workspace,
    size_t capacity,cs_image_description *out,cs_image_select_report *report);
#ifdef CLASSICK_IMAGE_TEST_FAULTS
/* Original trusted conformance schedule, inactive only; after_steps >= 1.
 * kind0 clears; HEADER mutates protected anchor, BINDING mutates owner snapshot,
 * QUIESCENCE leaves driver depth nonzero. No production injection symbol. */
#define CS_IMAGE_SELECT_TEST_HEADER 1u
#define CS_IMAGE_SELECT_TEST_BINDING 2u
#define CS_IMAGE_SELECT_TEST_QUIESCENCE 3u
cs_image_result cs_image_selection_test_fault(cs_image_service *image,
    uint32_t after_steps,uint32_t kind);
#endif
#endif
