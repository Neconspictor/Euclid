#include <nex/effects/DepthMapPass.hpp>

nex::DepthMapPass::DepthMapPass() : SimpleTransformPass(Shader::create(
	"depth_pass_vs.glsl", "depth_pass_fs.glsl"))
{
}