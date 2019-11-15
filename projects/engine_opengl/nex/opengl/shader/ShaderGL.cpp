#include <nex/shader/Shader.hpp>
#include "nex/opengl/opengl.hpp"

void nex::ComputeShader::dispatch(unsigned workGroupsX, unsigned workGroupsY, unsigned workGroupsZ)
{
	GLCall(glDispatchCompute(workGroupsX, workGroupsY, workGroupsZ));
}
