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
/*  CRASM.C stdvocabulary.c
    definition des pseudos standard
*/

#include "dcl.h"
#include "label.h"

#define defpseudo(n,f,c)  {0,0,n,f|NOREDEF,L_MNEMO,0,c,0},
#define defspecial(n,f,c,m) {0,0,n,f|NOREDEF,L_MNEMO,m,c,0},

struct label pseudos[] =
{
  defpseudo("EQU", DEFLABEL, Xequ)
  defpseudo("ASC", 0, Xasc)
  defpseudo("ALIGN", 0, Xalign)
  defpseudo("CLIST", NOLABEL, Xclist)
  defpseudo("CODE", NOLABEL, Xcode)
  defpseudo("CPU", NOLABEL, Xcpu)
  defspecial("DB", 0, Xdc, 1)
  defspecial("DDB", 0, Xdc, 2)
  defspecial("DL", 0, Xdc, 4)
  defspecial("DW", 0, Xdc, 3)
  defpseudo("DUMMY", NOLABEL, Xdummy)
  defpseudo("DS", 0, Xds)
  defpseudo("ELSE", NOLABEL  | DEFCOND, Xelse)
  defpseudo("ENDC", NOLABEL  | DEFCOND, Xendc)
  defpseudo("ENDM", NOLABEL  | DEFMACRO, Xendm)
  defpseudo("EXITM", NOLABEL  | DEFMACRO, Xexitm)
  defpseudo("FAIL", NOLABEL, Xfail)
  defpseudo("IF", NOLABEL  | DEFCOND, Xif)
  defpseudo("ILIST", NOLABEL, Xilist)
  defpseudo("INCLUDE", NOLABEL, Xinclude)
  defpseudo("LIST", NOLABEL, Xlist)
  defpseudo("MACRO", DEFLABEL | DEFMACRO, Xmacro)
  defpseudo("MLIST", NOLABEL, Xmlist)
  defpseudo("NAM", NOLABEL, Xnam)
  defpseudo("PAGE", NOLABEL, Xpage)
  defpseudo("OUTPUT", NOLABEL, Xoutput)
  defpseudo("SKIP", NOLABEL, Xskip)
  { (void*) -1, (void*) -1, {0}, 0, 0, 0, 0, 0 }
};

