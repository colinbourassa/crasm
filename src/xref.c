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
  CRASM: xref.c
  sorties des references croisees
  LYB 9/87
*/

#include "dcl.h"
#include "label.h"

extern unsigned long pc;
extern unsigned long codelen;
extern int segment;

#define RECLEN (LABLEN+13)
char* msgs[] =
{ "Undefined", "Mnemonic", "Macro", "Abs", "Rel", "Register(s)", "Bit%d" };

void printlabel(struct label* label)
{
  char buffer[RECLEN + 2];
  char* s;
  char* b;
  int newpos;
  register int i;

  if (lpos + RECLEN >= llen)
  {
    outputEOL();
  }

  newpos = lpos + RECLEN;

  if (label->type != L_ABSOLUTE && label->type != L_RELATIVE)
  {
    if (label->flags & NOREDEF)
    {
      return;
    }
  }

  if (label->type < 0)
  {
    label->type = 1;
  }

  if (label->name[0] == '*' || label->name[0] <= '9')
  {
    return;
  }

  if ((label->flags & UNDEF) && !(label->flags & FORWARD))
  {
    label->type = 0;
  }

  s = label->name;
  i = label->type;
  i = (i == 3 || i == 4 || i == 6);

  if (!(label->flags & USED))
  {
    outputcar('?');
  }
  else if (label->flags & FORWARD)
  {
    outputcar('^');
  }
  else
  {
    outputcar(' ');
  }

  if (i)
  {
    outputaddr(label->value);
  }

  sprintf(buffer, msgs[label->type], label->modifier);
  position(newpos - LABLEN - 2 - strlen(buffer));
  output(buffer);

  position(newpos - LABLEN - 1);
  b = buffer;

  for (i = 0; i < LABLEN && *s ; i++)
  {
    *b++ = *s++;
  }

  *b = 0;
  output(buffer);
  position(newpos);
}


static void xref2(struct label* label)
{
  if (label)
  {
    xref2(label->left);
    printlabel(label);
    xref2(label->right);
  }
}


void result(void)
{
  extern int errnumber;
  extern int warnnumber;
  char buffer[40];

  if (plen - ppos < 10)
  {
    outputEOP();
    outputEOL();
    outputEOL();
  }

  outputEOL();

  sprintf(buffer, "ERRORS:    %4d", errnumber);

  output(buffer);

  outputEOL();

  sprintf(buffer, "WARNINGS:  %4d", warnnumber);

  output(buffer);

  outputEOL();

  outputEOL();

  if (!errnumber)
  {
    output("Successful assembly...");
    outputEOL();
    sprintf(buffer, " Last address %8lx (%ld)", pc - 1, pc - 1);
    output(buffer);
    outputEOL();
    sprintf(buffer, " Code length  %8lx (%ld)", codelen, codelen);
    output(buffer);
    outputEOL();
  }
  else
  {
    output("No code generated...");
    outputEOL();
  }
}

void xref(void)
{
  outputEOP();
  xref2(lroot);
  outputEOL();
  outputEOP();
}

