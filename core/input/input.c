/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "input.h"

static int valid(const cs_input_queue *queue)
{
    return queue->capacity <= (size_t)PTRDIFF_MAX / sizeof(cs_input_record)
        && queue->count <= queue->capacity
        && (queue->capacity == 0 ? queue->head == 0
            : queue->storage != NULL && queue->head < queue->capacity);
}

static void copy_event(cs_input_event *out, const cs_input_event *in)
{
    out->source = in->source;
    out->key = in->key;
    out->action = in->action;
    out->repeat = in->repeat;
}

cs_input_result cs_input_init(cs_input_queue *queue, cs_input_record *storage, size_t capacity)
{
    if (queue == NULL) return CS_INPUT_ERR_ARGUMENT;
    if (capacity > (size_t)PTRDIFF_MAX / sizeof(cs_input_record)) return CS_INPUT_ERR_OVERFLOW;
    if (capacity != 0 && storage == NULL) return CS_INPUT_ERR_ARGUMENT;
    queue->storage = storage;
    queue->capacity = capacity;
    queue->head = 0;
    queue->count = 0;
    queue->next_sequence = 1;
    return CS_INPUT_OK;
}

cs_input_result cs_input_push(cs_input_queue *queue, const cs_input_event *event)
{
    size_t tail;
    cs_input_record *slot;
    if (queue == NULL || event == NULL) return CS_INPUT_ERR_ARGUMENT;
    if (!valid(queue)) return CS_INPUT_ERR_STATE;
    if (event->source == 0 || (event->key != CS_KEY_SPACE && event->key != CS_KEY_ESCAPE)
            || (event->action != CS_KEY_PRESS && event->action != CS_KEY_RELEASE)
            || event->repeat > 1 || (event->action == CS_KEY_RELEASE && event->repeat != 0))
        return CS_INPUT_ERR_EVENT;
    if (queue->count == queue->capacity) return CS_INPUT_ERR_FULL;
    if (queue->next_sequence == 0) return CS_INPUT_ERR_SEQUENCE;
    /* Avoid summing two potentially large indexes before wrap. */
    tail = queue->count < queue->capacity - queue->head
        ? queue->head + queue->count : queue->count - (queue->capacity - queue->head);
    slot = &queue->storage[tail];
    copy_event(&slot->event, event);
    slot->sequence = queue->next_sequence;
    queue->next_sequence = queue->next_sequence == UINT32_MAX ? 0 : queue->next_sequence + 1u;
    ++queue->count;
    return CS_INPUT_OK;
}

cs_input_result cs_input_pop(cs_input_queue *queue, cs_input_record *out)
{
    if (queue == NULL || out == NULL) return CS_INPUT_ERR_ARGUMENT;
    if (!valid(queue)) return CS_INPUT_ERR_STATE;
    if (queue->count == 0) return CS_INPUT_ERR_EMPTY;
    copy_event(&out->event, &queue->storage[queue->head].event);
    out->sequence = queue->storage[queue->head].sequence;
    queue->head = queue->head + 1u == queue->capacity ? 0 : queue->head + 1u;
    --queue->count;
    return CS_INPUT_OK;
}

cs_input_result cs_input_reset(cs_input_queue *queue)
{
    if (queue == NULL) return CS_INPUT_ERR_ARGUMENT;
    if (!valid(queue)) return CS_INPUT_ERR_STATE;
    queue->head = 0;
    queue->count = 0;
    queue->next_sequence = 1;
    return CS_INPUT_OK;
}
