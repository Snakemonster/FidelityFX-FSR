#include "FSRPostFx.h"

#include "FSR.h"
#include "Engine/Graphics/GPUContext.h"
#include "Engine/Graphics/RenderTargetPool.h"
#include "Engine/Graphics/RenderTask.h"
#include "Engine/Graphics/Textures/GPUTexture.h"
#include "Engine/Profiler/Profiler.h"
#include "Engine/Renderer/RenderList.h"
#include "Upscale/FSRUpsacle.h"

FSRPostFx::FSRPostFx(const SpawnParams& params)
    : PostProcessEffect(params)
{
    Location = PostProcessEffectLocation::CustomUpscale;
}

bool FSRPostFx::CanRender(const RenderContext& renderContext) const
{
    const auto fsr = FSR::GetInstance();
    return PostProcessEffect::CanRender() && fsr && fsr->GetSupport() == FSRSupport::Supported;
}

void FSRPostFx::PreRender(GPUContext* context, RenderContext& renderContext)
{
    if (!CanRender(renderContext)) return;
    renderContext.List->Setup.UpscaleLocation = RenderingUpscaleLocation::BeforePostProcessingPass;
    renderContext.List->Setup.UseTemporalAAJitter = true;
    renderContext.List->Settings.AntiAliasing.Mode = AntialiasingMode::None;
}

void FSRPostFx::Render(GPUContext* context, RenderContext& renderContext, GPUTexture* input, GPUTexture* output)
{
    PROFILE_GPU_CPU("FSR");
    GPUTexture* fsrOutput = output;
    if (!fsrOutput->IsUnorderedAccess())
    {
        GPUTextureDescription desc = output->GetDescription();
        desc.Flags &= ~GPUTextureFlags::BackBuffer;
        desc.Flags |= GPUTextureFlags::UnorderedAccess;
        fsrOutput = RenderTargetPool::Get(desc);
    }

    const auto fsr = FSR::GetInstance();
    const Float2 pixelOffset(renderContext.View.TemporalAAJitter.X * renderContext.View.ScreenSize.X / 2.f, renderContext.View.TemporalAAJitter.X * renderContext.View.ScreenSize.Y / 2.f);
    fsr->_fsrUpscale->TemporalResolve(context, renderContext, input, fsrOutput, pixelOffset);
    if (fsrOutput != output)
    {
        PROFILE_GPU("Copy");
        context->CopyResource(output, fsrOutput);
        RenderTargetPool::Release(fsrOutput);
    }
}
