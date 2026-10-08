# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
.text
.global cs_x64_inb
cs_x64_inb:
    movl %ecx,%edx
    xorl %eax,%eax
    inb %dx,%al
    ret
.global cs_x64_outb
cs_x64_outb:
    movl %edx,%eax
    movl %ecx,%edx
    outb %al,%dx
    ret
