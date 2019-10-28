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

		PSSR();
		~PSSR();

		Texture2D* getProjHashTexture();

		void renderProjectionHash(Texture* depth, const glm::mat4& viewProj, const glm::mat4& invViewProj, float waterHeight, 
			const glm::vec3& cameraDir);

		void resize(unsigned width, unsigned height);

	private:

		class ProjHashPass;
		class ProjHashClearPass;

		std::unique_ptr<Texture2D> mProjHashTexture;
		std::unique_ptr<ProjHashPass> mProjHashPass;
		std::unique_ptr<ProjHashClearPass> mProjHashClearPass;
	};
}