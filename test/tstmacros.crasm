;;; Author: Leon Bottou
;;; Public Domain.

	nam essai macro
	page 0,132
	
  depart = $1000
  fin    = $2000
  fcb = db

	
  lda	macro
         fcb $ad
         ddb \1
	endm

  ref	macro
  .L     dw \1
         if \1>0
	   ref \1-1
	 endc
	 dw .L
	endm
 
  final macro
  	  dw \#
	  if \#>=2
	    dl \1,\2
	    exitm
	  endc
	  asc "encore"
	endm
	
	page
	
   * = depart+fin
  
  	mlist on
	
 start  lda depart
	ref 4
	
	asc "espoir"
	final depart
 
 	mlist off
	
	ref 4
 end	final depart,fin
	ds  2
	ds  100,3

 enfin  asc "je m'interesse encore au calcul"
	asc " et a l'affichage des trees.\0"
