#version 420

in vec2 Frag_UV;
in vec4 Frag_Color;
out vec4 Out_Color;

layout(binding = 0) uniform samplerCube Texture;
uniform uint Side; 
uniform uint mipMapLevel;

void main()
{
    /**
     * For more info, see https://www.nvidia.com/object/cube_map_ogl_tutorial.html
     * (Chapter 'Mapping Texture Coordinates to Cube Map Faces')
     */

    float transformedS = (Frag_UV.s - 0.5);
    
    // flip T
    float transformedT = -(Frag_UV.t - 0.5);
    
    float x,y,z;
    
    
    switch(Side) {
        case(0):
            // right
            x = 0.5;
            y = -transformedT;
            z = -transformedS;
            break;
        case(1):
            // left
            x = -0.5;
            y = -transformedT;
            z = transformedS;
            break;    
        case(2):
            // top
            x = transformedS;
            y = 0.5;
            z = transformedT;
            break;
        case(3):
            // bottom
            x = transformedS;
            y = -0.5;
            z = -transformedT;
            break;    
        case(4):
            // front
            x = transformedS;
            y = -transformedT;
            z = 0.5;
            break;    
        case(5):
            // back
            x = -transformedS;
            y = -transformedT;
            z = -0.5;
            break;
    }
    
    Out_Color = Frag_Color * textureLod(Texture, normalize(vec3(x, y, z)), mipMapLevel);
}