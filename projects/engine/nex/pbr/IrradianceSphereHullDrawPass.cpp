#include <nex/pbr/IrradianceSphereHullDrawPass.hpp>

nex::IrradianceSphereHullDrawPass::IrradianceSphereHullDrawPass() : 
	TransformPass(Shader::create("pbr/probe/irradiance_sphere_hull_draw_vs.glsl", "pbr/probe/irradiance_sphere_hull_draw_fs.glsl"))
{
	bind();
	mColorUniform = {mShader->getUniformLocation("objectColor"), UniformType::VEC4};
	mPositionWsUniform = { mShader->getUniformLocation("probePositionWS"), UniformType::VEC3 };
	mProbeRadiusUniform = { mShader->getUniformLocation("probeRadius"), UniformType::FLOAT };
	mViewPortUniform = { mShader->getUniformLocation("viewport"), UniformType::VEC2 };
	mClipInfoUniform = { mShader->getUniformLocation("clipInfo"), UniformType::VEC3 };
	mDepthUniform = mShader->createTextureUniform("depth", UniformType::TEXTURE2D, 0);
	mInverseProjMatrixUniform = { mShader->getUniformLocation("inverseProjMatrix"), UniformType::MAT4 };

	mShader->setVec4(mColorUniform.location, glm::vec4(1.0f));
}

void nex::IrradianceSphereHullDrawPass::setColor(const glm::vec4& color)
{
	mShader->setVec4(mColorUniform.location, color);
}

void nex::IrradianceSphereHullDrawPass::setPositionWS(const glm::vec3& position)
{
	mShader->setVec3(mPositionWsUniform.location, position);
}

void nex::IrradianceSphereHullDrawPass::setProbeRadius(float radius)
{
	mShader->setFloat(mProbeRadiusUniform.location, radius);
}

void nex::IrradianceSphereHullDrawPass::setViewPort(const glm::vec2& viewPort)
{
	mShader->setVec2(mViewPortUniform.location, viewPort);
}

void nex::IrradianceSphereHullDrawPass::setClipInfo(const glm::vec3& clipInfo)
{
	mShader->setVec3(mClipInfoUniform.location, clipInfo);
}

void nex::IrradianceSphereHullDrawPass::setDepth(Texture* depth)
{
	mShader->setTexture(depth, Sampler::getLinear(), mDepthUniform.bindingSlot);
}

void nex::IrradianceSphereHullDrawPass::setInverseProjMatrix(const glm::mat4& mat)
{
	mShader->setMat4(mInverseProjMatrixUniform.location, mat);
}

nex::IrradianceSphereHullDrawTechnique::IrradianceSphereHullDrawTechnique() : Technique(&mIrradiancePass)
{
}

nex::IrradianceSphereHullDrawPass* nex::IrradianceSphereHullDrawTechnique::getIrradiancePass()
{
	return &mIrradiancePass;
}