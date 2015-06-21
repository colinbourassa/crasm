;;; Author: Leon Bottou
;;; Public Domain.

; Names for builin 6801 registers
; Bit testing macros

		dummy
		
vectors		= $fff0
	   *	= vectors
vector.sci 	dw 0
vector.tof 	dw 0
vector.ocf 	dw 0
vector.icf 	dw 0
vector.irq	dw 0
vector.swi	dw 0
vector.nmi	dw 0
vector.reset	dw 0

	   *    = $0
ddr1		db 0
ddr2		db 0
dr1		db 0
dr2		db 0
ddr3		db 0
ddr4		db 0
dr3		db 0
dr4		db 0
tcsr		db 0
counter		dw 0
ocr		dw 0
icr		dw 0
p3csr		db 0
rmcr		db 0
trcsr		db 0
rdr		db 0
tdr		db 0
ramcr		db 0
		

p3csr.is3	= p3csr{7}
p3csr.eis3i	= p3csr{6}
p3csr.os3	= p3csr{4}
p3csr.le	= p3csr{3}

ramcr.stby	= ramcr{7}
ramcr.rami	= ramcr{6}

tcsr.icf	= tcsr{7}
tcsr.ocf	= tcsr{6}
tcsr.tof	= tcsr{5}
tcsr.eici	= tcsr{4}
tcsr.eoci	= tcsr{3}
tcsr.etoi	= tcsr{2}
tcsr.iedg	= tcsr{1}
tcsr.olvl	= tcsr{0}

rmcr.cc1	= rmcr{3}
rmcr.cc0	= rmcr{2}
rmcr.ss1	= rmcr{1}
rmcr.ss0	= rmcr{0}

trcsr.rdrf	= trcsr{7}
trcsr.orfe	= trcsr{6}
trcsr.tdre	= trcsr{5}
trcsr.rie	= trcsr{4}
trcsr.re	= trcsr{3}
trcsr.tie	= trcsr{2}
trcsr.te	= trcsr{1}
trcsr.wu	= trcsr{0}


;; bset BITSPEC ---
;;  Sets bit BITSPEC
;;  Clobbers A.

bset	macro
	 ldaa #1<< BIT(\1)
	 oraa ADDR(\1)
	 staa ADDR(\1)
	endm

;; bclr BITSPEC ---
;;  Clears bit BITSPEC
;;  Clobbers A.

bclr	macro
	 ldaa # $ff ^ (1<< BIT(\1))
	 bnomask \1
	 anda ADDR(\1)
	 staa ADDR(\1)
	endm

;; btst BITSPEC ---
;;  Clears bit BITSPEC
;;  Set/Reset Z bit.
;;  Clobbers A.
	
btst	macro
	 ldaa #1<< BIT(\1)
	 bmask \1
	 anda ADDR(\1)
	endm

	code