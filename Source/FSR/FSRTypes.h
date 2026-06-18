#pragma once

#include "Engine/Core/Types/BaseTypes.h"

API_ENUM(Namespace="AMD") enum class FSRSupport
{
    // FSR is supported
    Supported,

    // FSR is not supported due to incompatible platform
    NotSupportedPlatform,

    // FSR is not supported due to incompatible rendering backend
    NotSupportedRenderingBackend,

    MAX
};

API_ENUM(Namespace="AMD") enum class FSRQuality
{
    NativeAA                = 0,
    Quality                 = 1,
    Balanced                = 2,
    Performance             = 3,
    UltraPerformance        = 4,
};
