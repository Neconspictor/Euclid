#version 460 core

in vec2 Frag_UV;
in vec4 Frag_Color;
out vec4 Out_Color;

layout(binding = 0) uniform samplerCube Texture;
uniform uint Side; 
uniform int MipMapLevel;
uniform int UseTransparency;
uniform int UseGammaCorrection;
uniform int UseToneMapping;


vec3 getDirection(uint side, in vec2 uv) {
switch(side) {
		case 0: // positive X
			return vec3(1.0, -2.0 * uv.y + 1.0, -2.0 * uv.x + 1.0);
		case 1: // negative X
		return vec3(-1.0, -2.0 * uv.y + 1.0, 2.0 * uv.x - 1.0);
		case 2: // positive Y
		return vec3(2.0 * uv.x - 1.0, 1.0, 2.0 * uv.y - 1.0);
		case 3: // negative Y
		return vec3(2.0 * uv.x - 1.0, -1.0, -2.0 * uv.y + 1.0);
		case 4: // positive Z
		return vec3(2.0 * uv.x - 1.0, -2.0 * uv.y + 1.0, 1);
		case 5: // negative Z
		return vec3(-2.0 * uv.x + 1.0, -2.0 * uv.y + 1.0, -1);
	}
}

void main()
{    
	vec3 direction = getDirection(Side, Frag_UV);
	
    vec4 color = textureLod(Texture, normalize(direction), MipMapLevel);
    
    if (bool(UseToneMapping)) {
        const float exposure = 1.0;
        color *= exposure;
        color.rgb = color.rgb / (color.rgb + vec3(1.0));
    }
    
    if (bool(UseGammaCorrection)) {
        // gamma correct
        const float gamma = 2.2f;
        color.rgb = pow(color.rgb, vec3(1.0/gamma)); 
    }
    
    if (bool(UseTransparency)) {
        Out_Color = Frag_Color * color;
    } else {
        Out_Color = Frag_Color * vec4(color.rgb, 1.0);
    }
}



//old code; just for later references
    //
    // For more info, see https://www.nvidia.com/object/cube_map_ogl_tutorial.html
    // (Chapter 'Mapping Texture Coordinates to Cube Map Faces')
    //



    /*float transformedS = (Frag_UV.s - 0.5);
    
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
    }*/