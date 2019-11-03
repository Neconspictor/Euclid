#version 450

in VS_OUT {
    vec2 texCoord;
} fs_in;

out vec3 fragColor;

layout(binding = 0) uniform sampler2D depthMap;
layout(binding = 1) uniform sampler2D normalMap;

uniform mat4 invProj;
uniform mat4 proj;


// Raymarching constants

// How far a fragment can reflect -> maximum length of the reflection ray.
const float maxDistance = 15;

// Percentage of how many fragments should be skipped while marching the reflection ray 
// during the first pass (hit finding)
// range: [0,1]
// Zero means no reflections and 1 means that all (100%) fragments on the ray direction will be tested.
const float resolution = 0.3;

// Determines the cutoff (in view space) what counts as a possible reflection hit and what does not.
// -> Should be as small as possible.
const float thickness = 0.5;


vec3 computeViewPositionFromDepth(in vec2 texCoord, in float depth) {
  vec4 clipSpaceLocation;
  clipSpaceLocation.xy = texCoord * 2.0f - 1.0f;
  clipSpaceLocation.z = depth * 2.0f - 1.0f;
  clipSpaceLocation.w = 1.0f;
  vec4 homogenousLocation = invProj * clipSpaceLocation;
  return homogenousLocation.xyz / homogenousLocation.w;
};

void main() 
{

    vec2 texSize = textureSize(depthMap, 0).xy;
    
    // Iteration count in the second pass   
    int steps = 10;
    
    vec2 texCoord = fs_in.texCoord;
    
    vec3 positionFrom = computeViewPositionFromDepth(texCoord, texture(depthMap, texCoord).r);
    vec3 positionTo; // Position of the intersection point 
    vec3 unitPositionFrom = normalize(positionFrom);
    vec3 normal = normalize(texture(normalMap, texCoord).xyz);
    vec3 pivot = normalize(reflect(unitPositionFrom, normal)); //reflection ray
    
    // Start and end point of the reflection ray in viewspace
    vec4 startView = vec4(positionFrom, 1);
    vec4 endView = vec4(positionFrom + pivot * maxDistance, 1);
    
    vec4 startFrag = startView;
    vec4 endFrag = endView;
    
        startFrag = proj * startFrag; // clip space
        endFrag = proj *  endFrag;
        
        startFrag.xyz /= startFrag.w; // perspective division
        endFrag.xyz /= endFrag.w;
        
        startFrag.xy = 0.5 * startFrag.xy + 0.5; // texture space
        endFrag.xy = 0.5 * endFrag.xy + 0.5;
        
        startFrag.xy *= texSize; // screen space
        endFrag.xy *=  texSize;
        
    
    vec2 frag = startFrag.xy;
    vec3 uv;
    uv.xy = frag / texSize;
        
    // deltas in x and y direction. We use the larger delta for marching
    // The larger delta will help to determine how much to travel in X and Y direction in each iteration,
    // how many iterations are needed to travel the entire line, and what percentage of the line the 
    // current position does represent.
    float deltaX = endFrag.x - startFrag.x;
    float deltaY = endFrag.y - startFrag.y;
    
    float useX = abs(deltaX) >= abs(deltaY) ? 1 : 0;
   
    float delta = mix(abs(deltaY), abs(deltaX), useX) * clamp(resolution, 0, 1);
    
    // How much to increment the X and Y position.
    vec2 increment = vec2(deltaX, deltaY) / max(delta, 0.001);
    
    // last position percentage (startFrag -> endFrag)
    float search0 = 0;
    
    // current position percentage (startFrag -> endFrag)
    float search1 = 0;    
    
    // records if there was a hit in the first pass (hit finding)
    int hit0;
    
    // records if there was a hit in the second pass (hit refining)
    int hit1;
    
    // records how far away from the camera the current point on the ray is (viewspace).
    //float viewDistance = -startView.z;
    
    // view distance difference between the current ray point and scene position.
    // It tells us how far behind or in front of the scene the ray currently is.
    float depth = thickness;
    
    // first pass (hit finding)
    for (int i = 0; i < int(delta); ++i) {
        frag += increment;
        uv.xy = frag / texSize;
        positionTo = computeViewPositionFromDepth(uv.xy, texture(depthMap, uv.xy).r);
        
        search1 = mix((frag.y - startFrag.y) / deltaY, (frag.x - startFrag.x) / deltaX, useX);
        
        // Note: We need perspective correct interpolation!
        // https://www.comp.nus.edu.sg/~lowkl/publications/lowk_persp_interp_techrep.pdf
        float viewDistance = (-startView.z * (-endView.z)) / mix(-endView.z, -startView.z, search1);
        depth = viewDistance - (-positionTo.z);
        
        if (depth > 0 && depth < thickness) {
            hit0 = 1;
            break;
        } else {
            search0 = search1; 
        }
    }
    
    //first pass has finished
    
    //for the second pass we began at half distance between miss and hit 
    search1 = search0 + ((search1 - search0) / 2.0);
    
    // sets steps to zero if no hit was found in first pass
    steps *= hit0;
    
    // second pass (hit refining)
    for (int i = 0; i < steps; ++i) {
        frag = mix(startFrag.xy, endFrag.xy, search1);
        uv.xy = frag / texSize;
        positionTo = computeViewPositionFromDepth(uv.xy, texture(depthMap, uv.xy).r);
        float viewDistance = (-startView.z * (-endView.z)) / mix(-endView.z, -startView.z, search1);
        depth = viewDistance - (-positionTo.z);
        
        if (depth > 0 && depth < thickness) {
            hit1 = 1;
            search1 = search0 + (search1 - search0)/2.0;
        } else {
            float temp = search1;
            search1 = search0 + (search1 - search0)/2.0;
            search0 = temp;
        }
    }
    
    // second pass finished
    
    float visibility = 
        hit1
        
        //* positionTo.a // we have no alpha data (since we use depth buffer)
        // TODO: For transparency maybe use position map(?)
        
        // If the reflection ray points toward the camera and hits something,
        // it's most likely hitting the back side of something faceing away from the camera.
        // Gradually fade out reflection
        * (1.0 - max(dot(-unitPositionFrom, pivot), 0))
        
        // Fade out the further away the intersection point of the reflection ray is.
        * (1.0 - clamp(depth/thickness, 0, 1))
        
        // Fade out reflection based on how far away the reflected point is from the initial starting point.
        * (1.0 - clamp(length(positionTo - positionFrom) / maxDistance, 0, 1))
        
        //visibility should be zero if reflected uv coordinate is out of bounds.
        // Actually shouldn't happen -> TODO
        * (uv.x < 0 || uv.x > 1 ? 0 : 1)
        * (uv.y < 0 || uv.y > 1 ? 0 : 1);
        
    visibility = clamp(visibility, 0, 1);
    
    // we store the visibility in the z coordinate of the uv vector
    uv.z = visibility; 
        
    

    fragColor = uv;
}