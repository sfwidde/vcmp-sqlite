#include "console.h"
#ifdef OS_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <stdarg.h>
#endif
#include <stdio.h>

void PrintConsoleMessage(enum ConsoleMessageType messageType, const char* format, ...)
{
	static const char* const messageTags[3] =
	{
		// Ugh
#ifdef OS_WINDOWS
		"[MODULE]  ", "[WARNING] ", "[ERROR]   "
#else
		"[MODULE]", "[WARNING]", "[ERROR]"
#endif
	};

	// Prepare variadic arguments...
	va_list ap;
	va_start(ap, format);

#ifdef OS_WINDOWS
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	if (!consoleHandle || consoleHandle == INVALID_HANDLE_VALUE)
	{
		fputs(messageTags[messageType], stdout);
		vprintf(format, ap);
		putchar('\n');
		va_end(ap);
		return;
	}

	CONSOLE_SCREEN_BUFFER_INFO initialCSBI;
	if (!GetConsoleScreenBufferInfo(consoleHandle, &initialCSBI))
	{
		fputs(messageTags[messageType], stdout);
		vprintf(format, ap);
		putchar('\n');
		va_end(ap);
		return;
	}

	// https://learn.microsoft.com/en-us/windows/console/console-screen-buffers#character-attributes
	static const WORD messageTagAttributes[3] =
	{
		FOREGROUND_GREEN, // Green
		FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, // Bright yellow
		FOREGROUND_RED | FOREGROUND_INTENSITY // Bright red
	};

	SetConsoleTextAttribute(consoleHandle, messageTagAttributes[messageType]);
	fputs(messageTags[messageType], stdout);

	SetConsoleTextAttribute(consoleHandle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	vprintf(format, ap);

	SetConsoleTextAttribute(consoleHandle, initialCSBI.wAttributes);
	putchar('\n');
#else
	// https://en.wikipedia.org/wiki/ANSI_escape_code#3-bit_and_4-bit
	static const int messageTagColourCodes[3] =
	{
		32, // Green
		93, // Bright yellow
		91 // Bright red
	};

	printf("\x1B[%dm%s\x1B[0m ", messageTagColourCodes[messageType], messageTags[messageType]);
	vprintf(format, ap);
	putchar('\n');
#endif

	va_end(ap);
}
