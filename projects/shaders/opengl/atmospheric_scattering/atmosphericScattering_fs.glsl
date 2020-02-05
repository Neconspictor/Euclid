/**
 * The idea is to trace a ray for each pixel from the observer position. Along a fixed number of sampling 
 * points along that ray, rays are shot towards the sun. The depth of atmosphere traversed is measured. So 
 * the collected light at each sample point is know, and hence the reflected light (towards the observer) 
 * can be calculated.
 */


#version 460 core
layout(early_fragment_tests) in;


out vec4 color;
out vec4 luminance;
out vec2 motion;

#include "atmospheric_scattering/fast_atmospheric_scattering_fs.glsl"

#include "interface/buffers.h"


/**
 * You can look the factors of absorption for different gases up. I used the absorption profile for
 * Nitrogen.
 * As you can see, it absorbs a lot of blue light, which means that if something travels a lot trough the 
 * atmosphere it gets redder. This is also the same profile you use to reflect, and hence the sky appears 
 * blue.
 */
const vec3 Kr = vec3(0.18867780436772762, 0.4978442963618773, 0.666616065586417131);
const vec3 Kr2 = vec3(0.0714867780436772762, 0.17978442963618773, 0.8616065586417131);
//const vec3 Kr = vec3(0.18867780436772762, 0.4978442963618773, 0.6616065586417131);



vec3 get_world_normal(){
    vec2 frag_coord = gl_FragCoord.xy/constants.viewport.xy;
    frag_coord = (frag_coord-0.5)*2.0;
    vec4 device_normal = vec4(frag_coord, 0.0, 1.0);
    vec3 eye_normal = normalize((constants.invProjectionGPass * device_normal).xyz);
    vec3 world_normal = normalize(mat3(constants.invViewRotGPass) * eye_normal);
    return world_normal;
}

/**
 * We need a phase function to calculate different kinds of reflections.
 * This is taken straight from GPU Gems 2 , Chapter 16
 */
float phase(float alpha, float g){
    float a = 3.0*(1.0-g*g);
    float b = 2.0*(2.0+g*g);
    float c = 1.0+alpha*alpha;
    float d = pow(1.0+ g*g -2.0*g*alpha, 1.5); //g*g
    return (a/b)*(c/d);
}

/**
 * Calcs intersection of the view direction with the atmosphere.
 * This function is based on the line-sphere intersection algorithm by Paul Bourke (1992):
 * http://www.homer.com.au/webdoc/geometry/sphereline.htm
 * 
 * It is assumed, that the sphere (the atmosphere) has a radius of 1 unit and that the position 
 * is inside the sphere. The origin of the sphere is assumed to be (0,0,0)
 */
float atmospheric_depth(vec3 position, vec3 dir, vec3 lightDir){

	position.y -= 0.03 * (1 - abs(lightDir.y));
	//position.y = max(position.y, 0);
    float a = dot(dir, dir);
    float b = 2.0*dot(dir, position);
    float c = dot(position, position)-1.0;
    
    // position is inside sphere. Thus, the determinant will always be bigger than zero.
    float det = b*b-4.0*a*c;
    float detSqrt = sqrt(det);
    float q = (-b - detSqrt)/2.0;
    float t1 = c/q;
    return t1;
}

/**
 * Due to the fact that solid bodies block light, I should really have some function that can tell me 
 * how much a ray I occluded as well. After some experimentation I settled on projecting the ray 
 * direction onto the negative position. If u (the factor by which to multiply position) is bigger then 
 * 0, the ray points above the horizon. If it's smaller then some given radius, then It's assumed to be 
 * completely occluded. In between some crude approximation to angular extinction is used I arrived at 
 * after more experimenting. I'm pretty sure there's better ways to solve the problem, but this one was 
 * easy to implement.
 */
float horizon_extinction(vec3 position, vec3 dir, float radius){
    float u = dot(dir, -position);
    if(u<0.0){
        return 1.0;
    }
    vec3 near = position + u*dir;
    if(length(near) < radius){
        return 0.0;
    }
    else{
        vec3 v2 = normalize(near)*radius - position;
        float diff = acos(dot(normalize(v2), dir));
        return smoothstep(0.0, 1.0, pow(diff*2.0, 3.0));
    }
}


/**
 * I modelled the light absorption inside the atmosphere as an exponential function of the inverse depth. 
 * It is an exponential function because if light traverses a certain distance and 50% get absorbed, and it 
 * travels the same distance again, then 50% of 50% are absorbed.
 * The idea behind this is, that given a color, a certain amount of that color is absorbed. And the 
 * smaller the distance is, the less is absorbed. The power is taken to the base Kr, which is the color 
 * distribution of air (absorbs slightly reddish).
 */
vec3 absorb(float dist, vec3 color, float factor){
    return color-color*pow(Kr, vec3(factor/dist));
}


void main() {

    vec2 frag_coord = gl_FragCoord.xy/constants.viewport.xy;
    frag_coord = (frag_coord-0.5)*2.0;
	
	vec4 device_normal = vec4(frag_coord, 0.0, 1.0);
    vec3 eye_normal = normalize((constants.invProjectionGPass * device_normal).xyz);
    vec3 eyedir = normalize(mat3(constants.invViewRotGPass) * eye_normal);
	
	//mainImage(color, frag_coord, lightdir, eyedir);
	
	
	vec3 lightdir = -constants.dirLight.directionWorld.xyz;
    
    
	    
    
    float alpha = max(dot(eyedir, lightdir), 0.1);
	//alpha = min(alpha, 0.7);
    
    // reflection distribution factors
    
    //
    //  Due to the nature of molecular reflection, light does not bounce of small particles and molecules the 
    //  way it bounces of solid surfaces. For Nitrogen it bounces least if you are perpendicular to the 
    //  incoming light direction. And for aerosols (dust/water) it bounces strongest the smaller the angle 
    //  between the light and viewing direction.
    //
    float rayleigh_factor = phase(alpha, 0.1)*constants.atms_rayleigh_brightness;
    float mie_factor = phase(alpha, constants.atms_mie_distribution)*constants.atms_mie_brightness; //constants.atms_mie_distribution 
    float spot = smoothstep(0.0, 3.0, phase(alpha, 0.999985))*constants.atms_spot_brightness;
    
    // atmospheric depth
    vec3 eye_position = vec3(0.0, constants.atms_surface_height, 0.0);
    float eye_depth = atmospheric_depth(eye_position, eyedir, lightdir);
    float step_length = eye_depth/float(constants.atms_step_count);
    
    
    // I choose a small radius 0.15 smaller then the surface height, which makes it smooth and extend a 
    // bit beyond the horizon
    float eye_extinction = horizon_extinction(eye_position, eyedir, constants.atms_surface_height); //-0.15 //constants.atms_surface_height-0.15
    //eye_extinction = 1.0f;
    
    // I need some variables to hold the collected light. Since Nitrogen and aerosols reflect different, 
    // I need two of them.
    vec3 rayleigh_collected = vec3(0.0, 0.0, 0.0);
    vec3 mie_collected = vec3(0.0, 0.0, 0.0);
    
    for(uint i=0; i<constants.atms_step_count; i++){
        float sample_distance = step_length*float(i);
        vec3 position = eye_position + eyedir*sample_distance;
        float extinction = horizon_extinction(position, lightdir, constants.atms_surface_height); //constants.atms_surface_height-0.35
        float sample_depth = atmospheric_depth(position, lightdir, lightdir);
        
        vec3 influx = absorb(sample_depth, vec3(constants.atms_intensity), constants.atms_scatter_strength)*extinction;
        
        rayleigh_collected += absorb(sample_distance, Kr2*influx, constants.atms_rayleigh_strength);
        mie_collected += absorb(sample_distance, influx, constants.atms_mie_strength);
    }
    
    //
    // Each result collection needs to be multiplied by the eye extinction. It is also divided by the 
    // step_count in order to normalize the value. The multiplication by the eye_depth is done in order to 
    // simulate the behavior that the longer a ray travels in the atmosphere, the more light it can return 
    // (because more particles between it and the observer reflected some light). The power is some 
    // fine-tuning to adjust the result.
    //
    //eye_extinction = 1.0f;
    rayleigh_collected = (rayleigh_collected * 
                         pow(eye_depth, constants.atms_rayleigh_collection_power)) /float(constants.atms_step_count);
    mie_collected = (mie_collected * 
                        pow(eye_depth, constants.atms_mie_collection_power))/float(constants.atms_step_count);
                      
        
    //* eye_extinction
        
    // Finally the color is the summation of the collected light with the reflection distribution
    // factors.                    
    vec3 result = vec3(
        //pow(spot*mie_collected, vec3(1.0/Gamma)) +
		spot*mie_collected * 0.5 +
        mie_factor*mie_collected +
        rayleigh_factor*rayleigh_collected
    ); 

    result = mix(result, rayleigh_collected, 1-eye_extinction); 

//color = vec4(pow(result, vec3(Gamma)), 1.0);
color = vec4(result, 1.0);    


if (eyedir.y < 0) {
	color = vec4(vec3(pow(0.1, Gamma)),1);
}


luminance = vec4(pow(result.rgb, vec3(2.2)) + result.rgb * 0.1, 1.0);

    //motion vector
    
   // Note, that we render on the far plane -> depth is 1.0
   float depth = gl_FragCoord.z;
    // H is the viewport position at this pixel in the range -1 to 1.
   vec4 H = vec4(frag_coord.x, frag_coord.y, depth, 1);
    // Transform by the view-projection inverse.
   vec4 D = constants.invViewProjectionGPass * H;
    // Divide by w to get the world position.
   vec4 worldPos = D / D.w;
   
   
   // Current viewport position
   vec4 currentPos = H;
   // Use the world position, and transform by the previous view-
   // projection matrix.
   vec4 previousPos = constants.prevViewProjectionGPass * worldPos;
    // Convert to nonhomogeneous points [-1,1] by dividing by w.
    previousPos /= previousPos.w;
    // Use this frame's position and last frame's to compute the pixel
   // velocity.
   vec2 velocity = (currentPos.xy - previousPos.xy);
    
motion = velocity;//vec2(0,0);
}