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
	CRASM: crasm.c
	noyau central
	LYB 9/87
*/

#include "dcl.h"
#include "version.h"
#include "label.h"

#define SCODE 1
#define LIST  2
#define XREF  4
#define WARN  8
#define INPUT 128


extern struct label *searchlabel();
extern struct label pseudos[];

jmp_buf errorjump;
int errnumber,warnnumber;
int segment;
extern int includelevel, thiscall;
struct label* starlabel;
int llen;
int ppos;
int plen;
int lpos;
int linenumber;
char lineprefix;
char* filename;
char* scodename;
char curline[256];

FILE* file;
FILE* scode;

int asmflags;
int passnumber;
int macrolevel;
int advance;

void 
jmp_buf_copy(jmp_buf out, jmp_buf in)
{
#ifdef JBUFSIZE
  int i;
  for (i=0; i<JBUFSIZE; i++)
    out[i] = in[i];
#else
  memcpy(out, in, sizeof(jmp_buf));
#endif
}


int
main(int argc, char **argv)
{
	int flags;
	char *s;

	passnumber = 0;
	flags= LIST | WARN;
#ifdef DEBUG
	flags |= XREF;
#endif
	if (argc==0)
		fatal("NOT from WB");
	while ( --argc )
	{	if ( **(++argv)=='-' )
		{	for ( s=(*argv)+1; *s; s++ )
				switch ( *s)
				{
				case 's':	/* suppress warnings */
					flags &= ~WARN;
					break;
				case 'l':	/* suppress listing  */
					flags &= ~LIST;
					break;
				case 'x':	/* force XREF	     */
					flags |= XREF;
					break;
				case 'o':	/* output name	     */
					if (!s[1] && argc>1 && !(flags&SCODE) )
					{	flags |= SCODE;
						scodename=*++argv;
						--argc;
					}
					break;
				default:
					syntax("Bad Option(s)");
				}
		}
		else
		{	if ( flags & INPUT )
				syntax("One input only");
			filename=*argv;
			flags |= INPUT;
		};
	}
	if ( flags & INPUT )
                crasm(flags);
	else
		syntax("No input!");
	return 0;
}


void
crasm(int flag)
{
	init_label_list(pseudos);
	file=fopen(filename,"r");
	if ( file==NULL )
		fatal("can't open source file");

	scode = NULL;
	llen=76; plen=66; ppos=0 ; lpos=0;
	
#ifdef DEBUG
	asmflags = F_NOCODE;
#else
	asmflags = F_NOLIST | F_NOWARNING| F_NOERROR | F_NOCODE;
#endif

	pass(1);

#ifdef DEBUG
	outputEOP();
#endif
	asmflags &= ~F_NOERROR;
	if (flag & SCODE)
		asmflags &= ~F_NOCODE;
	if (flag & LIST)
		asmflags &= ~F_NOLIST;
	if (flag & WARN)
		asmflags &= ~F_NOWARNING;
	

	
	undeflabels(lroot);
	pass(2);

	if ( scode )
		closescodefile();
	
	asmflags  |= F_LIST_ON;

	if (errnumber==0 && flag & LIST)
		flag |= XREF;

	result();
	if (errnumber && (flag & SCODE) )
		remove(scodename);
		
	if (flag & XREF)
		xref();
}



void 
pass(int n)
{
	clearerr(file);
	rewind(file);

	asmflags |= F_CODE_ON;
	if ( ! (asmflags & F_NOLIST) )
		asmflags |= F_LIST_ON;
#ifdef DEBUG
	asmflags |= F_IFLIST_ON | F_MACROLIST_ON;
#else
	asmflags &= ~ F_MACROLIST_ON;
	asmflags |=   F_IFLIST_ON;
#endif	
	macrolevel	=0;
	thiscall	=0;
	segment		=0;
	includelevel	=0;
	linenumber	=0;
	lineprefix	=0;
	
	starlabel = findlabel("*");
	starlabel->flags |= UNDEF;
	starlabel->flags &= ~(NOREDEF|FORWARD);
	
	
	passnumber=n;
        fprintf(stderr,"Pass #%d\n",n);
        
	errnumber=warnnumber=0;
	
	while ( !feof(file) )
	{	if (setjmp(errorjump) )
			errnumber++;
                linegets(curline,250);
		if (asmline(curline,3))
			error("ELSE, ENDC, ENDM, EXITM illegal here");
	}
}


static void
herelabel(char *label)
{
	if ( strcmp(label,"*") == 0 )
		error ( "illegal star definition");
	if ( asmflags & F_ORG_GV )
		deflabel(label,NOREDEF,
			 asmflags & F_RELATIF ? L_RELATIVE : L_ABSOLUTE,
			 starlabel->value );
	else
		error ( "undefinable label: no org given" );
	
	if ( !(asmflags & F_CODE_ON) )
		if ( macrolevel==0 || asmflags & F_MACROLIST_ON )
	 		if ( lpos==0 )
			{	position(2);
				outputaddr(starlabel->value);
			};

}

static struct label *
findmnemo(char *mnemo)
{
	char *aux1,*aux2;
	register struct label *labmnemo;
	
	labmnemo = searchlabel(mnemo);
	if ( labmnemo == NULL )		/* sizer 68k */
	{	if ( filter(mnemo,"?.?",&aux1,&aux2) )
			if (( labmnemo = searchlabel(aux1) ))
				if (labmnemo->modifier >=0)
					labmnemo = NULL;
	};
	if ( labmnemo )
		if ( !(labmnemo->flags & UNDEF ) )
			if ( labmnemo->type == L_MNEMO ||
				labmnemo->type == L_MACRO )
					return labmnemo;
	return NULL;
}


static int 
nospacein(char *s)
{
 	while( *s )
	{	if ( *s == ' ' || *s == '\t' )
			return FALSE;
		s++;
	}
	return TRUE;
}


int
asmline(char *s, int status)
{
	char *label,*mnemo,*oper;
	register struct label *labmnemo;
	
	advance    = 0;
	if (starlabel->flags & UNDEF)
		asmflags &= ~F_ORG_GV;
	else
	{	asmflags |= F_ORG_GV;
		setpc ( starlabel->value );
	};
	
	zerobuffer();

/* on traite
  XXX ; REMARK
   puis les cas
  XXX = blanc
*/
	if ( filter( s,"_?_;?",&label,&mnemo ) )
		s=label;
	else
	{ 	register char *p;
	  	p=s;
	  	while(*p) p++;
	  	while ( p>s && (*--p==' ' || *p=='\t') ) *p=0;
	  	while (*s==' ' || *s=='\t') s++;
	}
	
	if (*s==0)
	{	if (status & 1)
			outputline();
		return 0;
	}

/*   autorise:
    LABEL= MNEMO
	MNEMO
  LABEL MNEMO
  	MNEMO OPERAND
  LABEL MNEMO OPERAND
*/

	if (filter ( s, "?_=_?", &label,&oper ) && nospacein(label) )
	{	mnemo="equ";
		labmnemo=findmnemo(mnemo);
		if (checklabel(label) == FALSE )
			error("malformed label");
	}
	else
	{	label = mnemo = oper = NULL;	
	start:	if (filter(s,"? _?",&mnemo,&oper) && *mnemo && *oper)
		{	labmnemo = findmnemo(mnemo);
			if ( labmnemo==NULL && label==NULL )
			{	label=mnemo;
				if (checklabel(label) == FALSE )
					error("malformed label");
				s=oper;
				goto start;
			}
		}
		else
		{	labmnemo=findmnemo(mnemo=s);
			oper = NULL;
		}
	}
	
	if ( labmnemo==NULL )
		error( "unknown macro or mnemonic");		
	if ( labmnemo->flags & NOLABEL )
		if (label)
			error("labels not allowed here");
	if ( labmnemo->flags & DEFLABEL )
	{	if (!label)
			error("label required here");
	}
	else if (label)
	{	if (status & 2)
			herelabel(label);			
	}

	if ( labmnemo->flags & DEFCOND )
		return	(*labmnemo->ptr)(	status,
						label,mnemo,oper    );
			
	if ( labmnemo->flags & DEFMACRO )
		return	(*labmnemo->ptr)(	status,
						label,mnemo,oper    );

	if ( labmnemo->type == L_MACRO )
	{	if (label && oper && !strncmp(oper,"MACRO",5))
			error("macro is already defined");
		if (status & 2)
			macrocall( labmnemo,status,oper );
		else if (status & 1)
			outputline();
		return 0;
	}
	else if ( labmnemo->type == L_MNEMO )
	{	if (status & 2)
			(*labmnemo->ptr)(	labmnemo->modifier,
						label,mnemo,oper    );
		starlabel->value += advance;
		if ( status & 1 )
			outputline();
		return 0;
	}
	else
		fatal ("proc asmline failure");
	return 0;
}


/*   F_LIST_ON est pris en compte dans outputline()
     F_CODE_ON est pris en compte dans insertbyte()
     F_NOLIST  est pris en compte dans LIST ON
     F_NOCODE  est pris en compte dans CODE ON ou OFF
     
     F_NOERROR   dans  error
     F_NOWARNING dans  warning
*/



/*
	ERRORS ROUTINES
*/

void
error(char *s)
{
	char raw[80];
	int oldflags;
	char c = toupper(*s);
	
	oldflags=asmflags;
	
	if ( !(asmflags & F_NOERROR) )
	{	sprintf(raw,"%7d ERROR: %c%s",linenumber,c,s+1);
		for ( s=raw; s[1]==' '; s++ )
			s[0]='>';
		asmflags |= F_LIST_ON;
		outputraw(raw);
	}
	outputline();
	
	if ( advance != 0 )
		starlabel->value += advance;
	
	asmflags = oldflags;
	longjmp( errorjump, -1 );
}

void
warning(char *s)
{
	char raw[80];
	char c = toupper(*s);
	if ( !(asmflags & F_NOWARNING) )
	{	sprintf(raw,"%7d WARNING: %c%s",linenumber,c,s+1);
		for ( s=raw; s[1]==' '; s++ )
			s[0]='>';
		outputraw(raw);
	}
}


void 
fatal(char *s)
{
	printf("FATAL ERROR: %s\n",s);
	exit(10);
}


void
fileerror(char *s)
{
  printf("FILE ERROR on %s file: %s\n", s, strerror(errno));
}
	


/*  aff CPU known  */

extern struct cpulist {  char *name;
		  int code;
		  int  (*init)(); } cpulist[];


void
syntax(char *s)
{
	register struct cpulist *q;
	register int(*a)();
	
	a=NULL;
	printf ("%s\n",s);
	printf ("Syntax:  crasm [-slx] [-o SCODEFILE] INPUTFILE\n");
 	printf ("Crasm %s known CPUs:",CRASMVERSION);
	for ( q=cpulist; q->name ; q++ )
	{	if (a!=q->init)
			printf("\n\t");
		printf(" %s",q->name);
		a=q->init;
	}
	printf("\n");
	exit(10);
}


void
setflag(int f)
{
	asmflags |= f;
}

void
clrflag(int f)
{
	asmflags &= ~f;
}
