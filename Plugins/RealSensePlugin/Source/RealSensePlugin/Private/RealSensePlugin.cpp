#include "RealSensePluginPrivatePCH.h"

class FRealSensePlugin : public IRealSensePlugin
{
	void StartupModule() override {}	// This code will execute after the module is loaded into memory
	void ShutdownModule() override {}	// This function may be called during shutdown or before reloading
};
IMPLEMENT_MODULE(FRealSensePlugin, RealSensePlugin)