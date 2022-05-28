    org 1800h

tone:   equ 05e4h

ringbk:
    ld      a,20h
ring:
    ex      af,af'
    ld      c,211
    ld      hl,8
    call    tone
    ld      c,120
    ld      hl,12
    call    tone
    ex      af,af'
    dec     a
    jr      nz,ring

    ld      bc,5000h
    call    delay
    jr      ringbk

delay:
    ex      (sp),hl
    ex      (sp),hl
    cpi
    ret     po
    jr      delay

    end
