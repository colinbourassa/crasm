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
/*  Liste des CPUs connus */

#include "cpu.h"

extern void init6800(int code);
extern void init6502(int code);
extern void initz80(int code);

startcpu

newcpu("6800", init6800, 0)  /* Motorola 8 bit family father  */
newcpu("6801", init6800, 1)  /* 6800+rom+I/Os+16bits ALU      */
newcpu("6803", init6800, 1)  /* ROMless 6801                  */
newcpu("6500", init6502, 0)  /* First 6502 procs, without ROR */
newcpu("6502", init6502, 1)  /* Standard 6502 (Apple,Cbm..)   */
newcpu("65C02", init6502, 2) /* CMOS 6502, ext. instr. set    */
newcpu("Z80", initz80, 0)    /* 8080 revisited                */

endcpu

