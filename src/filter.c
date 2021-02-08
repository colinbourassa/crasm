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
  CRASM:  filter.c
  comparaison et filtrage
  LYB 9/87
*/

#include "dcl.h"

#define WILDCARD '?'    /* n'importe quoi. test () '' "" \x */
#define SPACES   '_'    /* saute X espaces, X>=0    */

char* bufferpos;
char buffer[4000];
unsigned int chars_consumed_by_filter;

/* zerobuffer()
   Efface les buffers de tous les filter(s,f) precedents
*/
void zerobuffer(void)
{
  bufferpos = buffer;
}


/* filter(s,f, ..&{char*}..)
   Retourne TRUE si s correspond au filtre f.
   WILDCARD signifie n'importe quelle chaine,
   SPACES   signifie un nombre quelconque de TABS ou SPACE, un au moins
   tient compte des '' "" et des () imbriquees.
*/
int filter(char* s, char* fil, ...)
{
  va_list ap;
  register char* starts;
  register char* startf;
  register int quote;
  char* oldbufferpos;
  char* str_start = (s) ? s : "";

  s = str_start;
  oldbufferpos = bufferpos;
  starts = NULL;
  startf = NULL;
  va_start(ap, fil);
  quote = 0;

  do
  {
    if (*fil == WILDCARD)
    {
      char** strptr = va_arg(ap, char**);
      *bufferpos = 0;
      *strptr = ++bufferpos;
      chars_consumed_by_filter = (s - str_start);

      if (fil[1] == 0)
      {
        while (*s)
        {
          *bufferpos++ = *s++;
        }

        *bufferpos = 0;
        va_end(ap);
        return TRUE;
      }
      else
      {
        startf = fil;
        starts = s;
      }
    }
    else if (*fil == SPACES)
    {
      while (*s == ' ' || *s == '\t')
      {
        s++;
      }
    }
    else if (!quote &&
             (toupper(*fil) == toupper(*s) ||
              (*fil == ' ' && *s == '\t')))
    {
      s++;
    }

    else
    {
      if (startf == NULL || *s == 0)
      {
        *bufferpos = 0;
        bufferpos = oldbufferpos;
        return FALSE;
      }

      if (quote & 0x4)
      {
        quote &= ~0x4;  /* "\x"  ok  */
      }
      else
      {
        switch (*starts)
        {
        case '\\':
          if (quote & 0x3)        /* str only  */
          {
            quote |= 0x4;
          }
          break;

        case '\'':
          if (!(quote & 0x2))     /* 'x"x' ok  */
          {
            quote ^= 0x1;
          }
          break;

        case '\"':
          if (!(quote & 0x1))     /* "x'x" ok  */
          {
            quote ^= 0x2;
          }
          break;

        case '(' :
          if (!(quote & 0x3))     /* "x(x" ok  */
          {
            quote += 0x8;  /* bonnes () */
          }
          break;

        case ')' :
          if (!(quote & 0x3))     /* "x)x" ok  */
          {
            quote -= 0x8;
          }
          break;
        }
      }

      *bufferpos++ = *starts;
      s = ++starts;
      fil = startf;
    }
  } while (*fil++ != 0);

  *bufferpos = 0;
  va_end(ap);
  return TRUE;
}


void reverse(char* s)
{
  register char* d;
  register char* f;
  register char c;
  register int quote = 0;

  d = s;
  f = s;

  while (*d)
  {
    d++;
  }

  while (--d > f)
  {
    c = *f;
    *f++ = *d;
    *d = c;
  }

  for (d = s; *d; d++)
  {
    switch (*d)
    {
    case '\'':
      if (! quote)
      {
        quote |= 0x1;
      }
      else if (quote & 0x1)
      {
        if (d[1] != '\\')
        {
          quote &= ~0x1;
        }
      }
      break;

    case '\"':
      if (! quote)
      {
        quote |= 0x2;
      }
      else if (quote & 0x2)
      {
        if (d[1] != '\\')
        {
          quote &= ~0x2;
        }
      }
      break;

    case '\\':
      if (quote)
      {
        d[0] = d[-1];
        d[-1] = '\\';
      }
      break;
    }
  }
}

