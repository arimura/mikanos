bit64
section .text

global IoOut32
IoOut32:
    mov dx, di
    mov eax, esi
    out dx, eax
    ret

global IoInt32
IoIn32:
    mov dx, di
    in eax, dx
    ret
