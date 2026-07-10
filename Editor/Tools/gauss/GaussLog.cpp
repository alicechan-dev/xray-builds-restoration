#include "StdAfx.h"
#include "GaussLog.h"

void __cdecl Msg(const char* format, ...)
{
	char buffer[4096];
	va_list args;
	va_start(args, format);
	_vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, format, args);
	va_end(args);

	OutputDebugStringA(buffer);
	OutputDebugStringA("\n");
}
