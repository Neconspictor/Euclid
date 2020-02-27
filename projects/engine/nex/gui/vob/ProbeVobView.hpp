#pragma once

#include <nex/gui/vob/VobView.hpp>
#include <nex/gui/TextureView.hpp>

namespace nex {
	class ProbeManager;
}

namespace nex::gui
{

	class Picker;

	class ProbeVobView : public VobView {
	public:

		ProbeVobView(ProbeManager* probeManager, nex::Window* window);
		virtual ~ProbeVobView();

		bool draw(Vob* vob, Scene* scene, Picker* picker, Camera* camera, bool doOneTimeChanges) override;

	private:

		class SphericalHarmonicsToCubeMapSide;

		TextureView mBrdfView;
		TextureView mIrradianceView;
		TextureView mReflectionView;
		ProbeManager* mProbeManager;

		std::unique_ptr<SphericalHarmonicsToCubeMapSide> mShader;

		void renderCubeMapSideWithSH(const ImGUI_TextureDesc& desc, const glm::mat4& proj);
	};
}