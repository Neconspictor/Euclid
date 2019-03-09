#ifndef SDSM_INCLUDE
#define SDSM_INCLUDE

// Uint version of the bounds structure for atomic usage
// NOTE: This version cannot represent negative numbers!
struct BoundsUint
{
    uvec4 minCoord;
    uvec4 maxCoord;
};

// Float version of structure for convenient
// NOTE: We still tend to maintain the non-negative semantics of the above for consistency
struct BoundsFloat
{
    vec3 minCoord; //base alignment: 4N
    vec3 maxCoord; //base alignment: 4N
};

BoundsFloat EmptyBoundsFloat()
{
    BoundsFloat f;
    f.minCoord = vec3(9999999.0);
    f.maxCoord = vec3(0);
    
    return f;
}

struct Partition
{
    vec3 scale; // These are given in texture coordinate [0, 1] space
    float intervalBegin;
    vec3 bias;  // These are given in texture coordinate [0, 1] space
    float intervalEnd;
};


struct SurfaceData {
    float depth;
    vec3 positionView;
    vec3 lightTexCoord;
};


#endif // SDSM_INCLUDE