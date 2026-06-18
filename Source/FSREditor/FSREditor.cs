using System;
using System.Collections.Generic;
using FlaxEditor;
using FlaxEditor.Content;
using FlaxEngine;

namespace AMD;

/// <summary>
/// FSREditor GamePlugin.
/// </summary>
public class FSREditor : EditorPlugin
{
    private AssetProxy _assetProxy;
    /// <inheritdoc/>
    public override void Initialize()
    {
        _description = new PluginDescription
        {
            Name = "FSR",
            Category = "Rendering",
            Author = "AMD",
            RepositoryUrl = "https://github.com/FlaxEngine/FSR",
            Description = "This is an FSR plugin project.",
            Version = new Version(0, 1, 0, 0),
            IsAlpha = true,
        };
    }

    public override Type GamePluginType => typeof(FSR);

    public override void InitializeEditor()
    {
        base.InitializeEditor();
        // _assetProxy = new CustomSettingsProxy()
        // Editor.ContentDatabase.Proxy.Add(_assetProxy);
    }

    /// <inheritdoc/>
    public override void Deinitialize()
    {
        // Editor.ContentDatabase.Proxy.Remove(_assetProxy);
        _assetProxy = null;
        base.Deinitialize();
    }
}