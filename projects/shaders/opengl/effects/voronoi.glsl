/**
 *  Voronoi generation (from Unity and translated to glsl) 
 */
 
vec2 voronoiRandomVector (vec2 UV, float offset)
{
    mat2 m = mat2(15.27, 47.63, 99.41, 89.98);
    UV = fract(sin(mul(UV, m)) * 46839.32);
    return vec2(sin(UV.y*+offset)*0.5+0.5, cos(UV.x*offset)*0.5+0.5);
}

void voronoi(vec2 UV, float AngleOffset, float CellDensity, out float Out, out float Cells)
{
    vec2 g = floor(UV * CellDensity);
    vec2 f = fract(UV * CellDensity);
    float t = 8.0;
    vec3 res = vec3(8.0, 0.0, 0.0);

    for(int y=-1; y<=1; y++)
    {
        for(int x=-1; x<=1; x++)
        {
            vec2 lattice = vec2(x,y);
            vec2 offset = voronoiRandomVector(lattice + g, AngleOffset);
            float d = distance(lattice + offset, f);

            if(d < res.x)
            {
                res = vec3(d, offset.x, offset.y);
                Out = res.x;
                Cells = res.y;
            }
        }
    }
}