/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "input.h"
#include <stdio.h>
#include <string.h>

#define CHECK(x) do { if (!(x)) { fprintf(stderr, "input:%d: %s\n", __LINE__, #x); return 0; } } while (0)
static const cs_input_event a = {7, CS_KEY_SPACE, CS_KEY_PRESS, 0};
static const cs_input_event b = {9, CS_KEY_ESCAPE, CS_KEY_RELEASE, 0};
static int same_event(cs_input_event x, cs_input_event y)
{ return x.source == y.source && x.key == y.key && x.action == y.action && x.repeat == y.repeat; }
static int same_record(cs_input_record x, cs_input_record y)
{ return same_event(x.event, y.event) && x.sequence == y.sequence; }
static int same_queue(cs_input_queue x, cs_input_queue y)
{ return x.storage == y.storage && x.capacity == y.capacity && x.head == y.head
    && x.count == y.count && x.next_sequence == y.next_sequence; }

static int ordering(void)
{
    cs_input_record slots[2], out = {0};
    cs_input_queue q = {0};
    CHECK(cs_input_init(&q, slots, 2) == CS_INPUT_OK);
    CHECK(cs_input_push(&q, &a) == CS_INPUT_OK);
    CHECK(cs_input_push(&q, &b) == CS_INPUT_OK);
    CHECK(cs_input_push(&q, &a) == CS_INPUT_ERR_FULL);
    CHECK(cs_input_pop(&q, &out) == CS_INPUT_OK && same_event(out.event, a) && out.sequence == 1);
    CHECK(cs_input_push(&q, &a) == CS_INPUT_OK);
    CHECK(cs_input_pop(&q, &out) == CS_INPUT_OK && same_event(out.event, b) && out.sequence == 2);
    CHECK(cs_input_pop(&q, &out) == CS_INPUT_OK && same_event(out.event, a) && out.sequence == 3);
    CHECK(cs_input_pop(&q, &out) == CS_INPUT_ERR_EMPTY);
    q.next_sequence = UINT32_MAX;
    CHECK(cs_input_push(&q, &b) == CS_INPUT_OK && q.next_sequence == 0);
    CHECK(cs_input_pop(&q, &out) == CS_INPUT_OK && out.sequence == UINT32_MAX);
    CHECK(cs_input_push(&q, &a) == CS_INPUT_ERR_SEQUENCE);
    CHECK(cs_input_reset(&q) == CS_INPUT_OK);
    CHECK(cs_input_push(&q, &a) == CS_INPUT_OK);
    CHECK(cs_input_pop(&q, &out) == CS_INPUT_OK && out.sequence == 1);
    return 1;
}

static int errors(void)
{
    cs_input_record slots[3], before_slots[3], out = {{31,32,33,34},35}, prior_out = out;
    cs_input_queue q = {0}, before;
    cs_input_event invalids[] = {{0,1,1,0},{1,0,1,0},{1,3,1,0},{1,1,0,0},
        {1,1,3,0},{1,1,1,2},{1,1,2,1},{UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX}};
    memset(slots, 0xA5, sizeof(slots)); memcpy(before_slots, slots, sizeof(slots));
    CHECK(cs_input_init(NULL, slots, 2) == CS_INPUT_ERR_ARGUMENT);
    before = q;
    CHECK(cs_input_init(&q, NULL, (size_t)PTRDIFF_MAX / sizeof(cs_input_record) + 1u)
        == CS_INPUT_ERR_OVERFLOW && same_queue(q, before));
    CHECK(cs_input_init(&q, NULL, 1) == CS_INPUT_ERR_ARGUMENT && same_queue(q, before));
    CHECK(cs_input_init(&q, NULL, 0) == CS_INPUT_OK);
    CHECK(cs_input_push(&q, &a) == CS_INPUT_ERR_FULL);
    CHECK(cs_input_pop(&q, &out) == CS_INPUT_ERR_EMPTY && same_record(out, prior_out));
    CHECK(cs_input_init(&q, slots, 2) == CS_INPUT_OK && memcmp(slots, before_slots, sizeof(slots)) == 0);
    CHECK(cs_input_push(NULL, &a) == CS_INPUT_ERR_ARGUMENT);
    CHECK(cs_input_push(&q, NULL) == CS_INPUT_ERR_ARGUMENT);
    CHECK(cs_input_pop(NULL, &out) == CS_INPUT_ERR_ARGUMENT);
    CHECK(cs_input_pop(&q, NULL) == CS_INPUT_ERR_ARGUMENT);
    CHECK(cs_input_reset(NULL) == CS_INPUT_ERR_ARGUMENT);
    for (size_t i = 0; i < sizeof(invalids)/sizeof(invalids[0]); ++i) {
        before = q;
        CHECK(cs_input_push(&q, &invalids[i]) == CS_INPUT_ERR_EVENT && same_queue(q, before));
        CHECK(memcmp(slots, before_slots, sizeof(slots)) == 0);
    }
    for (unsigned i = 0; i < 5; ++i) {
        CHECK(cs_input_init(&q, slots, 2) == CS_INPUT_OK);
        if (i == 0) q.capacity = (size_t)PTRDIFF_MAX / sizeof(cs_input_record) + 1u;
        if (i == 1) q.head = 2;
        if (i == 2) q.count = 3;
        if (i == 3) q.storage = NULL;
        if (i == 4) { q.capacity = 0; q.head = 1; }
        before = q;
        CHECK(cs_input_push(&q, &invalids[0]) == CS_INPUT_ERR_STATE && same_queue(q, before));
        CHECK(cs_input_pop(&q, &out) == CS_INPUT_ERR_STATE && same_queue(q, before));
        CHECK(cs_input_reset(&q) == CS_INPUT_ERR_STATE && same_queue(q, before));
        CHECK(same_record(out, prior_out) && memcmp(slots, before_slots, sizeof(slots)) == 0);
    }
    CHECK(cs_input_init(&q, slots, 1) == CS_INPUT_OK);
    q.next_sequence = 0;
    before = q;
    CHECK(cs_input_push(&q, &a) == CS_INPUT_ERR_SEQUENCE && same_queue(q, before));
    CHECK(cs_input_push(&q, &invalids[0]) == CS_INPUT_ERR_EVENT && same_queue(q, before));
    q.count = 1;
    CHECK(cs_input_push(&q, &a) == CS_INPUT_ERR_FULL);
    CHECK(memcmp(slots, before_slots, sizeof(slots)) == 0);
    return 1;
}

static int traces(void)
{
    size_t cases = 0;
    /* Independent oracle is a shifting list, with no ring indexing. */
    for (size_t cap = 0; cap <= 4; ++cap) for (uint32_t trace = 0; trace < 65536u; ++trace) {
        cs_input_record backing[6], prior_backing[6], expected[4], out = {0};
        cs_input_queue q = {0};
        size_t count = 0;
        uint32_t sequence = 1, code = trace;
        memset(backing, 0xA5, sizeof(backing));
        CHECK(cs_input_init(&q, &backing[1], cap) == CS_INPUT_OK);
        for (unsigned step = 0; step < 8; ++step) {
            unsigned op = code % 4u;
            cs_input_queue before = q;
            cs_input_record prior_out = out;
            cs_input_result result;
            code /= 4u;
            memcpy(prior_backing, backing, sizeof(backing));
            if (op < 2) {
                const cs_input_event *event = op == 0 ? &a : &b;
                result = cs_input_push(&q, event);
                CHECK(same_event(*event, op == 0 ? a : b));
                if (count == cap) CHECK(result == CS_INPUT_ERR_FULL);
                else {
                    CHECK(result == CS_INPUT_OK);
                    expected[count].event = *event; expected[count].sequence = sequence++;
                    ++count;
                }
            } else if (op == 2) {
                result = cs_input_pop(&q, &out);
                if (count == 0) CHECK(result == CS_INPUT_ERR_EMPTY && same_record(out, prior_out));
                else {
                    CHECK(result == CS_INPUT_OK && same_record(out, expected[0]));
                    --count;
                    for (size_t i = 0; i < count; ++i) expected[i] = expected[i+1];
                }
                CHECK(memcmp(backing, prior_backing, sizeof(backing)) == 0);
            } else {
                result = cs_input_reset(&q);
                CHECK(result == CS_INPUT_OK && q.head == 0);
                count = 0; sequence = 1;
                CHECK(memcmp(backing, prior_backing, sizeof(backing)) == 0);
            }
            CHECK(q.count == count && q.next_sequence == sequence);
            /* Check every pending record before a following reset can hide damage. */
            {
                size_t index = q.head;
                for (size_t i = 0; i < count; ++i) {
                    CHECK(same_record(backing[index+1], expected[i]));
                    ++index;
                    if (index == cap) index = 0;
                }
            }
            if (result != CS_INPUT_OK)
                CHECK(same_queue(q, before) && memcmp(backing, prior_backing, sizeof(backing)) == 0);
            /* No writes outside caller capacity, including unused trailing records. */
            CHECK(memcmp(backing, prior_backing, sizeof(backing[0])) == 0);
            CHECK(memcmp(&backing[cap+1], &prior_backing[cap+1], (5u-cap)*sizeof(backing[0])) == 0);
        }
        for (size_t i = 0; i < count; ++i)
            CHECK(cs_input_pop(&q, &out) == CS_INPUT_OK && same_record(out, expected[i]));
        CHECK(cs_input_pop(&q, &out) == CS_INPUT_ERR_EMPTY);
        ++cases;
    }
    printf("SPEC-0004 independent list oracle: %zu traces, eight operations each\n", cases);
    return 1;
}

static int independent(void)
{
    cs_input_record xs[2], ys[1], out;
    cs_input_queue x = {0}, y = {0};
    cs_input_event repeat = {UINT32_MAX, CS_KEY_SPACE, CS_KEY_PRESS, 1};
    CHECK(cs_input_init(&x, xs, 2) == CS_INPUT_OK && cs_input_init(&y, ys, 1) == CS_INPUT_OK);
    CHECK(cs_input_push(&x, &a) == CS_INPUT_OK && cs_input_push(&y, &repeat) == CS_INPUT_OK);
    CHECK(cs_input_reset(&x) == CS_INPUT_OK);
    CHECK(cs_input_pop(&y, &out) == CS_INPUT_OK && same_event(out.event, repeat) && out.sequence == 1);
    CHECK(cs_input_pop(&x, &out) == CS_INPUT_ERR_EMPTY);
    CHECK(cs_input_push(&y, &b) == CS_INPUT_OK);
    CHECK(cs_input_pop(&y, &out) == CS_INPUT_OK && same_event(out.event, b) && out.sequence == 2);
    return 1;
}
int main(int argc, char **argv)
{
    int passed = 0;
    if (argc != 2) return 2;
    if (strcmp(argv[1], "ordering") == 0) passed = ordering();
    else if (strcmp(argv[1], "errors") == 0) passed = errors();
    else if (strcmp(argv[1], "traces") == 0) passed = traces();
    else if (strcmp(argv[1], "independent") == 0) passed = independent();
    else return 2;
    if (passed) puts("SPEC-0004 keyboard queue: PASS");
    return passed ? 0 : 1;
}
