/*  Copyright (c) 1987 Leon Bottou
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
	CRASM: scode.c
	sorties du scode
	LYB 9/87
*/


#include "dcl.h"

unsigned long pc;
unsigned long codelen;
unsigned long rawstart;
int rawnumber;
int rawextended;
char raw[16];
extern int includelevel;

void
setpc(long unsigned int addr)
{
	if ( addr != pc )
	{	flushscoderaw();
		rawstart=pc=addr;
		rawnumber=0;
	}
}

void
insert8(unsigned char x)
{
	if ( ! (asmflags & F_ORG_GV) )
		error ( "no org given");
	if ( ! (asmflags & F_NOCODE) )
	{	if ( scode == NULL )
		{
			scode = fopen ( scodename,"w" );
			if ( scode == NULL )
				fatal("can't open scode file");
			rawextended = 0;
		}
	}
	if ( asmflags & F_CODE_ON )
	{	if ( scode != NULL )
		{	if ( rawnumber >= 16 )
				flushscoderaw();
			raw[rawnumber++]=x;
		}
		codelen++;
		outputbyte(x);
	}
	advance++;
	pc++;
	
}

void
insert16(short unsigned int x)
{
	if (asmflags & F_LOHI)
	{	insert8(x);
		insert8(x>>8);
	}
	else
	{	insert8(x>>8);
		insert8(x);
	}
}

void
insert32(unsigned int x)
{
	if (asmflags & F_LOHI)
	{	insert16(x);
		insert16(x>>16);
	}
	else
	{	insert16(x>>16);
		insert16(x);
	}
}

static unsigned char checksum;

static void
scodebyte(unsigned char b)
{
	checksum += b;
		
	fputc( hexa[ (b>>4) & 0xf ] , scode );
	fputc( hexa[   b & 0xf    ] , scode );
	if ( ferror(scode) )
		fileerror(scodename);
}


void
closescodefile(void)
{
	flushscoderaw();
	if ( asmflags & F_CODE_HEX )
		fputs(":00000001FF\n",scode);
	else
		fputs("S9030000FC\n",scode);
	if ( scode )
		fclose(scode);
}

void
flushscoderaw(void)
{
	register int i;
	
	if ( rawnumber )
	{
		if ( asmflags & F_CODE_HEX )
		{	
			if ((rawextended ^ rawstart) & 0xffff0000)
			{
				fputs(":02000004",scode);
	        		checksum = (unsigned char)(0xFF + 0x02 + 0x04);
				scodebyte(rawstart>>24);
				scodebyte(rawstart>>16);
				scodebyte(~checksum);
				fputc('\n',scode);
				rawextended = rawstart;
			}
			fputc(':',scode);
	        	checksum = 0xFF;
			scodebyte(rawnumber);
			scodebyte(rawstart>>8);
			scodebyte(rawstart);
			scodebyte(0x00);
		}
		else 
		{	fputc('S',scode);
	        	checksum = 0;
			if (rawstart & 0xff000000)
			{
				fputc('3',scode);
				scodebyte(rawnumber+5);
				scodebyte(rawstart>>24);
				scodebyte(rawstart>>16);
			} 
			else if (rawstart & 0xff0000)
			{
				fputc('2',scode);
				scodebyte(rawnumber+4);
				scodebyte(rawstart>>16);
			}
			else
			{
				fputc('1',scode);
				scodebyte(rawnumber+3);
			}
			scodebyte(rawstart>>8);
			scodebyte(rawstart);
		}
		for ( i=0; i<rawnumber; i++ )
			scodebyte( raw[i] );
		scodebyte  ( ~ checksum );
		fputc('\n',scode);
	}
	rawstart=pc;
	rawnumber=0;
}


