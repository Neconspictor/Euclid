#version 460 core


out VS_OUT {
	vec2 texCoord;
} vs_out;
    
    /**
     * Calculates a fullscreen triangles out of three vertex indices
     * The triangles will have the following positions:
     * (-1.0, -1.0), (3.0, -1.0), (-1.0, 3.0) 
     */
void main()
{
  uint idx = gl_VertexID % 3; // allows rendering multiple fullscreen triangles
  vec4 pos =  vec4(
      (float( idx     &1U)) * 4.0 - 1.0,
      (float((idx>>1U)&1U)) * 4.0 - 1.0,
      0, 1.0);
  gl_Position = pos;
  vs_out.texCoord = pos.xy * 0.5 + 0.5;
}