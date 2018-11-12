#pragma once
#include <nex/opengl/shader/ShaderGL.hpp>

/**
 * This shader draws a scaled up version of a mesh in a simple color.
 * The scaling is performed on the mesh vertex normals.
 */
class SimpleExtrudeShaderGL : public ShaderConfigGL
{
public:
	SimpleExtrudeShaderGL();

	virtual ~SimpleExtrudeShaderGL();

	const glm::vec4& getObjectColor() const;

	/**
	 * Sets the extrude value for scaling the mesh vertices on their normals.
	 * A extrude value of 0 means no extrusion, 1.0f means means an extrusion
	 * of one in screen space corrdinates along the vertex normal.
	 */
	void setExtrudeValue(float extrudeValue);

	void setObjectColor(glm::vec4 color);

	void update(const MeshGL& mesh, const TransformData& data) override;

private:
	glm::vec4 objectColor;
	float extrudeValue;
	glm::mat4 transform;
};