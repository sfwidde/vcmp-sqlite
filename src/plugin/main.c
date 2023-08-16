#include "../squirrel/SQImports.h"
#include "console.h"
#include <vcmp.h>
#include <string.h>

// Plugin info.
#define PLUGIN_NAME "SQLite"
#define PLUGIN_VERSION 0x000 // v0.0.0

PluginFuncs* vcmpFunctions;
HSQAPI sq;
HSQUIRRELVM v;

// @functions.c
void RegisterSQLiteFunctions(void);

static void HookSquirrel(void)
{
	int32_t pluginId = vcmpFunctions->FindPlugin("SQHost2");
	if (pluginId == -1)
	{
		PrintConsoleMessage(CONSOLE_ERROR_MESSAGE, "Unable to find Squirrel module - "
			"SQLite functions will be unavailable.");
		return;
	}

	size_t importsSize;
	SquirrelImports** imports = (SquirrelImports**)vcmpFunctions->GetPluginExports(pluginId, &importsSize);
	if (!imports || !*imports || importsSize != sizeof(SquirrelImports))
	{
		PrintConsoleMessage(CONSOLE_ERROR_MESSAGE, "The Squirrel module provided an invalid SquirrelImports structure - "
			"SQLite functions will be unavailable.");
		return;
	}

	// Do the hook.
	sq = *(*imports)->GetSquirrelAPI();
	v = *(*imports)->GetSquirrelVM();
	RegisterSQLiteFunctions();
	PrintConsoleMessage(CONSOLE_INFO_MESSAGE, "Loaded SQLite module by sfwidde ([R3V]Kelvin).");
}

static uint8_t OnPluginCommand(uint32_t commandIdentifier, const char* message)
{
	switch (commandIdentifier)
	{
	case 0x7D6E22D8:
		HookSquirrel();
	default:
		return 1;
	}
}

#ifdef OS_WINDOWS
__declspec(dllexport)
#endif
uint32_t VcmpPluginInit(PluginFuncs* pluginFuncs, PluginCallbacks* pluginCalls, PluginInfo* pluginInfo)
{
	strcpy(pluginInfo->name, PLUGIN_NAME);
	pluginInfo->pluginVersion = PLUGIN_VERSION;
	pluginInfo->apiMajorVersion = PLUGIN_API_MAJOR;
	pluginInfo->apiMinorVersion = PLUGIN_API_MINOR;

	vcmpFunctions = pluginFuncs;

	pluginCalls->OnPluginCommand = OnPluginCommand;

	return 1;
}
