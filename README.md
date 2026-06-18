# AMD FidelityFX Super Resolution for Flax Engine
![img.png](amd-fsr-flax.png)

[AMD Fidelity FX Super Resolution](https://gpuopen.com/fidelityfx-superresolution/) is a cutting edge super-optimize spatial upsampling technology that produces impressive image quality at fast framerates. This repository contains a plugin project for [Flax Engine](https://flaxengine.com/) games with FSR.

Minimum supported Flax version: `1.12`.

> [!IMPORTANT]
> This project works only on Windows and only on graphics api Directx12.

> [!IMPORTANT]
> FSR upscaler version 4 and frame generation (ml) is officially working only on Rx 9000 series (yet).

> [!IMPORTANT]
> Currently only Upscaler FSR is implemented.

## Installation

You can download this plugin in two ways:
### Automated git cloning 
1. In Flax Editor click on **Tools** -> **Plugins** -> **Clone Project**
2. In popup in field **Name**: `FSR` (or whatever you wanna call it), in field **Git Path**: `<link to this repo>`

### Manual creation
1. Clone repo into `<game-project>\Plugins\FSR`

2. Add reference to FSR project in your game by modyfying your game `<game-project>.flaxproj` as follows:


```
...
"References": [
    {
        "Name": "$(EnginePath)/Flax.flaxproj"
    },
    {
        "Name": "$(ProjectPath)/Plugins/FidelityFX-FSR/FidelityFX-FSR.flaxproj"
    }
]
```
Test it out!
## Usage
Finally open Flax Editor - FSR will be visible in Plugins window (under Rendering category). It implements `CustomUpscale` postFx to increase visual quality when using low-res rendering. To test put lines below in your code:
```cs
AMD.FSR.Instance.ApplyUpscaler();
AMD.FSR.Instance.Upscaler.Sharpness = 1.0f // in range from -1 to 1
AMD.FSR.Instance.Upscaler.Quality = FSRQuality.Quality; // select from quality modes
AMD.FSR.Instance.Upscaler.SetDebugView(false) // for debug view
```

## License

Both this plugin and FSR are released under **MIT License**.

## Trademarks

© 2021 Advanced Micro Devices, Inc. All rights reserved. AMD, the AMD Arrow logo, Radeon, RDNA, Ryzen, and combinations thereof are trademarks of Advanced Micro Devices, Inc.
