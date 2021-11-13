#include "Buffer.h"

error_t initBuffer(buffer_t* buf) {
	memset(buf->b.smallBuf, 0, BUFF_SIZE);
	buf->size = 0;
	buf->capacity = BUFF_SIZE;
	buf->end = &buf->b.smallBuf;
	return ERROR_OK;
}

/// <summary>
/// </summary>
/// <param name="buf"></param>
/// <param name="data"></param>
/// <param name="dataLen"></param>
/// <returns></returns>
error_t writeBuffer(buffer_t* buf, BYTE* data, SIZE_T dataLen) {
	BYTE* dataCopy = NULL;
	BYTE* baseBufferPtr = GET_BUF_PTR(buf);

	if (buf->capacity == BUFF_SIZE && buf->size + dataLen > BUFF_SIZE) {
		dataCopy = malloc(buf->capacity + dataLen + BUFF_EXTRA);
		memcpy(dataCopy, baseBufferPtr, buf->size);
		buf->b.largeBuf = dataCopy;
		buf->end = buf->b.largeBuf + buf->size;
		buf->capacity += dataLen + BUFF_EXTRA;
	} 
	else if (buf->end + dataLen > baseBufferPtr + buf->capacity) {
		// This is big buf, meaning size = buffer capacity
		dataCopy = realloc(baseBufferPtr, buf->capacity + dataLen + BUFF_EXTRA);
		buf->b.largeBuf = dataCopy;
		buf->end = dataCopy + (buf->end - baseBufferPtr);
		buf->capacity += dataLen + BUFF_EXTRA;
	}

	memcpy(buf->end, data, dataLen);
	buf->size += dataLen;
	buf->end += dataLen;
	return ERROR_OK;
}

void cleanupBuffer(buffer_t* buffer) {
	if (buffer) {
		if (!IS_SMALL_BUFFER(buffer)) {
			free(buffer->b.largeBuf);
			buffer->b.largeBuf = NULL;
		}
		buffer->end = NULL;
		buffer->size = 0;
	}
}