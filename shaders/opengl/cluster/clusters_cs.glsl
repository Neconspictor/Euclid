/**
 * Divides the camera frustum into clusters
 */
#version 430 core

layout(local_size_x = 1, local_size_y = 1) in;


struct AABB
{
    vec4 min;
    vec4 max;
};


layout (std430, binding = 0) buffer ClusterAABB{
    AABB clusters[ ];
};
layout (std430, binding = 1) buffer Constants{
    mat4 invProj;
    uvec4 tileSizes;
    vec4 zNearFar; // x: near z value in view space; y: far z value in view space; z and w component unused
    uvec2 screenDimension;
};

/**
 * Transforms a clip space vector/position to view space.
 */
vec4 clipToView(vec4 clip);

/**
 * @param screen: x and y coordinate specify screen coordinates with origin at left bottom(!) corner. 
 *                z and w component are expected to be in clip space.
 * @return : the screen space position in view space.
 *
 * Note: Normally screen space is defined to have its origin at the left top corner. But for convenience in cluster creation
 * we use the left bottom corner.
 */
vec4 screen2View(vec4 screen);

/**
 * Provides an intersection point from a line and a plane. The line is specified by two points.
 * The plane is indirectly defined by a z value in view space which defines the (signed) distance from the origin (view space)
 * to the plane.
 *
 * @param firstPoint: starting point for defining the line. Is expected to be in view space.
 * @param secondPoint: The second point for defining the line. Is expected to be in view space.
 * @param zValueViewSpace: a z value in view space defining the (signed) distance from the origin to the plane.
 */
vec3 lineIntersectionToZPlane(vec3 firstPoint, vec3 secondPoint, float zValueViewSpace);


void main(void)
{
   //Eye position is zero in view space
    const vec3 eyePos = vec3(0.0);

    //Per tile variables
    
    // How many pixels in x does a square tile use
    uint tileSizePx = tileSizes[3];
    
    // Linear ID of the thread/cluster
    uint clusterIndex = gl_WorkGroupID.x +
                     gl_WorkGroupID.y * gl_NumWorkGroups.x +
                     gl_WorkGroupID.z * (gl_NumWorkGroups.x * gl_NumWorkGroups.y);

    //Calculating the min and max point in screen space with origin at bottom(!) left corner
    vec4 maxScreenSpace = vec4(vec2(gl_WorkGroupID.x + 1, gl_WorkGroupID.y + 1) * tileSizePx, -1.0, 1.0); // Top Right
    vec4 minScreenSpace = vec4(gl_WorkGroupID.xy * tileSizePx, -1.0, 1.0); // Bottom left
    
    //Pass min and max to view space
    vec3 maxViewSpace = screen2View(maxScreenSpace).xyz;
    vec3 minViewSpace = screen2View(minScreenSpace).xyz;

    //Near and far values of the cluster in view space
    float clusterNear  = zNearFar.x * pow(zNearFar.y / zNearFar.x, gl_WorkGroupID.z/float(gl_NumWorkGroups.z));
    float clusterFar   = zNearFar.x * pow(zNearFar.y / zNearFar.x, (gl_WorkGroupID.z + 1) /float(gl_NumWorkGroups.z));

    //Finding the 4 intersection points made from the maxPoint to the cluster near/far plane
    vec3 minNear = lineIntersectionToZPlane(eyePos, minViewSpace, clusterNear );
    vec3 minFar  = lineIntersectionToZPlane(eyePos, minViewSpace, clusterFar );
    vec3 maxNear = lineIntersectionToZPlane(eyePos, maxViewSpace, clusterNear );
    vec3 maxFar  = lineIntersectionToZPlane(eyePos, maxViewSpace, clusterFar );

    vec3 minAABB = min(min(minNear, minFar),min(maxNear, maxFar));
    vec3 maxAABB = max(max(minNear, minFar),max(maxNear, maxFar));

    //Getting the 
    clusters[clusterIndex].min  = vec4(minAABB , 0.0);
    clusters[clusterIndex].max  = vec4(maxAABB , 0.0);
}


//Creates a line from the eye to the screenpoint, then finds its intersection
//With a z oriented plane located at the given distance to the origin
vec3 lineIntersectionToZPlane(vec3 firstPoint, vec3 secondPoint, float zValueViewSpace){
    //Because this is a Z based normal this is fixed
    vec3 normal = vec3(0.0, 0.0, 1.0);

    vec3 lineDirection =  secondPoint - firstPoint;

    // Computing the intersection length for the line and the plane
    // For formula see 'Mathematics for 3D Game Programming and Computer Graphics', p.99, Eric Lengyel
    // Note: dot(normal, lineDirection) == lineDirection.z since normal.x == normal.y == 0
    // and normal.z == 1.0!
    // Anologously we get dot(normal, firstPoint) == firstPoint.z 
    float t = (-zValueViewSpace - firstPoint.z) / lineDirection.z;

    //Computing the actual xyz position of the point along the line
    vec3 result = firstPoint + t * lineDirection;

    return result;
}

vec4 clipToView(vec4 clip){
    //View space transform
    vec4 view = invProj * clip;

    //Perspective division    
    return view / view.w;
}

vec4 screen2View(vec4 screen){
    //Convert to NDC
    vec2 texCoord = screen.xy / screenDimension.xy;

    //Convert to clipSpace
    vec4 clip = vec4(vec2(texCoord.x, texCoord.y)* 2.0 - 1.0, screen.z, screen.w);

    return clipToView(clip);
}