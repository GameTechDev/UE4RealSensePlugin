using System;
using System.IO;

namespace UnrealBuildTool.Rules 
{
	public class RealSensePlugin : ModuleRules 
    {
        public RealSensePlugin(TargetInfo Target)
        {
            // https://answers.unrealengine.com/questions/51798/how-can-i-enable-unwind-semantics-for-c-style-exce.html
            UEBuildConfiguration.bForceEnableExceptions = true;

            PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine" });
            PrivateDependencyModuleNames.AddRange(new string[] { "RHI", "RenderCore", "ShaderCore" });

            PrivateIncludePaths.AddRange(new string[] { "RealSensePlugin/Private" });

            string RealSenseDirectory = Environment.GetEnvironmentVariable("RSSDK_DIR");
            string RealSenseIncludeDirectory = RealSenseDirectory + "include";
            string RealSenseLibrary32Directory = RealSenseDirectory + "lib\\Win32\\libpxc.lib";
            string RealSenseLibrary64Directory = RealSenseDirectory + "lib\\x64\\libpxc.lib";

            PublicIncludePaths.Add(RealSenseIncludeDirectory);
            PublicAdditionalLibraries.Add(RealSenseLibrary32Directory);
            PublicAdditionalLibraries.Add(RealSenseLibrary64Directory);
        }
	}
}