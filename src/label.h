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
  crasm: label.h
  definition des labels
  LYB 9/87
*/

#ifndef LABEL_H
#define LABEL_H

#define LABLEN 48

struct label
{
  struct label* left;
  struct label* right;
  char name[LABLEN];
  byte flags;
  byte type;
  short modifier;
  int (*ptr)();
  long value;
};

struct result
{
  byte flags;
  byte type;
  short modifier;
  int (*ptr)();
  long value;
};

/* type */
#define L_MNEMO    -1
#define L_MACRO     2
#define L_ABSOLUTE  3
#define L_RELATIVE  4
#define L_REGS      5
#define L_DIRECTBIT 6 /* etc. */

/* flags
modifier < 0 =>         size field allowed 68k-like */

#define UNDEF     (1<<0)
#define FORWARD   (1<<1)
#define USED      (1<<2)
#define NOREDEF   (1<<3)

#define NOLABEL   (1<<4)  /* pour mnemos uniquement */
#define DEFLABEL  (1<<5)
#define DEFMACRO  (1<<6)
#define DEFCOND   (1<<7)

/* external: label.c */
extern int checklabel(char* name);
extern struct label* searchlabel(char* name);
extern struct label* findlabel(char* name);
extern struct result* parse(char* expr);
extern struct label* makelabel(char* name, unsigned char flags, unsigned char type);
extern struct label* deflabel(char* name, unsigned char flags, unsigned char type, long value);
extern void init_label_list(struct label* array);
extern void undeflabels(struct label* q);

/* external: xref.c */
extern void printlabel(struct label* label);
extern void result(void);
extern void xref(void);

/* external: parse.c */
extern struct result* parse(char* expr);

/* external: macros.c */
extern int macrocall(struct label* labmacro, int status, char* oper);

/* external: pseudos.c */
extern void checktype(struct result* r, int type);

/* common */
extern struct label* lroot;
extern struct label* starlabel;

#endif

