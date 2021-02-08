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
  CRASM: label.c
  utilisation des labels
  LYB 9/87
*/

#include "dcl.h"
#include <ctype.h>
#include "label.h"
#define LABELARRAYSIZE 100

struct labelarray
{
  int position;
  struct label label[LABELARRAYSIZE];
}* labelarray_inst = NULL;

struct label* lroot = NULL;

/* checklabel(name)
   Retourne TRUE si name est une chaine acceptable pour un label
*/
int checklabel(char* name)
{
  register char c;

  if (strlen(name) > LABLEN)
  {
    return FALSE;
  }

  c = *name;

  if (c == '*' && name[1] == 0)
  {
    return TRUE;
  }

  if (!isalpha(c) && c != '_' && c != '.')
  {
    return FALSE;
  }

  while ((c = *++name) != 0)
  {
    if (!isalnum(c) && c != '_'  && c != '.')
    {
      return FALSE;
    }
  }

  return TRUE;
}

/* searchlabel(name)
   Retourne NULL ou l'adresse du label 'name'
   stocke dans  addrpos  l'adresse du pointeur vers ce label
*/
static struct label** addrpos;
static char localname[LABLEN + 2];

struct label* searchlabel(char* name)
{
  register struct label* q;
  register int r;
  register char* s;
  extern int segment;

  s = localname;

  if (name[0] == '.')
  {
    sprintf(s, "%d", segment);

    while (*s)
    {
      s++;
    }
  }

  while (s - localname < LABLEN)
  {
    if (*name)
    {
      *s++ = toupper(*name++);
    }
    else
    {
      *s++ = 0;
    }
  }

  s = localname;

  q = lroot;
  addrpos = &lroot;

  while (q != NULL && (r = strncmp(s, q->name, LABLEN)) != 0)
  {
    if (r > 0)
    {
      q = * (addrpos = &(q->right)) ;
    }
    else
    {
      q = * (addrpos = &(q->left)) ;
    }
  }

  return q;
}

/* makelabel(name,flags,type)
   Cree et place le label ci dessus.
*/
struct label* makelabel(char* name, unsigned char flags, unsigned char type)
{
  register struct label* q;

  q = searchlabel(name);

  if (q != NULL)
  {
    fatal("XX: makelabel");
  }

  if (labelarray_inst == NULL)
  {
    if ((labelarray_inst = malloc(sizeof * labelarray_inst)))
    {
      memset(labelarray_inst, 0, sizeof(*labelarray_inst));
    }
    else
    {
      fatal("no memory");
    }
  }

  q = &(labelarray_inst->label[ labelarray_inst->position++ ]);

  if (labelarray_inst->position >= LABELARRAYSIZE)
  {
    labelarray_inst = NULL;
  }

  q->left = NULL;
  q->right = NULL;

  *addrpos = q;

  strncpy(q->name, localname, LABLEN);
  q->type  = type;
  q->flags = flags;

  return q;
}

/* findlabel(name)
   Renvoie un pointeur sur le label 'name'. Si necessaire, cree un
   label UNDEF|FORWARD type  L_ADDR: seule reference en avant autorisee.
*/
struct label* findlabel(char* name)
{
  register struct label* q;

  q = searchlabel(name);

  if (q == NULL)
  {
    q = makelabel(name, UNDEF | FORWARD | NOREDEF,
                  asmflags & F_RELATIF ? L_RELATIVE : L_ABSOLUTE);
  }

  q->flags |= USED;
  return q;
}

/* deflabel(name,flags,type,value)
   Definit un label name, de type precise, pour une valeur val1.
   Si ce label existe deja, il doit etre redefini du meme type, et si
   ce type est L_ADDR, il doit etre indefini.
*/
struct label* deflabel(char* name, unsigned char flags, unsigned char type, long value)
{
  register struct label* q;

  q = searchlabel(name);

  if (q == NULL)
  {
    q = makelabel(name, UNDEF, type);
  }

  if (!(q->flags & UNDEF))
  {
    if ((q->flags & NOREDEF) || (flags & NOREDEF))
    {
      if (q->type == L_ABSOLUTE || q->type == L_RELATIVE)
      {
        error("illegal label redefinition");
      }
      else
      {
        error("attempt to redefine a keyword");
      }
    }
  }

  q->flags &= USED | NOREDEF | FORWARD;
  flags    &= ~USED;

  q->type  = type;
  q->flags |= flags;
  q->ptr = NULL;
  q->value = value;
  return q;
}

/* init_label_list
   Ordonne en arbre un tableau de labels
*/
void init_label_list(struct label* array)
{
  register int i;
  register int count;
  register int incr;
  register struct label* q;

  q = array;
  count = 0;

  while (q++->left != (void*) -1)
  {
    count++;
  }

  incr = 1;
  array--;

  while (incr <= count)
  {
    incr <<= 1;
  }

  while (incr > 1)
  {
    i = incr >> 1;

    while (i <= count)
    {
      q = array + i;

      if (searchlabel(q->name) != NULL)
      {
        if (q->flags & UNDEF)
        {
          q->flags &= ~UNDEF;
        }
        else
        {
          warning("user's symbol overrides new keyword");
        }
      }
      else
      {
        *addrpos = q;
        strncpy(q->name, localname, LABLEN);
      }

      i += incr;
    }

    incr >>= 1;
  }
}

/* undeflabels(lroot)
   UNDEF tous les labels de type positif, sauf les FORWARD
*/
void undeflabels(struct label* q)
{
  extern struct label pseudos[];

  if (q)
  {
    if (!(q->flags & NOREDEF) || (q->flags & UNDEF))
    {
      q->flags &= ~FORWARD;
    }

    /* FORWARD ne subsiste que si on a
       - un label non redefinissable,
       - ce label est effectivement defini en avant quelque-part
    */
    q->flags |= UNDEF;
    undeflabels(q->left);
    undeflabels(q->right);
  }

  for (q = pseudos; q->left != (struct label*) - 1; q++)
  {
    q->flags &= ~UNDEF;
  }
}

