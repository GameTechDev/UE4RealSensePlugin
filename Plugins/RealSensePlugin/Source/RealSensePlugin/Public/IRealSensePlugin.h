#pragma once

#include "ModuleManager.h"

struct IRealSensePlugin : public IModuleInterface
{
	static inline IRealSensePlugin& Get() { return FModuleManager::LoadModuleChecked<IRealSensePlugin>("RealSensePlugin"); }
	static inline bool IsAvailable() { return FModuleManager::Get().IsModuleLoaded("RealSensePlugin"); }
};

