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
/*  CRASM: cpu.h
    include pour les fichiers de definition de CPU
*/

#ifndef CPU_H
#define CPU_H

#include "dcl.h"
#include "label.h"

/***************************************************************************/

#define startmnemos(n)   struct label n[] = {
#define endmnemos        {(void*)-1,(void*)-1,{0},0,0,0,0,0} };
#define mnemo(n,c,p)     {0,0,n,NOREDEF,L_MNEMO,p,c,0},
#define directive(n,c)   {0,0,n,NOREDEF|NOLABEL,L_MNEMO,0,c,0},
#define address(n,a)     {0,0,n,NOREDEF,L_ABSOLUTE,0,a,0},
#define directbit(n,a,b) {0,0,n,NOREDEF,L_DIRECTBIT,b,a,0},
#define regs(n,r)        {0,0,n,NOREDEF,L_REGS,r,0,(1<<r)},

#define bindvocabulary(n) init_label_list(n)

#define startcpu      struct cpulist cpulist[] = {
#define endcpu        {0,0,0} };
#define newcpu(c,i,a) {c,a,i,},


struct cpulist
{
  char*  name;
  int    code;
  void (*init)();
};

#endif

