#define SMAA_GLSL_4
#define SMAA_PRESET_ULTRA
#define SMAA_AREATEX_SELECT(sample) sample.rg
#define SMAA_SEARCHTEX_SELECT(sample) sample.r

//#ifndef SMAA_RT_METRICS
//#define SMAA_RT_METRICS float4(1.0 / 800.0, 1.0 / 600.0, 800.0, 600.0)
//#endif
