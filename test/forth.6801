;;; Author: Leon Bottou
;;; Public Domain.

; Beginnings of a forth kernel.
; Good test for macros.
;
;  http://www.forth.org/
;  http://www.zetetics.com/bj/papers/moving1.htm



     cpu 6801
     mlist off
     page 0,132
     
     * = $1000


smudge = 1<<7
precedence = 1<<6
keep = 1<<5


;; **********************************
;; -- create INVOCNAME, WORDNAME [, FLAGS]
;; Create a forth word WORDNAME into vocabulary INVOCNAME
;; This macro outputs the word header and defines useful labels
;;   nfa_WORDNAME - address of header
;;   lfa_WORDNAME - address of pointer to previous word in vocabulary
;;   cfa_WORDNAME - address of executable data (just after header)

create	macro
 .start  = *
   if \3
 nfa_\2  db .len | smudge | \3
   else
 nfa_\2  db .len | smudge
   endc
 	 asc "\2"
 lfa_\2  dw lstw_\1
 lstw_\1 = .start
 cfa_\2  = *
 .len    = lfa_\2-nfa_\2
	endm


;; **********************************
;; -- createvoc INVOCNAME,VOCNAME
;; Create a forth vocabulary VOCNAME in vocabulary INVOCNAME
;; This macro outputs the word header and defines useful labels
;;   nfa_VOCNAME  - address of word header
;;   lfa_VOCNAME  - address of pointer to previous word in voc INVOCNAME
;;   cfa_VOCNAME  - address of word executable data (jsr dovoc)
;;   pfa_VOCNAME  - address of vocabulary data for VOCNAME
;;   lst_VOCNAME  - address of pointer to last word in vocabulary
;;   vlnk_VOCNAME - address of pointer to parent vocabulary.
;; The following symbol is modified whenever 
;; a word is added into the vocabulary VOCNAME
;;   lstw_VOCNAME - address of last word in vocabulary VOCNAME
;; until one calls endvoc

createvoc macro
lstw_\2  = pfa_\1
         create \1,\2
	 jsr dovoc
pfa_\2   db smudge|1,' '
lst_\2   dw 0
vlnk_\2  dw pfa_\1
	endm

;; **********************************
;; -- endvoc VOCNAME
;; Terminates definition of vocabulary VOCNAME
;; This sets the value of pointer at address lst_VOCNAME
;; No words should be added to the vocabulary.

endvoc macro
    	  asc "LYB Forth."
    .loc  = *
    *     = lst_\1
	  dw lstw_\1
    *     = .loc
	 endm

;; **********************************
;; -- createforth
;; Creates the initial vocabulary named FORTH.	

createforth macro
lstw_forth = 0
nfa_forth  db 6|smudge
	   asc "forth"
lfa_forth  = 0
cfa_forth  jsr dovoc
pfa_forth  db smudge|1,' '
lst_forth  db 0
vlnk_forth dw 0
	endm

;; **********************************
;; -- start, end, compile
;; These macros are used to define a forth word.
;; Usage:
;;     create INVOCNAME, WORDNAME
;;     start
;;       compile WORDNAME
;;       compile WORDNAME
;;       ...
;;     end
;; What about constants...


start   macro
	 jsr docol
	endm

end	macro
	 dw endcol
	endm

compile macro
	 dw cfa_\1
	endm


;; **********************************
;; -- docol, endcol, next
;; The forth interpreter engine.
;; The forth data stack is the 6801 stack.


ip  = $80   ; instruction pointer
rp  = $82   ; return stack pointer
dp  = $84   ; used to save the data stack pointer


;; docol - start interpreting a forth thread

docol	ldd ip
	ldx rp
	std 0,x
	dex
	dex
	stx rp
	pulx
	stx ip
	ldx ,x
	jmp ,x

;; endcol -- return from interpreting a forth thread

endcol	ldx rp
	inx
	inx
	stx rp
	ldx ,x
	inx
	inx
	stx ip
	ldx ,x
	jmp ,x

;; next -- return from assembly code primitive	
	
next 	ldx ip
	inx
	inx
	stx ip
	ldx ,x
	jmp ,x

;; dovoc -- undefined yet

dovoc   rts



;; **********************************
;; Forth words

	code
	createforth

	code
	create forth,dup
	 pulx
	 pshx
	 pshx
	 jmp next
	
	code
	create forth,drop
	 pulx
	 jmp next
	
	code
	create forth,ndrop
	 pulx
    .1	 beq .2
         pula
         pula
	 dex
	 bra .1
    .2	 jmp next
    
    	code
	create forth,swap
	 pulx
	 pula
	 pulb
	 pshx
	 pshb
	 psha
	 jmp next
	
	code
	create forth,pick
	 pula
	 pulb
	 lsrd
	 sts dp
	 addd dp
	 std dp
	 ldx dp
	 ldx ,x
	 pshx
	 jmp next
	
	code
	create forth,over
	 pula
	 pulb
	 pulx
	 pshx
	 pshb
	 psha
	 pshx
	 jmp next
	
	code
	create forth,rot
	 pulx
	 stx dp
	 pulx
	 pula
	 pulb
	 pshx
	 ldx dp
	 pshx
	 pshb
	 psha
	 jmp next

	;; USER VOCABULARY
	code
	createvoc forth,uservoc

	code
	create uservoc,dupdrop,precedence	
	start
	compile dup
	compile drop
	end
	

	endvoc uservoc		; Terminate vocabulary USERVOC
	
	endvoc forth		; Terminate vocabulry FORTH
