;;; Author: Leon Bottou
;;; Public Domain.

; All Z80 opcodes

	cpu z80	
	
      *=4000H
    adr=12345	
     dd=23H
      n=45Q
     nn=23456

	adc a,(hl)
	adc a,(ix+dd)
	adc a,(iy+dd)
	adc a,a
	adc a,b
	adc a,c
	adc a,d
	adc a,e
	adc a,h
	adc a,l
	adc a,n
	adc hl,bc
	adc hl,de
	adc hl,hl
	adc hl,sp

	add a,(hl)
	add a,(ix+dd)
	add a,(iy+dd)
	add a,a
	add a,b
	add a,c
	add a,d
	add a,e
	add a,h
	add a,l
	add a,n
	add hl,bc
	add hl,de
	add hl,hl
	add hl,sp
	add ix,bc
	add ix,de
	add ix,ix
	add ix,sp
	add iy,bc
	add iy,de
	add iy,iy
	add iy,sp

	and (hl)
	and (ix+dd)
	and (iy+dd)
	and a
	and b
	and c
	and d
	and e
	and h
	and l
	and n

	bit 0,(hl)
	bit 0,(ix+dd)
	bit 0,(iy+dd)
	bit 0,a
	bit 0,b
	bit 0,c
	bit 0,d
	bit 0,e
	bit 0,h
	bit 0,l
	bit 1,(hl)
	bit 1,(ix+dd)
	bit 1,(iy+dd)
	bit 1,a
	bit 1,b
	bit 1,c
	bit 1,d
	bit 1,e
	bit 1,h
	bit 1,l
	bit 2,(hl)
	bit 2,(ix+dd)
	bit 2,(iy+dd)
	bit 2,a
	bit 2,b
	bit 2,c
	bit 2,d
	bit 2,e
	bit 2,h
	bit 2,l
	bit 3,(hl)
	bit 3,(ix+dd)
	bit 3,(iy+dd)
	bit 3,a
	bit 3,b
	bit 3,c
	bit 3,d
	bit 3,e
	bit 3,h
	bit 3,l
	bit 4,(hl)
	bit 4,(ix+dd)
	bit 4,(iy+dd)
	bit 4,a
	bit 4,b
	bit 4,c
	bit 4,d
	bit 4,e
	bit 4,h
	bit 4,l
	bit 5,(hl)
	bit 5,(ix+dd)
	bit 5,(iy+dd)
	bit 5,a
	bit 5,b
	bit 5,c
	bit 5,d
	bit 5,e
	bit 5,h
	bit 5,l
	bit 6,(hl)
	bit 6,(ix+dd)
	bit 6,(iy+dd)
	bit 6,a
	bit 6,b
	bit 6,c
	bit 6,d
	bit 6,e
	bit 6,h
	bit 6,l
	bit 7,(hl)
	bit 7,(ix+dd)
	bit 7,(iy+dd)
	bit 7,a
	bit 7,b
	bit 7,c
	bit 7,d
	bit 7,e
	bit 7,h
	bit 7,l

	call adr
	call c,adr
	call m,adr
	call nc,adr
	call nz,adr
	call p,adr
	call pe,adr
	call po,adr
	call z,adr

	ccf
	
	cp  (hl)
	cp  (ix+dd)
	cp  (iy+dd)
	cp  a
	cp  b
	cp  c
	cp  d
	cp  e
	cp  h
	cp  l
	cp  n
	
	cpd
	cpdr
	cpi
	cpir
	
	cpl
	daa
	
	dec (hl)
	dec (ix+dd)
	dec (iy+dd)
	dec a
	dec b
	dec bc
	dec c
	dec d
	dec de
	dec e
	dec h
	dec hl
	dec ix
	dec iy
	dec l
	dec sp
	
	di
here1	djnz here1
	ei
	
	ex (sp),hl
	ex (sp),ix
	ex (sp),iy
	ex af,af'
	ex de,hl
	
	exx
	halt
	im 0
	im 1
	im 2
	
	in a,(c)
	in a,(n)
	in b,(c)
	in c,(c)
	in d,(c)
	in e,(c)
	in h,(c)
	in l,(c)
	
	inc (hl)
	inc (ix+dd)
	inc (iy+dd)
	inc a
	inc b
	inc bc
	inc c
	inc d
	inc de
	inc e
	inc h
	inc hl
	inc ix
	inc iy
	inc l
	inc sp
	
	ind
	indr
	ini
	inir
	
	jp (hl)
	jp (ix)
	jp (iy)
	jp adr
	jp c,adr
	jp m,adr
	jp nc,adr
	jp nz,adr
	jp p,adr
	jp pe,adr
	jp po,adr
	jp z,adr
	
	jr c,here2
here2	jr here2
	jr nc,here2
	jr nz,here2
	jr z,here2
	
	ld (bc),a
	ld (de),a
	
	ld (hl),a
	ld (hl),b
	ld (hl),c
	ld (hl),d
	ld (hl),e
	ld (hl),h
	ld (hl),l
	ld (hl),n

	ld (ix+dd),a
	ld (ix+dd),b
	ld (ix+dd),c
	ld (ix+dd),d
	ld (ix+dd),e
	ld (ix+dd),h
	ld (ix+dd),l
	ld (ix+dd),n
	ld (iy+dd),a
	ld (iy+dd),b
	ld (iy+dd),c
	ld (iy+dd),d
	ld (iy+dd),e
	ld (iy+dd),h
	ld (iy+dd),l
	ld (iy+dd),n
	
	ld (nn),a
	ld (nn),bc
	ld (nn),de
	ld (nn),hl
	ld (nn),ix
	ld (nn),iy
	ld (nn),sp
	
	ld a,(bc)
	ld a,(de)
	ld a,(hl)
	ld a,(ix+dd)
	ld a,(iy+dd)
	ld a,(nn)
	ld a,a
	ld a,b
	ld a,c
	ld a,d
	ld a,e
	ld a,h
	ld a,i
	ld a,l
	ld a,n
	ld a,r
	
	ld b,(hl)
	ld b,(ix+dd)
	ld b,(iy+dd)
	ld b,a
	ld b,b
	ld b,c
	ld b,d
	ld b,e
	ld b,h
	ld b,l
	ld b,n
	
	ld bc,(nn)
	ld bc,nn
	
	ld c,(hl)
	ld c,(ix+dd)
	ld c,(iy+dd)
	ld c,a
	ld c,b
	ld c,c
	ld c,d
	ld c,e
	ld c,h
	ld c,l
	ld c,n
	
	ld d,(hl)
	ld d,(ix+dd)
	ld d,(iy+dd)
	ld d,a
	ld d,b
	ld d,c
	ld d,d
	ld d,e
	ld d,h
	ld d,l
	ld d,n
	
	ld de,(nn)
	ld de,nn
	
	ld e,(hl)
	ld e,(ix+dd)
	ld e,(iy+dd)
	ld e,a
	ld e,b
	ld e,c
	ld e,d
	ld e,e
	ld e,h
	ld e,l
	ld e,n
	
	ld h,(hl)
	ld h,(ix+dd)
	ld h,(iy+dd)
	ld h,a
	ld h,b
	ld h,c
	ld h,d
	ld h,e
	ld h,h
	ld h,l
	ld h,n
	
	ld hl,(nn)
	ld hl,nn
	
	ld i,a
	
	ld ix,(nn)
	ld ix,nn
	ld iy,(nn)
	ld iy,nn
	
	ld l,(hl)
	ld l,(ix+dd)
	ld l,(iy+dd)
	ld l,a
	ld l,b
	ld l,c
	ld l,d
	ld l,e
	ld l,h
	ld l,l
	ld l,n
	
	ld r,a
	
	ld sp,(nn)
	ld sp,hl
	ld sp,ix
	ld sp,iy
	ld sp,nn
	
	ldd
	lddr
	ldi
	ldir
	
	neg
	nop
	
	or (hl)
	or (ix+dd)
	or (iy+dd)
	or a
	or b
	or c
	or d
	or e
	or h
	or l
	or n
	
	otdr
	otir
	
	out (c),a
	out (c),b
	out (c),c
	out (c),d
	out (c),e
	out (c),h
	out (c),l
	out (n),a
	
	outd
	outi
	
	pop af
	pop bc
	pop de
	pop hl
	pop ix
	pop iy
	
	push af
	push bc
	push de
	push hl
	push ix
	push iy
	
	res 0,(hl)
	res 0,(ix+dd)
	res 0,(iy+dd)
	res 0,a
	res 0,b
	res 0,c
	res 0,d
	res 0,e
	res 0,h
	res 0,l
	res 1,(hl)
	res 1,(ix+dd)
	res 1,(iy+dd)
	res 1,a
	res 1,b
	res 1,c
	res 1,d
	res 1,e
	res 1,h
	res 1,l
	res 2,(hl)
	res 2,(ix+dd)
	res 2,(iy+dd)
	res 2,a
	res 2,b
	res 2,c
	res 2,d
	res 2,e
	res 2,h
	res 2,l
	res 3,(hl)
	res 3,(ix+dd)
	res 3,(iy+dd)
	res 3,a
	res 3,b
	res 3,c
	res 3,d
	res 3,e
	res 3,h
	res 3,l
	res 4,(hl)
	res 4,(ix+dd)
	res 4,(iy+dd)
	res 4,a
	res 4,b
	res 4,c
	res 4,d
	res 4,e
	res 4,h
	res 4,l
	res 5,(hl)
	res 5,(ix+dd)
	res 5,(iy+dd)
	res 5,a
	res 5,b
	res 5,c
	res 5,d
	res 5,e
	res 5,h
	res 5,l
	res 6,(hl)
	res 6,(ix+dd)
	res 6,(iy+dd)
	res 6,a
	res 6,b
	res 6,c
	res 6,d
	res 6,e
	res 6,h
	res 6,l
	res 7,(hl)
	res 7,(ix+dd)
	res 7,(iy+dd)
	res 7,a
	res 7,b
	res 7,c
	res 7,d
	res 7,e
	res 7,h
	res 7,l

	ret
	ret c
	ret m
	ret nc
	ret nz
	ret p
	ret pe
	ret po
	ret z
	reti
	retn
	
	rl (hl)
	rl (ix+dd)
	rl (iy+dd)
	rl a
	rl b
	rl c
	rl d
	rl e
	rl h
	rl l
	
	rla
	
	rlc (hl)
	rlc (ix+dd)
	rlc (iy+dd)
	rlc a
	rlc b
	rlc c
	rlc d
	rlc e
	rlc h
	rlc l
	
	rlca
	rld
	
	rr (hl)
	rr (ix+dd)
	rr (iy+dd)
	rr a
	rr b
	rr c
	rr d
	rr e
	rr h
	rr l
	
	rra
	
	rrc (hl)
	rrc (ix+dd)
	rrc (iy+dd)
	rrc a
	rrc b
	rrc c
	rrc d
	rrc e
	rrc h
	rrc l
	
	rrca
	rrd
	
	rst 00H
	rst 08H
	rst 10H
	rst 18H
	rst 20H
	rst 28H
	rst 30H
	rst 38H

	sbc a,(hl)
	sbc a,(ix+dd)
	sbc a,(iy+dd)
	sbc a,a
	sbc a,b
	sbc a,c
	sbc a,d
	sbc a,e
	sbc a,h
	sbc a,l
	sbc a,n
	sbc hl,bc
	sbc hl,de
	sbc hl,hl
	sbc hl,sp
	
	scf
	
	set 0,(hl)
	set 0,(ix+dd)
	set 0,(iy+dd)
	set 0,a
	set 0,b
	set 0,c
	set 0,d
	set 0,e
	set 0,h
	set 0,l
	set 1,(hl)
	set 1,(ix+dd)
	set 1,(iy+dd)
	set 1,a
	set 1,b
	set 1,c
	set 1,d
	set 1,e
	set 1,h
	set 1,l
	set 2,(hl)
	set 2,(ix+dd)
	set 2,(iy+dd)
	set 2,a
	set 2,b
	set 2,c
	set 2,d
	set 2,e
	set 2,h
	set 2,l
	set 3,(hl)
	set 3,(ix+dd)
	set 3,(iy+dd)
	set 3,a
	set 3,b
	set 3,c
	set 3,d
	set 3,e
	set 3,h
	set 3,l
	set 4,(hl)
	set 4,(ix+dd)
	set 4,(iy+dd)
	set 4,a
	set 4,b
	set 4,c
	set 4,d
	set 4,e
	set 4,h
	set 4,l
	set 5,(hl)
	set 5,(ix+dd)
	set 5,(iy+dd)
	set 5,a
	set 5,b
	set 5,c
	set 5,d
	set 5,e
	set 5,h
	set 5,l
	set 6,(hl)
	set 6,(ix+dd)
	set 6,(iy+dd)
	set 6,a
	set 6,b
	set 6,c
	set 6,d
	set 6,e
	set 6,h
	set 6,l
	set 7,(hl)
	set 7,(ix+dd)
	set 7,(iy+dd)
	set 7,a
	set 7,b
	set 7,c
	set 7,d
	set 7,e
	set 7,h
	set 7,l

	sla (hl)
	sla (ix+dd)
	sla (iy+dd)
	sla a
	sla b
	sla c
	sla d
	sla e
	sla h
	sla l
	
	sra (hl)
	sra (ix+dd)
	sra (iy+dd)
	sra a
	sra b
	sra c
	sra d
	sra e
	sra h
	sra l
	
	srl (hl)
	srl (ix+dd)
	srl (iy+dd)
	srl a
	srl b
	srl c
	srl d
	srl e
	srl h
	srl l
	
	sub (hl)
	sub (ix+dd)
	sub (iy+dd)
	sub a
	sub b
	sub c
	sub d
	sub e
	sub h
	sub l
	sub n
	
	xor (hl)
	xor (ix+dd)
	xor (iy+dd)
	xor a
	xor b
	xor c
	xor d
	xor e
	xor h
	xor l
	xor n
