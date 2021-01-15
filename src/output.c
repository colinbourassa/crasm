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
  CRASM: output.c
  sorties des listings
  LYB 9/87
*/

#include "dcl.h"
#include "version.h"
#include <ctype.h>

#define NMARGIN 21
#define LMARGIN 28

static int lastoffset;

extern unsigned int chars_consumed_by_filter;
char linebuffer[1000];
char title[31] = "";
int pagenumber = 0;

void settitle(char* s)
{
  static byte flag = 0;

  if (flag)
  {
    warning("title redefinition");
  }

  if (strlen(s) > 30)
  {
    error("too long title");
  }

  strcpy(title, s);

  if (passnumber == 2)
  {
    flag = 1;
  }
}

void setpage(int xplen, int xllen)
{
  static byte flag = 0;

  if (flag)
  {
    warning("page redefinition");
  }

  if (xplen > 200 || (xplen && xplen < 10) || xllen < 39 || xllen > sizeof(linebuffer))
  {
    error("illegal values");
  }

  plen = xplen;
  llen = xllen;

  if (passnumber == 2)
  {
    flag = 1;
  }
}

void outputraw(char* s)
{
  register int i;

  if (asmflags & F_LIST_ON)
  {
    if (ppos == 0)
    {
      printf("Crasm %s:   %30s", CRASMVERSION, title);

      for (i = 58; i < llen; i++)
      {
        putchar(' ');
      }

      printf("page%3d\n\n", ++pagenumber);
      ppos = 2;
    }

    puts(s);

    ppos++;

    if (ppos >= plen - 2 && plen)
    {
      printf("\n\n");
      ppos = 0;
    }
  }

  if (ferror(stdout))
  {
    fileerror("output");
  }
}

void outputEOP(void)
{
  if (asmflags & F_LIST_ON)
  {
    if (plen)
    {
      while (ppos)
      {
        outputEOL();
      }
    }
    else
    {
      printf("\n\n\n");
    }
  }
}

static char* outpat[] = { "_? _? _?", "_? _?", "_?", NULL };

void output(char* s)
{
  while (*s)
  {
    outputcar(*s++);
  }
}

void outputcar(char c)
{
  char* dummy;
  char** pat;

  if (!isspace((unsigned char)c))
  {
    if (lpos >= llen)
    {
      outputEOL();

      if (filter(linebuffer + LMARGIN, "?;_?", &dummy, &dummy))
        /* suite de commentaire */
      {
        position(chars_consumed_by_filter);
      }
      else if (c == ';')
        /* debut de commentaire */
      {
        position(LMARGIN);
      }
      else if (lastoffset >= LMARGIN)
      {
        position(lastoffset);
      }
      else if (c != ';')

        /* suite d'un champ quelconque */
        for (pat = outpat; *pat; pat++)
        {
          if (filter(linebuffer + LMARGIN,
                     *pat,
                     &dummy, &dummy, &dummy))
          {
            position(chars_consumed_by_filter);
            break;
          }
        }

      lastoffset = lpos;
    }

    if (isprint((unsigned char)c) || !isascii((unsigned char)c))
    {
      linebuffer[lpos++] = c;
    }

    linebuffer[lpos] = 0;
  }
  else if (c == ' ')
  {
    if (lpos < llen)
    {
      linebuffer[lpos++] = ' ';
      linebuffer[lpos] = 0;
    }
    else
    {
      lpos++;
    }
  }
  else if (c == '\t')
    do
    {
      outputcar(' ');
    } while ((lpos - LMARGIN) % 8);
}

void position(int n)
{
  if (n >= llen)
  {
    fatal("line length must be greater");
  }

  while (lpos < n)
  {
    outputcar(' ');
  }
}

void outputEOL(void)
{
  outputraw(linebuffer);
  lpos = 0;
  *linebuffer = 0;
}

/* peut etre ameliore pour tabuler automatiquement */

void outputline(void)
{
  char linnum[10];
  register int i;

  lastoffset = 0;

  if (linenumber == 0)
  {
    return;  /* special pour les include.. */
  }

  if (lineprefix)
  {
    sprintf(linnum, "     ");
    i = 5 - macrolevel;

    if (i < 0)
    {
      i = 0;
    }

    if (i > 4)
    {
      i = 4;
    }

    linnum[i] = lineprefix;
  }
  else
  {
    sprintf(linnum, "%5d", linenumber);
  }

  position(NMARGIN);
  output(linnum);
  position(LMARGIN);
  output(curline);
  outputEOL();
}


/* special output */

char hexa[] = "0123456789ABCDEF";
extern unsigned long pc;

void outputbyte(int b)
{
  char buffer[3];
  static unsigned long mypc;

  if (advance > 20 && lpos > NMARGIN - 5)
  {
    if (lpos <= NMARGIN - 3)
    {
      output("...");
    }

    return;
  }

  if (lpos > NMARGIN - 3)
  {
    outputEOL();
  }

  if (pc != mypc && lpos != 0)
  {
    outputEOL();
  }

  if (lpos == 0)
  {
    outputaddr(pc);
  }

  buffer[0] = hexa[(b >> 4) & 0xF];
  buffer[1] = hexa[b & 0xF];
  buffer[2] = 0;
  output(buffer);
  mypc = pc + 1;
}

void outputaddr(unsigned long a)
{
  char buffer[20];
  register int i;
  register char* s;

  s = buffer;
  i = 28;

  if (asmflags & F_ADDR24)
  {
    i = 20;
  }
  else if (asmflags & F_ADDR16)
  {
    i = 12;
  }

  for (; i >= 0; i -= 4)
  {
    *s++ = hexa [(a >> i) & 0xF];
  }

  *s++ = ' ';
  *s  = 0 ;

  output(buffer);
}

