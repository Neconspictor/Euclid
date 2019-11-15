#pragma once
#include <nex/shader/Shader.hpp>

namespace nex
{
	class CubeMap;

	class SkyBoxPass : public Shader
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

	class PanoramaSkyBoxPass : public Shader
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

	class EquirectangularSkyBoxPass : public Shader
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