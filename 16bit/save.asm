save:
    push ax
    push bx
    push cx
    push dx


    mov bx, 0x2000 ;buffer address to bx
    
    mov ah, 0x03
    mov al, 1
    mov ch, 0x00
    mov dh, 0x00
    mov dl, 0x00
    mov cl, 0x05 ;we have sector 1 bootsector, sector 2, 3, 4 edit
    int 13h

    pop dx
    pop cx
    pop bx
    pop ax

    ret