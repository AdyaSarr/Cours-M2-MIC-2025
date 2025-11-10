.text

sleep_500_ms:
    ; prologue : sauver les registres utilisés
    push  r18
    push  r19
    push  r20

    ; V = 1_599_996 = 0x18 69 FC
    ldi   r20, 0x18
    ldi   r19, 0x69
    ldi   r18, 0xFC

.Lloop:
    subi  r18, 1
    sbci  r19, 0
    sbci  r20, 0
    brne  .Lloop
                

    ; ajustement fin pour tomber pile sur 8_000_000 cycles
    nop
    nop

    ; épilogue : restaurer et sortir
    pop   r20
    pop   r19
    pop   r18
    ret      

