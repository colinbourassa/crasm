/*  Copyright (C) 1987- Leon Bottou
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
/*
	CRASM: macro.c
	gestion des macros et des pseudos conditionnelles
	LYB 9/87
*/

#include "dcl.h"
#include "label.h"
#include <setjmp.h>
#include <ctype.h>

#define MAXSOURCEDEEP 10

extern jmp_buf errorjump;
extern int errnumber;



/* code des instructions */

/* if, macro retournent 0 */

#define ENDC	0x11
#define ELSE	0x12
#define ENDM	0x21
#define EXITM	0x22

/*  if, else, endc  */

int
Xelse(int status, char *label, char *mnemo, char *oper)
{
	if ( oper )
		error ( "no operand allowed for ELSE");
	return ELSE;
}


int
Xendc(int status, char *label, char *mnemo, char *oper)
{
	if ( oper )
		error ( "no operand allowed for ENDC");
	return ENDC;
}

#define EQ 1
#define GT 2
#define LT 4
#define NE 8

static struct compar {	char *comp;
			int  flag;  }  compar[] = 
{	{ "?_>=_?",	GT|EQ, },
	{ "?_<=_?",	LT|EQ, },
	{ "?_==_?",	EQ, },	/* habitues du C */
	{ "?_!=_?",	NE, },
	{ "?_=_?",	EQ, },
	{ "?_<>_?",	NE, },
	{ "?_<_?",	LT, },
	{ "?_>_?",	GT, },
	{ NULL,		NE }
};

static struct result zero = { 0,L_ABSOLUTE,0,0 };

static int
testtrue (char *oper)
{
	char *s1,*s2;
	register struct compar *q;
	struct result r1,r2;
	
	if ( !oper || !*oper )
		return FALSE;

	q=compar;
	while ( q->comp )
	{	if ( filter(oper,q->comp,&s1,&s2) )
			break;
		q++;
	};

	if ( !q->comp )
	{	r1=*parse(oper);
		r2=zero;
	}
	else
	{	r1= *parse(s1);
		r2= *parse(s2);
	}

	if ( ( (r1.flags & UNDEF) && (r1.flags & FORWARD) ) ||
	     ( (r2.flags & UNDEF) && (r2.flags & FORWARD) )  )
		error( "forward defined expressions are illegal here" );
	if (r1.type != r2.type)
		error( "Uncomparable expressions" );


	if ( q->flag & (GT | LT) )
	{	if ( (r1.type != L_ABSOLUTE) && (r1.type != L_RELATIVE) )
			error ( "not a numeric expression" );
		if ( (q->flag & GT) && (r1.value > r2.value) )
			return TRUE;
		if ( (q->flag & LT) && (r1.value < r2.value) )
			return TRUE;
	}			
	if ( q->flag & EQ )
		if (    r1.modifier == r2.modifier 
		     &&	r1.value == r2.value )
				return TRUE;

	if ( q->flag & NE )
		if (    r1.modifier != r2.modifier 
		     || r1.value != r2.value )
				return TRUE;
	
	return FALSE;
}

int
Xif(int status, char *label, char *mnemo, char *oper)
{
	int flag;
	int mystatus;
	int booleanvalue;
	char oldprefix;
	jmp_buf errjmpsav;
	
	
	booleanvalue = FALSE;
	if ( status&2 )
		booleanvalue = testtrue(oper);

	mystatus=0;
	if (  booleanvalue )
		mystatus|=3;
	if (  asmflags & F_IFLIST_ON )
		mystatus|=1;

	if ( asmflags & F_IFLIST_ON )
		if (status & 1)
			outputline();
			
	oldprefix = lineprefix;
	if ( status & 2)
		if ( ! (mystatus & 2) )
			lineprefix = 'C';

	jmp_buf_copy(errjmpsav,errorjump);
	do
	{	if (setjmp(errorjump) )
			errnumber++;
		linegets(curline,250);
		if (feof (file) )
		{	jmp_buf_copy(errorjump,errjmpsav);
			error("Unexpected End of File");
		}
		flag=asmline(curline,status & mystatus);
		if ( !(status & mystatus & 2) )
			if ( flag==EXITM )
				flag=0;
	} while ( flag == 0);
	jmp_buf_copy(errorjump,errjmpsav);
	lineprefix = oldprefix;
	

	if ( flag == ENDM )
		error ("ENDM illegal here");
	if ( asmflags & F_IFLIST_ON )
		if (status & 1)
			outputline();
	if ( flag == ENDC )
		return 0;
	if ( flag == EXITM )
		return EXITM;
		
	mystatus = 0;
	if ( !booleanvalue )
		mystatus |= 3;
	if ( asmflags & F_IFLIST_ON )
		mystatus |= 1;
	
	if ( status & 2)
		if ( ! (mystatus & 2) )
			lineprefix = 'C';

	do
	{	if (setjmp(errorjump) )
			errnumber++;
		linegets(curline,250);
		if (feof (file) )
		{	jmp_buf_copy(errorjump,errjmpsav);
			error("Unexpected End of File");
		}
		flag=asmline(curline,status & mystatus);
		if ( !(status & mystatus & 2) )
			if ( flag==EXITM )
				flag=0;
	} while ( flag == 0 );
	jmp_buf_copy(errorjump,errjmpsav);
	lineprefix = oldprefix;

	if ( flag == ENDM || flag == ELSE)
		error ("ELSE or ENDM illegal here");
	if ( asmflags & F_IFLIST_ON )
		if (status & 1)
			outputline();
	if ( flag == ENDC )
		return 0;
	if ( flag == EXITM )
		return EXITM;
        return 0;
}

/*  MACRO, ENDM, EXITM	*/
/*  On aborde la gestion des Macros	*/


struct macroline {	struct macroline *next;
			char line[1 /* variable */ ];
		 };
		 
		 
int
Xexitm(int status, char *label, char *mnemo, char *oper)
{
	if (oper)
		error ( "no operand allowed in EXITM statement");
	if (!macrolevel && (status & 1))
		outputline();
	return EXITM;
}

int
Xendm(int status, char *label, char *mnemo, char *oper)
{
	if (oper)
		error ( "no operand allowed in ENDM statement");
	if (!macrolevel && (status & 1))
		outputline();
	return ENDM;
}

int
Xmacro(int status, char *label, char *mnemo, char *oper)
{
	jmp_buf errjmpsav;
	char *s1,*s2;
	struct macroline *startmacro;
	register struct macroline **where;
	char mname[LABLEN+1];
	struct label *lbl = 0;
	
	where = &startmacro;
	strncpy(mname,label,LABLEN);
	mname[LABLEN]=0;
	
	if ( oper )
		error ( "no operand allowed in macro statement" );
	if ( macrolevel )
		error ( "nested macro definition");
	if ( status & 1 )
		outputline();
	if ( status & 2 )
	{	lbl = deflabel(mname,0,L_MACRO,0);
		lbl->ptr = 0;
	}
	jmp_buf_copy(errjmpsav,errorjump);
	do
	{	if (setjmp(errorjump) )
			errnumber++;
		zerobuffer();
		linegets(curline,250);
		if (feof (file) )
		{	jmp_buf_copy(errorjump,errjmpsav);
			error("Unexpected End of File");
		}
		if ( !filter(curline,"?_;?",&s1,&s2) )
			s1=curline;
		if (status & 2)
		{
			*where= ( struct macroline *) 
				malloc(	sizeof( struct macroline ) +
					strlen( s1 ) );
			(*where)->next = NULL;
			strcpy ( (*where)->line , s1 );
			where = & ( (*where)->next );
		};
		if (status & 1)
			outputline();
	} while ( ! filter(s1,"_?_ENDM_?",&s1,&s2 ) );

	jmp_buf_copy(errorjump,errjmpsav);
	if ( *s1 || *s2 )
		error ("illegal ENDM instruction");
	if ( lbl )
		lbl->ptr = (void*)startmacro;
	return 0;
		
}


/*   linegets(buffer,length)
     access principal
*/

union {  byte type;
	 struct { byte type;
		  byte oldlist,oldprefix;
		  int  oldlinnum;
	 	  FILE *file;			} filedesc;
	 struct { byte type;
	 	  byte oldlist,oldprefix;
		  int  oldlinnum;
		  char (*replace)[60];
		  int  numarg;
	 	  struct macroline *mlineptr;	} macrodesc;
       } source[MAXSOURCEDEEP];

int sourcelevel=-1;
int includelevel;

#define S_END   0
#define S_FILE  1
#define S_MACRO 2

void
linegets(char *buffer, int length)
{
	register int slevel;
	slevel=sourcelevel;
	if ( slevel<0 )
	{	slevel=sourcelevel=0;
		source[slevel].type = S_FILE;
		source[slevel].filedesc.file = file;
	};
	
	while ( source[slevel].type == S_END )
	{
		if ( slevel==0 )
		{	source[slevel].type = S_FILE;
			break;
		};
		linenumber=source[slevel].filedesc.oldlinnum;
		lineprefix=source[slevel].filedesc.oldprefix;
		asmflags &= ~ F_LIST_ON;
		if ( source[slevel].filedesc.oldlist )
			asmflags |= F_LIST_ON;
		includelevel--;
		fclose( source[slevel].filedesc.file );
		sourcelevel=--slevel;
	};
	if ( source[slevel].type == S_FILE )
	{
		register char *s1,*s2;
		buffer[0] = 0;
		fgets(buffer,length,source[slevel].filedesc.file);
		s1=s2=buffer;
		while ( *s1 && *s1!='\n' && s1-buffer<length )
                  {	if ( *s1=='\t' || isprint((unsigned char)*s1) 
                             || !isascii((unsigned char)*s1) )
				*s2++=*s1;
			s1++;
		}
		*s2=0;
		if ( s1!=s2 )
			warning("Unprintable characters removed");
		linenumber++;
                if ( (linenumber & 7)==1 && passnumber==1) {
                    fprintf(stderr,"%d    \r",linenumber);
                    fflush(stderr);
                }
		if ( feof( source[slevel].filedesc.file ) )
			source[slevel].type = S_END;
	}
	else
	{
		register struct macroline *q;
		int numarg;
		char (*replace)[60];
		register char *s1,*s2;
		
		q=source[slevel].macrodesc.mlineptr;
		numarg=source[slevel].macrodesc.numarg;
		replace=source[slevel].macrodesc.replace;
		s1 =buffer;
		s2 =q->line;
		if (!q)
			fatal("internal macrocall error");
		source[slevel].macrodesc.mlineptr=q->next;
		while ( *s2 )
		{	if ( s1-buffer > length-10 )
			{	warning ( "Too long line" );
				break;
			}
			if ( *s2 != '\\' )
			{	*s1++=*s2++;
				continue;
			};
			
			s2++;
			if ( *s2 == '#' )
			{	s2++;
				*s1++= numarg+'0';
			}
			else if ( *s2>='1' && *s2<='9' )
			{	register char *s;
				s= replace[ *s2-'1' ];
				if ( *s2++ <= numarg+'0')
					while ( *s )
					{	if (s1-buffer>length-10)
							break;
						*s1++=*s++;
					}
			}
			else
				*s1++='\\';
		}
		*s1=0;			
	}
}


/* include   */

int
Xinclude(int modifier, char *label, char *mnemo, char *oper)
{
	FILE *f;
	register int slevel;
	
	if ( !oper)
		error("no filename");
	f=fopen(oper,"r");
	if ( f==NULL )
		error("can't open include file");

	slevel=sourcelevel+1;
	if ( slevel >= MAXSOURCEDEEP )
		fatal ( "too many nested INCLUDEs and MACROs" );
	source[slevel].type     = S_FILE;
	source[slevel].filedesc.oldlinnum=linenumber;
	source[slevel].filedesc.oldprefix=lineprefix;
	source[slevel].filedesc.oldlist = (asmflags&F_LIST_ON) ? 1 : 0;
	source[slevel].filedesc.file  = f;
	
	if ( asmflags & F_LIST_ON )
		outputline();

	linenumber = 0;
	sourcelevel=slevel;
	includelevel++;
	if ( !(asmflags & F_INCLUDELIST_ON ) )
		asmflags &= ~F_LIST_ON;
	
	return 0;
}
/* macrocall */

int thiscall;
extern int segment;

int
macrocall(struct label *labmacro, int status, char *oper)
{
	char replace[9][60];
	register int slevel,flag;
	int numarg,oldsegment,mystatus;
	jmp_buf errjmpsav;
	char curlinesav[256];

	labmacro->flags |= USED;
	strcpy(curlinesav,curline);
	slevel=sourcelevel+1;
	if ( slevel >= MAXSOURCEDEEP )
		fatal ( "too many nested INCLUDEs and MACROs" );
	source[slevel].type     = S_MACRO;
	source[slevel].macrodesc.oldlinnum=linenumber;
	source[slevel].macrodesc.oldprefix=lineprefix;
	source[slevel].macrodesc.oldlist = (asmflags&F_LIST_ON) ? 1 : 0;
		
	numarg=0;
	oldsegment=segment;
	
	while ( oper && *oper )
	{	char *arg1,*arg2;
		if ( numarg > 8 )
			error ( "Too many arguments in a macro call" );
		if ( filter ( oper,"?_,_?",&arg1,&arg2 ) )
		{	oper = arg2;
			strncpy(replace[numarg],arg1,59);
			replace[numarg++][59]=0;
		}
		else
		{	strncpy(replace[numarg],oper,59);
			replace[numarg++][59]=0;
			oper = NULL;
		};
	};
	
	source[slevel].macrodesc.mlineptr =
			(struct macroline * ) labmacro->ptr;
	source[slevel].macrodesc.numarg   = numarg;
	source[slevel].macrodesc.replace  = replace;
	
	if ( asmflags & F_MACROLIST_ON )
		if ( status & 1 )
			outputline();
	
	lineprefix = 'M';
	sourcelevel=slevel;
	macrolevel++;
	segment=++thiscall;
	
	mystatus = 2;
	if ( (asmflags & F_MACROLIST_ON ) )
		mystatus |= 1;
	
	jmp_buf_copy(errjmpsav,errorjump);
	do
	{	if (setjmp(errorjump) )
			errnumber++;
		linegets(curline,250);
		flag=asmline(curline,status & mystatus);
	} while ( flag == 0  );
	macrolevel--;
	linenumber=source[slevel].filedesc.oldlinnum;
	lineprefix=source[slevel].filedesc.oldprefix;
	asmflags &= ~ F_LIST_ON;
	if ( source[slevel].filedesc.oldlist )
		asmflags |= F_LIST_ON;
	sourcelevel=--slevel;
	segment=oldsegment;
	jmp_buf_copy(errorjump,errjmpsav);
	
	if ( flag != ENDM && flag != EXITM )
		error("ELSE or ENDC illegal here");

	if ( !(asmflags & F_MACROLIST_ON ) )
	{	strcpy(curline,curlinesav);
		if ( status & 1)
			outputline();
	};
	return 0;
}

