#include <nex/effects/BlitPass.hpp>

nex::BlitPass::BlitPass() : Pass(Shader::create("screen_space_vs.glsl", "blit_fs.glsl"))
{
}

void nex::BlitPass::setInput(Texture* texture)
{
}
