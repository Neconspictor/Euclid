/****************************************************************************************************************
/__/\\\\\\\\\\\\\\\_____/\\\\\\\\\________/\\\\\\\\\____________/                                               |
/__\///////\\\/////____/\\\\\\\\\\\\\____/\\\\\\\\\\\\\_________/   Callum James Glover                         |
/_________\/\\\________/\\\/////////\\\__/\\\/////////\\\_______/   NCCA, Bournemouth University                |
/__________\/\\\_______\/\\\_______\/\\\_\/\\\_______\/\\\______/   s4907224@bournemouth.ac.uk                  |
/___________\/\\\_______\/\\\\\\\\\\\\\\\_\/\\\\\\\\\\\\\\\_____/   callum@glovefx.com                          |
/____________\/\\\_______\/\\\/////////\\\_\/\\\/////////\\\____/   07946 750075                                |
/_____________\/\\\_______\/\\\_______\/\\\_\/\\\_______\/\\\___/   Level 6 Computing for Animation Project     |
/______________\/\\\_______\/\\\_______\/\\\_\/\\\_______\/\\\__/   https://github.com/NCCA/CA1-2018-s4907224   |
/_______________\///________\///________\///__\///________\///__/                                               |
****************************************************************************************************************/
//---------------------------------------------------------------------------------------------------------------
/// @file taa_f.glsl
/// @brief The main TAA routine (i.e. where the bulk of the processing is happening for TAA to work)
//---------------------------------------------------------------------------------------------------------------
#version 460 core

//Inverse VP for the current frame to unproject the fragment
uniform mat4 inverseViewProjectionCURRENT;
//VP for the previous frame to then reproject the fragment to it's previous position.
uniform mat4 viewProjectionHISTORY;
//The three incoming textures from the current frame
layout(binding = 0) uniform sampler2D colourRENDER;
layout(binding = 1) uniform sampler2D velocityBUF;
layout(binding = 2) uniform sampler2D depthRENDER;
//The input result from the previous TAA pass
layout(binding = 3) uniform sampler2D colourANTIALIASED;
//Variables needed so that we're sampling the textures at the correct scale in each dimension.
uniform vec2 windowSize;
uniform vec2 pixelSize;
//Jitter so that we can unjitter the incoming rendered textures.
uniform vec2 jitter;
//uniform vec2 jitterHISTORY;
//How much of each frame we are keeping, this is controlled by the user.
uniform float feedback;
uniform vec4 clipInfo;
uniform float _FeedbackMin;
uniform float _FeedbackMax;


layout (location=0) out vec4 fragColor;

#define SOURCE_GAMMA_SPACE
#include "util/util.glsl"

#ifndef ZCMP_GT
#define ZCMP_GT(a, b) (a > b)
#endif

//From the vertex shader

in VS_OUT {
    vec2 texCoord;
} fs_in;

//Matrices to transform the colour space for neighbourhood clipping
uniform mat3 YCoCGMatrix = mat3(0.25f, 0.5f, -0.25f, 0.5f, 0.f, 0.5f, 0.25f, -0.5f, -0.25f);
uniform mat3 RGBMatrix = mat3(1.f, 1.f, 1.f, 1.f, 0.f, -1.f, -1.f, 1.f, -1.f);


const float FLT_EPS = 0.00000001f;



//Flag to be set when a fragment is clipped so that we can reduce flickering slightly
bool clipped = false;

/// @brief Function to transform RGB colour to YCoCg space.
/// @param _inRGB The RGB input
vec3 YCoCg(vec3 _inRGB)
{
  return _inRGB;
  return YCoCGMatrix * _inRGB;
}

/// @brief Function to transform YCoCg colour to RGB space.
/// @param _inYCoCg The YCoCg input
vec3 RGB(vec3 _inYCoCG)
{
  return _inYCoCG;
  return RGBMatrix * _inYCoCG;
}

/// @brief Function to sample the rendered texture and convert it to YCoCg (makes code more readable later)
/// @param _UV the coordinate we are sampling.
vec3 sampleRenderYCoCg(vec2 _UV)
{
  return YCoCg(texture(colourRENDER, _UV).rgb);
}

/// @brief Function to return the minimum YCoCg value of the current pixel and its 8 neighbours
/// @param _uvCURRENT the coordinate we are sampling.
vec3 minSample3x3(vec2 _uvCURRENT)
{
  vec3 minSamp = vec3(1.f, 1.f, 1.f);
  vec3 samp[9];
  samp[0] = sampleRenderYCoCg(_uvCURRENT);
  samp[1] = sampleRenderYCoCg(_uvCURRENT + vec2( pixelSize.x, 0.f));
  samp[2] = sampleRenderYCoCg(_uvCURRENT + vec2(-pixelSize.x, 0.f));
  samp[3] = sampleRenderYCoCg(_uvCURRENT + vec2(0.f,  pixelSize.y));
  samp[4] = sampleRenderYCoCg(_uvCURRENT + vec2(0.f, -pixelSize.y));
  samp[5] = sampleRenderYCoCg(_uvCURRENT + vec2( pixelSize.x,  pixelSize.y));
  samp[6] = sampleRenderYCoCg(_uvCURRENT + vec2(-pixelSize.x,  pixelSize.y));
  samp[7] = sampleRenderYCoCg(_uvCURRENT + vec2( pixelSize.x, -pixelSize.y));
  samp[8] = sampleRenderYCoCg(_uvCURRENT + vec2(-pixelSize.x, -pixelSize.y));

  for (int i = 0; i < 9; i++)
  {
    minSamp = min(samp[i], minSamp);
  }
  return minSamp;
}

/// @brief Function to return the minimum YCoCg value of the current pixel and its 4 direct neighbours
/// @param _uvCURRENT the coordinate we are sampling.
vec3 minSampleDirectNeighbours(vec2 _uvCURRENT)
{
  vec3 minSamp = vec3(1.f, 1.f, 1.f);
  vec3 samp[5];
  samp[0] = sampleRenderYCoCg(_uvCURRENT);
  samp[1] = sampleRenderYCoCg(_uvCURRENT + vec2( pixelSize.x, 0.f));
  samp[2] = sampleRenderYCoCg(_uvCURRENT + vec2(-pixelSize.x, 0.f));
  samp[3] = sampleRenderYCoCg(_uvCURRENT + vec2(0.f,  pixelSize.y));
  samp[4] = sampleRenderYCoCg(_uvCURRENT + vec2(0.f, -pixelSize.y));

  for (int i = 0; i < 5; i++)
  {
    minSamp = min(samp[i], minSamp);
  }
  return minSamp;
}

/// @brief Function to return the maximum YCoCg value of the current pixel and its 8 neighbours
/// @param _uvCURRENT the coordinate we are sampling.
vec3 maxSample3x3(vec2 _uvCURRENT)
{
  vec3 maxSamp = vec3(0.f, 0.f, 0.f);
  vec3 samp[9];
  samp[0] = sampleRenderYCoCg(_uvCURRENT);
  samp[1] = sampleRenderYCoCg(_uvCURRENT + vec2( pixelSize.x, 0.f));
  samp[2] = sampleRenderYCoCg(_uvCURRENT + vec2(-pixelSize.x, 0.f));
  samp[3] = sampleRenderYCoCg(_uvCURRENT + vec2(0.f,  pixelSize.y));
  samp[4] = sampleRenderYCoCg(_uvCURRENT + vec2(0.f, -pixelSize.y));
  samp[5] = sampleRenderYCoCg(_uvCURRENT + vec2( pixelSize.x,  pixelSize.y));
  samp[6] = sampleRenderYCoCg(_uvCURRENT + vec2(-pixelSize.x,  pixelSize.y));
  samp[7] = sampleRenderYCoCg(_uvCURRENT + vec2( pixelSize.x, -pixelSize.y));
  samp[8] = sampleRenderYCoCg(_uvCURRENT + vec2(-pixelSize.x, -pixelSize.y));

  for (int i = 0; i < 9; i++)
  {
    maxSamp = max(samp[i], maxSamp);
  }
  return maxSamp;
}

/// @brief Function to return the minimum YCoCg value of the current pixel and its 4 direct neighbours
/// @param _uvCURRENT the coordinate we are sampling.
vec3 maxSampleDirectNeighbours(vec2 _uvCURRENT)
{
  vec3 maxSamp = vec3(0.f, 0.f, 0.f);
  vec3 samp[5];
  samp[0] = sampleRenderYCoCg(_uvCURRENT);
  samp[1] = sampleRenderYCoCg(_uvCURRENT + vec2( pixelSize.x, 0.f));
  samp[2] = sampleRenderYCoCg(_uvCURRENT + vec2(-pixelSize.x, 0.f));
  samp[3] = sampleRenderYCoCg(_uvCURRENT + vec2(0.f,  pixelSize.y));
  samp[4] = sampleRenderYCoCg(_uvCURRENT + vec2(0.f, -pixelSize.y));

  for (int i = 0; i < 5; i++)
  {
    maxSamp = max(samp[i], maxSamp);
  }
  return maxSamp;
}

/// @brief Function to determine if the current fragment's history sample lies in or outside the bounding box of its neighours' chrominance values in the current frame.  If it does, it clips the colour against this bounding box, otherwise it returns the sample.
/// @param _colourSample the current fragment's history sample.
/// @param _uvCURRENT the coordinate we are sampling.
vec3 clipNeighbourhood(vec3 _colourSample, vec2 _uvCURRENT)
{
  vec3 colourMIN = mix(minSample3x3(_uvCURRENT), minSampleDirectNeighbours(_uvCURRENT), 0.5f);
  vec3 colourMAX = mix(maxSample3x3(_uvCURRENT), maxSampleDirectNeighbours(_uvCURRENT), 0.5f);

  vec3 YCoCgSample = YCoCg(_colourSample.rgb);

  vec3 aabbCentre = 0.5f * (colourMAX + colourMIN);
  vec3 aabbWidth = 0.5f * (colourMAX - colourMIN);
  vec3 centreToSample = YCoCgSample - aabbCentre;
  vec3 ctsUnitSpace = centreToSample / aabbWidth;
  float maximumDimension = max(abs(ctsUnitSpace.x), max(abs(ctsUnitSpace.y), abs(ctsUnitSpace.z)));
  if (maximumDimension > 1.f)
  {
    vec3 clippedColour = aabbCentre + (centreToSample / maximumDimension);
    clipped = true;
    return RGB(clippedColour);
  }
  else {return RGB(YCoCgSample);}
}

/// @brief Function to determine the coordinates in screen space of the neighbouring fragment with the closest depth value.  This allows us to `dilate' the velocity buffer when sampling it, as it is aliased.
/// @param _coord The coordinate of the fragment we are processing.
vec2 frontMostNeigbourCoord(vec2 _coord)
{
  float samp[9];
  samp[0] = texture(depthRENDER, _coord).r;
  samp[1] = texture(depthRENDER, _coord + vec2( pixelSize.x, 0.f)).r;
  samp[2] = texture(depthRENDER, _coord + vec2(-pixelSize.x, 0.f)).r;
  samp[3] = texture(depthRENDER, _coord + vec2(0.f,  pixelSize.y)).r;
  samp[4] = texture(depthRENDER, _coord + vec2(0.f, -pixelSize.y)).r;
  samp[5] = texture(depthRENDER, _coord + vec2( pixelSize.x,  pixelSize.y)).r;
  samp[6] = texture(depthRENDER, _coord + vec2(-pixelSize.x,  pixelSize.y)).r;
  samp[7] = texture(depthRENDER, _coord + vec2( pixelSize.x, -pixelSize.y)).r;
  samp[8] = texture(depthRENDER, _coord + vec2(-pixelSize.x, -pixelSize.y)).r;

  int neighbour = 0;
  float minSamp = samp[0];
  for (int i = 0; i < 9; i++)
  {
    if (samp[i] < minSamp) {minSamp = samp[i]; neighbour = i;}
  }
  //Switch statement to avoid a horrible if-else mess.
  switch (neighbour)
  {
    case 0:
      return vec2(0.f, 0.f) + _coord;
    case 1:
      return vec2(pixelSize.x, 0.f) + _coord;
    case 2:
      return vec2(-pixelSize.x, 0.f) + _coord;
    case 3:
      return vec2(0.f, pixelSize.y) + _coord;
    case 4:
      return vec2(0.f, -pixelSize.y) + _coord;
    case 5:
      return vec2(pixelSize.x, pixelSize.y) + _coord;
    case 6:
      return vec2(-pixelSize.x, pixelSize.y) + _coord;
    case 7:
      return vec2(pixelSize.x, -pixelSize.y) + _coord;
    case 8:
      return vec2(-pixelSize.x, -pixelSize.y) + _coord;
  }
}

vec4 getWorldSpacePosition(in vec2 uv, float depth) 
{
    float z = depth * 2.0 - 1.0;
    vec4 CVVPos = vec4((uv - jitter) * 2.f - 1.f, z, 1.f);
    vec4 worldSpacePosition = inverseViewProjectionCURRENT * CVVPos;
    return worldSpacePosition / worldSpacePosition.w;
}

vec2 getPreviousUV(in vec2 uvCurrent, float depth) 
{
    vec4 worldSpacePosition = getWorldSpacePosition(uvCurrent, depth);
    vec4 rp_cs_pos = viewProjectionHISTORY * worldSpacePosition;
    vec2 rp_ss_ndc = rp_cs_pos.xy / rp_cs_pos.w;
    return 0.5 * rp_ss_ndc + 0.5;
}

vec2 calcVelocity(in vec2 uvCurrent, in vec2 uvHistory) 
{
    vec2 vel = uvCurrent - uvHistory;
    // We ignore for now the velocity buffer TODO: reactivate!
    //vel = texture(velocityBUF, frontMostNeigbourCoord(uvCurrent - jitter)).rg;
    return vel;
}



vec4 clip_aabb(in vec3 aabb_min, in vec3 aabb_max, in vec4 p, in vec4 q)
{
#if USE_OPTIMIZATIONS
    // note: only clips towards aabb center (but fast!)
    vec3 p_clip = 0.5 * (aabb_max + aabb_min);
    vec3 e_clip = 0.5 * (aabb_max - aabb_min) + FLT_EPS;

    vec4 v_clip = q - vec4(p_clip, p.w);
    vec3 v_unit = v_clip.xyz / e_clip;
    vec3 a_unit = abs(v_unit);
    float ma_unit = max(a_unit.x, max(a_unit.y, a_unit.z));

    if (ma_unit > 1.0)
        return vec4(p_clip, p.w) + v_clip / ma_unit;
    else
        return q;// point inside aabb
#else
    vec4 r = q - p;
    vec3 rmax = aabb_max - p.xyz;
    vec3 rmin = aabb_min - p.xyz;

    const float eps = FLT_EPS;

    if (r.x > rmax.x + eps)
        r *= (rmax.x / r.x);
    if (r.y > rmax.y + eps)
        r *= (rmax.y / r.y);
    if (r.z > rmax.z + eps)
        r *= (rmax.z / r.z);

    if (r.x < rmin.x - eps)
        r *= (rmin.x / r.x);
    if (r.y < rmin.y - eps)
        r *= (rmin.y / r.y);
    if (r.z < rmin.z - eps)
        r *= (rmin.z / r.z);

    return p + r;
#endif
}




vec3 find_closest_fragment_3x3(in vec2 uv)
{
	vec2 dd = pixelSize;
	vec2 du = vec2(dd.x, 0.0);
	vec2 dv = vec2(0.0, dd.y);

	vec3 dtl = vec3(-1, -1, texture(depthRENDER, uv - dv - du).x);
	vec3 dtc = vec3( 0, -1, texture(depthRENDER, uv - dv).x);
	vec3 dtr = vec3( 1, -1, texture(depthRENDER, uv - dv + du).x);

	vec3 dml = vec3(-1, 0, texture(depthRENDER, uv - du).x);
	vec3 dmc = vec3( 0, 0, texture(depthRENDER, uv).x);
	vec3 dmr = vec3( 1, 0, texture(depthRENDER, uv + du).x);

	vec3 dbl = vec3(-1, 1, texture(depthRENDER, uv + dv - du).x);
	vec3 dbc = vec3( 0, 1, texture(depthRENDER, uv + dv).x);
	vec3 dbr = vec3( 1, 1, texture(depthRENDER, uv + dv + du).x);

	vec3 dmin = dtl;
	if (ZCMP_GT(dmin.z, dtc.z)) dmin = dtc;
	if (ZCMP_GT(dmin.z, dtr.z)) dmin = dtr;

	if (ZCMP_GT(dmin.z, dml.z)) dmin = dml;
	if (ZCMP_GT(dmin.z, dmc.z)) dmin = dmc;
	if (ZCMP_GT(dmin.z, dmr.z)) dmin = dmr;

	if (ZCMP_GT(dmin.z, dbl.z)) dmin = dbl;
	if (ZCMP_GT(dmin.z, dbc.z)) dmin = dbc;
	if (ZCMP_GT(dmin.z, dbr.z)) dmin = dbr;

	return vec3(uv + dd.xy * dmin.xy, dmin.z);
}





vec4 temporal_reprojection(vec2 ss_txc, vec2 ss_vel, float vs_dist)
{
    // read texels
//#if UNJITTER_COLORSAMPLES
    vec4 texel0 = texture(colourRENDER, ss_txc - jitter);
//#else
//    vec4 texel0 = texture(colourRENDER, ss_txc);
//#endif
    vec4 texel1 = texture(colourANTIALIASED, ss_txc + ss_vel); //-

    // calc min-max of current neighbourhood
//#if UNJITTER_NEIGHBORHOOD
    vec2 uv = ss_txc - jitter;
//#else
//    vec2 uv = ss_txc;
//#endif


#if MINMAX_3X3 || MINMAX_3X3_ROUNDED

    vec2 du = vec2(pixelSize.x, 0.0);
    vec2 dv = vec2(0.0, pixelSize.y);

    vec4 ctl = texture(colourRENDER, uv - dv - du);
    vec4 ctc = texture(colourRENDER, uv - dv);
    vec4 ctr = texture(colourRENDER, uv - dv + du);
    vec4 cml = texture(colourRENDER, uv - du);
    vec4 cmc = texture(colourRENDER, uv);
    vec4 cmr = texture(colourRENDER, uv + du);
    vec4 cbl = texture(colourRENDER, uv + dv - du);
    vec4 cbc = texture(colourRENDER, uv + dv);
    vec4 cbr = texture(colourRENDER, uv + dv + du);

    vec4 cmin = min(ctl, min(ctc, min(ctr, min(cml, min(cmc, min(cmr, min(cbl, min(cbc, cbr))))))));
    vec4 cmax = max(ctl, max(ctc, max(ctr, max(cml, max(cmc, max(cmr, max(cbl, max(cbc, cbr))))))));

    #if MINMAX_3X3_ROUNDED || USE_YCOCG || USE_CLIPPING
        vec4 cavg = (ctl + ctc + ctr + cml + cmc + cmr + cbl + cbc + cbr) / 9.0;
    #endif

    #if MINMAX_3X3_ROUNDED
        vec4 cmin5 = min(ctc, min(cml, min(cmc, min(cmr, cbc))));
        vec4 cmax5 = max(ctc, max(cml, max(cmc, max(cmr, cbc))));
        vec4 cavg5 = (ctc + cml + cmc + cmr + cbc) / 5.0;
        cmin = 0.5 * (cmin + cmin5);
        cmax = 0.5 * (cmax + cmax5);
        cavg = 0.5 * (cavg + cavg5);
    #endif

#elif MINMAX_4TAP_VARYING// this is the method used in v2 (PDTemporalReprojection2)

    const float _SubpixelThreshold = 0.5;
    const float _GatherBase = 0.5;
    const float _GatherSubpixelMotion = 0.1666;

    vec2 texel_vel = ss_vel / pixelSize.xy;
    float texel_vel_mag = length(texel_vel) * vs_dist;
    float k_subpixel_motion = clamp(0.0, 1.0, _SubpixelThreshold / (FLT_EPS + texel_vel_mag));
    float k_min_max_support = _GatherBase + _GatherSubpixelMotion * k_subpixel_motion;

    vec2 ss_offset01 = k_min_max_support * vec2(-pixelSize.x, pixelSize.y);
    vec2 ss_offset11 = k_min_max_support * pixelSize;
    vec4 c00 = texture(colourRENDER, uv - ss_offset11);
    vec4 c10 = texture(colourRENDER, uv - ss_offset01);
    vec4 c01 = texture(colourRENDER, uv + ss_offset01);
    vec4 c11 = texture(colourRENDER, uv + ss_offset11);

    vec4 cmin = min(c00, min(c10, min(c01, c11)));
    vec4 cmax = max(c00, max(c10, max(c01, c11)));

    #if USE_YCOCG || USE_CLIPPING
        vec4 cavg = (c00 + c10 + c01 + c11) / 4.0;
    #endif

#else
    #error "missing keyword MINMAX_..."
#endif

    // shrink chroma min-max
#if USE_YCOCG
    vec2 chroma_extent = vec2(0.25 * 0.5 * (cmax.r - cmin.r));
    vec2 chroma_center = texel0.gb;
    cmin.yz = chroma_center - chroma_extent;
    cmax.yz = chroma_center + chroma_extent;
    cavg.yz = chroma_center;
#endif

    // clamp to neighbourhood of current sample
#if USE_CLIPPING
    texel1 = clip_aabb(cmin.xyz, cmax.xyz, clamp(cavg, cmin, cmax), texel1);
#else
    texel1 = clamp(texel1, cmin, cmax);
#endif

    // feedback weight from unbiased luminance diff (t.lottes)
#if USE_YCOCG
    float lum0 = texel0.r;
    float lum1 = texel1.r;
#else
    float lum0 = rgb2luma(texel0.rgb);
    float lum1 = rgb2luma(texel1.rgb);
#endif
    float unbiased_diff = abs(lum0 - lum1) / max(lum0, max(lum1, 0.2));
    float unbiased_weight = 1.0 - unbiased_diff;
    float unbiased_weight_sqr = unbiased_weight * unbiased_weight;
    float k_feedback = mix(_FeedbackMin, _FeedbackMax, unbiased_weight_sqr);

    // output
    return mix(texel0, texel1, k_feedback);
}


// https://software.intel.com/en-us/node/503873
vec3 YCoCg_RGB(in vec3 c)
{
    // R = Y + Co - Cg
    // G = Y + Cg
    // B = Y - Co - Cg
    return clamp(vec3(0.0), vec3(1.0), 
        vec3(
            c.x + c.y - c.z,
            c.x + c.z,
            c.x - c.y - c.z
        ));
}


vec4 resolve_color(in vec4 c)
{
#if USE_YCOCG
    return vec4(YCoCg_RGB(c.rgb).rgb, c.a);
#else
    return c;
#endif
}



void main()
{
  /* 
//#if UNJITTER_REPROJECTION
    vec2 uvCURRENT = fs_in.texCoord;
//#else
//    vec2 uvCURRENT = fs_in.texCoord;
//#endif
   
#if USE_DILATION
    //--- 3x3 norm (sucks)
    //vec2 ss_vel = sample_velocity_dilated(_VelocityBuffer, uv, 1);
    //float vs_dist = depth_sample_linear(uv);

    //--- 5 tap nearest (decent)
    //vec3 c_frag = find_closest_fragment_5tap(uv);
    //vec2 ss_vel = texture(_VelocityBuffer, c_frag.xy).xy;
    //float vs_dist = depth_resolve_linear(c_frag.z);

    //--- 3x3 nearest (good)
    vec3 c_frag = find_closest_fragment_3x3(uvCURRENT);
    float depth = c_frag.z;
    vec2 uvHISTORY = getPreviousUV(c_frag.xy, depth);
    vec2 ss_vel = calcVelocity(uvCURRENT, uvHISTORY);
    
    //float depth = texture(depthRENDER, uvCURRENT).r;
#else
    vec2 uvHISTORY = getPreviousUV(uvCURRENT, depth);
    vec2 ss_vel = calcVelocity(uvCURRENT, uvHISTORY);
    float depth = texture(depthRENDER, uvCURRENT).r;
#endif 


 float vs_dist = reconstructViewSpaceZ(depth, clipInfo);
  
 fragColor = resolve_color(temporal_reprojection(uvCURRENT, ss_vel, vs_dist));*/

    vec2 uvCURRENT = fs_in.texCoord;
    float depth = texture(depthRENDER, uvCURRENT - jitter).r;  
    vec2 uvHISTORY = getPreviousUV(uvCURRENT, depth);
    vec2 ss_vel = calcVelocity(uvCURRENT, uvHISTORY);
    uvHISTORY += ss_vel;


  //Get current and previous color data
  vec4 colorCURRENT = texture(colourRENDER, uvCURRENT - jitter);
  vec4 colorHISTORY = texture(colourANTIALIASED, vec2(uvHISTORY));

  //Clip it
  vec3 colorHISTORYCLIPPED = clipNeighbourhood(colorHISTORY.rgb, uvHISTORY);
  
  
/*  // feedback weight from unbiased luminance diff (t.lottes)
#if USE_YCOCG
    float lum0 = colorCURRENT.r;
    float lum1 = colorHISTORY.r;
#else
    float lum0 = rgb2luma(colorCURRENT.rgb);
    float lum1 = rgb2luma(colorHISTORY.rgb);
#endif
    float unbiased_diff = abs(lum0 - lum1) / max(lum0, max(lum1, 0.2));
    float unbiased_weight = 1.0 - unbiased_diff;
    float unbiased_weight_sqr = unbiased_weight * unbiased_weight;
    float k_feedback = mix(_FeedbackMin, _FeedbackMax, unbiased_weight_sqr);
*/  
  
  

  if (colorHISTORY.a == 0.f) {fragColor.a = float(clipped);} //If there's nothing, store the clipped flag (could still be nothing)
  else {fragColor.a = mix(colorHISTORY.a, float(clipped), feedback);} //If there is something, blend the previous clipped value with the current one (using same feedback as rest of AA)

  //This just makes the next line easier to read
  float clipBlendFactor = fragColor.a;
  //Lerp based on recent clipping events
  vec3 colorHISTORYCLIPPEDBLEND = mix(colorHISTORY.rgb, colorHISTORYCLIPPED, clamp(clipBlendFactor, 0.f, 1.f));
  //Now we have our two colour values, lerp between them based on the feedback factor.
  fragColor.rgb = mix(colorHISTORYCLIPPEDBLEND, colorCURRENT.rgb, feedback);
 
}