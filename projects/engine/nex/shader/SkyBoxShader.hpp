#pragma once
#include <nex/shader/Shader.hpp>

namespace nex
{
	class CubeMap;

	class SkyBoxShader : public Shader
	{
	public:
		SkyBoxShader();

		void setMVP(const glm::mat4& mat);
		void setProjection(const glm::mat4& mat);
		void setView(const glm::mat4& mat);

		void setSkyTexture(const CubeMap* sky);

		void setupRenderState() override;

		void reverseRenderState() override;

	private:

		UniformTex mSkyTexture;
		Uniform mTransform;
		Uniform mProjection;
		Uniform mView;
	};

	class PanoramaSkyBoxShader : public Shader
	{
	public:
		PanoramaSkyBoxShader();

		void setProjection(const glm::mat4& mat);
		void setView(const glm::mat4& mat);
		void setSkyTexture(const Texture* tex);

	private:

		UniformTex mSkyTexture;
		Uniform mProjection;
		Uniform mView;
	};

	class EquirectangularSkyBoxShader : public Shader
	{
	public:
		EquirectangularSkyBoxShader();

		void setProjection(const glm::mat4& mat);
		void setView(const glm::mat4& mat);
		void setSkyTexture(const Texture* texture);

	private:
		UniformTex mSkyTexture;
		Uniform mProjection;
		Uniform mView;
	};
}