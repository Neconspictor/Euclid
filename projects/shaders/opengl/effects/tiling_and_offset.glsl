//
//  Tiling and offset (from Unity and translated to glsl) 
//
 
void tilingAndOffset(vec2 UV, vec2 Tiling, vec2 Offset, out vec2 Out)
{
    Out = UV * Tiling + Offset;
}