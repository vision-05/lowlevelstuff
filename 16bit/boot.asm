[org 0x7C00]

xor ax, ax
mov es, ax
mov ss, ax
mov bx, 0x9000
mov bp, 0x1000
mov sp, bp
mov dh, 0x03
call read_disk
call 0x9000
jmp $

%include "disk.asm"
 
times 510 - ($ - $$)  db 0    ;Zerofill up to 510 bytes
 
        dw 0xAA55    