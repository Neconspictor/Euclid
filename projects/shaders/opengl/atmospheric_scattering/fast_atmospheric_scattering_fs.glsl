//Based on Naty Hoffmann and Arcot J. Preetham. Rendering out-door light scattering in real time.
//http://renderwonk.com/publications/gdm-2002/GDM_August_2002.pdf

#define fov tan(radians(60.0))

#define cameraheight 50.//5e1 //50.

#define Gamma 2.2

#define Rayleigh 1.
#define Mie 1.
#define RayleighAtt 1.
#define MieAtt 1.2

//float g = -0.84;
//float g = -0.97;
float g = -0.9;

#if 1
vec3 _betaR = vec3(1.95e-2, 1.1e-1, 2.94e-1); 
vec3 _betaM = vec3(4e-2, 4e-2, 4e-2);
#else
vec3 _betaR = vec3(6.95e-2, 1.18e-1, 2.44e-1); 
vec3 _betaM = vec3(4e-2, 4e-2, 4e-2);
#endif


const float ts= (cameraheight / 2.5e5);

//vec3 Ds = normalize(vec3(0., 0., -1.)); //sun 

vec3 ACESFilm( vec3 x )
{
    float tA = 2.51;
    float tB = 0.03;
    float tC = 2.43;
    float tD = 0.59;
    float tE = 0.14;
    return clamp((x*(tA*x+tB))/(x*(tC*x+tD)+tE),0.0,1.0);
}

void mainImage( out vec4 fragColor, in vec2 uv, in vec3 lightdir, in vec3 viewDir) {
        
	float M = 1.0; //canvas.innerWidth/M //canvas.innerHeight/M --res
	vec3 O = vec3(0., cameraheight, 0.);
	//vec3 D = normalize(vec3(uv, -(fov*M)));

	vec3 color = vec3(0.);
    
	
	//viewDir.y -= 0.2f;
	viewDir = normalize(viewDir);
	
	if (viewDir.y < 0) {
		fragColor = vec4(0,0.0, pow(0.1, Gamma),1);
		return;
	}
	
	/*if (viewDir.y < -ts) {
		float L = - O.y / viewDir.y;
		O = O + viewDir * L;
        viewDir.y = -viewDir.y;
		viewDir = normalize(viewDir);
	}
    else{
     	float L1 =  O.y / viewDir.y;
		vec3 O1 = O + viewDir * L1;

    	vec3 D1 = vec3(1.);
    	D1 = normalize(viewDir);
    }*/

      // optical depth -> zenithAngle
      float sR = RayleighAtt / (1.0 * viewDir.y) ;
      float sM = MieAtt / (100.0 * viewDir.y) ;

  	  float cosine = clamp(dot(viewDir, lightdir),0.0,1.0);
      vec3 extinction = exp(-(_betaR * sR + _betaM * sM));

       // scattering phase
      float g2 = g * g;
      float fcos2 = cosine * cosine;
      float miePhase = Mie * pow(1. + g2 + 2. * g * cosine, -1.5) * (1. - g2) / (2. + g2);
        //g = 0;
      float rayleighPhase = Rayleigh;

      vec3 inScatter = (1. + fcos2) * vec3(rayleighPhase + _betaM / _betaR * miePhase);

      color = inScatter*(1.0-extinction); // *vec3(1.6,1.4,1.0)

        // sun
      //color += 0.47*vec3(1.6,1.4,1.0)*pow( cosine, 350.0 ) * extinction;
      // sun haze
      //color += 0.4*vec3(0.8,0.9,1.0)*pow( cosine, 2.0 )* extinction;
    
	  color = ACESFilm(color);
    
      color = pow(color, vec3(Gamma));
	  
	   
	  float sunExponent = 2400.0;
	  float sunGradient = 2.0 * pow( cosine, sunExponent );  
   
      vec3 skyColor = vec3(0,0,1);
	  vec3 sunColor = vec3(100,100,100);
      //color.rgb = normalize(mix(color , sunColor, sunGradient ));  
	  
    
	  fragColor = vec4(color, 1.);
}