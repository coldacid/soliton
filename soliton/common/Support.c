/* Soliton cardgames for Amiga computers
 * Copyright (C) 1997-2002 Kai Nickel and other authors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdarg.h>
#include <string.h>

#include <proto/exec.h>

#include "Support.h"

#if defined(__amigaos4__) || defined(__MORPHOS__)
VARARGS68K LONG SPrintf(STRPTR buffer, CONST_STRPTR format,...)
{
    VA_LIST argptr;
    APTR args;

    VA_START(argptr, format);
    args = VA_ARG(argptr, APTR);
    RawDoFmt((STRPTR)format, args, NULL, buffer);
    VA_END(argptr);

    return (LONG)strlen(buffer);
}
#else

VARARGS68K LONG SPrintf(STRPTR buffer, CONST_STRPTR format,...)
{
	static const ULONG cpy_func = 0x16c04e75; /* move.b d0,(a3)+ ; rts */
	RawDoFmt(format, (((ULONG *)&format)+1), (void(*)())&cpy_func, buffer);
	return (int)strlen(buffer);
}

#endif /* __amigaos4__ || __MORPHOS__ */

#ifndef __amigaos4__
struct snprintf_msg
{
	int size;
	char *buf;
};

/************************************************************
 Snprintf function for RawDoFmt()
*************************************************************/
static ASM void snprintf_func(REG(d0,UBYTE chr), REG(a3,struct snprintf_msg *msg))
{
    if (msg->size)
    {
		  *msg->buf++ = chr;
    	msg->size--;
    }
}

#ifdef __MORPHOS__
static void snprintf_gate(void)
{
	UBYTE               chr  = (UBYTE)                REG_D0;
	struct snprintf_msg *msg = (struct snprintf_msg *)REG_A3;

	snprintf_func(chr, msg);
}

static struct EmulLibEntry snprintf_trap = { TRAP_LIB, 0, &snprintf_gate, };
#endif

VARARGS68K LONG SNPrintf(STRPTR buffer,LONG buffer_size, CONST_STRPTR format,...)
{
    struct snprintf_msg msg;
		if (!buffer_size) return 0;

    msg.size = buffer_size;
    msg.buf = buffer;

#ifdef __MORPHOS__
    {
		VA_LIST argptr;
        APTR args;

        VA_START(argptr, format);
        args = VA_ARG(argptr, APTR);
        RawDoFmt(format, args, (void *)&snprintf_trap, &msg);
        VA_END(argptr);
    }
#else
    RawDoFmt(format, (((ULONG *)&format)+1), snprintf_func, &msg);
#endif

    buffer[buffer_size-1] = 0;

    return (int)strlen(buffer);
}

#endif /* __amigaos4__ */

