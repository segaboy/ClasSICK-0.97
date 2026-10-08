/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_INPUT_H
#define CLASSICK_INPUT_H
#include <stddef.h>
#include <stdint.h>

enum { CS_KEY_SPACE = 1, CS_KEY_ESCAPE = 2 };
enum { CS_KEY_PRESS = 1, CS_KEY_RELEASE = 2 };
typedef struct { uint32_t source, key, action, repeat; } cs_input_event;
typedef struct { cs_input_event event; uint32_t sequence; } cs_input_record;
typedef struct {
    cs_input_record *storage;
    size_t capacity, head, count;
    uint32_t next_sequence;
} cs_input_queue;
typedef enum {
    CS_INPUT_OK = 0, CS_INPUT_ERR_ARGUMENT = 1, CS_INPUT_ERR_STATE = 2,
    CS_INPUT_ERR_EVENT = 3, CS_INPUT_ERR_FULL = 4, CS_INPUT_ERR_EMPTY = 5,
    CS_INPUT_ERR_SEQUENCE = 6, CS_INPUT_ERR_OVERFLOW = 7
} cs_input_result;
/* Ownership, non-aliasing and epoch rules: SPEC-0004. */
cs_input_result cs_input_init(cs_input_queue *queue, cs_input_record *storage, size_t capacity);
cs_input_result cs_input_push(cs_input_queue *queue, const cs_input_event *event);
cs_input_result cs_input_pop(cs_input_queue *queue, cs_input_record *out);
cs_input_result cs_input_reset(cs_input_queue *queue);
#endif
