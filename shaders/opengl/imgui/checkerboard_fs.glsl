#version 420

in vec2 Frag_UV;
in vec4 Frag_Color;
out vec4 Out_Color;

float checker(vec2 uv, float repeats) 
{
  float cx = floor(repeats * uv.x);
  float cy = floor(repeats * uv.y); 
  float result = mod(cx + cy, 2.0);
  return sign(result);
}

void main (void)
{
  vec2 uv = Frag_UV;
  vec2 texsize = vec2(128,128);
  float checker_size = 20;
  uv.x *= texsize.x / texsize.y;
  float c = mix(1.0, 0.9, checker(uv, checker_size));
  vec4 color = vec4(c, c, c, 1.0);  
  Out_Color = Frag_Color *color;
}