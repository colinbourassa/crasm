;;; Author: Leon Bottou
;;; Public Domain.


	cpu 6502

; 16 bit calculus functions.
; Two pseudoregisters: ACC, ARG
; Four temporary bytes: TEMP

	dummy
	
      *	 = $20
acc	 ds 2
arg	 ds 2
temp	 ds 4


movi	macro
	lda #(\1) & $ff
	sta \2
	lda #(\1) >> 8
	sta (\2)+1
	endm
	
movm	macro
	lda \1
	sta \2
	lda (\1)+1
	sta (\2)+2
	endm
	
   * = $8000
   
	code
	
add16   lda acc   ; ACC <- ACC + ARG
	clc
	adc arg
	sta acc
	lda acc+1
	adc arg+1
	sta acc+1
	rts

clr16	lda #0    ; clear ACC
	sta acc
	sta acc+1
.1	rts
	
abs16 	bit acc+1 ; absolute value of ACC
	bpl .1
neg16	lda #0    ; negate ACC
	sec
	sbc acc
	sta acc
	lda #0
	sbc acc+1
	sta acc+1
	rts

neg16a  lda #0    ; negate ARG
	sec
	sbc arg
	sta arg
	lda #0
	sbc arg+1
	sta arg+1
	rts
	
sub16   lda acc   ; ACC <- ACC - ARG
	sec
	sbc arg
	sta acc
	lda acc+1
	sbc arg+1
	sta acc+1
	rts

	code

ovfl	bit .ov1
.ov1	rts
	
	code
	
muls16	lda acc+1 ; ACC <- ACC * ARG
	eor arg+1
	sta temp+3

	bit arg+1
	bpl .0
	jsr neg16a
.0	jsr abs16

	lda #$80
	sta temp+1
	lda #0
	sta temp
	sta temp+2
.1	lsr acc+1
	lsr acc
	bcc .2
	pha
	lda temp+2
	clc
	adc arg
	sta temp+2
	pla
	adc arg+1
.2	ror
	ror temp+2
	ror temp+1
	ror temp
	bcc .1
	ora temp+2
	php
	movm temp,acc
	plp
	bne ovfl
	bit acc+1
	bmi ovfl
	bit temp+3
	bpl .3
	jsr neg16
.3	rts

	code
	
divmod  movm acc,temp    ; ACC <- ACC / ARG (unsigned)
	lda #$01         ; TEMP+2 <- ACC % ARG (unsigned)
	sta acc
	asl
	sta temp+2
	sta temp+3
	sta acc+1
	lda arg
	ora arg
	beq ovfl
.1	asl temp
	rol temp+1
	rol temp+2
	rol temp+3
	lda temp+2
	cmp arg
	lda temp+3
	sbc arg+1
	bcc .2
	sta temp+3
	lda temp+2
	sbc arg
	sta temp+2
	sec
.2	rol acc
	rol acc+1
	bcc .1
	rts
	
	code
	
divs16  lda acc+1   ; ACC <- ACC / ARG
	eor arg+1
	pha
	bit arg+1
	bpl .0
	jsr neg16a
.0	jsr abs16
	jsr divmod
	pla
	bpl .1
	jmp neg16
.1	rts

	code

mod16	lda acc+1   ; ACC <- ACC % ARG
	pha
	bit arg+1
	bpl .0
	jsr neg16a
.0	jsr abs16
	jsr divmod
	movm temp+2,acc
	pla
	bpl .1
	jmp neg16
.1	rts
