#pragma once

#include "Structs.h"

int parseHeader(BYTE* buffer, SIZE_T buffSize, header_t* out);

error_t getHeaderValue(BYTE* buffer, SIZE_T size, PCHAR header, PCHAR** outValue);