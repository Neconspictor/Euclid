/**
 *  Gradient noise (from unity and translated to glsl) 
 */
 
vec2 gradientNoiseDir(vec2 p)
{
    // Permutation and hashing used in webgl-nosie goo.gl/pX7HtC
    p = mod(p, 289);
    float x = (34 * p.x + 1) * mod(p.x, 289) + p.y;
    x = (34 * x + 1) * mod(x, 289);
    x = fract(x / 41) * 2 - 1;
    return normalize(vec2(x - floor(x + 0.5), abs(x) - 0.5));
}

void gradientNoise(vec2 UV, float Scale, out float Out)
{ 
    vec2 p = UV * Scale;
    vec2 ip = floor(p);
    vec2 fp = fract(p);
    float d00 = dot(gradientNoiseDir(ip), fp);
    float d01 = dot(gradientNoiseDir(ip + vec2(0, 1)), fp - vec2(0, 1));
    float d10 = dot(gradientNoiseDir(ip + vec2(1, 0)), fp - vec2(1, 0));
    float d11 = dot(gradientNoiseDir(ip + vec2(1, 1)), fp - vec2(1, 1));
    fp = fp * fp * fp * (fp * (fp * 6 - 15) + 10);
    Out = mix(mix(d00, d01, fp.y), mix(d10, d11, fp.y), fp.x) + 0.5;
}