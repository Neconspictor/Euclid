#version 460 core

out vec4 FragColor;

in VS_OUT {	
    
    vec3 lightDirectionWorld;
    vec3 normalWorld;
    vec3 viewDirectionWorld;
    
    vec3 light_direction_tangent;
    vec3 view_direction_tangent;
    
    vec3 fragPosWorld;
    
    mat3 TBN;
    mat3 TBN_Inverse;
} fs_in;

vec3 phongModel(in vec3 normal, in vec3 diffuseColor, in vec3 lightDir, in vec3 viewDirection) {	
    vec3 ambient = 0.2 * diffuseColor;
    float sDotN = max(dot(lightDir, normal), 0.0 );
    vec3 diffuse = diffuseColor * sDotN;
    vec3 spec = vec3(0.0);
    if( sDotN > 0.0 ) {
		
		vec3 halfwayDir = normalize(lightDir + viewDirection); 
		float shininess = pow( max( dot(normal, halfwayDir), 0.0 ), 8.0 );
        spec = vec3(0.2) * shininess;	
	}
	
    return ambient + diffuse + spec;
}

#define SWITCH 3

void main()
{  

#if SWITCH == 0   
    // this variant doesn't work properly!
    const vec3 normal = vec3(0,0,1);
    const vec3 diffuse = vec3(1.0f,0,0);
    const vec3 viewDir = fs_in.view_direction_tangent;
    const vec3 lightDir = fs_in.light_direction_tangent;
#elif SWITCH == 1
    const vec3 normal = normalize(fs_in.normalWorld);
    const vec3 diffuse = vec3(1.0f,0,0);
    const vec3 viewDir = normalize(fs_in.viewDirectionWorld);
    const vec3 lightDir = normalize(fs_in.lightDirectionWorld);
#elif SWITCH == 2
    const vec3 normal = fs_in.TBN * vec3(0,0,1);
    const vec3 diffuse = vec3(1.0f,0,0);
    const vec3 viewDir = normalize(fs_in.viewDirectionWorld);
    const vec3 lightDir = normalize(fs_in.lightDirectionWorld);
#elif SWITCH == 3
    const vec3 normal = vec3(0,0,1);
    const vec3 diffuse = vec3(1.0f,0,0);
    const vec3 viewDir = fs_in.TBN_Inverse * normalize(fs_in.viewDirectionWorld);
    const vec3 lightDir = fs_in.TBN_Inverse * normalize(fs_in.lightDirectionWorld);
#endif  
    
	vec3 color = phongModel(normal, diffuse, lightDir, viewDir);
	
	FragColor = vec4(color, 1);
}