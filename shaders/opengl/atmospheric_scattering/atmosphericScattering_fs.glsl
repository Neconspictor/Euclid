/**
 * The idea is to trace a ray for each pixel from the observer position. Along a fixed number of sampling 
 * points along that ray, rays are shot towards the sun. The depth of atmosphere traversed is measured. So 
 * the collected light at each sample point is know, and hence the reflected light (towards the observer) 
 * can be calculated.
 */


#version 420

uniform vec2 viewport;
uniform mat4 inv_proj;
uniform mat3 inv_view_rot;
uniform vec3 lightdir;

// phase (molecular reflection) factors
uniform float rayleigh_brightness;
uniform float mie_distribution;
uniform float mie_brightness;
uniform float spot_brightness;

uniform float surface_height; // in range [0.15, 1]

uniform uint step_count; // defines the sample count for light scattering

uniform float intensity; // the light intensity (strength)
uniform float scatter_strength;

uniform float rayleigh_collection_power;
uniform float mie_collection_power;

uniform float rayleigh_strength;
uniform float mie_strength;

out vec3 color;


/**
 * You can look the factors of absorption for different gases up. I used the absorption profile for
 * Nitrogen.
 * As you can see, it absorbs a lot of blue light, which means that if something travels a lot trough the 
 * atmosphere it gets redder. This is also the same profile you use to reflect, and hence the sky appears 
 * blue.
 */
const vec3 Kr = vec3(0.18867780436772762, 0.4978442963618773, 0.6616065586417131);



vec3 get_world_normal(){
    vec2 frag_coord = gl_FragCoord.xy/viewport;
    frag_coord = (frag_coord-0.5)*2.0;
    vec4 device_normal = vec4(frag_coord, 0.0, 1.0);
    vec3 eye_normal = normalize((inv_proj * device_normal).xyz);
    vec3 world_normal = normalize(inv_view_rot*eye_normal);
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
    float d = pow(1.0+g*g-2.0*g*alpha, 1.5);
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
float atmospheric_depth(vec3 position, vec3 dir){
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

    vec3 eyedir = get_world_normal();
    float alpha = dot(eyedir, lightdir);
    
    // reflection distribution factors
    
    /*
      Due to the nature of molecular reflection, light does not bounce of small particles and molecules the 
      way it bounces of solid surfaces. For Nitrogen it bounces least if you are perpendicular to the 
      incoming light direction. And for aerosols (dust/water) it bounces strongest the smaller the angle 
      between the light and viewing direction.
     */
    float rayleigh_factor = phase(alpha, -0.01)*rayleigh_brightness;
    float mie_factor = phase(alpha, mie_distribution)*mie_brightness;
    float spot = smoothstep(0.0, 15.0, phase(alpha, 0.9995))*spot_brightness;
    
    // atmospheric depth
    vec3 eye_position = vec3(0.0, surface_height, 0.0);
    float eye_depth = atmospheric_depth(eye_position, eyedir);
    float step_length = eye_depth/float(step_count);
    
    
    // I choose a small radius 0.15 smaller then the surface height, which makes it smooth and extend a 
    // bit beyond the horizon
    float eye_extinction = horizon_extinction(eye_position, eyedir, surface_height-0.15);
    //eye_extinction = 1.0f;
    
    // I need some variables to hold the collected light. Since Nitrogen and aerosols reflect different, 
    // I need two of them.
    vec3 rayleigh_collected = vec3(0.0, 0.0, 0.0);
    vec3 mie_collected = vec3(0.0, 0.0, 0.0);
    
    for(uint i=0; i<step_count; i++){
        float sample_distance = step_length*float(i);
        vec3 position = eye_position + eyedir*sample_distance;
        float extinction = horizon_extinction(position, lightdir, surface_height-0.35);
        float sample_depth = atmospheric_depth(position, lightdir);
        
        vec3 influx = absorb(sample_depth, vec3(intensity), scatter_strength)*extinction;
        
        rayleigh_collected += absorb(sample_distance, Kr*influx, rayleigh_strength);
        mie_collected += absorb(sample_distance, influx, mie_strength);
    }
    
    /*
     Each result collection needs to be multiplied by the eye extinction. It is also divided by the 
     step_count in order to normalize the value. The multiplication by the eye_depth is done in order to 
     simulate the behavior that the longer a ray travels in the atmosphere, the more light it can return 
     (because more particles between it and the observer reflected some light). The power is some 
     fine-tuning to adjust the result.
    */
    //eye_extinction = 1.0f;
    rayleigh_collected = (rayleigh_collected * 
                         pow(eye_depth, rayleigh_collection_power)) /float(step_count);
    mie_collected = (mie_collected * 
                        pow(eye_depth, mie_collection_power))/float(step_count);
                      
        
    //* eye_extinction
        
    // Finally the color is the summation of the collected light with the reflection distribution
    // factors.                    
    color = vec3(
        spot*mie_collected +
        mie_factor*mie_collected +
        rayleigh_factor*rayleigh_collected
    ); 

    color = 10.0*mix(color, rayleigh_collected, 1-eye_extinction);  

}