/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
OUTPUT_FORMAT("elf32-m68k")
OUTPUT_ARCH(m68k)
ENTRY(cs_mac_reset)
MEMORY { ROM (rx) : ORIGIN = 0x400000, LENGTH = 64K
         RAM (rw) : ORIGIN = 0xB00, LENGTH = 0x1500 }
SECTIONS {
    .vectors : { KEEP(*(.vectors)) } > ROM
    .seal : { KEEP(*(.seal)) } > ROM
    .text : { *(.text*) *(.rodata*) . = ALIGN(4); } > ROM
    __rom_code_end = .;
    .data : AT(__rom_code_end) {
        __data_start = .; *(.data*) . = ALIGN(4); __data_end = .;
    } > RAM
    __data_load = LOADADDR(.data);
    .bss (NOLOAD) : {
        __bss_start = .; *(.bss*) *(COMMON) . = ALIGN(4); __bss_end = .;
    } > RAM
    __rom_end = __rom_code_end + SIZEOF(.data);
    /DISCARD/ : { *(.comment) *(.note*) *(.eh_frame*) }
}
ASSERT(SIZEOF(.vectors) == 256, "Reset/vector span differs")
ASSERT(ADDR(.seal) == 0x400100 && SIZEOF(.seal) == 4, "Seal location differs")
ASSERT(__bss_end <= 0x2000, "Own data/BSS exceeds reserved span")
ASSERT(__rom_end <= 0x410000, "Demonstrator ROM exceeds 64 KiB")
