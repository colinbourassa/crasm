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
/* Z80 */

#include "cpu.h"

static int offset,offsetflag,indexflag;
static int arg, arg8flag, arg16flag;


#define isreg8(n)  (n>=0 && n<8)	/* registres a 8 bits  */
#define isreg16(n) (n>=8 && n<16)	/* registres a 16 bits */
#define isspecialreg(b) (n>=16)		/* registres speciaux  */

#define IX 0xdd
#define IY 0xfd
#define HL 0xff

#define ACCUMULATOR  7
#define MREGISTER    6
#define HLREGISTER  10
#define MEMORY      -1
#define IMDATA      -2
#define SPBC	    -4
#define SPDE	    -5

#define CLRALL	indexflag=offsetflag=arg8flag=arg16flag=0

static char *needreg8 = "operates with reg8 or (HL),(IX+nn),(IY+nn)";
static char *needacc  = "operates on A only";
static char *badop    =	"illegal addressing mode";



/*  GETREG prend un registre ou un mode d'addressage  */

static char *indexed[] = 
{ "(_IX_)", "(_IY_)", "(_IX_+_?_)", "(_IY_+_?_)", "(_IX_-_?_)", "(_IY_-_?_)" };

static int
getreg(char *oper)
{
	register struct result *r;
	register int i;
	char *oper2;
	
	for (i=0; i<6; i++)
		if (filter (oper,indexed[i],&oper2))
		{	offset = 0;	
			if ( i>1)
			{	r=parse(oper2);
				checktype(r,L_ABSOLUTE);
				offset = r->value;
				if (i>3)
					offset = -offset;
			};

			if (indexflag || offsetflag )
				error(badop);
			offsetflag=TRUE;
			indexflag = i&1 ? IY : IX;
			return MREGISTER;
		};
	if (filter(oper,"(_HL_)"))
	{	if (indexflag && indexflag != HL)
			error(badop);
		indexflag=HL;
		return MREGISTER;
	}
	if (filter(oper,"(_BC_)"))
		return SPBC;
	if (filter(oper,"(_DE_)"))
		return SPDE;
	if (filter(oper,"(_?_)",&oper2))
	{	r=parse(oper2);
		checktype(r,L_ABSOLUTE);
		if (arg8flag || arg16flag)
			error(badop);
		arg16flag=TRUE;
		arg=r->value;
		return MEMORY;
	}
	r=parse(oper);
	if (r->type == L_ABSOLUTE)
	{	if (arg8flag || arg16flag)
			error(badop);
		arg16flag=TRUE;
		arg=r->value;
		return IMDATA;
	};
	checktype(r,L_REGS);
	if ( r->value != (1 << r->modifier) )
		error ("illegal register list");

	if (r->modifier==12 || r->modifier==13 || r->modifier==HLREGISTER )
	{	switch (r->modifier)
		{
		case 12: i=IX;	break;
		case 13: i=IY;	break;
		default: i=HL;	break;
		}
		if (indexflag && indexflag != i)
			error(badop);
		indexflag=i;
		return HLREGISTER;
	}
	return r->modifier;
}



static int
docode(int code)
{
	if (indexflag==IX || indexflag==IY || offsetflag )
		insert8(indexflag);
	if (code & 0xff00)
		insert8(code>>8);
	else
		insert8(code);
	if (offsetflag)
	{	insert8(offset);
		if (offset != (byte) offset)
			warning ("offset overflow");
	}
	if (code & 0xff00)
		insert8(code);
	if (arg8flag)
	{	insert8(arg);
		if (arg & 0xff00)
			warning ("argument overflow");
	}
	else if (arg16flag)
		insert16(arg);
	return 0;
}



static int
relbranch(int code, char *label, char *mnemo, char *oper) /* relative branchs */
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
	*  forward references even when a ghost error occurs.		*/



static int
simple(int code, char *label, char *mnemo, char *oper) /* simple instruction */
{
	if (code & 0xff00)
		insert8(code>>8);
	insert8(code);
	if (oper)				/* no operand ! 	     */
		error ("no operands allowed");
	return 0;
}

static int
im(int code, char *label, char *mnemo, char *oper)
{
	register struct result *r;
	r=parse(oper);
	checktype(r,L_ABSOLUTE);
	switch (r->value)
	{
	case 0:		insert16(0x46ed); break;
	case 1:		insert16(0x56ed); break;
	case 2: 	insert16(0x5eed); break;
	default:	insert16(0);
			error( "Bad interruption mode" );
	}
	return 0;
}

static int
rst(int code, char *label, char *mnemo, char *oper)
{
	register struct result *r;
	r=parse(oper);
	checktype(r,L_ABSOLUTE);
	if (! (r->value & 0xfff8))
		r->value <<= 3;
	insert8( 0xc7 | r->value );
	if ( r->value & 0xffc7 )
		error("Bad RST vector");
	return 0;
}


static char *condcodes[] =
{ "NZ","Z","NC","C","PO","PE","P","M",NULL };

static int 
searchcond(int maxcond, char *oper, char **pres)
{	
	register char **q;
	char *ccode;
	
	if ( filter(oper,"?_,_?",&ccode,pres) )
	{	for ( q=condcodes; maxcond--; q++ )
			if ( filter( ccode,*q) )
				return q-condcodes;
		error ("Bad condition code");
	}
	*pres=oper;
	return -1;
}

static int
jr(int code, char *label, char *mnemo, char *oper)
{
	char *operand2;
	code=0x20+ ( searchcond(4,oper,&operand2) << 3 );
	return relbranch(code,label,mnemo,operand2);
}

static int
ret(int code, char *label, char *mnemo, char *oper)
{	
	register char **q;
	
	if ( !oper || !*oper )
		insert8(0xc9);
	else
	{	for ( q=condcodes; *q; q++ )
			if ( filter( oper,*q) )
			{	insert8(0xc0+ ((q-condcodes)<<3) );
				return 0;
			}
		error ("Bad condition code");
	}
	return 0;
}


static int
call(int code, char *label, char *mnemo, char *oper)
{
	char *operand2;
	register int cond;
	register struct result *r;
	
	cond = searchcond(8,oper,&operand2) << 3;
	r=parse(operand2);
	checktype(r,L_ABSOLUTE);
	if (cond >=0 )
		insert8 (0xc4 + cond );
	else
		insert8 (0xcd );
	insert16 (r->value);
	return 0;
}

static int
jp(int code, char *label, char *mnemo, char *oper)
{
	char *operand2;
	register int cond;
	register struct result *r;
	
	CLRALL;
	if ( filter (oper,"(_?_)",&operand2) )
	{	if (getreg(operand2) == HLREGISTER)
			docode(0xe9);
		else
			error("illegal indirection");
	}
	else
	{
		cond = searchcond(8,oper,&operand2) << 3;
		r=parse(operand2);
		checktype(r,L_ABSOLUTE);
		if (cond >=0 )
			insert8 (0xc2 + cond );
		else
			insert8 (0xc3 );
		insert16 (r->value);
	}
	return 0;
}


static int
ex(int code, char *label, char *mnemo, char *oper)
{	
	char *oper2;

	CLRALL;
		
	if (filter (oper,"DE_,_HL") || filter (oper,"HL_,_DE"))
		return docode(0xeb);

	if ( filter(oper, "(_SP_)_,_?" , &oper2) ||
	     filter(oper, "?_,_(_SP_)" , &oper2)  )
		if ( getreg(oper2)==HLREGISTER  )
			return docode(0xe3);

	/* EX AF,AF'
	   ' is a special filter char. So, if there is one in OPER,
	   then OPER is OPER+SPACES+REMARK... We have to manage with  */
	

	if ( filter(oper,"AF_,_AF'_?",&oper2 ) ||
	     filter(oper,"AF'_,_AF_?",&oper2 )    )
		if ( oper2==NULL || !*oper2 || *oper2==';' )
			return docode(0x08);

	error("Illegal EX instruction");
	return 0;	
}


static int
inout(int code, char *label, char *mnemo, char *oper)
{
	char *op1,*op2;
	register int reg;
	
	CLRALL;
	if ( (!(code & 8)) ?
		filter(oper,"(?)_,_?",&op1,&op2)
	      : filter(oper,"?_,_(?)",&op2,&op1)  )
	{
		reg=getreg(op1);
		if (reg==IMDATA)
		{	reg=getreg(op2);
			if (reg != ACCUMULATOR )
				error(needacc);
			arg8flag=TRUE;
			docode(code);
		}
		else if ( reg==1 )
		{	reg=getreg(op2);
			if ( !isreg8(reg) || reg==6 )
				error("operates on reg8 only");
			reg <<= 3;
			if (! (code & 8))
				reg += 1;
			docode(0xed40+reg);
		}
		else
			error("port is (C) or (nn)");
	}
	else
		error(badop);
	return 0;
}

static int
bitop(int code, char *label, char *mnemo, char *oper)
{
	register struct result *r;
	char *oper1,*oper2;
	register int bit,reg;
	
	CLRALL;
	if ( !filter(oper,"?_,_?",&oper1,&oper2))
		error("two operands required");
	r=parse(oper1);
	checktype(r,L_ABSOLUTE);
	bit=r->value;
	if (bit <0 || bit >7 )
		error("illegal bit number");
	reg=getreg(oper2);
	if (! isreg8(reg) )
		error(needreg8);
	docode(code+reg+(bit<<3));
	return 0;
}


static int
onreg8(int code, char *label, char *mnemo, char *oper)
{
	register int reg;

	CLRALL;
	reg=getreg(oper);
	if (!isreg8(reg))
		error(needreg8);
	return docode(code+reg);
}

static int
logical(int code, char *label, char *mnemo, char *oper)
{
	register int reg;
	char *op1,*op2;

	CLRALL;
	if (filter (oper,"?_,_?",&op1,&op2) )
	{	if (getreg(op1) != ACCUMULATOR)
			error(needacc);
		oper=op2;
		warning("One operand only. but i understand!");
	}
	reg=getreg(oper);
	if (reg == IMDATA)
	{	reg=0x46;
		arg8flag=TRUE;
	}
	return docode(code+reg);
}


static int
stackop(int code, char *label, char *mnemo, char *oper)
{
	register int reg;

	CLRALL;
	reg=getreg(oper);
	if (reg==19)
		reg=11;
	else if (reg==11)
		reg=19;
	if (!isreg16(reg))
		error("Operates with AF,BC,DE,HL,IX,IY");
	return docode(code+ (reg<<4) );
}

static int
arith(int code, char *label, char *mnemo, char *oper)
{
	register int reg;
	char *op1,*op2;

	CLRALL;
	if (!filter(oper,"?_,_?",&op1,&op2))
		error(badop);
	reg=getreg(op1);
	
	if (reg==ACCUMULATOR)
		logical(code,label,mnemo,op2);
	else if (reg==HLREGISTER)
	{	if ( indexflag!=HL && code != 0x80 )
			error("illegal index operation");
		switch(code)
		{
		case 0x80:	code=0x09  -0x80; break;
		case 0x88:	code=0xed4a-0x80; break;
		case 0x98:	code=0xed42-0x80; break;
		};
		reg=getreg(op2);
		if (!isreg16(reg) )
			error(badop);
		docode(code+ (reg<<4) );
	}
	else
		error(badop);
	return 0;
}

			
static int
incr(int code, char *label, char *mnemo, char *oper)
{
	register int reg;
	
	CLRALL;
	reg=getreg(oper);

	if (isreg8(reg))
	{	code >>=8;
		docode( (reg<<3) + code);
	}
	else if (isreg16(reg))
	{	code &= 0xff;
		reg -=8;
		docode(code+ (reg<<4));
	}
	else
		error(badop);
	return 0;
}



static int
ld(int code, char *label, char *mnemo, char *oper)
{
	register int reg1,reg2;
	char *op1,*op2;
	
	if (!filter(oper,"?_,_?",&op1,&op2))
		error(badop);
	CLRALL;
	reg1=getreg(op1);
	reg2=getreg(op2);
	
	if ( isreg8(reg1) )
	{	if ( isreg8(reg2) )
		{
			if (reg1==reg2 && reg1==MREGISTER)
				error(badop);
			return docode( 0x40+ (reg1<<3) + reg2 );
		}
		if ( reg2==IMDATA )
		{	arg8flag=TRUE;
			return docode( 0x06 + (reg1<<3) );
		};	
		if (reg1 == ACCUMULATOR)
		{	if ( reg2 == MEMORY )
				return docode(0x3a);
			if ( reg2 == SPBC )
				return docode(0x0a);
			if ( reg2 == SPDE )
				return docode(0x1a);
			if ( reg2 == 17 )
				return docode(0xed57);
			if ( reg2 == 18 )
				return docode(0xed5f);
		};
	};
	if ( isreg16(reg1) )
	{	if (reg2 == IMDATA)
			return docode( 0x01 + ( (reg1-8)<<4 ) );
		if (reg2 == MEMORY )
		{	if (reg1 == HLREGISTER )
				return docode(0x2a);
			else
				return docode(0xed4b+ ((reg1-8)<<4) );
		};
		if (reg2 == HLREGISTER && reg1 == 11 )
				return docode(0xf9);
	}
	if ( reg1 == MEMORY )
	{	if (reg2 == ACCUMULATOR )
			return docode(0x32);
		if (reg2 == HLREGISTER )
			return docode(0x22);
		if ( isreg16(reg2) )
			return docode(0xed43+ ((reg2-8)<<4) );
	};
	if ( reg2 == ACCUMULATOR )
	{	if (reg1 == SPBC)
			return docode(0x02);
		if (reg1 == SPDE)
			return docode(0x12);
		if (reg1 == 17)
			return docode(0xed47);
		if (reg1 == 18)
			return docode(0xed4f);
	}
	error(badop);
	return 0;
}




startmnemos(mz80)				/* Z80 mnemonics	      */

regs	("a",	7)				/* registres */
regs	("b",	0)
regs	("c",	1)
regs	("d",	2)
regs	("e",	3)
regs	("h",	4)
regs	("l",	5)

regs	("bc",	8)
regs	("de",	9)
regs	("hl",	10)
regs	("sp",	11)
regs	("ix",	12)
regs	("iy",	13)

regs	("f",	16)
regs	("i",	17)
regs	("r",	18)
regs	("af",	19)

mnemo	("ccf",		simple,		0x3f  ) /* instructions without oper  */
mnemo	("cpd",		simple,		0xeda9)
mnemo	("cpdr",	simple,		0xed89)
mnemo	("cpi",		simple,		0xeda1)
mnemo	("cpir",	simple,		0xedb1)
mnemo	("cpl",		simple,		0x2f  )
mnemo	("daa",		simple,		0x27  )
mnemo	("exx",		simple,		0xd9  )
mnemo	("halt",	simple,		0x76  )
mnemo	("ind",		simple,		0xedaa)
mnemo	("indr",	simple,		0xedba)
mnemo	("ini",		simple,		0xeda2)
mnemo	("inir",	simple,		0xedb2)
mnemo	("di",		simple,		0xf3  )
mnemo	("ei",		simple,		0xfb  )
mnemo	("ldd",		simple,		0xeda8)
mnemo	("lddr",	simple,		0xedb8)
mnemo	("ldi",		simple,		0xeda0)
mnemo	("ldir",	simple,		0xedb0)
mnemo	("neg",		simple,		0xed44)
mnemo	("nop",		simple,		0x00  )
mnemo	("otdr",	simple,		0xedbb)
mnemo	("otir",	simple,		0xedb3)
mnemo	("outd",	simple,		0xedab)
mnemo	("outi",	simple,		0xeda3)
mnemo	("reti",	simple,		0xed4d)
mnemo	("retn",	simple,		0xed45)
mnemo	("rla",		simple,		0x17  )
mnemo	("rlca",	simple,		0x07  )
mnemo	("rld",		simple,		0xed6f)
mnemo	("rra",		simple,		0x1f  )
mnemo	("rrca",	simple,		0x0f  )
mnemo	("rrd",		simple,		0xed67)
mnemo	("scf",		simple,		0x37  )

mnemo	("im",		im,		0    )	/* Specials 		      */
mnemo   ("rst",		rst,		0    )
mnemo	("djnz",	relbranch,	0x10 )
mnemo	("jr",		jr,		0    )
mnemo	("jp",		jp,		0    )
mnemo	("call",	call,		0    )
mnemo	("ret"	,	ret,		0    )
mnemo	("ex",		ex,		0    )
mnemo	("in",		inout,		0xdb )
mnemo	("out",		inout,		0xd3 )


mnemo	("bit",		bitop,		0xcb40)	/* Quasi regulars	      */
mnemo	("set",		bitop,		0xcbc0)
mnemo	("res",		bitop,		0xcb80)

mnemo	("sla",		onreg8,		0xcb20)
mnemo	("sra",		onreg8,		0xcb28)
mnemo	("srl",		onreg8,		0xcb38)
mnemo	("rl",		onreg8,		0xcb10)
mnemo	("rr",		onreg8,		0xcb18)
mnemo	("rlc",		onreg8,		0xcb00)
mnemo	("rrc",		onreg8,		0xcb08)

mnemo	("or",		logical,	0xb0  )
mnemo	("and",		logical,	0xa0  )
mnemo	("xor",		logical,	0xa8  )
mnemo	("sub",		logical,	0x90  )
mnemo	("cp",		logical,	0xb8  )

mnemo	("push",	stackop,	0xc5-0x80)	
mnemo   ("pop",		stackop,	0xc1-0x80)

mnemo	("adc",		arith,		0x88)  /* no index */
mnemo	("sbc",		arith,		0x98)  /* no index */
mnemo	("add",		arith,		0x80)  /* index ok */

mnemo	("inc",		incr,		0x0403)  /*  (*8)  */
mnemo	("dec",		incr,		0x050b)

mnemo	("ld",	 	ld,		0)       /* heavy magic !! */
endmnemos



void
initz80(int code)
{
  	setflag( F_CODE_HEX );		/* Intel hex by default	      */
	setflag( F_ADDR16 );		/* 16 bits address	      */
	setflag( F_LOHI );		/* LSB first		      */
	clrflag( F_RELATIF );		/* no translatable code	      */
	bindvocabulary(mz80);		/* add Z80 mnemos	      */
}
