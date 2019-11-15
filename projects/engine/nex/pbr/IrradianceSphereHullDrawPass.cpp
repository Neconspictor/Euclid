#include <nex/pbr/IrradianceSphereHullDrawPass.hpp>

nex::IrradianceSphereHullDrawPass::IrradianceSphereHullDrawPass() : 
	TransformShader(ShaderProgram::create("pbr/probe/irradiance_sphere_hull_draw_vs.glsl", "pbr/probe/irradiance_sphere_hull_draw_fs.glsl"))
{
	bind();
	mColorUniform = {mProgram->getUniformLocation("objectColor"), UniformType::VEC4};
	mPositionWsUniform = { mProgram->getUniformLocation("probePositionWS"), UniformType::VEC3 };
	mProbeRadiusUniform = { mProgram->getUniformLocation("probeRadius"), UniformType::FLOAT };
	mViewPortUniform = { mProgram->getUniformLocation("viewport"), UniformType::VEC2 };
	mClipInfoUniform = { mProgram->getUniformLocation("clipInfo"), UniformType::VEC3 };
	mDepthUniform = mProgram->createTextureUniform("depth", UniformType::TEXTURE2D, 0);
	mInverseProjMatrixUniform = { mProgram->getUniformLocation("inverseProjMatrix"), UniformType::MAT4 };

	mProgram->setVec4(mColorUniform.location, glm::vec4(1.0f));
}

void nex::IrradianceSphereHullDrawPass::setColor(const glm::vec4& color)
{
	mProgram->setVec4(mColorUniform.location, color);
}

void nex::IrradianceSphereHullDrawPass::setPositionWS(const glm::vec3& position)
{
	mProgram->setVec3(mPositionWsUniform.location, position);
}

void nex::IrradianceSphereHullDrawPass::setProbeRadius(float radius)
{
	mProgram->setFloat(mProbeRadiusUniform.location, radius);
}

void nex::IrradianceSphereHullDrawPass::setViewPort(const glm::vec2& viewPort)
{
	mProgram->setVec2(mViewPortUniform.location, viewPort);
}

void nex::IrradianceSphereHullDrawPass::setClipInfo(const glm::vec3& clipInfo)
{
	mProgram->setVec3(mClipInfoUniform.location, clipInfo);
}

void nex::IrradianceSphereHullDrawPass::setDepth(Texture* depth)
{
	mProgram->setTexture(depth, Sampler::getLinear(), mDepthUniform.bindingSlot);
}

void nex::IrradianceSphereHullDrawPass::setInverseProjMatrix(const glm::mat4& mat)
{
	mProgram->setMat4(mInverseProjMatrixUniform.location, mat);
}