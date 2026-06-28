# pragma once

#include "ffx_api.hpp"
#include "FSR/FSRTypes.h"
#include "Engine/Core/Types/BaseTypes.h"
#include "Engine/Core/Math/Vector2.h"
#include "Engine/Graphics/RenderTask.h"
#include "Engine/Core/Collections/Dictionary.h"

class GPUTexture;
class GPUContext;

API_CLASS(Namespace="AMD") class FSR_API FSRUpscale : public ScriptingObject
{
    DECLARE_SCRIPTING_TYPE(FSRUpscale);
public:
    /// <summary>
    /// Softening or sharpening factor to apply during the FSR pass. Negative values soften the image, positive values sharpen. In range [-1; 1].
    /// </summary>
    API_FIELD(Attributes="Range(-1.0f, 1.0f)") float Sharpness = 0.0f;
protected:
private:
    ffx::Context _ffxContext;

    bool _debugView = true;
    Dictionary<String, uint64_t> _upscalerVersions;
    String _selectedUpscalerVersion;
    uint64_t _selectedUpscalerId;
    FSRQuality _quality = FSRQuality::NativeAA;

public:
    /// <summary>
    /// Initialize FSR context
    /// </summary>
    bool Initialize(FSRSupport& support);

    /// <summary>
    /// Initialize FSR context
    /// </summary>
    void Shutdown();

    /// <summary>
    /// Render FSR upscaled picture
    /// </summary>
    void TemporalResolve(GPUContext* context, RenderContext& renderContext, GPUTexture* input, GPUTexture* output, const Float2& pixelOffset);

    /// <summary>
    /// Get available upscaler versions
    /// </summary>
    API_PROPERTY() Array<String> GetUpscalerVersions() const;

    /// <summary>
    /// Get selected upscaler version
    /// </summary>
    API_PROPERTY() String GetUpscalerVersion() const;

    /// <summary>
    /// Set upscaler version
    /// </summary>
    API_PROPERTY() void SetUpscalerVersion(String newVersion);

    /// <summary>
    /// Get current Quality
    /// </summary>
    API_PROPERTY() FSRQuality GetQuality() const;

    /// <summary>
    /// Set render scale that depend on FSR quality preset
    /// </summary>
    API_PROPERTY() void SetQuality(const FSRQuality& quality);

    /// <summary>
    /// Set debug FSR view
    /// </summary>
    API_FUNCTION() void SetDebugView(bool debugEnabled);

    /// <summary>
    /// Get render scale ratio from selected Quality mode 
    /// </summary>
    float GetUpscaleRatioFromQualityMode(const FSRQuality& quality);

protected:
private:
    void UpdateFSRContext();
    void FillUpscalerVersions();
    static void ffxDebugMessage(uint32_t type, const wchar_t* message);
};