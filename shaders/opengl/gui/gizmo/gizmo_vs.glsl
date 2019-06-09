#version 430 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

layout(binding = 0) buffer TransformBuffer {
    mat4 model;
    mat4 view;
    mat4 projection;
    mat4 transform;
    mat4 prevTransform;
    mat4 modelView;
    mat3 normalMatrix;
} transforms;

out vec3 interpolatedVertexColor;

void main()
{ 
    const float reciprScaleOnscreen = 0.2; // change value to match resolution.    = (2 * ObjectSizeOnscreenInPixels / ScreenWidthInPixels)
    float w = (transforms.transform * vec4(0,0,0, 1)).w;
    w *= reciprScaleOnscreen;
    gl_Position = transforms.transform * vec4(position, 1.0);
    
    interpolatedVertexColor = color;
} 