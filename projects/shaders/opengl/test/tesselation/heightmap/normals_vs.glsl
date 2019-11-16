#version 450 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

out VS_OUT {
    vec3 normal;
    vec3 tangent;
    vec3 bitangent;
    vec3 positionViewSpace;
} vs_out;

uniform mat4 modelView;
uniform mat4 transform;
uniform mat3 normalMatrix;

layout(binding = 0)uniform sampler2D heightMap;
uniform vec3 worldDimension;
// the segment count for the height map (for sampling height map)
// Note: tesselation shaders have to consider tesselation as well!
uniform vec2 segmentCount;


vec3 samplePosition(vec2 coord) {
    const float x = coord.x * worldDimension.x - worldDimension.x/2.0; //
    const float y = worldDimension.y * texture(heightMap, coord).r; 
    const float z = -(coord.y * worldDimension.z - worldDimension.z/2.0); // - worldDimension.z/2.0
    
    return vec3(x, y, z);
}

vec3 calcNormalFromTriangle(in vec3 a, in vec3 b, in vec3 c) {
    return cross(b-a, c-a);
}

vec3 calcTangent(in vec3 vecAB, in vec3 vecAC, in vec2 uvAB, in vec2 uvAC) {
    const float r = 1.0 / (uvAB.x * uvAC.y - uvAC.x * uvAB.y);
	return vec3((uvAC.y * vecAB.x - uvAB.y * vecAC.x) * r,
                (uvAC.y * vecAB.y - uvAB.y * vecAC.y) * r,
                (uvAC.y * vecAB.z - uvAB.y * vecAC.z) * r);
}

vec3 calcBitangent(in vec3 vecAB, in vec3 vecAC, in vec2 uvAB, in vec2 uvAC) {
    const float r = 1.0 / (uvAB.x * uvAC.y - uvAC.x * uvAB.y);
	return vec3((uvAB.x * vecAC.x - uvAC.x * vecAB.x) * r,
                (uvAB.x * vecAC.y - uvAC.x * vecAB.y) * r,
                (uvAB.x * vecAC.z - uvAC.x * vecAB.z) * r);
}

void calcTBNObjectSpace() {

    vec3 accumulatedNormal = vec3(0.0f);
    vec3 accumulatedTangent = vec3(0.0f);
    vec3 accumulatedBitangent = vec3(0.0f);
    
    const vec3 position = samplePosition(texCoords);
    const float SAMPLE_SIZE = 8.0;
    
    const vec2 bottomLeftUV = texCoords + vec2(-1.0/segmentCount.x, -1.0/segmentCount.y);
    const vec2 bottomUV = texCoords + vec2(0.0, -1.0/segmentCount.y);
    const vec2 bottomRightUV = texCoords + vec2(1.0/segmentCount.x, -1.0/segmentCount.y);
    const vec2 rightUV = texCoords + vec2(1.0/segmentCount.x, 0.0);
    const vec2 topRightUV = texCoords + vec2(1.0/segmentCount.x, 1.0/segmentCount.y);
    const vec2 topUV = texCoords + vec2(0.0, 1.0/segmentCount.y);
    const vec2 topLeftUV = texCoords + vec2(-1.0/segmentCount.x, 1.0/segmentCount.y);
    const vec2 leftUV = texCoords + vec2(-1.0/segmentCount.x, 0.0);

    const vec3 bottomLeft = samplePosition(bottomLeftUV);
    const vec3 bottom = samplePosition(bottomUV);
    const vec3 bottomRight = samplePosition(bottomRightUV);
    const vec3 right = samplePosition(rightUV);
    const vec3 topRight = samplePosition(topRightUV);
    const vec3 top = samplePosition(topUV);
    const vec3 topLeft = samplePosition(topLeftUV);
    const vec3 left = samplePosition(leftUV);
    
    // for each sample create a triangle in CCW and calculate face normal
    accumulatedNormal += calcNormalFromTriangle(position, bottomLeft, bottom);
    accumulatedNormal += calcNormalFromTriangle(position, bottom, bottomRight);
    accumulatedNormal += calcNormalFromTriangle(position, bottomRight, right);
    accumulatedNormal += calcNormalFromTriangle(position, right, topRight);
    accumulatedNormal += calcNormalFromTriangle(position, topRight, top);
    accumulatedNormal += calcNormalFromTriangle(position, top, topLeft);
    accumulatedNormal += calcNormalFromTriangle(position, topLeft, left);
    accumulatedNormal += calcNormalFromTriangle(position, left, bottomLeft);
    
    // average and normalize
    vs_out.normal = normalMatrix * normalize(accumulatedNormal / SAMPLE_SIZE);
    
    
    // tangents
    
    // bottom left
    vec3 vecAB = bottomLeft - position;
    vec3 vecAC = bottom - position;
    vec2 uvAB = bottomLeftUV - texCoords;
    vec2 uvAC = bottomUV - texCoords;
    accumulatedTangent += calcTangent(vecAB, vecAC, uvAB, uvAC);
    accumulatedBitangent += calcBitangent(vecAB, vecAC, uvAB, uvAC);
    
    //bottom 
    vecAB = bottom - position;
    vecAC = bottomRight - position;
    uvAB = bottomUV - texCoords;
    uvAC = bottomRightUV - texCoords;
    accumulatedTangent += calcTangent(vecAB, vecAC, uvAB, uvAC);
    accumulatedBitangent += calcBitangent(vecAB, vecAC, uvAB, uvAC);
    
    //bottomRight 
    vecAB = bottomRight - position;
    vecAC = right - position;
    uvAB = bottomRightUV - texCoords;
    uvAC = rightUV - texCoords;
    accumulatedTangent += calcTangent(vecAB, vecAC, uvAB, uvAC);
    accumulatedBitangent += calcBitangent(vecAB, vecAC, uvAB, uvAC);
    
    //right 
    vecAB = right - position;
    vecAC = topRight - position;
    uvAB = rightUV - texCoords;
    uvAC = topRightUV - texCoords;
    accumulatedTangent += calcTangent(vecAB, vecAC, uvAB, uvAC);
    accumulatedBitangent += calcBitangent(vecAB, vecAC, uvAB, uvAC);
    
    //topRight 
    vecAB = topRight - position;
    vecAC = top - position;
    uvAB = topRightUV - texCoords;
    uvAC = topUV - texCoords;
    accumulatedTangent += calcTangent(vecAB, vecAC, uvAB, uvAC);
    accumulatedBitangent += calcBitangent(vecAB, vecAC, uvAB, uvAC);
    
    //top 
    vecAB = top - position;
    vecAC = topLeft - position;
    uvAB = topUV - texCoords;
    uvAC = topLeftUV - texCoords;
    accumulatedTangent += calcTangent(vecAB, vecAC, uvAB, uvAC);
    accumulatedBitangent += calcBitangent(vecAB, vecAC, uvAB, uvAC);
    
    //topLeft 
    vecAB = topLeft - position;
    vecAC = left - position;
    uvAB = topLeftUV - texCoords;
    uvAC = leftUV - texCoords;
    accumulatedTangent += calcTangent(vecAB, vecAC, uvAB, uvAC);
    accumulatedBitangent += calcBitangent(vecAB, vecAC, uvAB, uvAC);
    
    //left 
    vecAB = left - position;
    vecAC = bottomLeft - position;
    uvAB = leftUV - texCoords;
    uvAC = bottomLeftUV - texCoords;
    accumulatedTangent += calcTangent(vecAB, vecAC, uvAB, uvAC);
    accumulatedBitangent += calcBitangent(vecAB, vecAC, uvAB, uvAC);
    
    
    vs_out.tangent = normalMatrix * normalize(accumulatedTangent / SAMPLE_SIZE);
    vs_out.bitangent = normalMatrix * normalize(accumulatedBitangent / SAMPLE_SIZE);
}

void main()
{
    const vec4 positionFinal = vec4(samplePosition(texCoords), 1.0);
    //const vec4 positionFinal = vec4(position, 1.0);
    gl_Position = transform * positionFinal;
    calcTBNObjectSpace();
    //vs_out.normal = normalize(normalMatrix * normal);
    //vs_out.tangent = normalize(normalMatrix * tangent);
    //vs_out.bitangent = normalize(normalMatrix * bitangent);
    vs_out.positionViewSpace = vec3(modelView * positionFinal); 
}