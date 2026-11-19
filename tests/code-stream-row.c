/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../personality/system1/code-stream-row.h"
#include "../core/storage/startup-files.h"
#include <stdio.h>
#include <string.h>

/* Only scripts/code_stream_fixture.py's default independently authored geometry. */
static uint8_t disk[409600];
static cs_startup_workspace startup_work;
static cs_startup_files files;
static cs_cstream_workspace work;
static cs_cstream_manifest manifest;
static unsigned checks;
#define CHECK(x) do { ++checks; if (!(x)) { fprintf(stderr, "row check failed at %d\n", __LINE__); return 1; } } while (0)

static cs_block_io_result block_read(void *ctx, uint32_t block, uint8_t *out)
{
    (void)ctx;
    if (block >= 800) return CS_BLOCK_IO_ERROR;
    memcpy(out, disk + block * 512u, 512);
    return CS_BLOCK_IO_OK;
}

typedef struct {
    cs_rfs_source backing;
    uint32_t first_row, last_read;
    uint16_t bytes;
    unsigned calls, translate, fail, corrupt_id;
} fixture_reader;

static cs_rfs_io read_fixture(void *ctx, uint32_t at, uint16_t count, uint8_t *out)
{
    fixture_reader *reader = ctx;
    cs_rfs_io result;
    unsigned i;
    reader->last_read = at;
    reader->bytes = count;
    ++reader->calls;
    if (reader->fail) return CS_RFS_IO_UNAVAILABLE;
    result = reader->backing.read(reader->backing.context, at, count, out);
    if (result != CS_RFS_IO_OK) return result;
    /* Only the low offset byte in each known synthetic row changes. */
    for (i = 0; reader->translate && i < count; ++i) {
        uint32_t pos = at + i;
        if (pos >= reader->first_row && pos - reader->first_row < 180u * 8u &&
            (pos - reader->first_row) % 8u == 1u)
            out[i] = (uint8_t)(out[i] - 4u);
    }
    if (reader->corrupt_id && count == 8) out[5] = 0;
    return CS_RFS_IO_OK;
}

int main(int argc, char **argv)
{
    FILE *input;
    cs_block_device device = {800, block_read, NULL};
    cs_startup_report opened;
    cs_cstream_report report;
    cs_cstream_request request = {0};
    cs_rfs_view source;
    fixture_reader reader = {0};
    cs_cstream_row row, saved;
    uint32_t slot;
    unsigned base;
    CHECK(argc == 2);
    input = fopen(argv[1], "rb");
    CHECK(input != NULL);
    CHECK(fread(disk, 1, sizeof disk, input) == sizeof disk && fgetc(input) == EOF);
    CHECK(fclose(input) == 0);
    CHECK(cs_startup_files_open(&device, &startup_work, &files, &opened) == CS_STARTUP_OK);
    source = files.resources[CS_STARTUP_SHELL];
    reader.backing = source.source;
    reader.first_row = 65265u + 16u;
    source.source.read = read_fixture;
    source.source.context = &reader;
    request.source = &source;
    request.maximum_source = 409600;
    request.maximum_entries = 180;
    request.maximum_segments = 20;
    request.context = CS_CSTREAM_ORDINARY;
    for (base = 0; base <= 1; ++base) {
        reader.translate = base;
        request.routine_base = base ? CS_CSTREAM_AFTER_HEADER : CS_CSTREAM_RESOURCE_START;
        CHECK(cs_code_stream_inspect(&request, &work, sizeof work, &manifest, &report) == CS_CSTREAM_OK);
        memset(&work, 0xbb, sizeof work);
        for (slot = 0; slot < 180; ++slot) {
            reader.calls = 0;
            CHECK(cs_code_stream_resolve_row(&manifest, slot, &row) == CS_CSTREAM_OK);
            CHECK(reader.calls == 1 && reader.last_read == reader.first_row + slot * 8 && reader.bytes == 8);
            CHECK(row.segment_index == slot / 9 && row.resource_offset == 4 + 2 * ((slot % 9) % 8));
        }
        saved = row;
        reader.calls = 0;
        CHECK(cs_code_stream_resolve_row(&manifest, UINT32_MAX, &row) == CS_CSTREAM_ARGUMENT);
        CHECK(cs_code_stream_resolve_row(&manifest, 180, &row) == CS_CSTREAM_ARGUMENT);
        CHECK(cs_code_stream_resolve_row(&manifest, 0, (cs_cstream_row *)&manifest) == CS_CSTREAM_ARGUMENT);
        CHECK(reader.calls == 0);
        reader.fail = 1;
        CHECK(cs_code_stream_resolve_row(&manifest, 0, &row) == CS_CSTREAM_SOURCE);
        reader.fail = 0;
        reader.corrupt_id = 1;
        CHECK(cs_code_stream_resolve_row(&manifest, 0, &row) == CS_CSTREAM_ENTRY_FORM);
        reader.corrupt_id = 0;
        CHECK(memcmp(&saved, &row, sizeof row) == 0);
        CHECK(cs_code_stream_resolve_row(&manifest, 0, &row) == CS_CSTREAM_OK);
    }
    printf("Code stream row: %u checks passed\n", checks);
    return 0;
}
