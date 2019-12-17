#version 460 core

in vec3 interpolatedVertexColor;
in vec3 positionLocal;

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 luminance;


/**
 * X axis : 0
 * Y axis : 1
 * Z axis : 2 
 * No axis: 3 
 */
uniform uint selectedAxis;
uniform vec3 axisColor;

const vec3 selectedAxisColor = vec3(10,10,0);

void main()
{
    /*vec3 normalizedPosition = normalize(abs(positionLocal));
    
    uint axis = 0;
    vec3 axisColor = vec3(1,0,0);
    
    if (normalizedPosition.y > normalizedPosition[axis]) {
        axis = 1;
        axisColor = vec3(0,1,0);
    }
    
    if (normalizedPosition.z > normalizedPosition[axis]) {
        axis = 2;
        axisColor = vec3(0,0,1);
    }
    
    if (length(positionLocal) < 0.01) {
        axis = 3;
        axisColor = vec3(1.0);
    }*/
    
    vec3 finalColor = axisColor;
    
    
    // Only one color channel is != 0
    // So this optimization can be used to avoid branches.
    const uint axis = uint(0*finalColor.r + 1*finalColor.g + 2*finalColor.b);

    const uint selected = uint(axis == selectedAxis && axis < 3);
    
    
    
    finalColor += selected * selectedAxisColor;
    
    //if axis is the selected one, we have to clear the blue channel
    finalColor.b = (1-selected) * finalColor.b;
    
    
    color = vec4(finalColor, 1.0);
    
    
    /*return;
    const float dist = length(positionLocal);
    
    const float distX = length(positionLocal - vec3(1,0,0));
    const float distY = length(positionLocal - vec3(0,1,0));
    const float distZ = length(positionLocal - vec3(0,0,-1));
    
    if (dist < 0.1) {
        color = vec4(0.0, 0.0, 0.0, 1.0);
    } else if (distX < distY && distX < distZ){
        color = vec4(1.0, 0.0, 0.0, 1.0);
    } else if (distY < distZ) {
        color = vec4(0.0, 1.0, 0.0, 1.0);
    } else {
        color = vec4(0.0, 0.0, 1.0, 1.0);
    }*/
	
	luminance = vec4(0.0);
        
}