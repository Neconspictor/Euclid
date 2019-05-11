#version 450


in VS_OUT {
    vec3 normal;
    //vec2 texCoords;
} vs_out;

//in vec2 texCoord_tcs_in;

layout(location = 0)out vec4 color;
layout(location = 1)out vec4 luminance;
layout(location = 2)out vec2 motion;
layout(location = 3)out float depth;

uniform vec3 lightDirViewSpace;
uniform mat3 normalMatrix;

void main() {

   vec3 normal = normalize(vs_out.normal);
   vec3 lightDir = normalize(-lightDirViewSpace);

  float angle = max(dot(normal, lightDir), 0.0);
  color =  angle * vec4(1.0, 1.0, 1.0, 1.0) + vec4(0.1);
  luminance = vec4(0.0);
  motion = vec2(0.0);
  depth = gl_FragCoord.z;
}