#include <nex/gui/OceanGenerator.hpp>
#include <nex/renderer/RenderEngine.hpp>
#include <nex/water/Ocean.hpp>

nex::gui::OceanGenerator::OceanGenerator(nex::Scene* scene, CascadedShadow* csm, PSSR* pssr, const Camera* camera, const RenderContext* renderContext) : Drawable(),
mScene(scene), mCsm(csm), mPssr(pssr), mCamera(camera), mContext(renderContext)
{
}

nex::gui::OceanGenerator::~OceanGenerator() = default;

void nex::gui::OceanGenerator::setScene(nex::Scene* scene)
{
	mScene = scene;
}

void nex::gui::OceanGenerator::drawSelf()
{
	ImGui::Text("Ocean properties:");

	/*
	unsigned N, 
			unsigned maxWaveLength, 
			float dimension,
			float spectrumScale, 
			const glm::vec2& windDirection, 
			float windSpeed, 
			float periodTime,
			const glm::uvec2& tileCount,
	*/

	if (ImGui::InputScalar("Point number in one Dimension", ImGuiDataType_U32, &mN)) {
		unsigned log = log2l(mN);
		mN = std::powl(2, log);
	}
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("Number has to be a power of 2.\nThe resulting map will have the squared result of points.\nDefault value: 64");
	}

	ImGui::InputScalar("Max wave length", ImGuiDataType_U32, &mMaxWaveLength);
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("The maximum wave length (in frequency space).\nInfluences wave height and turbulence.\nDefault matches point number.\nDefault value: 64");
	}
	
	if (ImGui::InputFloat("World dimension", &mDimension)) {
		mDimension = std::fmaxf(mDimension, 0.0f);
	}
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("Default value: 10.0");
	}

	if (ImGui::InputFloat("Spectrum scale", &mSpectrumScale)) {
		mSpectrumScale = std::fmaxf(mSpectrumScale, 0.0f);
	}
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("Scales the wave spectrum.\nLower values flattens the waves but reduces geometry intersections.\nMin value: 0.0f\nDefault value: 1.0");
	}

	ImGui::InputFloat2("Wind direction", (float*)&mWindDirection);
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("Specifies wind direction in X-Z dimension.\nNote: Result will be normalized before ocean is created.\nDefault value: (0.707, 0.707)");
	}

	ImGui::SameLine();

	if (ImGui::Button("Normalize")) {
		mWindDirection = normalize(mWindDirection);
	}
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("Normalizes the wind direction.");
	}

	if (ImGui::InputFloat("Wind speed", &mWindSpeed)) {
		mWindSpeed = std::fmaxf(mWindSpeed, 0.0f);
	}
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("Min value: 0.0\nDefault value: 12.0");
	}

	if (ImGui::InputFloat("Murk", &mMurk)) {
		mMurk = std::clamp<float>(mMurk, 0.0f, 1.0f);
	}
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("Specifies how murky the water is.\nThis influences how deep an observer can see through the water.\nMin value: 0.0\nMax value: 1.0\nDefault value: 0.5");
	}

	if (ImGui::InputFloat("Period time (s)", &mPeriodTime)) {
		mPeriodTime = std::fmaxf(mPeriodTime, 0.0f);
	}
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("The time period (in seconds) before wave simulation is repeated.\nMin value: 0.0\nDefault value: 20.0");
	}

	if (ImGui::InputFloat("Roughness", &mRoughness)) {
		mMurk = std::clamp<float>(mRoughness, 0.0f, 1.0f);
	}
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("Defines roughness of water surface\nMin value: 0.0\nMax value: 1.0\nDefault value: 0.1");
	}

	ImGui::InputScalarN("Tile count", ImGuiDataType_U32, &mTileCount, 2);
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("Specifies the initial tile count in local X-Z dimension.\nDefault value: (1,1)");
	}


	if (ImGui::Button("Generate ocean")) {
		mWindDirection = normalize(mWindDirection);

		auto* commandQueue = RenderEngine::getCommandQueue();

		commandQueue->push([&]() {

			auto oceanVob = std::make_unique<OceanVob>();
			oceanVob->setPositionLocalToParent(mCamera->getPosition() + mCamera->getLook() * 2.0f);

			//ocean
			auto ocean = std::make_unique<OceanGPU>(
				mN, //N
				mMaxWaveLength, // maxWaveLength
				mDimension, //dimension
				mSpectrumScale, //spectrumScale
				mWindDirection, //windDirection
				mWindSpeed, //windSpeed
				mPeriodTime, //periodTime
				mTileCount, // tileCount
				mMurk, // murk
				mRoughness,
				mCsm,
				mPssr);

			ocean->init();

			auto* oceanVobPtr = (OceanVob*)oceanVob.get();

			oceanVobPtr->setOcean(std::move(ocean));
			oceanVobPtr->updateTrafo(true, true);
			oceanVobPtr->updateWorldTrafoHierarchy(true);
			oceanVobPtr->resize(mContext->windowWidth, mContext->windowHeight);
			auto lock = mScene->acquireLock();
			mScene->addVobUnsafe(std::move(oceanVob));
		});




	}
}