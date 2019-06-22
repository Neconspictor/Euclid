#version 420

in vec2 Frag_UV;
in vec4 Frag_Color;
out vec4 Out_Color;

layout(binding = 0) uniform samplerCube Texture;
uniform vec3 Axis; 

void main()
{
    /**
     * For more info, see https://www.nvidia.com/object/cube_map_ogl_tutorial.html
     * (Chapter 'Mapping Texture Coordinates to Cube Map Faces')
     */

    float transformedS = (Frag_UV.s - 0.5);
    
    // flip T
    float transformedT = -(Frag_UV.t - 0.5);

    // top
    //float x = transformedS;
    //float y = 0.5;
    //float z = transformedT;
    
    // bottom
    //float x = transformedS;
    //float y = -0.5;
    //float z = -transformedT;
    
    // front
    //float x = transformedS;
    //float y = -transformedT;
    //float z = 0.5;
    
    // back
    //float x = -transformedS;
    //float y = -transformedT;
    //float z = -0.5;
    
    // right
    float x = 0.5;
    float y = -transformedT;
    float z = -transformedS;
    
    // left
    //float x = -0.5;
    //float y = -transformedT;
    //float z = transformedS;
    
    Out_Color = Frag_Color * texture( Texture, normalize(vec3(x, y, z)));
}