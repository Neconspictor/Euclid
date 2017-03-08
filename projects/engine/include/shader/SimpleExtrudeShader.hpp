#pragma once
#include <shader/SimpleColorShader.hpp>


/**
 * This shader draws a scaled up version of a mesh in a simple color. 
 * The scaling is performed on the mesh vertex normals.
 */
class SimpleExtrudeShader : public SimpleColorShader
{
public:
	virtual ~SimpleExtrudeShader() {};

	/**
	 * Sets the extrude value for scaling the mesh vertices on their normals.
	 * A extrude value of 0 means no extrusion, 1.0f means means an extrusion 
	 * of one in screen space corrdinates along the vertex normal.
	 */
	virtual void setExtrudeValue(float extrudeValue) = 0;
};