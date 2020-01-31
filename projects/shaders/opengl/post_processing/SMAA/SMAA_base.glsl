#define SMAA_GLSL_4
//#define SMAA_PRESET_ULTRA
#define SMAA_THRESHOLD 0.1
#define SMAA_MAX_SEARCH_STEPS 112
#define SMAA_MAX_SEARCH_STEPS_DIAG 20
#define SMAA_CORNER_ROUNDING 100

#define SMAA_AREATEX_SELECT(sample) sample.rg
#define SMAA_SEARCHTEX_SELECT(sample) sample.r

//#ifndef SMAA_RT_METRICS
//#define SMAA_RT_METRICS float4(1.0 / 800.0, 1.0 / 600.0, 800.0, 600.0)
//#endif
