; SPDX-License-Identifier: GPL-3.0-or-later
; Copyright (C) 2026 Dean Howell.
; Original experimental 16-bit arithmetic helpers for the observed Watcom
; small-model register ABI. Not yet qualified for a distribution.
.286
.model small
.code
public __I4M
public __U4M
public __U4D
public __I4D

; DX:AX * CX:BX, low 32 bits in DX:AX. Preserve SI, DI and BP.
__I4M proc near
__U4M label near
    push si
    push di
    mov si,ax
    mov di,dx
    mul bx
    push ax
    push dx
    mov ax,di
    mul bx
    mov di,ax
    mov ax,si
    mul cx
    add di,ax
    pop dx
    add dx,di
    pop ax
    pop di
    pop si
    ret
__I4M endp

; Unsigned DX:AX / CX:BX -> quotient DX:AX, remainder CX:BX.
; Restoring division, with the extra remainder carry retained before compare.
; Division by zero is outside the C operation's defined domain.
__U4D proc near
    or cx,cx
    jnz u4_wide
    or bx,bx
    jz u4_wide
    ; Two 16-bit divisions handle the common 16-bit-divisor case.
    push si
    mov si,ax
    mov ax,dx
    xor dx,dx
    div bx
    mov cx,ax
    mov ax,si
    div bx
    mov bx,dx
    mov dx,cx
    xor cx,cx
    pop si
    ret
u4_wide:
    push bp
    mov bp,sp
    sub sp,8
    push si
    mov [bp-2],bx
    mov [bp-4],cx
    mov word ptr [bp-6],0
    mov word ptr [bp-8],0
    mov si,32
u4_loop:
    shl ax,1
    rcl dx,1
    rcl word ptr [bp-6],1
    rcl word ptr [bp-8],1
    jc u4_subtract
    mov bx,[bp-8]
    cmp bx,[bp-4]
    ja u4_subtract
    jb u4_next
    mov bx,[bp-6]
    cmp bx,[bp-2]
    jb u4_next
u4_subtract:
    mov bx,[bp-2]
    sub [bp-6],bx
    mov bx,[bp-4]
    sbb [bp-8],bx
    or ax,1
u4_next:
    dec si
    jnz u4_loop
    mov bx,[bp-6]
    mov cx,[bp-8]
    pop si
    mov sp,bp
    pop bp
    ret
__U4D endp

; Signed division truncates toward zero; remainder follows dividend sign.
__I4D proc near
    push si
    push di
    mov si,dx
    mov di,cx
    test dx,8000h
    jz i4_dividend_positive
    neg ax
    adc dx,0
    neg dx
i4_dividend_positive:
    test cx,8000h
    jz i4_divisor_positive
    neg bx
    adc cx,0
    neg cx
i4_divisor_positive:
    call __U4D
    test si,8000h
    jz i4_remainder_positive
    neg bx
    adc cx,0
    neg cx
i4_remainder_positive:
    xor si,di
    test si,8000h
    jz i4_quotient_positive
    neg ax
    adc dx,0
    neg dx
i4_quotient_positive:
    pop di
    pop si
    ret
__I4D endp
end
