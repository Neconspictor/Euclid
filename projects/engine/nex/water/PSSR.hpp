#pragma once


namespace nex {

	class Texture;
	class Texture2D;

	/**
	 * Implementation for planar screen space reflections based an article of Remi Genin:
	 * http://remi-genin.fr/blog/screen-space-plane-indexed-reflection-in-ghost-recon-wildlands/
	 */
	class PSSR {
	public:

		~PSSR();

		Texture2D* getProjHashBuffer();

		void renderProjectionHash(Texture* color, const glm::mat4& viewProj, const glm::mat4& invViewProj);

		void resize(unsigned width, unsigned height);

	private:

		class ProjHashPass;

		std::unique_ptr<Texture2D> mProjHasBuffer;
		std::unique_ptr<ProjHashPass> mProjHashPass;
	};
}