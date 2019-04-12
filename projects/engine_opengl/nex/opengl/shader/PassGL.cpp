#include <nex/shader/Pass.hpp>
#include "nex/opengl/opengl.hpp"

void nex::ComputePass::dispatch(unsigned workGroupsX, unsigned workGroupsY, unsigned workGroupsZ)
{
	GLCall(glDispatchCompute(workGroupsX, workGroupsY, workGroupsZ));
}
