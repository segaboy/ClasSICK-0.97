# SPEC-0010 — ACPI PM timer discovery, counter extension and native probe

Status: finalized v1, 2026-10-08; implementation-lead self-review, human provenance
review pending. Owner SRC-0045; ACPI 6.6 tables SRC-0046 and PM timer SRC-0036;
UEFI GUID layout SRC-0038; internal SPEC-0005–0009. ADR-0014 / IMPL-0014 / TEST-0016.
Original native-PC platform contract. No historical Macintosh behavior is involved.

## Pre-exit RSDP capture

`cs_uefi_acpi20_rsdp(count, table)` scans at most 256 live 24-byte configuration
entries (16-byte GUID in UEFI binary layout, then a 64-bit pointer). It returns the
pointer of the first entry whose GUID is `8868e871-e4f1-11d3-bc22-0080c73c8881`
(ACPI 2.0 or later), or zero. A null table or a count above 256 reads nothing and
returns zero. ACPI 1.0-only systems (RSDT without XSDT) are unsupported in v1.
The loader captures this value after the table checks and before allocation. It
stores it in a new final `rsdp` handoff field, as a plain value: no
configuration-table pointer crosses the exit boundary, and the load never fails
because ACPI is absent.

## Bounded table walk

`cs_acpi_find_pm_timer(read, context, rsdp, out)` reads physical bytes only
through the caller's reader, which returns nonzero after copying exactly the
requested bytes. Fields are explicit little-endian offsets from ACPI 6.6.

1. RSDP: 36 bytes; signature `RSD PTR `; bytes 0–19 sum to zero; revision ≥ 2;
   length 36–1024; all `length` bytes sum to zero; XSDT address at offset 24.
2. XSDT: signature `XSDT`; length 36–65536, with `(length-36)` divisible by 8 and
   at most 256 entries; whole-table checksum. Each nonzero 64-bit entry has its
   36-byte header read; a zero entry is FORMAT.
3. Exactly one `FACP` entry is accepted (none: NOT_FOUND, two or more: DUPLICATE).
   FADT length 116–65536 with a whole-table checksum. At most 220 FADT bytes are
   then copied for field access.
4. Flags at offset 112: HW_REDUCED_ACPI (bit 20) means UNSUPPORTED; TMR_VAL_EXT
   (bit 8) selects a 32-bit counter, otherwise 24-bit.
5. If the FADT is at least 220 bytes long, the X_PM_TMR_BLK (offset 208) is used
   when it is usable: System I/O space (1), bit width 32, bit offset 0, access size
   0 or 3, and a nonzero port ≤ 0xFFFC. Otherwise the legacy PM_TMR_BLK (offset 76)
   is used when it is nonzero and ≤ 0xFFFC, with PM_TMR_LEN (offset 91) equal to 4.
   Memory-space timers and other layouts are UNSUPPORTED in v1.

Checksums are summed in 64-byte chunks with no large stack buffer. At most
`36+8*256` XSDT bytes and 65,536 bytes per table are read. Results: OK 0, ARGUMENT
1, ACCESS 2 (the reader refused), SIGNATURE 3, CHECKSUM 4, FORMAT 5, NOT_FOUND 6,
DUPLICATE 7, UNSUPPORTED 8. `out` (FADT address, port, 24/32 bits, legacy=1 or
extended=2 source) is written only on success. Tables other than the FADT are
identified by signature only; their contents are not validated.

## Counter extension and time

`cs_pmtimer_init(timer, bits, raw)` accepts 24 or 32 bits and a raw value within
the mask, then starts at zero ticks. `cs_pmtimer_sample` adds
`(raw - last) mod 2^bits` and reports that delta. Bits above the width are VALUE;
a corrupted mask or last value is ARGUMENT; `UINT64_MAX` accumulated ticks would
be OVERFLOW. On every error the state is unchanged. One sample per wrap period
(about 4.69 s at 24 bits, about 1,199.9 s at 32 bits) is a caller obligation:
multiple wraps between samples are undetectable.

`cs_pmtimer_time(ticks, out)` converts at 3,579,545 Hz into SPEC-0005 time:
`seconds = floor(ticks/3579545)` and
`nanoseconds = floor((ticks mod 3579545) * 10^9 / 3579545)`. It uses binary long
division, so no 64-bit division helper is needed on 32-bit hosts. Seconds above
`UINT32_MAX` give OVERFLOW and leave `out` unchanged.

## Native reader and probe

`cs_native_read` copies bytes only when the range is nonempty, does not overflow,
and lies inside the configured identity window. It must also lie entirely inside
one memory-map descriptor of type 0 (reserved), 6 (runtime data), 9 (ACPI
reclaim) or 10 (ACPI NVS). Conventional, loader, boot-service, MMIO and unknown
ranges are refused. Reads are volatile byte loads; nothing is ever written.

`cs_native_timer_probe` runs after SPEC-0009 presentation in the native stop path,
behind the same successful-exit and ready-descriptor gates. It validates the
captured memory map, walks ACPI from the handoff RSDP, and reads the port through
the supplied 32-bit port reader (`cs_x64_inl`: `mov edx,ecx; in eax,dx; ret`).
It polls until at least 3,580 ticks (about 1 ms) have accumulated, or stops after
20,000,000 reads. It writes a 48-byte little-endian record at trace offset 32:
magic `0x31545043` at 0, version 1 at 4, result at 8, ACPI result at 12, source at
16, port at 20, bits at 24, reads at 28, ticks (u64) at 32, then elapsed seconds
and nanoseconds at 40 and 44. Results: OK 0, ARGUMENT 1, NOT_EXITED 2,
NOT_READY 3, NO_RSDP 4, ACPI 5 (map or table failure), UNSUPPORTED 6, VALUE 7,
STALLED 8. A null trace returns 1 and writes nothing. Gate failures read no port
and no table.

## Acceptance and limitations

Independent hosted tests build original synthetic ACPI images from the published
field tables. They cover every result, precedence, bounds, reader refusal at each
stage, read-volume limits, extended/legacy selection and fallback. They also cover
wrap extension, conversion against a native-division oracle, the map-type reader
rules, and probe success, stall, value and failure traces. Both compilers, actual
i686 where portable, sanitizers and import audits apply. The EFI image links the
new objects, audits the exact port-read bytes and rejects an omitted ACPI object.

No firmware, VM or port access has executed. Real table placement, chipset read
quirks, the counter rate and accuracy, behavior while the VM is paused, the
60-second loop, keyboard, diagnostics, media, firmware eligibility and B2 remain
unverified. Mac/mini vMac and the physical-edition gates are unchanged.
