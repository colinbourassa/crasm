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
	CRASM: parse.c
	evaluation d'expressions
	LYB 9/87
*/

#include "dcl.h"
#include <ctype.h>
#include "label.h"

extern void error();
extern void fatal();


/*   La suite prouve que les methodes classiques ont
     du bon, et que j'ai eu tort de n'en pas vouloir.
     REVERSE est une horreur imposee par le sens 
     d'unification de FILTER
*/


struct result calcresult;

extern void opadd(struct result *presult, struct result *parg);
extern void opsub(struct result *presult, struct result *parg);
extern void opmul(struct result *presult, struct result *parg);
extern void opdiv(struct result *presult, struct result *parg);
extern void oprlist(struct result *presult, struct result *parg);
extern void opminus(struct result *presult);
extern void opor(struct result *presult, struct result *parg);
extern void opand(struct result *presult, struct result *parg);
extern void opxor(struct result *presult, struct result *parg);
extern void oplsh(struct result *presult, struct result *parg);
extern void oprsh(struct result *presult, struct result *parg);
extern void opnot(struct result *presult);
extern void opbit(struct result *presult, struct result *parg);
extern void csthexa(struct result *presult, char *s);
extern void cstdecimal(struct result *presult, char *s);
extern void cstbinary(struct result *presult, char *s);
extern void cstascii(struct result *presult, char *s);
extern void cstlabel(struct result *presult, char *s);
extern void cstoctal(struct result *presult, char *s);
extern void opbitnumb(struct result *presult, struct result *parg);
extern void opbitaddr(struct result *presult, struct result *parg);

struct oplist 
{	char *filtre;
	short  type,nxt;
	void (*callop)();	/*	(*callop)(&result,&arg )  */
}  oplist[] = {
	/* ICI a l'envers , filtres des operations			*/
	{ "?_\\_?"	,2 ,1	/* rlist	*/		,oprlist,},
	{ "?_+_?"	,2 ,2	/* add		*/		,opadd,},
	{ "?_-_?"	,2 ,1	/* subtract, rrange	*/	,opsub,},
	{ "?_*_?"	,2 ,2	/* multiply	*/		,opmul,},
	{ "?_/_?"	,2 ,1	/* divide	*/		,opdiv,},
	{ "?_|_?"	,2 ,3	/* logical OR	*/		,opor,},
	{ "?_&_?"	,2 ,2	/* logical AND	*/		,opand,},
	{ "?_^_?"	,2 ,1	/* logical XOR	*/		,opxor,},
	{ "?_<<_?"	,2 ,2	/* lshift	*/		,oplsh,},
	{ "?_>>_?"	,2 ,1	/* rshift	*/		,oprsh,},
	{ "?_-"		,1 ,1	/* unary minus	*/		,opminus,},
	{ "?_~"		,1 ,1	/* logical NOT	*/		,opnot,},
	{ ")_?_(_TIB"	,1 ,1   /* bit number   */		,opbitnumb,},
	{ ")_?_(_RDDA"	,1 ,1   /* bit addr     */		,opbitaddr,},
	{ "}_?_{_?"	,2 ,1	/* directbit	*/		,opbit,},
	{ ")_?_("	,-1,1	/* parenthesis	*/		,error,},

	/* ICI on teste les labels					*/
	{ ""		,-3,1	/* label	*/		,cstlabel,},

	/* ICI a l'endroit  constantes autres que ascii ou decimal 	*/
	{ "$?"		,-2,1	/* hexadecimal	*/		,csthexa,},
	{ "?H"		,-2,1	/* Intel hexa	*/		,csthexa,},
	{ "0X?"		,-2,1	/* C hexa	*/		,csthexa,},
	{ "%?"		,-2,1	/* binaire	*/		,cstbinary,},
	{ "?B"		,-2,1	/* Intel binaire*/		,cstbinary,},
	{ "0B?"		,-2,1	/* C binaire 	*/		,cstbinary,},
	{ "?Q"		,-2,1	/* Intel octal	*/		,cstoctal,},
	
	{ NULL	,0 }				};




static void
parse2(char *expr, struct result *presult)
{
	register char *c1 = 0;
	register char *c2 = 0;
	char *ca,*cb;
	register int i,j;
	struct result arg;
	struct oplist *q;

	q=oplist;
	while (q->type)
	{	
		j=-1;
		switch ( q->type )
		{
		case 2:			/* diadiques	*/
			for ( i=0; i<q->nxt; i++ )
			{	ca=cb=NULL;
				if ( filter(expr,q[i].filtre,&ca,&cb) )
					if ( cb<c2 || j<0 )
					{	c1=ca;
						c2=cb;
						j=i;
					};
			};
			if ( j<0 || c1==NULL || c2==NULL || !*c1 || !*c2 )
				break;
			parse2(c2, presult);
			parse2(c1, &arg);
			(q[j].callop)(presult,&arg);
			return;
		case 1:			/* monadiques	*/
			if ( !filter(expr,q->filtre,&ca) || ca==NULL || !*ca )
				break;
			parse2(ca, presult);
			(*q->callop)(presult);
			return;
		case -1:		/* parentheses	*/
			if ( !filter(expr,q->filtre,&ca) || ca==NULL || !*ca )
				break;
			parse2(ca, presult);
			return;
		case -2:		/* constante hex, bin ou octale */
			if ( !filter(expr,q->filtre,&ca) || ca==NULL || !*ca )
				break;
			(*q->callop)(presult,ca);
			reverse(expr);
			return;
		case -3:		/* label */
			reverse(expr);
			if ( checklabel(expr) )
			{	cstlabel (presult,expr);
				reverse(expr);
				return;
			}
			break;
		};
		q += q->nxt;
	}	
	/* Noop: c'est forcement un decimal ou une cst ascii */
	if ( isdigit(*expr) )
	{	cstdecimal(presult,expr);
		reverse(expr);
	} 
	else if (*expr=='\'' || *expr=='\"')
	{	cstascii (presult,expr);
		reverse(expr);
	}
	else
		error ( "syntax error in an expression" );
}

struct result *
parse(char *expr)
{
	char *dummy;
	if (!expr || !*expr)
		error ( "expression expected" );
	if ( filter(expr,"(?)",&dummy) )
		warning("external parenthesis ignored");
	reverse(expr);
	parse2(expr,&calcresult);
	reverse(expr);
	return &calcresult;

}


