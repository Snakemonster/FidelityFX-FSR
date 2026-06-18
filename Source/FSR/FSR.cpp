#include "FSR.h"

#include "FSRPostFx.h"
#include "Engine/Core/Log.h"
#include "Engine/Debug/DebugLog.h"
#include "Engine/Graphics/RenderTask.h"
#include "Engine/Scripting/SoftTypeReference.h"
#include "Engine/Scripting/Plugins/PluginManager.h"
#include "Upscale/FSRUpsacle.h"

FSR::FSR(const SpawnParams& params)
    : GamePlugin(params)
{
    
}

FSRSupport FSR::GetSupport() const
{
    return _support;
}

FSR* FSR::GetInstance()
{
    return PluginManager::GetPlugin<FSR>();
}

FSRUpscale* FSR::GetUpscale() const
{
    return _fsrUpscale;
}

void FSR::ApplyUpscaler()
{
    PostFX = New<FSRPostFx>();
    SceneRenderTask::AddGlobalCustomPostFx(PostFX);
    _fsrUpscale->SetQuality(_fsrUpscale->GetQuality());
}

void FSR::RemoveUpscaler()
{
    if (!PostFX) return;

    SceneRenderTask::RemoveGlobalCustomPostFx(PostFX);
    PostFX->DeleteObject();
    PostFX = nullptr;

    const auto task = MainRenderTask::Instance;
    if (!task) return;

    DebugLog::Log(LogType::Info, String::Format(TEXT("[FSR] Selected ratio from quality mode: 1.0")));
    task->RenderScale = 1.0f;
}

void FSR::Initialize()
{
    GamePlugin::Initialize();
    _fsrUpscale = SoftTypeReference<FSRUpscale>("AMD.FSRUpscale").NewObject();
    _fsrUpscale->Initialize(_support);
}

void FSR::Deinitialize()
{
    RemoveUpscaler();
    _fsrUpscale->Shutdown();
    _fsrUpscale->DeleteObject();
    GamePlugin::Deinitialize();
}

