#include <nex/gui/vob/ProbeVobView.hpp>
#include <imgui/imgui.h>
#include "nex/gui/ImGUI_Extension.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "nex/texture/TextureManager.hpp"
#include <nfd/nfd.h>
#include <nex/platform/Window.hpp>
#include <nex/resource/ResourceLoader.hpp>
#include <nex/gui/FileDialog.hpp>
#include <nex/mesh/MeshManager.hpp>
#include <boxer/boxer.h>
#include <nex/scene/Vob.hpp>
#include <nex/scene/Scene.hpp>
#include <nex/gui/Picker.hpp>
#include <nex/GI/Probe.hpp>
#include <nex/GI/ProbeManager.hpp>

namespace nex::gui
{
	ProbeVobView::ProbeVobView(ProbeManager* probeManager) : VobView(),
		mBrdfView({}, ImVec2(256, 256)),
		mIrradianceView({}, ImVec2(256, 256)),
		mReflectionView({}, ImVec2(256, 256)),
		mProbeManager(probeManager)
	{
		mShader = std::make_unique<SphericalHarmonicsToCubeMapSide>();
	}

	ProbeVobView::~ProbeVobView() = default;

	bool ProbeVobView::draw(Vob* vob, Scene* scene, Picker* picker, Camera* camera, bool doOneTimeChanges)
	{
		if (!VobView::draw(vob, scene, picker, camera, doOneTimeChanges)) {
			return false;
		}

		auto* probeVob = dynamic_cast<ProbeVob*>(vob);
		if (!probeVob) {
			return true;
		}

		auto* probe = probeVob->getProbe();
		const auto isIrrdadianceProbe = probe->getType() == Probe::Type::Irradiance;
		auto* factory = mProbeManager->getFactory();

		if (isIrrdadianceProbe) {
			ImGui::Text("Irradiance probe");
		}
		else {
			ImGui::Text("Reflection probe");
		}

		

		if (doOneTimeChanges) {
			auto& irradianceDesc = mIrradianceView.getTextureDesc();
			irradianceDesc.level = probe->getArrayIndex();

			auto& reflectionDesc = mReflectionView.getTextureDesc();
			reflectionDesc.level = probe->getArrayIndex();
		}

		if (ImGui::TreeNode("Brdf Lookup map"))
		{
			auto* texture = ProbeFactory::getBrdfLookupTexture();
			auto& lutDesc = mBrdfView.getTextureDesc();
			lutDesc.texture = texture;
			lutDesc.flipY = ImageFactory::isYFlipped();
			lutDesc.sampler = nullptr;

			mBrdfView.updateScale();
			mBrdfView.drawGUI();

			ImGui::TreePop();
		}

		if (isIrrdadianceProbe) {
			if (ImGui::TreeNode("Irradiance map"))
			{
				
				static bool useCubeMap = false;

				if (ImGui::Button("toogle cubemap/sh")) {
					useCubeMap = !useCubeMap;
				}

				auto& irradianceDesc = mIrradianceView.getTextureDesc();
				
				Texture* texture;

				if (useCubeMap) {
					texture = factory->getIrradianceMaps();
					irradianceDesc.customShadingFunc = std::nullopt;
				}
				else {
					texture = factory->getIrradianceSHMaps();
					irradianceDesc.customShadingFunc = std::bind(&nex::gui::ProbeVobView::renderCubeMapSideWithSH, this, std::placeholders::_1, std::placeholders::_2);
				}

				//
				
				irradianceDesc.texture = texture;
				irradianceDesc.flipY = ImageFactory::isYFlipped();
				irradianceDesc.sampler = nullptr;
				
				mIrradianceView.overwriteTextureSize(true, ImVec2(256, 256));
				mIrradianceView.updateTexture(true);
				mIrradianceView.setInterpretAsCubemap(true);
				mIrradianceView.drawGUI();

				ImGui::TreePop();
			}

			if (ImGui::Button("Reload shader")) {
				nex::RenderEngine::getCommandQueue()->push([&]() {
					try {
						mShader = std::make_unique<SphericalHarmonicsToCubeMapSide>();
					}
					catch (std::exception e) {
						LOG(Logger("ProbeVobView"), Error) << "Couldn't recompile shader: " << e.what();
					}
					
				});
			}
		}
		else {

			if (ImGui::TreeNode("Reflection map"))
			{
				auto* texture = factory->getReflectionMaps();
				auto& reflectionDesc = mReflectionView.getTextureDesc();
				reflectionDesc.texture = texture;
				reflectionDesc.flipY = ImageFactory::isYFlipped();
				reflectionDesc.sampler = nullptr;

				mReflectionView.updateTexture(true);
				mReflectionView.drawGUI();

				ImGui::TreePop();
			}
		}


		auto influenceType = probe->getInfluenceType();

		const char* items[2] = {
			"SPHERE",
			"BOX"
		};

		if (ImGui::Combo("Influence type", (int*)&influenceType, items, 2)) {
			probe->setInfluenceType(influenceType);
			picker->select(*scene, probeVob);
		}

		auto position = probe->getPosition();
		if (nex::gui::Vector3D(&position, "Influence center"))
			probe->setPosition(position);


		if (influenceType == Probe::InfluenceType::SPHERE) {
			auto radius = probe->getInfluenceRadius();
			if (ImGui::DragFloat("Influence radius", &radius, 0.1f, 0.0f, FLT_MAX)) {
				probe->setInfluenceRadius(radius);
			}
		}
		else {

			AABB2 box2(probe->getInfluenceBox());
			if (nex::gui::Vector3D(&box2.halfWidth, "Influence bounding box half width"))
				probe->setInfluenceBox(box2.halfWidth);
		}

		return true;
	}




	class ProbeVobView::SphericalHarmonicsToCubeMapSide : public Shader
	{
	public:
		SphericalHarmonicsToCubeMapSide() : 
			Shader(ShaderProgram::create("GI/probe/spherical_harmonics_to_cubemap_vs.glsl", "GI/probe/spherical_harmonics_to_cubemap_fs.glsl",
				nullptr, nullptr, nullptr, 
#ifndef USE_LEFT_HANDED_COORDINATE_SYSTEM
				{ "#define CONVERT_LH_TO_RH" } // Note: Imgui uses left-handed coordinate system, so we have to flip the z axis
#else
		{}
#endif		
			))
		{




			mProjMtx = { mProgram->getUniformLocation("ProjMtx"), nex::UniformType::MAT4 };
			mArrayIndex = { mProgram->getUniformLocation("arrayIndex"), nex::UniformType::INT };
			mCoefficients = mProgram->createTextureUniform("coefficients", UniformType::TEXTURE1D_ARRAY, 0);
			mCubeMapSide = { mProgram->getUniformLocation("cubeMapSide"), nex::UniformType::INT };
			mCubeMapSideView = { mProgram->getUniformLocation("cubeMapSideView"), nex::UniformType::MAT4 };
		}

		void setProjMtx(const glm::mat4& mat)
		{
			mProgram->setMat4(mProjMtx.location, mat);
		}

		void setCoefficients(const Texture1DArray* texture) {
			mProgram->setTexture(texture, Sampler::getPoint(), mCoefficients.bindingSlot);
		}

		void setArrayIndex(unsigned index) {
			mProgram->setInt(mArrayIndex.location, index);
		}

		void setCubeMapSide(unsigned side) {
			mProgram->setInt(mCubeMapSide.location, side);
		}

		void setCubeMapSideView(const glm::mat4& mat) {
			mProgram->setMat4(mCubeMapSideView.location, mat);
		}

		nex::Uniform mProjMtx;
		nex::UniformTex mCoefficients;
		nex::Uniform mArrayIndex;
		nex::Uniform mCubeMapSide;
		nex::Uniform mCubeMapSideView;
	};





	void ProbeVobView::renderCubeMapSideWithSH(const ImGUI_TextureDesc& desc, const glm::mat4& proj)
	{
		mShader->bind();
		mShader->setProjMtx(proj);
		mShader->setArrayIndex(desc.level);
		mShader->setCoefficients((Texture1DArray*)desc.texture);
		mShader->setCubeMapSide(static_cast<int>(desc.side));
		mShader->setCubeMapSideView(CubeMap::getViewLookAtMatrix(desc.side));
	}
}