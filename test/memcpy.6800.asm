            page 66,264
            cpu 6800
          * = $1000

; memcpy --
; Copy a block of memory from one location to another.
; Called as a subroutine, note return to saved PC addr on exit
; Entry parameters
;      cnt - Number of bytes to copy
;      src - Address of source data block
;      dst - Address of target data block

cnt         dw      $0000       ; sets aside space for memory addr
src         dw      $0000       ; sets aside space for memory addr
dst         dw      $0000       ; sets aside space for memory addr

memcpy      ldab    cnt+1       ;Set B = cnt.L
            beq     check       ;If cnt.L=0, goto check
loop        ldx     src         ;Set IX = src
            ldaa    ,x          ;Load A from (src)
            inx                 ;Set src = src+1
            stx     src
            ldx     dst         ;Set IX = dst
            staa    ,x          ;Store A to (dst)
            inx                 ;Set dst = dst+1
            stx     dst
            decb                ;Decr B
            bne     loop        ;Repeat the loop
            stab    cnt+1       ;Set cnt.L = 0
check       tst     cnt+0       ;If cnt.H=0,
            beq     done        ;Then quit
            dec     cnt+0       ;Decr cnt.H
            ; loop back and do 256*(cnt.H+1) more copies (B=0) 
            bra     loop        ;Repeat the loop
done        rts                 ;Return

