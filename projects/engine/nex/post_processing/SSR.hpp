#pragma once

#include <memory>
#include <glm/glm.hpp>
#include <vector>

namespace nex
{
	class Texture;
	class RenderTarget;

	class SSR {
	public:
		
		SSR();
		~SSR();

		void renderReflections(Texture* depth, Texture* normalsVS, Texture* color, const glm::mat4& proj, const glm::mat4& invProj,
			const glm::vec4& clipInfo);
		Texture* getReflectionUV();
		const Texture* getReflectionUV() const;

		void resize(unsigned width, unsigned height);

	private:

		class SSRComputeUVPass;
		std::unique_ptr<SSRComputeUVPass> mSSRComputeUVPass;
		std::unique_ptr<RenderTarget> mRenderTarget;
	};
}