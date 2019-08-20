#version 430

out vec4 FragColor;

in vec4 positionWorld;

void main()
{             
    // assign depthLinearDistance -z value;  Note: -z is positive (right handed)!
    float depthLinearDistance = 1.0 / gl_FragCoord.w;
    
    FragColor = vec4(length(positionWorld));
}  