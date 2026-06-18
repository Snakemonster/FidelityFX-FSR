#pragma once

#include "Engine/Core/Config/Settings.h"
#include "Engine/Scripting/ScriptingType.h"

API_CLASS(Namespace="FSR") class FSRSEttings : public SettingsBase
{
    API_AUTO_SERIALIZATION()
    DECLARE_SCRIPTING_TYPE_NO_SPAWN(FSRSettings);
    DECLARE_SETTINGS_GETTER(FSRSEttings);

public:
    API_FIELD(Attributes="EditorOrder(0)") uint32 AppId = 0;

    /// <summary>
    /// 
    /// </summary>
    API_FIELD(Attributes="EditorOrder(10)") StringAnsi ProjectId;

    /// <summary>
    /// If checked, FSR initialization will be delayed until actually used
    /// </summary>
    API_FIELD(Attributes="EditorOrder(100)") bool LazyInit = false;
};