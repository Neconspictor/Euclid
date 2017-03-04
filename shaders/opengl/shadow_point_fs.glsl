#version 330 core
in vec4 fragPos;

uniform vec3 lightPos;
uniform float range;

void main()
{
    // get distance between fragment and light source
    float lightDistance = length(fragPos.xyz - lightPos);
    
    // map to [0;1] range by dividing by range
    lightDistance = lightDistance / range;
		//lightDistance = 0.5f;
    
    // Write this as modified depth
    gl_FragDepth = lightDistance;
}