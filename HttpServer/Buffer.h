#pragma once

#include <Windows.h>

#include "Error.h"

#define BUFF_SIZE (256)
#define BUFF_EXTRA (100)
#define GET_BUF_PTR(buf) ((buf.size <= BUFF_SIZE) ? buf.b.smallBuf : buf.b.largeBuf)

typedef struct buffer_s {
	union{
		BYTE smallBuf[BUFF_SIZE];
		BYTE* largeBuf;
	} b;
	BYTE* end;
	SIZE_T size;
	SIZE_T capacity;
} buffer_t;


error_t initBuffer(buffer_t* buf);
error_t writeBuffer(buffer_t* buf, BYTE* data, SIZE_T dataLen);