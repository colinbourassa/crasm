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
  CRASM: operator.c
  evaluation d'expressions
  LYB 9/87
*/

#include "dcl.h"
#include <ctype.h>
#include "label.h"

/****************************************  Usage general */

static void cst(struct result* presult, long int val)
{
  presult->flags = 0;
  presult->type = L_ABSOLUTE;
  presult->value = val;
}

static char overflow[] = "overflow";
static char badtype[]  = "illegal operator";

/****************************************  Constantes */

void cstdecimal(struct result* presult, struct result* parg, char* s)
{
  register unsigned long val;
  register int r;

  val = 0;

  while (*s)
  {
    if (!isdigit(*s))
    {
      crasm_error("bad decimal number");
    }

    r = *s - '0';

    if (val > 0xffffffff / 10)
    {
      crasm_error(overflow);
    }

    val *= 10;

    if (val > 0xffffffff - r)
    {
      crasm_error(overflow);
    }

    val += r;
    s++;
  }

  cst(presult, val);
}


void csthexa(struct result* presult, struct result* parg, char* s)
{
  register unsigned long val;
  register int r;

  val = 0;

  while (*s)
  {
    if (!isxdigit(*s))
    {
      crasm_error("bad hexadecimal number");
    }

    if (*s >= 'a')
    {
      r = *s - 'a' + 10;
    }
    else if (*s >= 'A')
    {
      r = *s - 'A' + 10;
    }
    else
    {
      r = *s - '0';
    }

    if (val > 0xfffffff)
    {
      crasm_error(overflow);
    }

    val = (val << 4) + r;
    s++;
  };

  cst(presult, val);
}


void cstoctal(struct result* presult, struct result* parg, char* s)
{
  register unsigned long val;
  register int r;

  val = 0;

  while (*s)
  {
    if (*s < '0' || *s > '7')
    {
      crasm_error("bad octal number");
    }

    r = *s - '0';

    if (val > 0x1fffffff)
    {
      crasm_error(overflow);
    }

    val = (val << 3) + r;
    s++;
  }

  cst(presult, val);
}


void cstbinary(struct result* presult, struct result* parg, char* s)
{
  register unsigned long val;
  register int r;

  val = 0;

  while (*s)
  {
    if (*s != '1' && *s != '0')
    {
      crasm_error("bad binary number");
    }

    r = *s - '0';

    if (val > 0xefffffff)
    {
      crasm_error(overflow);
    }

    val = (val << 1) + r;
    s++;
  }

  cst(presult, val);
}


void cstascii(struct result* presult, struct result* parg, char* s)
{
  register unsigned long oval;
  register unsigned long val;
  register int r;
  register char delimit;

  val = 0;
  delimit = *s;

  while (*++s != delimit)
  {
    if (!isprint(*s))
    {
      crasm_error("bad ascii constant");
    }

    r = *s;

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
        r = '\0';
        break;

      case '\"':
      case '\\':
      case '\'':
        r = *s;
        break;

      default:
        crasm_error("Bad \\X sequence");
      }
    }

    oval = val;

    val = (val << 8) + r;

    if ((val >> 8) != oval)
    {
      crasm_error(overflow);
    }
  }

  if (*++s)
  {
    crasm_error("syntax error");
  }

  cst(presult, val);
}


void cstlabel(struct result* presult, struct result* parg, char* s)
{
  register struct label* q;

  q = findlabel(s);
  presult->flags    = q->flags;
  presult->type     = q->type;
  presult->value    = q->value;
  presult->modifier = q->modifier;
  presult->ptr      = q->ptr;

  if ((q->flags & UNDEF) && !(q->flags & FORWARD))
  {
    crasm_error("Undefined label");
  }
}

/****************************************  Ops monadiques */

void opminus(struct result* presult, struct result* parg, char* s)
{
  checktype(presult, L_ABSOLUTE);
  presult->value = -presult->value;
}


void opnot(struct result* presult, struct result* parg, char* s)
{
  checktype(presult, L_ABSOLUTE);
  presult->value = ~presult->value;
}


/****************************************  Ops diadiques */

void opadd(struct result* presult, struct result* parg, char* s)
{
  presult->value += parg->value;
  presult->flags |= parg->flags;

  if (presult->type == L_ABSOLUTE
      && parg->type == L_ABSOLUTE)
  {
    presult->type = L_ABSOLUTE;
  }
  else if ((presult->type == L_ABSOLUTE
            && parg->type == L_RELATIVE)
           || (presult->type == L_ABSOLUTE
               && parg->type == L_RELATIVE))
  {
    presult->type = L_RELATIVE;
  }
  else
  {
    crasm_error(badtype);
  }
}


void opsub(struct result* presult, struct result* parg, char* s)
{
  presult->flags |= parg->flags;

  if ((presult->type == parg->type)
      && (presult->type == L_RELATIVE
          || presult->type == L_ABSOLUTE))
  {
    presult->type = L_ABSOLUTE;
  }
  else if (presult->type == L_RELATIVE
           && parg->type == L_ABSOLUTE)
  {
    presult->type = L_RELATIVE;
  }
  else if (presult->type == L_REGS
           && parg->type == L_REGS)
  {
    register int lo = 0;
    register int hi = 0;

    while (1 << lo != presult->value  && lo < 32)
    {
      lo++;
    }

    while (1 << hi != parg->value     && hi < 32)
    {
      hi++;
    }

    if (hi > 31 || hi < lo)
    {
      crasm_error("illegal register list");
    }

    while (lo <= hi)
    {
      presult->value |= 1 << lo;
    }
  }
  else
  {
    crasm_error(badtype);
  }

  if (presult->type != L_REGS)
  {
    presult->value -= parg->value;
  }
}

void opbit(struct result* presult, struct result* parg, char* s)
{
  checktype(presult, L_ABSOLUTE);
  checktype(parg, L_ABSOLUTE);
  presult->flags |= parg->flags;
  presult->type  = L_DIRECTBIT;
  presult->modifier = parg->value;

  if (parg->value > 32)
  {
    crasm_error(overflow);
  }
}

void opbitaddr(struct result* presult, struct result* parg, char* s)
{
  checktype(presult, L_DIRECTBIT);
  presult->type = L_ABSOLUTE;
  presult->modifier = 0;
}

void opbitnumb(struct result* presult, struct result* parg, char* s)
{
  checktype(presult, L_DIRECTBIT);
  presult->type = L_ABSOLUTE;
  presult->value = presult->modifier;
  presult->modifier = 0;
}

void oplo(struct result* presult, struct result* parg, char* s)
{
  checktype(presult, L_ABSOLUTE);
  presult->value &= 0xff;
}

void ophi(struct result* presult, struct result* parg, char* s)
{
  checktype(presult, L_ABSOLUTE);
  presult->value >>= 8;
  presult->value &= 0xff;
}

void opmul(struct result* presult, struct result* parg, char* s)
{
  checktype(presult, L_ABSOLUTE);
  checktype(parg, L_ABSOLUTE);
  presult->flags |= parg->flags;
  presult->value *= parg->value;
}

void opdiv(struct result* presult, struct result* parg, char* s)
{
  presult->flags |= parg->flags;
  checktype(presult, L_ABSOLUTE);
  checktype(parg, L_ABSOLUTE);
  if (parg->value != 0)
  {
    presult->value /= parg->value;
  }
}

void oprlist(struct result* presult, struct result* parg, char* s)
{
  presult->flags |= parg->flags;

  checktype(presult, L_REGS);
  checktype(parg, L_REGS);
  presult->value |= parg->value;
}

void opor(struct result* presult, struct result* parg, char* s)
{
  checktype(presult, L_ABSOLUTE);
  checktype(parg, L_ABSOLUTE);
  presult->flags |= parg->flags;
  presult->value |= parg->value;
}

void opand(struct result* presult, struct result* parg, char* s)
{
  checktype(presult, L_ABSOLUTE);
  checktype(parg, L_ABSOLUTE);
  presult->flags |= parg->flags;
  presult->value &= parg->value;
}

void opxor(struct result* presult, struct result* parg, char* s)
{
  checktype(presult, L_ABSOLUTE);
  checktype(parg, L_ABSOLUTE);
  presult->flags |= parg->flags;
  presult->value ^= parg->value;
}

void oplsh(struct result* presult, struct result* parg, char* s)
{
  checktype(presult, L_ABSOLUTE);
  checktype(parg, L_ABSOLUTE);
  presult->flags |= parg->flags;
  presult->value <<= parg->value;
}

void oprsh(struct result* presult, struct result* parg, char* s)
{
  checktype(presult, L_ABSOLUTE);
  checktype(parg, L_ABSOLUTE);
  presult->flags |= parg->flags;
  presult->value >>= parg->value;
}

void operror(struct result* presult, struct result* parg, char* s)
{
  crasm_error(s);
}

