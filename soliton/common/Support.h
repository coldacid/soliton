#ifndef INCLUDE_SUPPORT_H
#define INCLUDE_SUPPORT_H

#include "SDI_stdarg.h"

VARARGS68K LONG SPrintf(STRPTR buffer, CONST_STRPTR format,...);

#ifndef __amigaos4__
VARARGS68K LONG SNPrintf(STRPTR buffer,LONG buffer_size, CONST_STRPTR format,...);
#endif

#endif
