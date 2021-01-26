;;; Author: Leon Bottou
;;; Public Domain.

; Ceci est le code source du programme
; contenu dans un modem pilote par 6801.
; 
; Le processeur (Motorola 6801)
;    - communiquait avec l'ordinateur par
;      son propre port serie a 1200 ou 9600 bauds
;      selon l'etat du switch INSP
;    - communiquait avec une puce modem EFCIS
;      via un ACIA 6850
;    - Ses ports // controlaient divers 
;      parametres du circuit modem, et une LED
; 
; Il y avait en outre
;    - 4k de ROM  (2532) de $f000 a $ffff
;    - 2k de RAM CMOS (6116) de $d800 a $dfff
;    - Une horloge temps reel 6818 sauvegardee
;
; Le programme ci dessous contient un Moniteur
; Hexa (Apple][ like) avec mini-assembleur 
; et desassembleur, et le programme de gestion
; du modem. Au reset, le CPU branche sur l'un ou
; l'autre, selon l'etat d'un switch INMOD.


; Cible: TI ou Motorola 2532
; placee aux adresses $F000->$FFFF

; Declarations
 
 page 0,132

 output scode
 cpu 6801
 
 fdb = dw	; equivalence de mnemoniques
 fcb = db
 fcc = db	; pour ne pas trop modifier...
 
 org macro
     * = \1
     endm


; quelques registres du 6801
; et leur signification ici.

ddr1 	equ $00
ddr2 	equ $01
dr1 	equ $02 ; /XRTS,HANG,/CTS,TEST,MC/BC,/CD,INMOD,INSP
dr2 	equ $03 ; b0: /LEDR

tcsr    equ $8 ; ICF,OCF,TOF,EICI,EOCI,ETOI,IEDG,OLVL
tim 	equ $9
ocr 	equ $b
rmcr 	equ $10
trcsr 	equ $11 ; RDRF,ORFE,TDRE,RIE,RE,TIE,TE,WU
rdr 	equ $12
tdr 	equ $13

; Les adresses de l'ACIA 6850

aciacr  equ $bffe ; crW: RIE,0,TIE,P,P,P,D,D ( 01001 )
aciadr 	equ $bfff ; crR: IRQ,PE,OV,FE,/CTS,/DCD,TDRE,RDRF

; Deux octets de ram sauvegardee
; dans l'horloge 6818

sav1 	equ $9fce ; RamOk:$87
sav2 	equ $9fcf ; /LF,/XonXoff,RTS,0,ECHO,HALF,TEST,MCBC

; Les registres du 6818

hbase 	equ $9fc0
rega 	equ $9fca ; UIP,div2-0(010) rs3-0(0000)
regb 	equ $9fcb ; SET,PIE,AIE,UIE,SQWE,DM,24/12,DSE
regc 	equ $9fcc ; IRQF,PF,AF,UF, 0000
regd 	equ $9fcd ; VRT,0000000

; Les adresses de base de :
;    la RAM CMOS 6116
;    la RAM du 6818

sram 	equ $d800
hram 	equ $9fd0

; La queue d'entree dans la 6116

dgo 	equ $dfae
xgo 	equ $dfac

; Le buffer de ligne pour le moniteur

inbuf 	equ $dfb1
endbuf 	equ $dfff

; Quelques emplacement dans
; la ram du 6801

possav 	equ $96
ocfv 	equ $80
tofv 	equ $82
icfv 	equ $84
nmiv 	equ $86

flashled equ $88
counter equ $88
flag 	equ $89 ; FLSH,-,+,.,:,sz2-0
mode 	equ $89

posxin 	equ $8a
posin 	equ $8c
rcvxin 	equ $8b
rcvin 	equ $8d

abort 	equ $1d
xinmask	equ $1f
inmask 	equ $7f

xsav 	equ $8e
r0 	equ $90
r1 	equ $92
r2 	equ $94

checksum 	equ $9e
aciamode 	equ $9f

; dont la queue d'entree du modem,
; et la pile CPU

xinqueue 	equ $a0
inqueue 	equ $df00
stkbase 	equ $ff


; Les vecteurs d'interruption

 	org $fff0
 	fdb sciirq
 	fdb ledirq
 	fdb ocfirq
 	fdb icfirq
 	fdb aciairq
 	fdb swiirq
 	fdb nmiirq
 	fdb reset


; Le programme lui meme

 	org $f000

msga 	asc "\n\rMoniteur LYB.\0"
msgb 	asc "Erreur\0"
msgc 	asc "Erreur de Checksum\0"
msgd 	asc "OVERFLOW\n\r\0"
msgswi 	asc "\n\rSWI: SP   P  A B  X\0"
msgwr 	asc "S9030000FC\0"


mydummy 	rti
ocfirq 	ldx ocfv
 	jmp 0,x
icfirq 	ldx icfv
 	jmp 0,x
nmiirq 	ldx nmiv
 	jmp 0,x

swiirq 	ldx #msgswi
 	jsr outmsg
 	tsx
 	ldx 5,x
 	jsr prx
 	jsr outsp
 	tsx
 	jsr prx
 	jsr outsp
 	ldab 0,x
 	jsr prb
 	jsr outsp
 	ldab 2,x
 	jsr prb
 	ldab 1,x
 	jsr prb
 	jsr outsp
 	ldx 3,x
 	jsr prx
 	jsr outcr
 	jmp monloop2

; Reset general


reset 	lds #stkbase

 	ldx #mydummy
 	stx ocfv
 	stx icfv
 	stx nmiv
 	stx tofv
 	clra
 	staa flashled
 	staa posin
 	staa posxin
 	staa rcvin
 	staa rcvxin

 	ldaa sav1
 	cmpa #$87
 	beq reset2
 	clra
 	staa sav2
reset2 	anda regd
 	anda #$80
 	staa flag
 	oraa #$7
 	staa sav1

 	sei
 	ldaa #$11
 	staa ddr2
 	ldaa #$f8
 	staa ddr1
 	ldaa #%10000000
 	staa dr1
 	ldaa #$01
 	staa dr2


 	ldaa #%011
 	staa aciacr
 	ldaa #%01001
 	staa aciacr
 	staa aciamode

 	ldaa #%11010
 	staa trcsr
 	ldab dr1
 	ldaa #%0110 ; 1200 bauds
 	bitb #$01
 	beq reset3
 	ldaa #%0101 ; 9600 bauds
reset3 	staa rmcr

 	ldaa #%0101
 	staa tcsr
 	cli

 	ldx #inbuf-1
reset1 	inx
 	clr 0,x
 	cpx #endbuf
 	bne reset1

 	bitb #$02
 	beq reset5
reset4 	jmp modem
reset5 	jmp monloop

	
; SP de controle des irq


eixin 	ldab #$80
 	orab aciamode
 	bra setxcr
dixin 	ldab #$7f
 	andb aciamode
setxcr 	stab aciamode
 	stab aciacr
 	rts


; SciIrq

sciirq 	ldaa rcvin
 	inca
 	tab
 	suba posin
 	anda #inmask
 	beq ovfl
 	ldx #inqueue
 	andb #inmask
 	abx
 	stab rcvin
 	ldab trcsr
 	ldab rdr
 	stab 0,x
 	cmpa #inmask-31
 	bne sciret
 	cli
 	ldab sav2
 	bitb #$40
 	bne noxoff
 	ldaa #'S'-$40
 	jsr out
noxoff 	ldab sav2
 	bitb #$20
 	beq sciret
 	sei
 	ldab dr1
 	orab #$20
 	stab dr1
sciret 	rti


ovfl 	sei
 	ldx #msgd
ovfl2 	ldaa trcsr
 	bita #$20
 	beq ovfl2
 	ldaa 0,x
 	beq sciret
 	staa tdr
 	inx
 	bra ovfl2


; Aciairq

aciairq 	ldab rcvxin
 	incb
 	andb #xinmask
 	tba
 	suba posxin
 	anda #xinmask
 	beq ovfl
 	ldx #xinqueue
 	abx
 	stab rcvxin
 	ldaa aciacr
 	ldaa aciadr
 	staa 0,x
 	rti


; StdIn

in 	bsr intst
 	beq in
rdret 	rts


; Intst

intst 	ldab posin
 	tba
 	suba rcvin
 	nega
 	anda #inmask
 	beq rdret
rdin 	incb
 	andb #inmask
 	stx xsav
 	ldx #inqueue
 	abx
 	cmpa #$8
 	bne rdin2
 	ldaa sav2
 	bita #$40
 	bne noxon
 	ldaa #'Q'-$40
 	jsr out
noxon 	ldaa sav2
 	bita #$20
 	beq rdin2
 	sei
 	ldaa dr1
 	anda #$df
 	staa dr1
 	cli
rdin2 	sei
 	inc posin
 	ldaa 0,x
 	ldx xsav
 	bra retour

	
; Xout

xout2 	cli
xout 	sei
 	ldab #$2
 	bitb aciacr
 	beq xout2
 	ldab aciacr
 	staa aciadr
 	cli
 	rts


; Xin

xin 	bsr xintst
 	beq xin
xinret 	rts


; Xintst

xintst 	ldab posxin
 	tba
 	suba rcvxin
 	anda #xinmask
 	beq xinret

 	incb
 	andb #xinmask
 	ldx #xinqueue
 	abx
 	sei
 	stab posxin
 	ldaa 0,x
retour 	cli
 	ldab #$ff
 	rts


; StdOut

out2 	sei
out 	cli
 	ldab #$20
 	bitb trcsr
 	beq out2
 	ldab trcsr
 	staa tdr
 	cli
 	rts


; OUT ceci et cela

outcr 	ldaa sav2
 	bmi outcr2
 	ldaa #$a
 	bsr out
outcr2 	ldaa #$d
 	bra out

	outbs ldaa #$08
 	bsr out
 	bsr outsp
 	ldaa #$08
 	bra out

outmsg 	ldaa 0,x
 	beq outcr
 	bsr out
 	inx
 	bra outmsg

outt 	ldaa #'-'
 	bra out
outsp 	ldaa #$20
 	bra out
out2p 	ldaa #':'
 	bra out


prx 	stx xsav
 	ldab xsav
 	bsr prb
 	ldab xsav+1
prb 	pshb
 	addb checksum
 	stab checksum
 	pula
 	psha
 	lsra
 	lsra
 	lsra
 	lsra
 	bsr prb2
 	pula
prb2 	anda #$f
 	oraa #$30
 	cmpa #$3a
 	bcs prb3
 	adda #$7
prb3 	bra out


heure 	jsr outcr
 	ldab sav1
 	ldaa #$20
 	cmpb #$87
 	beq heur1
 	ldaa #'#'
heur1 	jsr out
 	ldx #hbase
heur2 	ldaa rega
 	bmi heur2
 	ldab 7,x
 	bsr prb
 	bsr outt
 	ldab 8,x
 	bsr prb
 	bsr outt
 	ldab #$19
 	bsr prb
 	ldab 9,x
 	bsr prb
 	bsr outsp
 	ldab 4,x
 	bsr prb
 	bsr out2p
 	ldab 2,x
 	bsr prb
 	bsr out2p
 	ldab 0,x
 	bra prb


; IRQ:  ledirq  1/20s

ledirq 	inc flashled
 	ldaa tcsr
 	ldx tim
 	ldaa #$8
 	bita flashled
 	bne led2
 	ldaa flag
 	bmi tofirq
 	ldaa #$1
 	oraa dr2
 	bra led3
led2 	ldaa #$fe
 	anda dr2
led3 	staa dr2
tofirq 	ldx tofv
 	jmp 0,x

	

; MONITEUR: rdline

rdline2 ldaa #'\\'
 	jsr out
rdline 	jsr outcr
rdnoret ldaa inbuf-1
 	jsr out
 	ldx #inbuf
rdloop 	jsr in
 	cmpa #abort
 	beq rdline2
 	cmpa #$09
 	bne rdline1
 	ldaa 0,x
 	beq rdloop
rdline1 	cmpa #$08
 	bne rdline3
 	cpx #inbuf
 	beq rdline
 	jsr outbs
 	dex
 	bra rdloop
rdline3 	cmpa #$0a
 	beq rdloop
 	cmpa #$0d
 	bne rdline4
rdend 	clr 0,x
 	jmp outcr
rdline4 	cpx #endbuf
 	bcc rdloop
 	jsr out
 	staa 0,x
 	inx
 	bra rdloop


; Conversion en majuscules

maj 	cmpa #'a'
 	bcs maj2
 	cmpa #'z'
 	bhi maj2
 	suba #$20
maj2 	rts


; Lecture d'un hhhh

readhex ldaa flag
 	anda #$f8
 	staa flag
 	ldaa 0,x
 	cmpa #'\''
 	bne readhex2
 	clr r1
 	ldaa 1,x
 	staa r1+1
 	inc flag
 	ldaa 2,x
 	inx
 	inx
readhex5 inc flag
 	ldab flag
 	bitb #$70
 	bne readhex6
 	bitb #$6
 	beq readhex6
 	ldd r1
 	std r0
readhex6 ldaa 0,x
 	inx
 	bra maj
readhex2 ldaa 0,x
 	jsr maj
 	ldab #$4
 	bitb flag
 	bne readhex5
 	jsr conv
 	bcc readhex5
 	ldaa #$7
 	bita flag
 	bne readhex3
 	clra
 	staa r1
 	staa r1+1
readhex3 inc flag
 	aslb
 	aslb
 	aslb
 	aslb
 	ldaa #$4
readhex4 aslb
 	rol r1+1
 	rol r1
 	deca
 	bne readhex4
 	inx
 	bra readhex2


conv 	tab
 	subb #$30
 	cmpb #$11
 	bcc conv2
 	cmpb #$a
 	rts
conv2 	subb #$7
 	cmpb #$10
 	rts


; MONITEUR boucle et xeqline

monloop ldx #msga
 	jsr outmsg
monloop2 lds #stkbase
 	ldaa #'*'
 	staa inbuf-1
 	jsr rdline
 	ldx inbuf
 	cpx #$4154
 	bne goxeq

 	jsr hayes
 	bra monloop2

goxeq 	jsr xeqline
 	bra monloop2


xeqline ldx #inbuf
 	stx possav
 	ldaa flag
 	anda #$80
 	staa flag
xeq2 	ldx possav
 	jsr readhex
 	stx possav
 	ldx #jtable
xeq3 	cmpa 0,x
 	beq xeq5
 	tst 0,x
 	beq err
 	inx
 	inx
 	inx
 	bra xeq3

err 	ldx #msgb
 	jmp outmsg

xeq5 	ldx 1,x
 	ldab flag
 	tba
 	anda #$80
 	staa flag
 	jsr 0,x
 	bra xeq2


; Table des commandes

jtable 	fcb 'H'
 	fdb heure
 	fcb ' '
 	fdb space
 	fcb '?'
 	fdb ascii
 	fcb '.'
 	fdb point
 	fcb 'G'
 	fdb go
 	fcb 'R'
 	fdb read
 	fcb 'W'
 	fdb write
 	fcb ':'
 	fdb patchmode
 	fcb 'X'
 	fdb modem
 	fcb '>'
 	fdb prr2
 	fcb '+'
 	fdb plus
 	fcb '-'
 	fdb moins
 	fcb 'M'
 	fdb move
 	fcb '<'
 	fdb transf
 	fcb 'Y'
 	fdb goram
 	fcb '='
 	fdb setregs
 	fcb 'L'
 	fdb dasm
 	fcb '!'
 	fdb masm

 	fcb 0
 	fdb cr


; Commandes moniteur

cr 	pula
 	pula
 	ldx possav
 	dex
 	cpx #inbuf
 	bne space

 	ldd r0
 	orab #$f
 	std r1

dump2 	ldaa r0+1
 	anda #$f
 	bne dump3
dump 	ldx r0
 	jsr outcr
 	jsr prx
 	jsr outt
dump3 	jsr outsp
 	ldx r0
 	stx r2
 	ldab 0,x
 	jsr prb
 	jsr nxtr0
 	bcs dump2
 	rts


nxtr2 	ldx r2
 	inx
 	stx r2
nxtr0 	ldd r0
 	ldx r0
 	inx
 	stx r0
 	subd r1
 	rts


point 	ldaa #$10
 	bra orflag
plus 	ldaa #$20
 	bra orflag
moins 	ldaa #$40
orflag 	oraa flag
 	staa flag
space2 	rts


space 	tba
 	anda #$88
 	staa flag
 	bitb #$6
 	beq space2
 	bitb #$8
 	bne patch
 	bitb #$20
 	bne xplus
 	bitb #$40
 	bne xmoins
 	bra dump

	
patch 	ldx r2
 	ldaa r1+1
 	staa 0,x
 	bitb #$4
 	beq patch2
 	ldd r1
 	std 0,x
 	inx
patch2 	inx
 	stx r2
 	rts

	
xplus 	ldd r0
 	addd r1
 	bra xarith
xmoins 	ldd r0
 	subd r1
xarith 	std r0
 	std r1
 	ldx r0
 	ldaa #'='
 	jsr out
 	jmp prx

	
patchmode orab #$8
 	andb #$8f
 	stab flag
 	bitb #$6
 	beq patchmd2
 	ldd r0
 	std r2
patchmd2 rts

	
prr2 	ldx r2
 	jsr prx
 	jmp out2p

	
go 	ldaa #$7e
 	staa r0-1
 	ldd dgo
 	ldx xgo
 	jsr r0-1
 	swi

	
goram 	ldx r1
 	jmp 0,x

	
move 	ldx r0
 	ldaa 0,x
 	ldx r2
 	staa 0,x
 	jsr nxtr2
 	bcs move
 	rts

transf 	bitb #6
 	beq transf2
 	ldd r0
 	std r2
transf2 rts


ascii2 	ldaa r0+1
 	anda #$1f
 	bne ascii3
ascii 	ldx r0
 	jsr outcr
 	jsr prx
 	jsr outt
ascii3 	jsr outsp
 	ldx r0
 	stx r2
 	ldab 0,x
 	tba
 	andb #$7f
 	cmpb #$20
 	bcc ascii4
 	ldaa #'.'
ascii4 	jsr out
 	jsr nxtr0
 	bcs ascii2
 	rts


setregs	bitb #$10
 	beq setacc
 	ldd r1
 	std xgo
setacc 	ldd r0
 	std dgo
 	rts

	
; Transferts SCODE

lenframe 	equ $98
bufbyte 	equ $99
address 	equ $9a

write 	ldx r0
 	ldd r1
 	subd r0
 	tsta
 	bne write1
 	cmpb #$f
 	bcs write2
write1 	ldab #$f
write2 	incb
 	stab lenframe
 	ldaa #'S'
 	jsr out
 	ldaa #'1'
 	jsr out
 	clr checksum
 	ldab lenframe
 	addb #$3
 	jsr prb
 	jsr prx
write3 	ldab 0,x
 	inx
 	jsr prb
 	dec lenframe
 	bne write3
 	ldab checksum
 	comb
 	jsr prb
 	jsr outcr2
 	dex
 	stx r0
 	jsr nxtr0
 	bcs write
 	ldx #msgwr
 	jmp outmsg

	
rberr 	pula
 	pula
readerr  jsr err
readerr2 jsr in
 	cmpa #abort
 	bne readerr2
 	rts

readdone jsr in
 	jsr conv
 	bcs readdone
 	rts

rbyte 	jsr in
 	jsr conv
 	bcc rberr
 	aslb
 	aslb
 	aslb
 	aslb
 	stab bufbyte
 	jsr in
 	jsr conv
 	bcc rberr
 	orab bufbyte
 	tba
 	adda checksum
 	staa checksum
 	rts



read2 	jsr in
 	jsr conv
 	bcs read2
read 	jsr in
 	cmpa #$0a
 	beq read
 	cmpa #$0d
 	beq read
 	cmpa #'S'
 	bne readerr
 	jsr in
 	jsr conv
 	bcc readerr
 	tstb
 	beq read2
 	cmpb #$9
 	beq readdone
 	clr checksum
 	bsr rbyte
 	subb #3
 	stab lenframe
 	beq read2
 	bsr rbyte
 	stab address
 	bsr rbyte
 	stab address+1
 	ldx address
read3 	bsr rbyte
 	cpx r0
 	bcs read4
 	cpx r1
 	bhi read4
 	stab 0,x
read4 	inx
 	dec lenframe
 	bne read3
 	bsr rbyte
 	ldab checksum
 	comb
 	beq read
 	ldx #msgc
 	jsr outmsg
 	bra read

; Dessassembleur

dasm 	bitb #$10
 	bne dasm4
 	bitb #$6
 	beq dasm2
 	ldd r0
 	std r2
dasm2 	ldaa #$14
dasm3 	psha
 	jsr dasmins
 	pula
 	deca
 	bne dasm3
 	rts

dasm4 	ldd r0
 	std r2
dasm5 	jsr dasmins
 	ldd r2
 	subd r1
 	bcs dasm5
 	rts



; Miniassembleur

masm 	bitb #$6
 	beq masm2
 	ldd r0
 	std r2
masm2 	ldx r2
 	jsr prx
 	ldaa #'!'
 	staa inbuf-1
 	jsr rdnoret
 	ldx #inbuf-1
masm4 	inx
 	ldaa 0,x
 	cmpa #$20
 	beq masm4
 	stx possav
 	tsta
 	beq masm3
 	jsr masmins
 	bcs masm5
 	jsr dasmins
 	bra masm2
masm5 	jsr err
 	bra masm2
masm3 	jmp monloop2


; Hayes

msgh1 	asc "ERROR\0"
msgh2 	asc "NO CONNECT\0"
msgh3 	asc "OK\0"
msgh4 	asc "\r\nEND OF CONNECTION\0"
msghay 	asc "SmartModem non encore implemente\0"


hayes 	ldx #msgh1
 	jsr outmsg
 	ldx #msghay
 	jmp outmsg


; Modem Manuel

msg1 	asc "C Connecte\0"
msg2 	asc "Attente de la porteuse\0"

	
modemoff ldaa dr1
 	anda #$bf
 	oraa #$80
 	staa dr1
 	ldx #msgh3
 	jmp outmsg

validcd ldab counter
 	addb #$22
 	ldaa #4
valid2 	bita dr1
 	bne valid3
 	cmpb counter
 	bcc valid2
 	bita dr1
valid3 	rts

testcd 	ldaa #4
 	bita dr1
 	beq test3
 	ldab counter
 	addb #$19
test2 	bita dr1
 	beq test3
 	cmpb counter
 	bcc test2
 	bita dr1
test3 	rts

modemloop sei
 	jsr eixin
 	cli
mloop1 	jsr testcd
 	bne endmloop
 	ldaa #$2
 	bita aciacr
 	beq mloop2
 	jsr intst
 	beq mloop2
 	ldab sav2
 	bitb #$4
 	beq mloop11
 	jsr out
mloop11 jsr xout
mloop2 	ldaa #$20
 	bita trcsr
 	beq mloop1
 	jsr xintst
 	beq mloop1
 	ldab sav2
 	bitb #8
 	beq mloop21
 	jsr xout
mloop21 jsr out
 	bra mloop1

endmloop sei
 	jsr dixin
 	cli
 	rts



modem 	lds #stkbase
 	sei
 	ldaa posxin
 	staa rcvxin
 	cli
 	jsr setup
 	ldx #msg2
 	jsr outmsg

 	ldaa sav2
 	sei
 	ldab dr1
 	andb #$c7
 	bita #1
 	beq modset1
 	orab #$8
modset1 bita #2
 	beq modset2
 	orab #$10
modset2 bita #4
 	beq modset3
 	andb #$7f
modset3 orab #$40
 	stab dr1
 	cli

waitcd 	jsr intst
 	beq wait2
 	cmpa #abort
 	beq remodem
wait2 	jsr validcd
 	bne waitcd

 	sei
 	ldab dr1
 	andb #$7f
 	stab dr1
 	cli

 	ldab #$8
 	ldaa #'M'
 	bitb dr1
 	bne modem5
 	ldaa #'B'
modem5 	jsr out
 	ldx #msg1
 	jsr outmsg

 	jsr modemloop
remodem jsr modemoff
 	jmp modem

modem3 	cmpa #$0d
 	beq modem3

; setup


msgs1 	asc "M)oniteur, T)est, E)cho, D)uplex, "
 	asc "V)itesse, P)rotocole, C)onnection\0"

msgs2 	asc "Xon/Xoff  \0"
msgs3 	asc "RTS/CTS   \0"

msgs5 	asc "  Test    \0"
msgs6 	asc "  Normal  \0"

msgs7 	asc "Half  \0"
msgs8 	asc "Full  \0"

msgs9 	asc "Echo on   \0"
msgs10 	asc "Echo off  \0"

pmsg 	ldaa 0,x
 	beq pmsg2
 	jsr out
 	inx
 	bra pmsg
pmsg2 	rts


modeline ldaa #$0C
 	jsr out
 	ldaa sav2
	lsra
 	eora sav2
 	ldx #$1200
 	bita #$1
 	beq modln2
 	ldx #$75
modln2 	jsr prx
 	ldaa #'/'
 	jsr out
 	ldx #$1200
 	ldaa sav2
 	bita #$1
 	bne modln3
 	ldx #$75
modln3 	jsr prx
 	ldx #msgs5
 	ldaa sav2
 	bita #$2
 	bne modln4
 	ldx #msgs6
modln4 	jsr pmsg
 	ldx #msgs7
 	ldaa sav2
 	bita #$4
 	bne modln5
 	ldx #msgs8
modln5 	jsr pmsg
 	ldx #msgs9
 	ldaa sav2
 	bita #$8
 	bne modln6
 	ldx #msgs10
modln6 	jsr pmsg
 	ldx #msgs2
 	ldaa sav2
 	bita #$40
 	bne modln7
 	jsr pmsg
modln7 	ldx #msgs3
 	ldaa sav2
 	bita #$20
 	beq modln8
 	jsr pmsg
modln8 	jsr outcr
 	ldx #msgs1
 	jmp outmsg


setup0 	bitb #$2
 	beq setup1
 	andb #$F7
setup1 	stab sav2
setup 	jsr modeline
setupin jsr in
 	jsr maj
 	ldab sav2
 	cmpa #'M'
 	bne setup2
 	jmp monloop
setup2 	cmpa #'T'
 	bne setup3
 	eorb #2
 	bra setup0
setup3 	cmpa #'D'
 	bne setup4
 	eorb #4
 	bra setup0
setup4 	cmpa #'E'
 	bne setup5
 	eorb #8
 	bra setup0
setup5 	cmpa #'P'
 	bne setup6
 	addb #$20
 	bvc setup0
 	eorb #$80
 	bra setup0
setup6 	cmpa #'V'
 	bne setup7
 	andb #$f3
 	eorb #$1
 	bitb #$1
 	beq setup0
 	orab #$C
 	bra setup0
setup7 	cmpa #'C'
 	beq setup8
 	cmpa #$0D
 	bne setupin
setup8 	jmp outcr


; DASM

mnemosav 	equ $9a
formatsav 	equ $9b

decode 	ldx #mycode
 	abx
 	ldaa 0,x

 	cmpb #$8d
 	beq decod2
 	cmpb #$60
 	bcs decod3
 	lsrb
 	lsrb
 	lsrb
 	lsrb
 	andb #$3
 	ldx #format
 	tsta
 	bpl decod4
 	orab #$4
decod4 	abx
 	anda #$7f
 	beq decod5
 	ldab 0,x
 	rts

decod3 	andb #$F0
 	cmpb #$20
 	beq decod2
decod5 	ldab #$01
 	rts

decod2 	ldab #$72
 	rts

dasmins ldx r2
 	ldab 0,x
 	jsr decode
 	staa mnemosav
 	stab formatsav
 	jsr prhexa
 	jsr prmnemo
 	jsr prargs
 	ldab formatsav
 	andb #$3
 	ldx r2
 	abx
 	stx r2
 	jmp outcr


prmnemo ldab mnemosav
 	clra
 	asld
 	asld
 	addd #mnemo
 	std xsav
 	ldx xsav
 	ldab #$4
prmn2 	pshb
 	ldaa 0,x
 	inx
 	oraa #$20
 	jsr out
 	pulb
 	decb
 	bne prmn2

out3bl 	ldab #$3
outnbl 	pshb
 	jsr outsp
 	pulb
 	decb
 	bne outnbl
 	rts


prhexa 	ldx r2
 	jsr prx
 	jsr outt
 	jsr outsp
 	ldx r2
 	ldab 0,x
 	jsr prb
 	ldx r2
 	ldaa formatsav
 	anda #$3
 	ldab #$a
 	deca
 	beq prhexa1
 	deca
 	beq prhexa3
 	jsr outsp
 	ldx 1,x
 	jsr prx
 	bra prhexa2
prhexa3 jsr out3bl
 	ldab 1,x
 	jsr prb
prhexa2 ldab #$5
prhexa1 jmp outnbl


prargs 	ldaa formatsav
 	staa mnemosav
 	cmpa #$72
 	beq relatif
 	asl mnemosav
 	bcc prargs2
 	ldaa #'#'
 	jsr out
prargs2 asl mnemosav
 	bcc prargs3
 	ldaa #'$'
 	jsr out
prargs3 ldx r2
 	asl mnemosav
 	bcc prargs4
 	ldab 1,x
 	jsr prb
prargs4 asl mnemosav
 	bcc prargs5
 	ldx 1,x
 	jsr prx
prargs5 asl mnemosav
 	bcc prargs6
 	ldaa #','
 	jsr out
 	ldaa #'X'
 	jsr out
prargs6 rts

relatif ldaa #'$'
 	jsr out
 	ldx r2
 	ldab 1,x
 	clra
 	tstb
 	bpl relat2
 	coma
relat2 	addd r2
 	addd #$2
 	std xsav
 	ldx xsav
 	jmp prx

; MASM

masmins jsr parse
 	bcs masmerr
 	clrb
masmloop pshb
 	jsr decode
 	subd mnemosav
 	beq masmfound
 	pulb
 	incb
 	bne masmloop
 	ldaa formatsav

 	cmpa #$e2
 	bne ms2
 	ldaa #$d3
 	bra ms1

ms2 	cmpa #$62
 	bne ms3
 	ldaa #$53
 	bra ms1

ms3 	cmpa #$53
 	beq ms4
masmerr sec
 	rts

ms4 	ldaa #$72
ms1 	staa formatsav
 	bra masmloop


masmfound pulb
 	ldx r2
 	stab 0,x
 	ldaa formatsav
 	cmpa #$72
 	beq masmrel
 	anda #$3
 	cmpa #3
 	beq masmf2
 	cmpa #2
 	beq masmf1
 	clc
 	rts

masmrel 	ldd r1
 	subd #2
 	subd r2
 	tstb
 	bpl masmrel2
 	coma
masmrel2 tsta
 	bne masmerr
 	stab 1,x
 	clc
 	rts

masmf1 	ldab r1+1
 	stab 1,x
 	clc
 	rts

masmf2 	ldd r1
 	std 1,x
 	clc
 	rts

nxtchar stx xsav
 	ldx possav
 	ldaa 0,x
 	inx
 	stx possav
 	jsr maj
 	ldx xsav
 	rts

parse 	ldaa #(mnemend-mnemo)>>2
	staa mnemosav
 	ldd possav
 	std r1
parse2 	dec mnemosav
	beq masmerr
 	ldab mnemosav
	clra
 	asld
 	asld
 	addd #mnemo
 	std xsav
 	ldx xsav
 	ldaa 0,x
 	beq masmerr
 	ldd r1
 	std possav
 	ldab #$4
parse1 	ldaa 0,x
 	cmpa #$20
 	beq parsed1
 	tstb
 	beq parsed1
 	jsr nxtchar
 	cmpa 0,x
 	bne parse2
 	inx
 	decb
 	bra parse1


parsed1 clr formatsav
parse3 	jsr nxtchar
 	cmpa #$20
 	beq parse3

 	cmpa #'#'
 	bne parse4
 	ldaa #$80
 	oraa formatsav
 	staa formatsav
 	jsr nxtchar

parse4 	cmpa #'$'
 	bne parse5
 	jsr nxtchar

parse5 	ldx possav
 	dex
 	jsr readhex
 	ldab flag
 	ldaa #$1
 	bitb #$6
 	beq parse6
 	ldaa #$62
 	bitb #$4
 	beq parse6
 	ldaa #$53

parse6 	oraa formatsav
 	staa formatsav
 	dex
 	ldaa 0,x
 	cmpa #','
 	bne parse7
 	inx
 	ldaa 0,x
 	jsr maj
 	cmpa #'X'
 	bne parserr
 	inx
 	ldaa formatsav
 	oraa #$0C
 	staa formatsav

parse7 	ldaa 0,x
 	inx
 	cmpa #$20
 	beq parse7
 	tsta
 	beq parseok
 	cmpa #';'
 	beq parseok
parserr sec
 	rts
parseok clc
 	rts


; Tables de DASM et MASM


mnemo 	asc "??? "
 	asc "STAA"
 	asc "STAB"
 	asc "STD "
 	asc "STX "
 	asc "STS "
 	asc "LDAA"
 	asc "LDAB"
 	asc "LDD "
 	asc "LDX "
 	asc "LDS "
 	asc "BITA"
 	asc "BITB"
 	asc "CMPA"
 	asc "CMPB"
 	asc "CPX "

 	asc "ADDA"
 	asc "ADDB"
 	asc "ADDD"
 	asc "SUBA"
 	asc "SUBB"
 	asc "SUBD"
 	asc "ADCA"
 	asc "ADCB"
 	asc "SBCA"
 	asc "SBCB"
 	asc "EORA"
 	asc "EORB"
 	asc "ANDA"
 	asc "ANDB"
 	asc "ORAA"
 	asc "ORAB"

 	asc "ASR "
 	asc "ASRA"
 	asc "ASRB"
 	asc "LSR "
 	asc "LSRA"
 	asc "LSRB"
 	asc "LSRD"
 	asc "ASL "
 	asc "ASLA"
 	asc "ASLB"
 	asc "ASLD"
 	asc "INC "
 	asc "INCA"
 	asc "INCB"
 	asc "INS "
 	asc "INX "

 	asc "DEC "
 	asc "DECA"
 	asc "DECB"
 	asc "DES "
 	asc "DEX "
 	asc "PSHA"
 	asc "PSHB"
 	asc "PSHX"
 	asc "PULA"
 	asc "PULB"
 	asc "PULX"
 	asc "SBA "
 	asc "CBA "
 	asc "ABA "
 	asc "ABX "
 	asc "MUL "

 	asc "BRA "
 	asc "BRN "
 	asc "BHI "
 	asc "BLS "
 	asc "BCC "
 	asc "BCS "
 	asc "BNE "
 	asc "BEQ "
 	asc "BVC "
 	asc "BVS "
 	asc "BPL "
 	asc "BMI "
 	asc "BGE "
 	asc "BLT "
 	asc "BGT "
 	asc "BLE "

 	asc "BSR "
 	asc "JSR "
 	asc "JMP "
 	asc "RTI "
 	asc "WAIT"
 	asc "SWI "
 	asc "RTS "
 	asc "TST "
 	asc "TSTA"
 	asc "TSTB"
 	asc "ROL "
 	asc "ROLA"
 	asc "ROLB"
 	asc "ROR "
 	asc "RORA"
 	asc "RORB"

 	asc "TAB "
 	asc "TBA "
 	asc "TAP "
 	asc "TPA "
 	asc "TSX "
 	asc "TXS "
 	asc "DAA "
 	asc "NEG "
 	asc "NEGA"
 	asc "NEGB"
 	asc "COM "
 	asc "COMA"
 	asc "COMB"
 	asc "CLR "
 	asc "CLRA"
 	asc "CLRB"

 	asc "CLV "
 	asc "SEV "
 	asc "CLC "
 	asc "SEC "
 	asc "CLI "
 	asc "SEI "
 	asc "NOP "

mnemend equ *

mycode 	fcc $00,$76,$00,$00,$26,$2A,$62,$63
 	fcc $2F,$34,$70,$71,$72,$73,$74,$75

 	fcc $3B,$3C,$00,$00,$00,$00,$60,$61
 	fcc $00,$66,$00,$3D,$00,$00,$00,$00

 	fcc $40,$41,$42,$43,$44,$45,$46,$47
 	fcc $48,$49,$4A,$4B,$4C,$4D,$4E,$4F

 	fcc $64,$2E,$38,$39,$33,$65,$35,$36
 	fcc $3A,$56,$3E,$53,$37,$3F,$54,$55

	
 	fcc $68,$00,$00,$6B,$24,$00,$5E,$21
 	fcc $28,$5B,$31,$00,$2C,$58,$00,$6E

	
 	fcc $69,$00,$00,$6C,$25,$00,$5F,$22
 	fcc $29,$5C,$32,$00,$2D,$59,$00,$6F

 	fcc $67,$00,$00,$6A,$23,$00,$5D,$20
 	fcc $27,$5A,$30,$00,$2B,$57,$52,$6D

 	fcc $67,$00,$00,$6A,$23,$00,$5D,$20
 	fcc $27,$5A,$30,$00,$2B,$57,$52,$6D

	
 	fcc $13,$0D,$18,$95,$1C,$0B,$06,$00
 	fcc $1A,$16,$1E,$10,$8F,$50,$8A,$00

 	fcc $13,$0D,$18,$15,$1C,$0B,$06,$01
 	fcc $1A,$16,$1E,$10,$0F,$51,$0A,$05

 	fcc $13,$0D,$18,$15,$1C,$0B,$06,$01
 	fcc $1A,$16,$1E,$10,$0F,$51,$0A,$05

 	fcc $13,$0D,$18,$15,$1C,$0B,$06,$01
 	fcc $1A,$16,$1E,$10,$0F,$51,$0A,$05

	
 	fcc $14,$0E,$19,$92,$1D,$0C,$07,$00
 	fcc $1B,$17,$1F,$11,$88,$00,$89,$00

 	fcc $14,$0E,$19,$12,$1D,$0C,$07,$02
 	fcc $1B,$17,$1F,$11,$08,$03,$09,$04

 	fcc $14,$0E,$19,$12,$1D,$0C,$07,$02
 	fcc $1B,$17,$1F,$11,$08,$03,$09,$04

 	fcc $14,$0E,$19,$12,$1D,$0C,$07,$02
 	fcc $1B,$17,$1F,$11,$08,$03,$09,$04

	
format 	fcc $E2,$62,$6E,$53,$D3
