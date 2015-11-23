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
            PublicIncludePaths.Add("C:\\Program Files (x86)\\Intel\\RSSDK\\include");
            PublicAdditionalLibraries.Add("C:\\Program Files (x86)\\Intel\\RSSDK\\lib\\x64\\libpxc.lib");
        }
	}
}