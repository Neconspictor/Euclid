#version 330 core

in VS_OUT {
    vec2 texCoord;
} fs_in;

void main()
{ 
	//color = vec4(texture(depthMap, texCoordsFS).r);
		//float depthValue = texture(depthMap, texCoordsFS).r;
    //color = vec4(vec3(depthValue), 1.0); // orthographic
		//color = vec4(vec3(linearizeDepth(depthValue) / far_plane), 1.0); // perspective
}