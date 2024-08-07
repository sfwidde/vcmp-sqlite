#include "main.h"
#include "../squirrel/SQImports.h"
#include "console.h"
#include <VCMP.h>
#include <string.h>

PluginFuncs* vcmpFunctions;
HSQAPI sq;
HSQUIRRELVM v;

/* functions.c */
void RegisterSquirrelFunctions(HSQUIRRELVM v);

static void HookSquirrel(void)
{
	// Attempt to find Squirrel plugin.
	int32_t pluginId = vcmpFunctions->FindPlugin("SQHost2");
	if (pluginId == -1)
	{
		OUTPUT_ERROR("Unable to find Squirrel module. SQLite functions will be unavailable.");
		return;
	}

	// Attempt to retrieve exports data.
	size_t dataSize;
	const void** data = vcmpFunctions->GetPluginExports(pluginId, &dataSize);
	if (!data || !*data || dataSize != sizeof(SquirrelImports))
	{
		OUTPUT_ERROR("The Squirrel module provided an invalid SquirrelImports structure. "
			"SQLite functions will be unavailable.");
		return;
	}

	// Do the hook.
	SquirrelImports* sqImports = *(SquirrelImports**)data;
	sq = *sqImports->GetSquirrelAPI();
	v = *sqImports->GetSquirrelVM();

	// It is safe to register our new functions now.
	RegisterSquirrelFunctions(v);
	OUTPUT_INFO("Loaded SQLite3 module v" PLUGIN_VERSION_STR " by sfwidde ([R3V]Kelvin).");
}

static uint8_t OnPluginCommand(uint32_t commandIdentifier, const char* message)
{
	switch (commandIdentifier)
	{
	case 0x7D6E22D8: // Squirrel scripts load
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
	pluginInfo->pluginVersion   = PLUGIN_VERSION_INT;
	pluginInfo->apiMajorVersion = PLUGIN_API_MAJOR;
	pluginInfo->apiMinorVersion = PLUGIN_API_MINOR;

	vcmpFunctions = pluginFuncs;

	pluginCalls->OnPluginCommand = OnPluginCommand;

	return 1;
}
