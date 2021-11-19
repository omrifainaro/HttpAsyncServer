#pragma once

#include <Windows.h>

#include "Utils.h"
#include "Error.h"

#define BUFF_SIZE (256)
#define BUFF_EXTRA (100)
#define IS_SMALL_BUFFER(buf) ((buf)->size <= BUFF_SIZE) 
#define GET_BUF_PTR(buf) (IS_SMALL_BUFFER(buf) ? (buf)->b.smallBuf : (buf)->b.largeBuf)
#define GET_READ_SIZE(buf) ((buf)->end - (buf)->readPtr)

typedef struct buffer_s {
	union{
		BYTE smallBuf[BUFF_SIZE];
		BYTE* largeBuf;
	} b;
	BYTE* end;
	BYTE* readPtr;
	SIZE_T size;
	SIZE_T capacity;
} buffer_t;


error_t initBuffer(buffer_t* buf);
error_t writeBuffer(buffer_t* buf, BYTE* data, SIZE_T dataLen);
inline void rewindBufferRead(buffer_t* buf) { buf->readPtr = GET_BUF_PTR(buf); }
error_t readBuffer(buffer_t* buf, __out BYTE* data, SIZE_T dataLen);
error_t readBufferUntil(buffer_t* buf, BYTE* search, SIZE_T searchLen, __out BYTE** data, __out SIZE_T* dataLen);


void cleanupBuffer(buffer_t* buffer);