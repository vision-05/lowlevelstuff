global _start

;figure out why calling doesn't work, but jumping does

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
        mov ah, 2 ;interrupt 10 subfunction 2, set cursor position
        int 10h
        lodsb

        call print_char

        ;mov ah, 9 ;interrupts 10 subfunction 9, write character with colour
        ;int 10h

        inc dl
        cmp al, 0
        jne char

read:
        mov ah, 0
        int 16h
        ;call read_key

        cmp al, 0x08 ;character for backspace
        je back

        cmp al, 0x0D
        je ent

        cmp al, '1'
        je save_file

        ;call buffer
        mov ah, 9
        int 10h
        ;call print_char
        inc dl
        mov ah, 2
        int 10h
        ;call set_cursor

        jmp read

back:   
        push dx
        or dh, dl ;check cursor not at 0,0
        pop dx
        jz read

        cmp dl, 0
        jne back_char
        pop dx;remove last item on stack (row just deleted)
        call set_cursor
        jmp read

back_char:
        dec dl
        call set_cursor

        mov al, 0x20
        call print_char

        jmp buffer_back

ent:  
        push dx
        inc dh
        xor dl, dl
        call set_cursor

        call buffer
        jmp read

save_file:
        call save
        jmp read

buffer: ;parameters: ax is character and scancode
        push bp
        mov bp, sp

        push bx
        push ax
        mov bx, [bp-2] ;base
        and ax, 0x00FF ;0 out scancode so we get correct ascii character
        mov [es:bx+di], ax
        inc di

        pop ax
        pop bx
        cmp al, 0x0d

        mov sp, bp
        pop bp

        ret

buffer_back:
        push bx
        mov bx, [bp-2] ;base
        dec di
        xor ax, ax
        mov [es:bx+di], ax

        pop bx
        jmp read

set_cursor:
        mov ah, 2
        int 10h
        ret

print_char:
        mov ah, 9
        int 10h
        ret

read_key:
        mov ah, 0
        int 16h
        ret

%include "save.asm"

msg: db "Hi!", 0

times 1536 - ($ - $$) db 0 ;takes 3 sectors
;link with following instruction:
;ld -o edit.bin -m elf_i386 -Ttext 0x9000 edit.o --oformat binary
;cat boot.bin edit.bin > image.bin