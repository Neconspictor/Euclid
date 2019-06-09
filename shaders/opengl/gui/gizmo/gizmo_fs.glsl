#version 430 core

in vec3 interpolatedVertexColor;
out vec4 color;

/**
 * X axis : 0
 * Y axis : 1
 * Z axis : 2 
 * No axis: 3 
 */
uniform uint selectedAxis;

const vec3 selectedAxisColor = vec3(1,1,0);

void main()
{
    vec3 axisColor = normalize(interpolatedVertexColor);
    
    // Only one color channel is != 0
    // So this optimization can be used to avoid branches.
    const uint axis = uint(0*axisColor.r + 1*axisColor.g + 2*axisColor.b);

    const uint selected = uint(axis == selectedAxis);
    
    axisColor += selected * selectedAxisColor;
    
    //if axis is the selected one, we have to clear the blue channel
    axisColor.b = (1-selected) * axisColor.b;
    
    color = vec4(axisColor, 1.0);
}