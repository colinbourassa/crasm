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
/*  CRASM.C: pseudos.c
    execution des pseudos standard ( sauf IF..MACRO..CPU )
*/

#include "dcl.h"
#include "label.h"
#include "cpu.h"

extern int thiscall;


/*  label = value  */
int Xequ(int modifier, char* label, char* mnemo, char* oper)
{
  struct result* r;
  struct label* l;

  r = parse(oper);
  r->flags &= ~(UNDEF | USED | NOREDEF);

  if (r->type == L_ABSOLUTE || r->type == L_RELATIVE)
  {
    if (macrolevel == 0 || asmflags & F_MACROLIST_ON)
    {
      if (lpos == 0)
      {
        position(2);
        outputaddr(r->value);
      }
    }
  }

  l = deflabel(label, r->flags, r->type, r->value);

  l->modifier = r->modifier;

  l->ptr = r->ptr;

  return 0;
}


void checktype(struct result* r, int type)
{
  if (r->type == type)
  {
    return;
  }

  switch (r->type)
  {
  case L_MNEMO:
    error("illegal use of a mnemonic");

  case L_MACRO:
    error("illegal use of a macro");

  case L_ABSOLUTE:
    error("illegal use of an absolute reference");

  case L_RELATIVE:
    error("illegal use of a relative reference");

  case L_REGS:
    error("illegal use of registers");

  case L_DIRECTBIT:
    error("illegal use of direct bit reference");

  default:
    error("Bad expression type");
  }
}


static void stocke(char* s, int modifier)
{
  unsigned long mask = 0;
  struct result* r;

  r = parse(s);
  checktype(r, L_ABSOLUTE);

  switch (modifier)
  {
  case 1:
    mask = 0xffffff00;
    insert8(r->value);
    break;

  case 2:
    mask = 0xffff0000;
    asmflags ^= F_LOHI;
    insert16(r->value);
    asmflags ^= F_LOHI;
    break;

  case 3:
    mask = 0xffff0000;
    insert16(r->value);
    break;

  case 4:
    mask = 0;
    insert32(r->value);
    break;
  }

  if (r->value & mask)
  {
    warning("Operand overflow");
  }
}


/*  DCB DCW DCL datas */
int Xdc(int modifier, char* label, char* mnemo, char* oper)
{
  char* s1;
  char* s2;

  while (filter(oper, "?_,_?", &s1, &s2))
  {
    oper = s2;
    stocke(s1, modifier);
  }

  stocke(oper, modifier);
  return 0;
}


/* DS size ou DS size,fill  */
int Xds(int modifier, char* label, char* mnemo, char* oper)
{
  struct result* r;
  register int i;
  register int init;
  char* s1;
  char* s2;

  if (filter(oper, "?_,_?", &s1, &s2))
  {
    oper = s1;
    r = parse(s2);
    checktype(r, L_ABSOLUTE);
    init = r->value;

    if (init & 0xffffff00)
    {
      warning("multiple byte initialiser");
    }
  }
  else
  {
    init = 0;
  }

  r = parse(oper);
  checktype(r, L_ABSOLUTE);
  i = r->value;

  while (i--)
  {
    insert8(init);
  }

  advance = r->value;
  return 0;
}


/*  NAM  title  */
int Xnam(int modifier, char* label, char* mnemo, char* oper)
{
  if (!oper)
  {
    error("need an operand");
  }

  settitle(oper);
  return 0;
}


/*  ASC string  */
int Xasc(int modifier, char* label, char* mnemo, char* oper)
{
  register char* s;
  register char r;
  register char delimiter;

  s = oper;
  delimiter = *s;

  if (delimiter != '\'' && delimiter != '\"')
  {
    error("Bad operand syntax");
  }

  while ((r = *++s) != delimiter)
  {
    if (r == '\\')
    {
      switch (*++s)
      {
      case 't':
        r = '\t';
        break;

      case 'n':
        r = '\n';
        break;

      case 'r':
        r = '\r';
        break;

      case '0':
        r = 0;
        break;

      case '\'':
      case '\"':
      case '\\':
        r = *s;
        break;

      default:
        error("Bad \\X character");
      }
    }

    insert8(r);
  }

  if (*++s)
  {
    error("syntax error");
  }

  return 0;
}


/*  CODE or DUMMY  */

extern int segment;

int Xcode(int modifier, char* label, char* mnemo, char* oper)
{
  if (oper && *oper)
  {
    error("no operand for CODE pseudo");
  }

  segment = ++thiscall;
  asmflags |= F_CODE_ON;
  return 0;
}

int Xdummy(int modifier, char* label, char* mnemo, char* oper)
{
  if (oper && *oper)
  {
    error("no operand for DUMMY pseudo");
  }

  segment = ++thiscall;
  asmflags &= ~ F_CODE_ON;
  return 0;
}


/* LIST MLIST CLIST ILIST ON ou OFF  */
int Xlist(int modifier, char* label, char* mnemo, char* oper)
{
  if (filter(oper, "_ON_"))
  {
    if (!(asmflags & F_NOLIST))
    {
      asmflags |= F_LIST_ON;
    }
  }
  else if (filter(oper, "_OFF_"))
  {
    asmflags &= ~ F_LIST_ON;
  }
  else
  {
    error("please: use option ON or OFF");
  }

  return 0;
}


int Xmlist(int modifier, char* label, char* mnemo, char* oper)
{
  if (filter(oper, "_ON_"))
  {
    asmflags |=  F_MACROLIST_ON;
  }
  else if (filter(oper, "_OFF_"))
  {
    asmflags &= ~ F_MACROLIST_ON;
  }
  else
  {
    error("please: use option ON or OFF");
  }

  return 0;
}


int Xclist(int modifier, char* label, char* mnemo, char* oper)
{
  if (filter(oper, "_ON_"))
  {
    asmflags |=  F_IFLIST_ON;
  }
  else if (filter(oper, "_OFF_"))
  {
    asmflags &= ~ F_IFLIST_ON;
  }
  else
  {
    error("please: use option ON or OFF");
  }

  return 0;
}


int Xilist(int modifier, char* label, char* mnemo, char* oper)
{
  if (filter(oper, "_ON_"))
  {
    asmflags |=  F_INCLUDELIST_ON;
  }
  else if (filter(oper, "_OFF_"))
  {
    asmflags &= ~ F_INCLUDELIST_ON;
  }
  else
  {
    error("please: use option ON or OFF");
  }

  return 0;
}


/*  PAGE ou PAGE plen,llen   */
int Xpage(int modifier, char* label, char* mnemo, char* oper)
{
  char* s1;
  char* s2;
  register struct result* r;
  register int plen;

  if (!oper)
  {
    outputEOP();
    return 0;
  }

  if (!filter(oper, "?_,_?", &s1, &s2))
  {
    error("syntax: PAGE or PAGE plen,llen ");
  }

  checktype(r = parse(s1), L_ABSOLUTE);
  plen = r->value;
  checktype(r = parse(s2), L_ABSOLUTE);
  setpage(plen, r->value);
  return 0;
}


/* Xskip  SKIP nn    |  SKIP PAGE */
int Xskip(int modifier, char* label, char* mnemo, char* oper)
{
  register struct result* rr;
  register int r;

  checktype(rr = parse(oper), L_ABSOLUTE);
  r = rr->value;

  if (r > 100 || r < 0)
  {
    warning("are you sure ?");
  }
  else
  {
    while (r--)
    {
      outputEOL();
    }
  }

  return 0;
}


/* Xoutput  OUTPUT SCODE | OUTPUT HEX */
int Xoutput(int modifier, char* label, char* mnemo, char* oper)
{
  asmflags &= ~(F_CODE_HEX | F_CODE_SCODE);

  if (filter(oper, "_HEX_"))
  {
    asmflags |= F_CODE_HEX;
  }
  else if (filter(oper, "_SCODE_"))
  {
    asmflags |= F_CODE_SCODE;
  }
  else
  {
    error("please, use options SCODE or HEX");
  }

  return 0;
}


/* Xalign ALIGN EVEN | ALIGN ODD   */
int Xalign(int modifier, char* label, char* mnemo, char* oper)
{
  extern unsigned long pc;

  if (filter(oper, "_EVEN_"))
  {
    if (pc & 1)
    {
      insert8(0);
    }
  }
  else if (filter(oper, "_ODD_"))
  {
    if (!(pc & 1))
    {
      insert8(0);
    }
  }
  else
  {
    error("please, use options EVEN or ODD");
  }

  return 0;
}


/*  CPU cpuname */
int Xcpu(int modifier, char* label, char* mnemo, char* oper)
{
  struct cpulist* q;
  char* a;
  extern struct cpulist cpulist[];

  if (!oper)
  {
    error("Need CPU name");
  }

  if (asmflags & F_CPU_GV)
  {
    error("One CPU only!");
  }

  q = cpulist;

  while (q->name)
  {
    if (filter(oper, q->name, &a, &a, &a, &a))
    {
      break;
    }
    else
    {
      q++;
    }
  }

  if (! q->name)
  {
    error("Unknown CPU name");
  }

  if (passnumber == 2)
  {
    asmflags |= F_CPU_GV;
  }

  (*q->init)(q->code);
  return 0;
}


/* FAIL error */
int Xfail(int modf, char* label, char* mnemo, char* oper)
{
  if (oper && *oper)
  {
    error(oper);
  }
  else
  {
    error("Fail instruction");
  }

  return 0;
}

