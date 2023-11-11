read_disk:  push dx

            mov ah, 0x02
            mov al, dh
            mov ch, 0x00
            mov dh, 0x00
            mov cl, 0x02
            int 13h

            jc disk_error

            pop dx
            cmp dh, al
            jne disk_error
            ret

disk_error: mov al, 0x61
            mov bl, 0x0F
            mov ah, 9
            int 10h
            hlt