#pragma once

#define JucePlugin_Name                   "HyperDistortion"
#define JucePlugin_Desc                   "Advanced multi-algorithm distortion with modulation"
#define JucePlugin_Manufacturer           "YourCompany"
#define JucePlugin_ManufacturerWebsite    ""
#define JucePlugin_ManufacturerEmail      ""
#define JucePlugin_ManufacturerCode       0x4D616E75 // 'Manu'
#define JucePlugin_PluginCode             0x44697374 // 'Dist'
#define JucePlugin_IsSynth                0
#define JucePlugin_WantsMidiInput         0
#define JucePlugin_ProducesMidiOutput     0
#define JucePlugin_IsMidiEffect           0
#define JucePlugin_EditorRequiresKeyboardFocus  0
#define JucePlugin_Version                1.0.0
#define JucePlugin_VersionCode            0x10000
#define JucePlugin_VersionString          "1.0.0"
#define JucePlugin_VSTUniqueID            JucePlugin_PluginCode
#define JucePlugin_VSTCategory            kPlugCategEffect
#define JucePlugin_Vst3Category           "Fx|Distortion"
#define JucePlugin_AUMainType             'aufx'
#define JucePlugin_AUSubType              JucePlugin_PluginCode
#define JucePlugin_AUExportPrefix         HyperDistortionAU
#define JucePlugin_AUExportPrefixQuoted   "HyperDistortionAU"
#define JucePlugin_AAXIdentifier          com.yourcompany.HyperDistortion
#define JucePlugin_AAXManufacturerCode    JucePlugin_ManufacturerCode
#define JucePlugin_AAXProductId           JucePlugin_PluginCode
#define JucePlugin_AAXCategory            0
#define JucePlugin_AAXDisableBypass       0
#define JucePlugin_AAXDisableMultiMono    0
#define JucePlugin_IAAType                0x61757278 // 'aurx'
#define JucePlugin_IAASubType             JucePlugin_PluginCode
#define JucePlugin_IAAName                "YourCompany: HyperDistortion"
