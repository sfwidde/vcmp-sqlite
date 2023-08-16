#ifndef PLUGIN_CONSOLE_H
#define PLUGIN_CONSOLE_H

enum ConsoleMessageType
{
	CONSOLE_INFO_MESSAGE,
	CONSOLE_WARNING_MESSAGE,
	CONSOLE_ERROR_MESSAGE
};

void PrintConsoleMessage(enum ConsoleMessageType messageType, const char* format, ...);

#endif
