using Flax.Build;

public class FSRTarget : GameProjectTarget
{
    /// <inheritdoc />
    public override void Init()
    {
        base.Init();
        Platforms = new[]
        {
            TargetPlatform.Windows
        };

        Architectures = new[]
        {
            TargetArchitecture.x64
        };
        // Reference the modules for game
        Modules.Add("FSR");
    }
}
