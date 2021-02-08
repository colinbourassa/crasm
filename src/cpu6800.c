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
/* 6800, 6802, 6808, 6801, 6803 */

#include "cpu.h"

/* relative branchs */
static int branch(int code, char* label, char* mnemo, char* oper)
{
  byte d;
  int  dd;
  extern unsigned long pc;
  struct result* r;

  r = parse(oper);          /* operand search    */
  checktype(r, L_ABSOLUTE); /* is it a number?   */
  dd = r->value - pc - 2;   /* displacement calc */
  d = dd;                   /* -> signed byte    */
  insert8(code);            /* generate code     */
  insert8(dd);

  if (d != dd) /* test operand after: */
  {
    error("too long branch");  /* if an error occurs during */
  }

            /* the first pass, branch is */
  return 0; /* always too bytes long     */
}


/* IMPORTANT:                                                 *
*  You had better generate code before signaling errors about *
*  the operand values. So the first pass calculates right     *
*  forward references even when a ghost error occurs.         */

/* single byte instructions  */
static int single(int code, char* label, char* mnemo, char* oper) 
{
  insert8(code);

  if (oper) /* no operand! */
  {
    error("no operands allowed");
  }

  return 0;
}

/* addressing modes filters */
static struct addmodes 
{
  char* filt; /* and code shifts */
  int add;
} addmodes[] =
{
  { "#_?",   0x00,},
  { ",_x",   0x20,},
  { "?_,_x", 0x20,},
  { "?",     0x30,},
  { NULL }
};

/* test operands */
static int findmode(char* oper, int* pvalue)    
/* value -> *pvalue  and */
/* if it isn't a forward */
{
  /* reference, try direct */
  register struct addmodes* q; /* addressing */
  register struct result* r;
  char* address;

  address = "0";

  for (q = addmodes; q->filt ; q++)
  {
    if (oper && filter(oper, q->filt, &address))
    {
      r = parse(address);
      checktype(r, L_ABSOLUTE);
      *pvalue = r->value;

      if (q->add == 0x30)
      {
        if (!(r->flags & FORWARD) &&
            !(r->value & 0xff00))
        {
          return 0x10;
        }
      }

      return q->add;
    }
  }

  error("Unknown addressing mode");
  return 0;
}


/* generate code, given base */
static void codemode(int code, int add, int value)          
/* opcode, addressing  mode */
{
  /* shift and operand value */
  insert8(code + add);

  if (!add && /* 16 bits immediate */
      (code == 0x8c || code == 0x8e || code == 0xc3 ||
       code == 0xcc || code == 0xce || code == 0x83))
  {
    insert16(value);
  }
  else if (add == 0x30) /* extended */
  {
    insert16(value);
  }
  else
  {
    insert8(value); /* direct, indexed and */

    if (value & 0xff00) /* 8 bits immediate */
    {
      if (add == 0x20)
      {
        error("too long displacement");
      }
      else
      {
        warning("operand overflow");
      }
    }
  }
}


/* all addressing modes */
static int standard(int code, char* label, char* mnemo, char* oper) 
{
  register int add;
  int value;

  add = findmode(oper, &value);
  codemode(code, add, value);
  return 0;
}


/* all but immediate */
static int standard2(int code, char* label, char* mnemo, char* oper) 
{
  register int add;
  int value;

  add = findmode(oper, &value);
  codemode(code, add, value);

  if (add == 0) /* immediate -> error */
  {
    error("Bad addressing mode");
  }

  return 0;
}


/* only indexed and extended */
static int standard3(int code, char* label, char* mnemo, char* oper)
{
  register int add;
  int value;

  add = findmode(oper, &value);

  if (add == 0x10)
  {
    add = 0x30;  /* direct -> extended */
  }

  codemode(code, add, value);

  if (add <= 0x10) /* immediate -> error */
  {
    error("Bad addressing mode");
  }

  return 0;
}


/* 6800 mnemonics */
startmnemos(m6800)

mnemo("bra",   branch,   0x20)  /* branches */
mnemo("bsr",   branch,   0x8d)
mnemo("bhi",   branch,   0x22)
mnemo("bls",   branch,   0x23)
mnemo("bcc",   branch,   0x24)
mnemo("bcs",   branch,   0x25)
mnemo("bne",   branch,   0x26)
mnemo("beq",   branch,   0x27)
mnemo("bvc",   branch,   0x28)
mnemo("bvs",   branch,   0x29)
mnemo("bpl",   branch,   0x2a)
mnemo("bmi",   branch,   0x2b)
mnemo("bge",   branch,   0x2c)
mnemo("blt",   branch,   0x2d)
mnemo("bgt",   branch,   0x2e)
mnemo("ble",   branch,   0x2f)

mnemo("nop",   single,   0x01)  /* single byte instructions */
mnemo("rti",   single,   0x3b)
mnemo("rts",   single,   0x39)
mnemo("swi",   single,   0x3f)
mnemo("wai",   single,   0x3e)
mnemo("aba",   single,   0x1b)
mnemo("clra",  single,   0x4f)
mnemo("clrb",  single,   0x5f)
mnemo("cba",   single,   0x11)
mnemo("coma",  single,   0x43)
mnemo("comb",  single,   0x53)
mnemo("nega",  single,   0x40)
mnemo("negb",  single,   0x50)
mnemo("daa",   single,   0x19)
mnemo("deca",  single,   0x4a)
mnemo("decb",  single,   0x5a)
mnemo("inca",  single,   0x4c)
mnemo("incb",  single,   0x5c)
mnemo("psha",  single,   0x36)
mnemo("pshb",  single,   0x37)
mnemo("pula",  single,   0x32)
mnemo("pulb",  single,   0x33)
mnemo("rola",  single,   0x49)
mnemo("rolb",  single,   0x59)
mnemo("rora",  single,   0x46)
mnemo("rorb",  single,   0x56)
mnemo("asla",  single,   0x48)
mnemo("aslb",  single,   0x58)
mnemo("asra",  single,   0x47)
mnemo("asrb",  single,   0x57)
mnemo("lsra",  single,   0x44)
mnemo("lsrb",  single,   0x54)
mnemo("sba",   single,   0x10)
mnemo("tab",   single,   0x16)
mnemo("tba",   single,   0x17)
mnemo("tsta",  single,   0x4d)
mnemo("tstb",  single,   0x5d)

mnemo("dex",   single,   0x09)
mnemo("des",   single,   0x34)
mnemo("inx",   single,   0x08)
mnemo("ins",   single,   0x31)
mnemo("tsx",   single,   0x30)
mnemo("txs",   single,   0x35)

mnemo("clc",   single,   0x0c)
mnemo("clv",   single,   0x0a)
mnemo("cli",   single,   0x0e)
mnemo("sec",   single,   0x0d)
mnemo("sev",   single,   0x0b)
mnemo("sei",   single,   0x0f)
mnemo("tap",   single,   0x06)
mnemo("tpa",   single,   0x07)

mnemo("neg",   standard3,  0x40)  /* memory instructions */
mnemo("com",   standard3,  0x43)
mnemo("lsr",   standard3,  0x44)
mnemo("ror",   standard3,  0x46)
mnemo("asr",   standard3,  0x47)
mnemo("asl",   standard3,  0x48)
mnemo("rol",   standard3,  0x49)
mnemo("dec",   standard3,  0x4a)
mnemo("inc",   standard3,  0x4c)
mnemo("tst",   standard3,  0x4d)
mnemo("jmp",   standard3,  0x4e)
mnemo("clr",   standard3,  0x4f)

mnemo("suba",  standard, 0x80)  /* accumulator & memory */
mnemo("cmpa",  standard, 0x81)  /* instructions         */
mnemo("sbca",  standard, 0x82)
mnemo("anda",  standard, 0x84)
mnemo("bita",  standard, 0x85)
mnemo("ldaa",  standard, 0x86)
mnemo("staa",  standard2, 0x87) /* store instructions are */
mnemo("eora",  standard, 0x88)  /* never immediate        */
mnemo("adca",  standard, 0x89)
mnemo("oraa",  standard, 0x8a)
mnemo("adda",  standard, 0x8b)
mnemo("cpx",   standard, 0x8c)
mnemo("jsr",   standard3, 0x8d)
mnemo("lds",   standard, 0x8e)
mnemo("sts",   standard2, 0x8f)

mnemo("subb",  standard, 0xc0)
mnemo("cmpb",  standard, 0xc1)
mnemo("sbcb",  standard, 0xc2)
mnemo("andb",  standard, 0xc4)
mnemo("bitb",  standard, 0xc5)
mnemo("ldab",  standard, 0xc6)
mnemo("stab",  standard2, 0xc7)
mnemo("eorb",  standard, 0xc8)
mnemo("adcb",  standard, 0xc9)
mnemo("orab",  standard, 0xca)
mnemo("addb",  standard, 0xcb)
mnemo("ldx",   standard, 0xce)
mnemo("stx",   standard2, 0xcf)

endmnemos


startmnemos(m6801)

mnemo("lsrd",  single,   0x04)  /* 6801 special mnemonics */
mnemo("asld",  single,   0x05)
mnemo("brn",   branch,   0x21)
mnemo("pulx",  single,   0x38)
mnemo("abx",   single,   0x3a)
mnemo("pshx",  single,   0x3c)
mnemo("mul",   single,   0x3d)

mnemo("subd",  standard, 0x83)
mnemo("addd",  standard, 0xc3)
mnemo("ldd",   standard, 0xcc)
mnemo("std",   standard2,  0xcd)

endmnemos


void init6800(int code)
{
  setflag(F_ADDR16);  /* 16 bit address       */
  clrflag(F_LOHI);    /* MSB first            */
  clrflag(F_RELATIF); /* no translatable code */

  bindvocabulary(m6800); /* add 6800 mnemos   */

  if (code == 1)
  {
    bindvocabulary(m6801);  /* add 6801 mnemos if needed  */
  }
}

