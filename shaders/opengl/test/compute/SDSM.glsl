#ifndef SDSM_INCLUDE
#define SDSM_INCLUDE

// The number of partitions to produce
#ifndef PARTITIONS
#define PARTITIONS 4
#endif 

// Uint version of the bounds structure for atomic usage
// NOTE: This version cannot represent negative numbers!
struct BoundsInt
{
    ivec3 minCoord;
    ivec3 maxCoord;
};

// Reset bounds to [0, maxFloat]
BoundsInt EmptyBoundsInt()
{
    BoundsInt b;
    b.minCoord = int(0x7F7FFFFF).xxx;      // Float max
    b.maxCoord = int(0x0).xxx;//int(0xff7fffff).xxx;      // Float min
    return b;
}

// Float version of structure for convenient
// NOTE: We still tend to maintain the non-negative semantics of the above for consistency
struct BoundsFloat
{
    vec3 minCoord;
    vec3 maxCoord;
};

BoundsFloat BoundsIntToFloat(BoundsInt u)
{
    BoundsFloat f;
    f.minCoord.x = intBitsToFloat(u.minCoord.x);
    f.minCoord.y = intBitsToFloat(u.minCoord.y);
    f.minCoord.z = intBitsToFloat(u.minCoord.z);
    f.maxCoord.x = intBitsToFloat(u.maxCoord.x);
    f.maxCoord.y = intBitsToFloat(u.maxCoord.y);
    f.maxCoord.z = intBitsToFloat(u.maxCoord.z);
    return f;
}

BoundsFloat EmptyBoundsFloat()
{
    BoundsFloat f;
    f.minCoord = vec3(3.402823466e+38F);
    f.maxCoord = vec3(0);//vec3(-3.402823466e+38F);
    
    return f;
}


struct SurfaceData {
    uint depth;
    vec3 positionView;
    vec3 lightTexCoord;
};


#endif // SDSM_INCLUDE