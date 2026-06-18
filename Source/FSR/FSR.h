#pragma once

#include "Engine/Scripting/Plugins/GamePlugin.h"
#include "FSRTypes.h"

class FSRUpscale;
class TestScriptableObj;
class FSRPostFx;
API_CLASS(Namespace="AMD") class FSR_API FSR : public GamePlugin
{
    friend FSRPostFx;
    DECLARE_SCRIPTING_TYPE(FSR);
public:
    /// <summary>
    /// FSR post process effect.
    /// </summary>
    API_FIELD(ReadOnly) FSRPostFx* PostFX = nullptr;

protected:
private:
    FSRUpscale* _fsrUpscale = nullptr;
    FSRSupport _support = FSRSupport::MAX;

public:
    /// <summary>
    /// FSR support information
    /// </summary>
    API_PROPERTY() FSRSupport GetSupport() const;

    /// <summary>
    /// Get FSR plugin instance
    /// </summary>
    API_PROPERTY() static FSR* GetInstance();

    /// <summary>
    /// Get FSR upscale
    /// </summary>
    API_PROPERTY() FSRUpscale* GetUpscale() const;

    /// <summary>
    /// Apply FSR upscaler postfx and render scale
    /// </summary>
    API_FUNCTION() void ApplyUpscaler();

    /// <summary>
    /// Remove FSR upscaler postfx and set render scale back to 1
    /// </summary>
    API_FUNCTION() void RemoveUpscaler();

    void Initialize() override;
    void Deinitialize() override;
protected:
private:
};
