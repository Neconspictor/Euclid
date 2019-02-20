#extension GL_EXT_texture_array : enable

float shadowCompare(sampler2DArrayShadow depths, vec4 uvZCompareBias){
    return shadow2DArray(depths, uvZCompareBias.xyzw).r;
}

float shadowCompare(sampler2DArray depths, vec4 uvZCompareBias){
    float depth = texture(depths, uvZCompareBias.xyz).r;
    return step(uvZCompareBias.w, depth);
}

float shadowLerp(sampler2DArrayShadow depths, vec2 size, vec2 uv, float projectedZ, float compare, float bias){
    vec2 texelSize = vec2(1.0)/size;
    vec2 f = fract(uv*size+0.5);
    vec2 centroidUV = (uv*size+0.5)/size;

    float lb = shadowCompare(depths, vec4(centroidUV +texelSize*vec2(0.0, 0.0), projectedZ, compare - bias));
	float lt = shadowCompare(depths, vec4(centroidUV +texelSize*vec2(0.0, 1.0), projectedZ, compare - bias));
	float rb = shadowCompare(depths, vec4(centroidUV +texelSize*vec2(1.0, 0.0), projectedZ, compare - bias));
	float rt = shadowCompare(depths, vec4(centroidUV +texelSize*vec2(1.0, 1.0), projectedZ, compare - bias));
	float a = mix(lb, lt, f.y);
	float b = mix(rb, rt, f.y);
    float c = mix(a, b, f.x);
    return c;
}

float shadowLerp(sampler2DArray depths, vec2 size, vec2 uv, float projectedZ, float compare, float bias){
    vec2 texelSize = vec2(1.0)/size;
    vec2 f = fract(uv*size+0.5);
    vec2 centroidUV = (uv*size+0.5)/size;

    float lb = shadowCompare(depths, vec4(centroidUV +texelSize*vec2(0.0, 0.0), projectedZ, compare - bias));
	float lt = shadowCompare(depths, vec4(centroidUV +texelSize*vec2(0.0, 1.0), projectedZ, compare - bias));
	float rb = shadowCompare(depths, vec4(centroidUV +texelSize*vec2(1.0, 0.0), projectedZ, compare - bias));
	float rt = shadowCompare(depths, vec4(centroidUV +texelSize*vec2(1.0, 1.0), projectedZ, compare - bias));
	float a = mix(lb, lt, f.y);
	float b = mix(rb, rt, f.y);
    float c = mix(a, b, f.x);
    return c;
}