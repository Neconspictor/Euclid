#pragma once

#include <nex/shader/Pass.hpp>
#include <nex/shader/Technique.hpp>

namespace nex
{
	class IrradianceSphereHullDrawPass : public TransformPass
	{
	public:
		IrradianceSphereHullDrawPass();

		void setColor(const glm::vec4& color);
		void setPositionWS(const glm::vec3& position);
		void setProbeRadius(float radius);
		void setViewPort(const glm::vec2& viewPort);
		void setClipInfo(const glm::vec3& clipInfo);
		void setDepth(Texture* depth);
		void setInverseProjMatrix(const glm::mat4& mat);

	private:
		Uniform mColorUniform;
		Uniform mPositionWsUniform;
		Uniform mProbeRadiusUniform;
		Uniform mViewPortUniform;
		Uniform mClipInfoUniform;
		UniformTex mDepthUniform;
		Uniform mInverseProjMatrixUniform;

	};

	class IrradianceSphereHullDrawTechnique : public Technique
	{
	public:
		IrradianceSphereHullDrawTechnique();

		IrradianceSphereHullDrawPass* getIrradiancePass();

	private:
		IrradianceSphereHullDrawPass mIrradiancePass;
	};
}