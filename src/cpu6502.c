/*  Copyright (c) 1987 Leon Bottou
 * 
 *  This is free documentation; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 * 
 *  The GNU General Public License's references to "object code"
 *  and "executables" are to be interpreted as the output of any
 *  document formatting or typesetting system, including
 *  intermediate and printed output.
 * 
 *  This manual is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public
 *  License along with this manual. Otherwise check the web site
 *  of the Free Software Foundation at http://www.fsf.org.
 */
/* 6502, 65C02 */

#include "cpu.h"

#define mnemo2(n,t,b,f)		mnemo( n, standard, (t<<8)|b|f )
#define F_noimm			(1<<12)
#define F_noacc			(1<<13)
#define F_noindx		(1<<14)
#define F_noindy		(1<<15)

/* n: name
   t: instruction type
   f: special remarks about the type
   b: base address   : opcode = b+offset
*/

#define m2_offset		(offsettable[ (code>>8) & 0xf ])
#define m2_base			(code & 0xff)


static int cpuflag;

static int
branch(int code, char *label, char *mnemo, char *oper) /* relative branchs */
                         
{
	byte d;
	int  dd;
	extern long pc;
	struct result *r;
	
	r=parse(oper);				/* operand search 	     */
	checktype(r,L_ABSOLUTE);		/* is it a number?	     */
	dd=r->value-pc-2;			/* displacement calc	     */
	d=dd;					/* -> signed byte	     */
	insert8(code);				/* generate code	     */
	insert8(dd);
	if ( d!=dd )				/* test operand after:	     */
		error ("too long branch");	/* if an error occurs during */
	return 0;				/* the first pass, branch is */
}						/* always too bytes long     */


	/* IMPORTANT:							 *
	*  You had better generate code before signaling errors about    *
	*  the operand values. So the first pass calculates right        *
	*  forward references even when a ghost error occurs.	 	 */


static int
single(int code, char *label, char *mnemo, char *oper) /* single byte instructions  */
                         
{
	insert8(code);
	if (oper)				/* no operand ! 	     */
		error ("no operands allowed");
	return 0;
}



/* addressing modes description  */

static struct addmodes { char *filter;
			 short num;	  } addmodes[] = 

{	{ "(_?_,_X_)",	0,},		/* indirect pre-indexed */
	{ "@_?_,_X",	0,},		/* special syntax */
	{ "(_?_)_,_Y",	2,},		/* indirect post-indexed */
	{ "@_?_,_Y",	2,},
	{ "(_?_)",	4,},		/* indirect */
	{ "@_?",	4,},
	{ "_",		14,},		/* implicit */
	{ "#_?",	12,},		/* immediate */
	{ "?_,_X",	6,},		/* indexed by x */
	{ "?_,_Y",	8,},		/* indexed by y */
	{ "?",		10,},		/* direct or register */
	{ NULL,}
};

/* add 1 to mode for 16bits operands */
/* mode 1 and 4 are 65c02 specific */

/* offsettable  by type */

static unsigned char offsettable[][16]= {
/*
(.,x)     (.),y     (.)       .,x       .,y         .       #.        _
    (..,x)    (..),y     (..)      ..,x      ..,y      ..        #..       A
*/
{0x01,0x00,0x11,0x00,0x12,0x00,0x15,0x1d,0x00,0x19,0x05,0x0d,0x09,0x00,0x00,0x00,},
{0x00,0x00,0x00,0x00,0x00,0x00,0x16,0x1e,0x16,0x1e,0x06,0x0e,0x02,0x00,0x00,0x0a,},
{0x00,0x00,0x00,0x00,0x00,0x00,0x16,0x00,0x16,0x00,0x06,0x0e,0x00,0x00,0x00,0x00,},
{0x00,0x00,0x00,0x00,0x00,0x00,0x16,0x1e,0x00,0x00,0x06,0x0e,0x6b,0x00,0x00,0x3a,},
{0x00,0x00,0x00,0x00,0x00,0x00,0x16,0x1e,0x00,0x00,0x06,0x0e,0x00,0x00,0x00,0x7a,},
{0x00,0x7c,0x00,0x00,0x00,0x6c,0x00,0x00,0x00,0x00,0x00,0x4c,0x00,0x00,0x00,0x00,},
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x20,0x00,0x00,0x00,0x00,},
{0x00,0x00,0x00,0x00,0x00,0x00,0x74,0x9e,0x00,0x00,0x64,0x9c,0x00,0x00,0x00,0x00,},
};

/* 0: ora and eor adc lda cmp sbc (noimm):sta
   1: (noimm|noindy):asl,rol,lsr,ror   (noacc|noindx|noindy):cpx,cpy
      (noacc|noindx):ldx   (noacc|noindy):ldy
   2: (noindx)stx (noindy):sty
   3: (noimm):inc (noacc):bit			{ INC A and DEC A are too   }
   4: (noimm):dec (noacc|noindx):tsb,trb	{ strange to group together }
   5: jmp	{ modes:  ABS    (ABS)    (ABS,X)			    }
   6: jsr	{ only ABS						    }
   7: stz	{ very strange opcodes for  ZP   ABS    ZP,X    ABS,X	    }

*/


static int
standard(int code, char *label, char *mnemo, char *oper)
{
	register int mode,opcode;
	char *toparse;
	int  value = 0;
	
	/* search mode */
	if (! oper) 
	{
          	mode = 14;
	}
	else
	{	register struct addmodes *q;
	
		for ( q=addmodes; q->filter; q++ )
			if ( filter( oper,q->filter,&toparse) )
				break;
		if (! q->filter)
			error("unknown addressing mode");
		
		mode=q->num;
	}
	
	/* special case */
	if (	( (code & F_noacc) && mode >= 14 )
	   ||   ( (code & F_noimm) && mode == 12 )
	   ||	( (code & F_noindx) && mode == 6 )
	   ||	( (code & F_noindy) && mode == 8 )  )
	   		error("illegal addressing mode");
	
	/* extract value */
	if (mode < 14)
	{	register struct result *r;
		r=parse(toparse);
		if ( mode == 10 && r->type == L_REGS && r->modifier == 0 )
			mode = 15;
		else
			checktype(r,L_ABSOLUTE);
			
		value = r->value;
		if ( (value & 0xFF00) || (r->flags & FORWARD) )
		{	if (mode != 12)
				mode |= 1;
			else if (value & 0xFF00)
				warning("operand overflow");
		}
	}
	
	/* generate opcode */ 	
	{	register int offset;

		if (! (offset=m2_offset[mode]) )        /* if not found      */
			mode |= 1;			/* test long version */
		if (! (offset=m2_offset[mode]) )
			error("illegal addressing mode");

		insert8( opcode= (m2_base+offset) & 0xFF );
	}
	
	/* generate operand */
	if (mode < 14)
	{	if (mode & 1)
			insert16(value);
		else
			insert8(value);
	}
	/* test specials modes */

	if (cpuflag < 1)
	{	if ( code== ( 0x60 | 0x100 | F_noimm | F_noindy ) )
			error("no ROR instruction on 6500");
	};
	
	if (cpuflag < 2)
	{	if ( mode==4 || opcode == 0x7c || opcode == 0x89 ||
		     opcode == 0x1a || opcode == 0x3a 			)
				error("illegal use of 65C02 addressing mode");
	};
	return 0;
}



startmnemos(m6502)				/* 6502 mnemonics	      */

mnemo	("bpl",		branch,		0x10)	/* branchs		      */
mnemo	("bmi",		branch,		0x30)
mnemo	("bvc",		branch,		0x50)
mnemo	("bvs",		branch, 	0x70)
mnemo	("bcc",		branch, 	0x90)
mnemo	("bcs",		branch, 	0xb0)
mnemo	("bne",		branch, 	0xd0)
mnemo	("beq",		branch, 	0xe0)
						/* pseudos branchs */
mnemo	("bhi",		branch, 	0xb0)   /* A>=M unsigned */
mnemo	("blo",		branch, 	0x90)	/* A< M unsigned */ 
mnemo	("bgt",		branch,		0x10)   /* A>=M signed	 */
mnemo	("ble",		branch,		0x30)	/* A< M signed	 */


mnemo	("nop",		single,		0xea)	/* single byte instructions   */
mnemo	("brk",		single,		0x00)
mnemo	("rti",		single,		0x40)
mnemo	("rts",		single,		0x60)
mnemo	("pha",		single,		0x48)
mnemo	("pla",		single,		0x68)
mnemo	("php",		single,		0x08)
mnemo	("plp",		single,		0x28)
mnemo	("dex",		single,		0xca)
mnemo	("dey",		single,		0x88)
mnemo	("inx",		single,		0xe8)
mnemo	("iny",		single,		0xc8)
mnemo	("tsx",		single,		0xba)
mnemo	("txs",		single,		0x9a)
mnemo	("tax",		single,		0xaa)
mnemo	("txa",		single,		0x8a)
mnemo	("tay",		single,		0xa8)
mnemo	("tya",		single,		0x98)
mnemo	("clc",		single,		0x18)
mnemo	("clv",		single,		0xb8)
mnemo	("cli",		single,		0x58)
mnemo	("cld",		single,		0xd8)
mnemo	("sec",		single,		0x38)
mnemo	("sei",		single,		0x78)
mnemo	("sed",		single,		0xf8)

mnemo2	("ora",		0,0x00	,0				)
mnemo2	("and",		0,0x20	,0				)
mnemo2	("eor",		0,0x40	,0				)
mnemo2	("adc",		0,0x60	,0				)
mnemo2	("sta",		0,0x80	,F_noimm			)
mnemo2	("lda",		0,0xa0	,0				)
mnemo2	("cmp",		0,0xc0	,0				)
mnemo2	("sbc",		0,0xe0	,0				)

mnemo2	("asl",		1,0x00	,F_noimm|F_noindy		)
mnemo2	("rol",		1,0x20	,F_noimm|F_noindy		)
mnemo2	("lsr",		1,0x40	,F_noimm|F_noindy		)
mnemo2	("ror",		1,0x60	,F_noimm|F_noindy		)

mnemo2	("ldx",		1,0xa0	,F_noacc|F_noindx		)
mnemo2	("ldy",		1,0x9e	,F_noacc|F_noindy		)
mnemo2	("cpx",		1,0xbe	,F_noindy|F_noindx|F_noacc	)
mnemo2	("cpy",		1,0xde	,F_noindy|F_noindx|F_noacc	)

mnemo2	("stx",		2,0x80	,F_noindx			)
mnemo2	("sty",		2,0x7e	,F_noindy			)

mnemo2	("inc",		3,0xe0	,F_noimm			)
mnemo2	("bit",		3,0x1e	,F_noacc			)
mnemo2	("dec",		4,0xc0	,F_noimm			)

mnemo2	("jmp",		5,0x00	,0				)
mnemo2	("jsr",		6,0x00	,0				)


regs	( "a",	0)				/* 6502 registers.	      */
regs	( "x",	1)				/* for register mode ops as   */
regs	( "y",	2)				/* ASL A, ROL A etc..	      */
regs	( "p",	3)
regs	( "s",	4)

endmnemos


startmnemos(m65c02)

mnemo	("phx",		single,		0xda)	/* 65c02 special mnemonics     */
mnemo	("plx",		single,		0xfa)
mnemo	("phy",		single,		0x5a)
mnemo	("ply",		single,		0x7a)

mnemo	("bra",		branch,		0x80)

mnemo2	("stz",		7,0x00	,0				)
mnemo2	("tsb",		4,0xfe	,F_noacc|F_noindx		)
mnemo2	("trb",		4,0x0e	,F_noacc|F_noindx		)

endmnemos



void
init6502(int code)
{
	cpuflag=code;
	setflag( F_ADDR16 );			/* 16 bits address	      */
	setflag( F_LOHI );			/* LSB first		      */
	clrflag( F_RELATIF );			/* no translatable code	      */
	
	bindvocabulary(m6502);			/* add 6502 mnemos	      */
	if (code==2)
		bindvocabulary(m65c02);		/* add 65c02 mnemos if needed  */
}
