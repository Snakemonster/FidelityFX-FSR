using System;
using System.Collections.Generic;
using System.IO;
using Flax.Build;
using Flax.Build.NativeCpp;

public class FSR : GameModule
{
    public static bool ConditionalImport(BuildOptions options, List<string> dependecies)
    {
        var result = false;
        switch (options.Platform.Target)
        {
            case TargetPlatform.Windows:
                switch (options.Architecture)
                {
                    case TargetArchitecture.x64:
                        result = true;
                        break;
                }
                break;
        }
        if (result)
            dependecies.Add("FSR");
        return result;
    }
    /// <inheritdoc />
    public override void Setup(BuildOptions options)
    {
        base.Setup(options);

        BuildNativeCode = true;
        options.PrivateIncludePaths.Add(Path.Combine(FolderPath, "../ThirdParty/FidelityFX/api/include"));
        options.PrivateIncludePaths.Add(Path.Combine(FolderPath, "../ThirdParty/FidelityFX/upscalers/include"));
        // options.PrivateIncludePaths.Add(Path.Combine(FolderPath, "../ThirdParty/FidelityFX/upscalers/fsr3/include"));
        switch (options.Platform.Target)
        {
            case TargetPlatform.Windows:
            {
                switch (options.Architecture)
                {
                    case TargetArchitecture.x64:
                    {
                        var libPath =
                            Flax.Build.Utilities.RemovePathRelativeParts(Path.Combine(FolderPath,
                                "../ThirdParty/FidelityFX/signedbin"));
                        options.LinkEnv.InputFiles.Add(Path.Combine(libPath, "amd_fidelityfx_loader_dx12.lib"));
                        options.DependencyFiles.Add(Path.Combine(libPath, "amd_fidelityfx_loader_dx12.dll"));
                        options.DependencyFiles.Add(Path.Combine(libPath, "amd_fidelityfx_upscaler_dx12.dll"));
                        break;
                    }
                    default:
                        throw new InvalidArchitectureException(options.Architecture);
                }
                break;
            }
            default:
                throw new InvalidPlatformException(options.Platform.Target);
        }
    }
}
