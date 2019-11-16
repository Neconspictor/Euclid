#version 330 core
#define M_PI 3.1415926535897932384626433832795
#define M_TWO_PI 6.283185307179586476925286766559
in vec3 texCoordsFS;
out vec4 FragColor;

uniform sampler2D panorama;


const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{		
    vec2 uv = SampleSphericalMap(normalize(texCoordsFS)); // make sure to normalize localPos
    vec3 color = texture(panorama, uv).rgb;
    
    FragColor = vec4(color, 1.0);
}