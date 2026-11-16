; SPDX-License-Identifier: GPL-3.0-or-later
; Copyright (C) 2026 Dean Howell.
; Original fixed-profile 286 preview boot. Whole disk, no partition table.
.286
.model tiny
include preview-values.inc
.code
public start_
start_:
    db 0eah
    dw offset normalized
    dw 07c0h
normalized:
    cli
    xor ax,ax
    mov ss,ax
    mov sp,7c00h
    push cs
    pop ds
    sti
    cmp dl,80h
    jne bad_profile
    int 12h
    cmp ax,640
    jb bad_profile
    mov ax,0800h
    mov dx,0080h
    int 13h
    jc bad_profile
    cmp cx,6691h
    jne bad_profile
    cmp dh,3
    jne bad_profile
    mov ax,1000h
    mov es,ax
    xor bx,bx
    mov cx,2
    mov dx,0080h
    mov bp,PAYLOAD_SECTORS
load_sector:
    mov ax,0201h
    call retry_io
    jc bad_read
    add bx,512
    inc cl
    cmp cl,18
    jb next_sector
    mov cl,1
    inc dh
next_sector:
    dec bp
    jnz load_sector
    push ds
    mov ax,1000h
    mov ds,ax
    xor si,si
    mov cx,PAYLOAD_BYTES
    mov bx,0ffffh
    cld
crc_byte:
    lodsb
    xor bh,al
    push cx
    mov cx,8
crc_bit:
    shl bx,1
    jnc crc_next
    xor bx,1021h
crc_next:
    loop crc_bit
    pop cx
    loop crc_byte
    pop ds
    cmp bx,PAYLOAD_CRC
    jne bad_crc
    mov di,PAYLOAD_BYTES
    mov cx,PAYLOAD_BSS_END-PAYLOAD_BYTES
    xor ax,ax
    cld
    rep stosb
    cli
    mov ax,1000h
    mov ds,ax
    mov ss,ax
    mov sp,0fffeh
    sti
    db 0eah
    dw PAYLOAD_ENTRY
    dw 1000h
bad_profile:
    mov al,0e1h
    jmp failure
bad_read:
    mov al,0e2h
    jmp failure
bad_crc:
    mov al,0e3h
failure:
    mov cs:[error_code],al
    and al,0fh
    add al,'0'
    mov cs:[error_digit],al
    push cs
    pop ds
    mov ax,3
    int 10h
    mov si,offset error_text
print_error:
    cld
    lodsb
    or al,al
    jz stopped
    mov ah,0eh
    mov bx,7
    push si
    int 10h
    pop si
    jmp print_error
stopped:
    mov al,cs:[error_code]
    out 80h,al
spin:
    jmp spin

; All requests are reissued from the original registers. The BIOS status
; carry survives register restoration. Reset failure stops retrying.
retry_io:
    push di
    mov di,3
retry:
    call protected_io
    jnc retry_done
    dec di
    jz retry_failed
    push ax
    push dx
    xor ax,ax
    mov dx,0080h
    call protected_io
    pop dx
    pop ax
    jc retry_failed
    jmp retry
retry_failed:
    stc
retry_done:
    pop di
    ret
protected_io:
    pusha
    push ds
    push es
    int 13h
    pop es
    pop ds
    popa
    ret
error_code db 0
error_text db 'ClasSICK 0.97 boot error '
error_digit db '0',13,10,'See TRYING.txt. No payload started.',0
end start_
