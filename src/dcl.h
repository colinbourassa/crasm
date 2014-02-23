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
	CRASM: dcl.h
	definition generales
	LYB 9/87
*/

#ifndef DCL_H
#define DCL_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <ctype.h>
#include <setjmp.h>


#ifndef TRUE
# define TRUE	1
#endif
#ifndef FALSE
# define FALSE	0
#endif

typedef signed char byte;

/***************************************************************************/

int asmflags;
int passnumber;
int macrolevel;
int advance;

FILE *file,*scode;
char *filename,*scodename;

char curline[256];
int  linenumber;
char lineprefix;

/* asmflags */
#define F_CPU_GV		(1<<0)
#define F_ORG_GV		(1<<1)
#define F_CODE_ON		(1<<2)
#define F_LIST_ON		(1<<3)
#define F_IFLIST_ON		(1<<4)
#define F_MACROLIST_ON		(1<<6)
#define F_INCLUDELIST_ON	(1<<7)
#define F_LOHI			(1<<8)
#define F_ADDR16		(1<<9)
#define F_ADDR24		(1<<10)
#define F_RELATIF		(1<<11)
#define F_CODE_HEX		(1<<12)
#define F_CODE_SCODE		(1<<13)

#define F_NOLIST		(1<<16)
#define F_NOWARNING		(1<<17)
#define F_NOERROR		(1<<18)
#define F_NOCODE		(1<<19)

/* plus CPU defined FLAGS */

int plen,ppos,llen,lpos;


/***************************************************************************/

/* external: crasm.c */
extern void jmp_buf_copy(jmp_buf in, jmp_buf out);
extern void crasm(int flag);
extern void pass(int pass);
extern int  asmline(char *s, int status);
extern void error(char *s);
extern void warning(char *s);
extern void fatal(char *s);
extern void syntax(char *s);
extern void fileerror(char *s);
extern void setflag(int f);
extern void clrflag(int f);

/* external: filter.c */
extern void zerobuffer(void);
extern int  filter(char *s, char *fil, ...);
extern void reverse(char *s);

/* external: scode.c */
extern void flushscoderaw(void);
extern void closescodefile(void);
extern void setpc(long unsigned int addr);
extern void insert8(unsigned char x);
extern void insert16(short unsigned int x);
extern void insert32(unsigned int x);

/* external: output.c */
extern void settitle(char *s);
extern void setpage(int xplen, int xllen);
extern void outputraw(char *s);
extern void outputEOP(void);
extern void output(char *s);
extern void outputcar(char c);
extern void position(int n);
extern void outputEOL(void);
extern void outputline(void);
extern void outputbyte(int b);
extern void outputaddr(unsigned long a);
extern char hexa[];


/* external: macros.c */
extern void linegets(char *buffer, int length);
extern int  Xmacro(int status, char *label, char *mnemo, char *oper);
extern int  Xexitm(int status, char *label, char *mnemo, char *oper);
extern int  Xendm(int status, char *label, char *mnemo, char *oper);
extern int  Xif(int status, char *label, char *mnemo, char *oper);
extern int  Xelse(int status, char *label, char *mnemo, char *oper);
extern int  Xendc(int status, char *label, char *mnemo, char *oper);

/* external: pseudos.c */
extern int  Xequ(int modifier, char *label, char *mnemo, char *oper);
extern int  Xinclude(int modifier, char *label, char *mnemo, char *oper);
extern int  Xdc(int modifier, char *label, char *mnemo, char *oper);
extern int  Xds(int modifier, char *label, char *mnemo, char *oper);
extern int  Xalign(int modifier, char *label, char *mnemo, char *oper);
extern int  Xoutput(int modifier, char *label, char *mnemo, char *oper);
extern int  Xcpu(int modifier, char *label, char *mnemo, char *oper);
extern int  Xnam(int modifier, char *label, char *mnemo, char *oper);
extern int  Xasc(int modifier, char *label, char *mnemo, char *oper);
extern int  Xcode(int modifier, char *label, char *mnemo, char *oper);
extern int  Xdummy(int modifier, char *label, char *mnemo, char *oper);
extern int  Xskip(int modifier, char *label, char *mnemo, char *oper);
extern int  Xpage(int modifier, char *label, char *mnemo, char *oper);
extern int  Xlist(int modifier, char *label, char *mnemo, char *oper);
extern int  Xmlist(int modifier, char *label, char *mnemo, char *oper);
extern int  Xclist(int modifier, char *label, char *mnemo, char *oper);
extern int  Xilist(int modifier, char *label, char *mnemo, char *oper);
extern int  Xfail(int modifier, char *label, char *mnemo, char *oper);






#endif
