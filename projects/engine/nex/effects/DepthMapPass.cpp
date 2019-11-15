#include <nex/effects/DepthMapPass.hpp>

nex::DepthMapPass::DepthMapPass() : SimpleTransformShader(ShaderProgram::create(
	"depth_pass_vs.glsl", "depth_pass_fs.glsl"))
{
}