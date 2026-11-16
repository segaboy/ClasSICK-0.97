; SPDX-License-Identifier: GPL-3.0-or-later
; Copyright (C) 2026 Dean Howell.
; Original explicit-consent, all-zero-disk preview installer.
.286
.model tiny
include preview-values.inc
.code
public start_
start_:
    push cs
    pop ds
    push cs
    pop es
    cld
    mov ax,3
    int 10h
    mov si,offset greeting
    call print
    int 12h
    cmp ax,640
    jb profile_error
    mov ax,0800h
    mov dx,0080h
    int 13h
    jc profile_error
    cmp cx,6691h
    jne profile_error
    cmp dh,3
    jne profile_error
    mov si,offset warning
    call print
    mov al,0a0h
    out 80h,al
    mov si,offset consent
get_consent:
    xor ah,ah
    push si
    int 16h
    pop si
    cmp al,13
    je consent_enter
    or al,20h
    cmp al,[si]
    jne cancelled
    inc si
    jmp get_consent
consent_enter:
    cmp byte ptr [si],0
    jne cancelled
    ; Check the exact staged prefix before any disk operation can write.
    mov si,BLOB_OFFSET
    mov cx,PREFIX_BYTES
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
    cmp bx,PREFIX_CRC
    jne staged_error
    mov si,offset scanning
    call print
    mov word ptr [cylinder],0
    mov byte ptr [head],0
scan_track:
    call address_track
    mov ax,0211h
    mov bx,SCRATCH_OFFSET
    call retry_io
    jc read_error
    push ds
    pop es
    mov di,SCRATCH_OFFSET
    mov cx,4352
    xor ax,ax
    cld
    repe scasw
    jne occupied
    inc byte ptr [head]
    cmp byte ptr [head],4
    jb scan_track
    mov byte ptr [head],0
    inc word ptr [cylinder]
    cmp word ptr [cylinder],615
    jb scan_track
    mov si,offset installing
    call print
    ; Write and verify payload sectors before making the disk bootable.
    mov word ptr [source_offset],BLOB_OFFSET+512
    mov word ptr [sectors_left],PREFIX_SECTORS-1
    mov byte ptr [sector],2
    mov byte ptr [head],0
install_sector:
    mov ch,0
    mov cl,[sector]
    mov dh,[head]
    mov dl,80h
    mov bx,[source_offset]
    mov ax,0301h
    call retry_io
    jc write_error
    call verify_sector
    jc verify_error
    add word ptr [source_offset],512
    inc byte ptr [sector]
    cmp byte ptr [sector],18
    jb install_next
    mov byte ptr [sector],1
    inc byte ptr [head]
install_next:
    dec word ptr [sectors_left]
    jnz install_sector
    mov cx,1
    mov dx,0080h
    mov bx,BLOB_OFFSET
    mov word ptr [source_offset],BLOB_OFFSET
    mov ax,0301h
    call retry_io
    jc write_error
    call verify_sector
    jc verify_error
    mov si,offset success
    call print
    mov al,0ach
    out 80h,al
stopped:
    jmp stopped
cancelled:
    mov si,offset cancelled_text
    mov al,0adh
    jmp report
profile_error:
    mov si,offset profile_text
    mov al,0e1h
    jmp report
staged_error:
    mov si,offset staged_text
    mov al,0e2h
    jmp report
read_error:
    mov si,offset read_text
    mov al,0e3h
    jmp report
occupied:
    mov si,offset occupied_text
    mov al,0e4h
    jmp report
write_error:
    mov si,offset write_text
    mov al,0e5h
    jmp report
verify_error:
    mov si,offset verify_text
    mov al,0e6h
report:
    push ax
    call print
    pop ax
    out 80h,al
    jmp stopped

address_track:
    mov ax,[cylinder]
    mov ch,al
    mov cl,ah
    shl cl,6
    or cl,1
    mov dh,[head]
    mov dl,80h
    ret
verify_sector:
    mov ax,0201h
    mov bx,SCRATCH_OFFSET
    call retry_io
    jc verify_done
    push ds
    pop es
    mov si,[source_offset]
    mov di,SCRATCH_OFFSET
    mov cx,256
    cld
    repe cmpsw
    jne verify_bad
    clc
verify_done:
    ret
verify_bad:
    stc
    ret
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
print:
    cld
    lodsb
    or al,al
    jz print_done
    pusha
    push ds
    push es
    mov ah,0eh
    mov bx,7
    int 10h
    pop es
    pop ds
    popa
    jmp print
print_done:
    ret
cylinder dw 0
head db 0
sector db 0
source_offset dw 0
sectors_left dw 0
consent db 'install',0
greeting db 'ClasSICK 0.97 - 286 interactive preview installer',13,10,13,10,0
warning db 'Target: FIRST hard disk (80h), 615 cylinders, 4 heads, 17 sectors.',13,10
        db 'The ENTIRE disk must contain zeros. Occupied disks are refused.',13,10
        db 'This preview owns the whole disk and creates no partitions.',13,10
        db 'Type INSTALL then Enter to install. Any other input cancels.',13,10,0
scanning db 13,10,'Checking the entire target disk...',13,10,0
installing db 'Writing and verifying payload. Boot sector is written last...',13,10,0
success db 'Installation verified. Remove the floppy and restart.',13,10,0
cancelled_text db 13,10,'Cancelled. No disk writes.',13,10,0
profile_text db 'Unsupported target. No disk writes.',13,10,0
staged_text db 'Installer data damaged. No disk writes.',13,10,0
read_text db 'Cannot read target. No disk writes.',13,10,0
occupied_text db 'Target is occupied. No disk writes.',13,10,0
write_text db 'Write failed. Installation incomplete. Do not boot this disk.',13,10,0
verify_text db 'Verification failed. Installation incomplete.',13,10,0
end start_
