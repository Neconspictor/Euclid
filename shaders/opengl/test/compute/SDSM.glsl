#ifndef SDSM_INCLUDE
#define SDSM_INCLUDE

// Uint version of the bounds structure for atomic usage
// NOTE: This version cannot represent negative numbers!
struct BoundsUint
{
    uvec3 minCoord;
    float _pad0;
    uvec3 maxCoord;
    float _pad1;
};

// Reset bounds to [0, maxFloat]
BoundsUint EmptyBoundsUint()
{
    BoundsUint b;
    b.minCoord = uint(0x7F7FFFFF).xxx;      // Float max
    b.maxCoord = uint(0x0).xxx;//int(0xff7fffff).xxx;      // Float min
    return b;
}

// Float version of structure for convenient
// NOTE: We still tend to maintain the non-negative semantics of the above for consistency
struct BoundsFloat
{
    vec3 minCoord; //base alignment: 4N
    float _pad0;
    vec3 maxCoord; //base alignment: 4N
    float _pad1;
};

BoundsFloat BoundsUintToFloat(BoundsUint u)
{
    BoundsFloat f;
    f.minCoord.x = uintBitsToFloat(u.minCoord.x);
    f.minCoord.y = uintBitsToFloat(u.minCoord.y);
    f.minCoord.z = uintBitsToFloat(u.minCoord.z);
    f.maxCoord.x = uintBitsToFloat(u.maxCoord.x);
    f.maxCoord.y = uintBitsToFloat(u.maxCoord.y);
    f.maxCoord.z = uintBitsToFloat(u.maxCoord.z);
    return f;
}

BoundsFloat EmptyBoundsFloat()
{
    BoundsFloat f;
    f.minCoord = vec3(1.0);
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