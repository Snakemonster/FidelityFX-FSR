#include "FSRUpsacle.h"
#include "Engine/Core/Log.h"
#include "Engine/Engine/Time.h"
#include "Engine/Graphics/GPUDevice.h"
#include "Engine/Graphics/GPUContext.h"
#include "Engine/Level/Actors/Camera.h"
#include "Engine/Graphics/RenderBuffers.h"
#include "Engine/Graphics/Textures/GPUTexture.h"

#include "ffx_api.hpp"
#include "ffx_api_types.h"
#include "ffx_upscale.hpp"
#include "dx12/ffx_api_dx12.hpp"
#include "Engine/Debug/DebugLog.h"

FSRUpscale::FSRUpscale(const SpawnParams& params) 
    : ScriptingObject(params)
{ }

bool FSRUpscale::Initialize(FSRSupport& support)
{
#if !PLATFORM_WINDOWS
    support = FSRSupport::NotSupportedPlatform;
    return true;
#endif

    auto gpuDevice = GPUDevice::Instance;

    auto result = ffx::ReturnCode::Error;
    switch (gpuDevice->GetRendererType())
    {
    case RendererType::DirectX12:
        {
            ffx::CreateBackendDX12Desc createBackend { };
            createBackend.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_DX12;
            createBackend.device = static_cast<ID3D12Device*>(gpuDevice->GetNativePtr());

            ffx::CreateContextDescUpscale createUpscale{};
            createUpscale.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_UPSCALE;
#if BUILD_DEBUG || BUILD_DEVELOPMENT
            createUpscale.fpMessage = &FSRUpscale::ffxDebugMessage;
            createUpscale.flags = FFX_UPSCALE_ENABLE_DEBUG_CHECKING | FFX_UPSCALE_ENABLE_DEBUG_VISUALIZATION;
            _debugView = true;
#endif
            createUpscale.maxRenderSize = FfxApiDimensions2D{3840, 2160};
            createUpscale.maxUpscaleSize = FfxApiDimensions2D{3840, 2160};

            result = ffx::CreateContext(_ffxContext, nullptr, createUpscale, createBackend);
            break;
        }
#if GRAPHICS_API_VULKAN
    case RendererType::Vulkan:
        // Waiting until AMD finally add support for Vulkan...
        support = FSRSupport::NotSupportedRenderingBackend;
        return true;
#endif
    default:
        return true;
    }

    if (result == ffx::ReturnCode::Ok)
    {
        LOG(Info, "[FSR] ffx CreateContext is success");
    }
    else
    {
        LOG(Error, "[FSR] ffx CreateContext is failed");
        return true;
    }
    support = FSRSupport::Supported;
    FillUpscalerVersions();
    return false;
}

void FSRUpscale::Shutdown()
{
    ffx::DestroyContext(_ffxContext);
    this->_upscalerVersions.Clear();
}

void FSRUpscale::TemporalResolve(GPUContext* context, RenderContext& renderContext, GPUTexture* input,
                                 GPUTexture* output, const Float2& pixelOffset)
{
    //TODO need to adjust some setting to remove giant pixelation on lower upscale quality and noise on NativeAA
    const auto renderSize = input->Size();
    const auto upscaleSize = output->Size();
    const float sharpness = Math::Clamp(Sharpness, -1.0f, 1.0f);

    // Since fsr only on D3D12
    context->SetResourceState(output, 0x8); // D3D12_RESOURCE_STATE_UNORDERED_ACCESS
    context->SetResourceState(input, 0x40 | 0x80); // D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
    context->SetResourceState(renderContext.Task->Buffers->DepthBuffer, 0x40 | 0x80); // D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
    if (renderContext.Task->Buffers->MotionVectors)
        context->SetResourceState(renderContext.Task->Buffers->MotionVectors, 0x40 | 0x80); // D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE

    ffx::DispatchDescUpscale upscale{};
    upscale.header.type = FFX_API_DISPATCH_DESC_TYPE_UPSCALE;
    upscale.commandList = (ID3D12GraphicsCommandList*)context->GetNativePtr();

    upscale.color = ffxApiGetResourceDX12((ID3D12Resource*)input->GetNativePtr());
    upscale.depth = ffxApiGetResourceDX12((ID3D12Resource*)renderContext.Task->Buffers->DepthBuffer->GetNativePtr());
    upscale.motionVectors = ffxApiGetResourceDX12((ID3D12Resource*)(renderContext.Task->Buffers->MotionVectors ? renderContext.Task->Buffers->MotionVectors->GetNativePtr() : nullptr));
    upscale.output = ffxApiGetResourceDX12((ID3D12Resource*)output->GetNativePtr());

    upscale.jitterOffset = { pixelOffset.X, pixelOffset.Y };
    upscale.motionVectorScale = { static_cast<float>(renderSize.X), static_cast<float>(renderSize.Y) };
    upscale.renderSize = { static_cast<uint32_t>(renderSize.X), static_cast<uint32_t>(renderSize.Y) };
    upscale.upscaleSize = { static_cast<uint32_t>(upscaleSize.X), static_cast<uint32_t>(upscaleSize.Y) };

    upscale.enableSharpening = true;
    upscale.sharpness = sharpness;
    upscale.frameTimeDelta = static_cast<float>(Time::Draw.UnscaledDeltaTime.GetTotalMilliseconds());
    upscale.preExposure = 1.0f;
    upscale.reset = renderContext.Task->IsCameraCut;
    upscale.cameraNear = renderContext.Task->Camera.Get()->GetNearPlane();
    upscale.cameraFar = renderContext.Task->Camera.Get()->GetFarPlane();
    upscale.cameraFovAngleVertical = renderContext.Task->Camera.Get()->GetFieldOfView() * DegreesToRadians;
    upscale.viewSpaceToMetersFactor = 1.0f;
#if BUILD_DEVELOPMENT || BUILD_DEBUG
    if (_debugView) upscale.flags = FFX_UPSCALE_FLAG_DRAW_DEBUG_VIEW;
#endif
    auto result = ffx::Dispatch(_ffxContext, upscale);
    if (result != ffx::ReturnCode::Ok)
    {
        LOG(Error, "[FSR] FSR is failed! Something gone wrong");
        return;
    }
    context->ForceRebindDescriptors();
    context->ResetState();
}

Array<String> FSRUpscale::GetUpscalerVersions() const
{
    Array<String> result;
    _upscalerVersions.GetKeys(result);
    return result;
}

String FSRUpscale::GetUpscalerVersion() const
{
    return _selectedUpscalerVersion;
}

void FSRUpscale::SetUpscalerVersion(String newVersion)
{
    //TODO seems like somewhere here is memory leak
    const auto newUpscalerId = _upscalerVersions.TryGet(newVersion);
    if (!newUpscalerId)
    {
        LOG(Error, "[FSR] Cannot find upscaler version of {}", newVersion);
        return;
    }

    ffx::CreateBackendDX12Desc createBackend { };
    createBackend.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_DX12;
    createBackend.device = static_cast<ID3D12Device*>(GPUDevice::Instance->GetNativePtr());

    ffx::CreateContextDescUpscale createUpscale{};
    createUpscale.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_UPSCALE;
    createUpscale.fpMessage = &FSRUpscale::ffxDebugMessage;
    createUpscale.flags = FFX_UPSCALE_ENABLE_DEBUG_CHECKING;
    createUpscale.maxRenderSize = FfxApiDimensions2D{3840, 2160};
    createUpscale.maxUpscaleSize = FfxApiDimensions2D{3840, 2160};

    ffx::CreateContextDescOverrideVersion versionOverride {};
    versionOverride.versionId = *newUpscalerId;
    auto returnCode = ffx::CreateContext(_ffxContext, nullptr, createUpscale, createBackend, versionOverride);
    if (returnCode == ffx::ReturnCode::Ok)
    {
        _selectedUpscalerVersion = newVersion;
        LOG(Info, "[FSR] ffx Query get versionCount is success: version is {}", newVersion);
    }
    else
    {
        LOG(Error, "[FSR] ffx Query get versionCount failed");
    }
}

FSRQuality FSRUpscale::GetQuality() const
{
    return _quality;
}

void FSRUpscale::SetQuality(const FSRQuality& quality)
{
    const float selectedRatio = GetUpscaleRatioFromQualityMode(quality);
    if (selectedRatio == 0.0f) return;
    
    const auto task = MainRenderTask::Instance;
    if (!task) return;

    DebugLog::Log(LogType::Info, String::Format(TEXT("[FSR] Selected ratio from quality mode: {}"), selectedRatio));
    task->RenderScale = 1.0f / selectedRatio;
    _quality = quality;
}

void FSRUpscale::SetDebugView(bool debugEnabled)
{
    _debugView = debugEnabled;
}

Int2 FSRUpscale::GetUpscaleResolutionFromQualityMode(const FSRQuality& quality, const Int2& dsrResolution)
{
    uint32_t outHeight, outWidth;
    ffx::QueryDescUpscaleGetRenderResolutionFromQualityMode getRenderResolution;
    getRenderResolution.header.type = FFX_API_QUERY_DESC_TYPE_UPSCALE_GETRENDERRESOLUTIONFROMQUALITYMODE;
    getRenderResolution.qualityMode = static_cast<uint32_t>(quality);
    getRenderResolution.displayHeight = dsrResolution.X;
    getRenderResolution.displayWidth = dsrResolution.Y;
    getRenderResolution.pOutRenderHeight = &outHeight;
    getRenderResolution.pOutRenderWidth = &outWidth;
    auto result = ffx::Query(getRenderResolution);
    if (result != ffx::ReturnCode::Ok)
    {
        LOG(Error, "[FSR] Failed to get render resolution with this quality mode!");
        return Vector2Base<uint32_t>(0, 0);
    }
    return Vector2Base<uint32_t>(outHeight, outWidth);
}

float FSRUpscale::GetUpscaleRatioFromQualityMode(const FSRQuality& quality)
{
    float outRatio;
    ffx::QueryDescUpscaleGetUpscaleRatioFromQualityMode getResolutionRatio;
    getResolutionRatio.header.type = FFX_API_QUERY_DESC_TYPE_UPSCALE_GETUPSCALERATIOFROMQUALITYMODE;
    getResolutionRatio.qualityMode = (uint32_t)quality;
    getResolutionRatio.pOutUpscaleRatio = &outRatio;
    auto result = ffx::Query(getResolutionRatio);
    if (result != ffx::ReturnCode::Ok)
    {
        LOG(Error, "[FSR] Failed to get upscale ration with this {} quality mode!", (uint32_t)quality);
        return 0.0f;
    }
    return outRatio;
}

void FSRUpscale::FillUpscalerVersions()
{
    auto result = ffx::ReturnCode::Error;
    auto gpuDevice = GPUDevice::Instance;
    uint64_t versionCount;

    ffx::QueryDescGetVersions getAvailableVersions = {};
    getAvailableVersions.header.type = FFX_API_QUERY_DESC_TYPE_GET_VERSIONS;
    getAvailableVersions.createDescType = FFX_API_CREATE_CONTEXT_DESC_TYPE_UPSCALE_VERSION;
    getAvailableVersions.device = static_cast<ID3D12Device*>(gpuDevice->GetNativePtr());
    getAvailableVersions.outputCount = &versionCount;

    // First query: get count
    result = ffx::Query(getAvailableVersions);
    if (result == ffx::ReturnCode::Ok)
    {
        LOG(Info, "[FSR] ffx Query get versionCount is success: versionCount is {}", versionCount);
    }
    else
    {
        LOG(Error, "[FSR] ffx Query get versionCount failed");
        return;
    }

    // Allocate storage
    Array<uint64_t> versionIds;
    Array<const char*> versionNames;
    versionIds.Resize(versionCount);
    versionNames.Resize(versionCount);

    getAvailableVersions.outputCount = &versionCount;
    getAvailableVersions.versionIds = versionIds.Get();
    getAvailableVersions.versionNames = versionNames.Get();

    // Second query: fill arrays
    result = ffx::Query(getAvailableVersions);
    if (result == ffx::ReturnCode::Ok)
    {
        LOG(Info, "[FSR] ffx Query filled arrays, versionIds length is {}, versionNames length is {}", versionIds.Count(), versionNames.Count());
    }
    else
    {
        LOG(Error, "[FSR] ffx Query cannot filled arrays");
        return;
    }
    for (int i = 0; i < versionCount; ++i)
    {
        LOG(Info, "[FSR] The versionIds is {}, versionNames is {}", versionIds[i], String(versionNames[i]));
        this->_upscalerVersions.Add(String(versionNames[i]), versionIds[i]);
    }

    ffxQueryGetProviderVersion getVersion = {0};
    getVersion.header.type = FFX_API_QUERY_DESC_TYPE_GET_PROVIDER_VERSION;
    result = ffx::Query(_ffxContext, getVersion);
    LOG(Info, "[FSR] The current Upscaler versionId is: {}, versionName is {}", getVersion.versionId, String(getVersion.versionName));
    _selectedUpscalerVersion = String(getVersion.versionName);
}

void FSRUpscale::ffxDebugMessage(uint32_t type, const wchar_t* message)
{
    LOG(Info, "[FSR] Message from ffxUpscale type: {}, message: {}", type, String(message));
}
