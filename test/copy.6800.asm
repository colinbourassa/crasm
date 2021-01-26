;;; Author: Leon Bottou
;;; Public Domain.

	cpu 6800
	
	* = $8000
	
	begin  = $40
	dest   = $42
	len    = $44
	
	
	ldx  #$4000
	stx  begin
	ldx  #$1430
	stx  len
	ldx  #$6000
	stx  dest
	jsr  copy
	wai
	
	code

	; copy LEN bytes from BEGIN to DEST
	
copy	ldx  begin
	sts  begin
	txs
	ldx  dest
	
	ldab len+1
	ldaa len
	addb dest+1
	adca dest
	stab dest+1
	staa dest
	
.1	cpx dest
	beq .2
	pula
	staa 0,x
	inx
	bra .1

.2	tsx
	lds begin
	stx begin
	clr len
	clr len+1
	rts
	
	
	code
