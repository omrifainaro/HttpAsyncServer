#pragma once

#include <Windows.h>

#include "Error.h"

#define BUFF_SIZE (256)
#define BUFF_EXTRA (100)
#define IS_SMALL_BUFFER(buf) ((buf)->size <= BUFF_SIZE) 
#define GET_BUF_PTR(buf) (IS_SMALL_BUFFER(buf) ? (buf)->b.smallBuf : (buf)->b.largeBuf)
#define GET_READ_SIZE(buf) ((buf)->end - (buf)->readPtr)

#define CONTENT_LENGTH "Content-Length"
#define CONTENT_TYPE "Content-Type"

#define MAX_HEADERS (32)


#define DATETIME_FORMAT "%a, %d %b %Y %T GMT"
#define DATETIME_LEN 30

#define CRLF "\r\n"
#define CRLFCRLF "\r\n\r\n"
#define HEADER_DELIM ": "
#define HTTP_BODY_DELIM CRLFCRLF