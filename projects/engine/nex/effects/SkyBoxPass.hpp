#pragma once
#include <nex/shader/Pass.hpp>

namespace nex
{
	class CubeMap;

	class SkyBoxPass : public Pass
	{
	public:
		SkyBoxPass();

		void setMVP(const glm::mat4& mat);
		void setProjection(const glm::mat4& mat);
		void setView(const glm::mat4& mat);

		void setSkyTexture(const CubeMap* sky);

	private:

		UniformTex mSkyTexture;
		Uniform mTransform;
		Uniform mProjection;
		Uniform mView;
	};

	class PanoramaSkyBoxPass : public Pass
	{
	public:
		PanoramaSkyBoxPass();

		void setProjection(const glm::mat4& mat);
		void setView(const glm::mat4& mat);
		void setSkyTexture(const Texture* tex);

	private:

		UniformTex mSkyTexture;
		Uniform mProjection;
		Uniform mView;
	};

	class EquirectangularSkyBoxPass : public Pass
	{
	public:
		EquirectangularSkyBoxPass();

		void setProjection(const glm::mat4& mat);
		void setView(const glm::mat4& mat);
		void setSkyTexture(const Texture* texture);

	private:
		UniformTex mSkyTexture;
		Uniform mProjection;
		Uniform mView;
	};
}