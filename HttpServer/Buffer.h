#pragma once

#include "Utils.h"
#include "Structs.h"


error_t initBuffer(buffer_t* buf);
error_t writeBuffer(buffer_t* buf, BYTE* data, SIZE_T dataLen);
inline void rewindBufferRead(buffer_t* buf) { buf->readPtr = GET_BUF_PTR(buf); }
error_t readBuffer(buffer_t* buf, __out BYTE* data, SIZE_T dataLen);
error_t readBufferUntil(buffer_t* buf, BYTE* search, SIZE_T searchLen, __out BYTE** data, __out SIZE_T* dataLen);


void cleanupBuffer(buffer_t* buffer);