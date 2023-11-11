global _start

_start: 
        mov bx, 0x2000 ;start with string at 2000
        push bx ;base of string (0x2000)
        xor di, di
        push di ;current index (0)
        mov bx, 0x000F
        mov cx, 1
        xor dx, dx
        mov ds, dx
        cld

print:  
        mov si, msg ;si is offset for lodsb

char:   
        ;mov ah, 2 ;interrupt 10 subfunction 2, set cursor position
        ;int 10h

        call set_cursor
        lodsb

        call print_char

        ;mov ah, 9 ;interrupts 10 subfunction 9, write character with colour
        ;int 10h

        inc dl
        cmp al, 0
        jne char

msg: db "Hi!", 0

print_char:
    mov ah, 9
    int 10h
    ret

set_cursor:
    mov ah, 2
    int 10h
    ret

times 512 - ($ - $$) db 0