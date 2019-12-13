#include <nex/gui/TextureView.hpp>
#include <nex/texture/Texture.hpp>
#include <nex/math/Math.hpp>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>
#include <nex/shader/Shader.hpp>
#include <nex/renderer/RenderBackend.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class nex::gui::TextureView::CheckerboardPattern : public Shader
{
public:
	CheckerboardPattern() : Shader(ShaderProgram::create("imgui/checkerboard_vs.glsl", "imgui/checkerboard_fs.glsl"))
	{
		mProjMtx = { mProgram->getUniformLocation("ProjMtx"), nex::UniformType::MAT4 };
	}

	void setProjMtx(const glm::mat4& mat)
	{
		mProgram->setMat4(mProjMtx.location, mat);
	}

	nex::Uniform mProjMtx;
};


nex::gui::TextureView::TextureView(const ImGUI_TextureDesc& textureDesc, const ImVec2& viewSize) : 
	mDesc(textureDesc), 
	mViewSize(viewSize),
mScale(1.0f), 
mOpacity(1.0f), 
mScrollPaneID(mId + "ScrollPane"), 
mSelectedFiltering(0),
mShowMipMapSelection(true),
mShowScaleConfig(true),
mShowOpacityConfig(true),
mShowShowTransparencyConfig(true),
mShowToneMappingConfig(true),
mShowFilteringConfig(true)

{
	updateScale();
	SamplerDesc state;
	state.minFilter = TexFilter::Near_Mipmap_Near;
	mSampler.setState(state);
}

nex::gui::ImGUI_TextureDesc& nex::gui::TextureView::getTextureDesc()
{
	return mDesc;
}

void nex::gui::TextureView::updateTexture(bool updateScaleWhenChanged)
{
	auto oldSize = mTextureSize;
	mTextureSize = calcTextureSize(mDesc);
	if (updateScaleWhenChanged)
	{
		if (mTextureSize.x != oldSize.x && mTextureSize.y != oldSize.y)
		{
			updateScale();
		} 
	}
}

void nex::gui::TextureView::updateScale()
{
	auto minTex = std::min<float>(mTextureSize.x, mTextureSize.y);
	auto minView = std::min<float>(mViewSize.x, mViewSize.y);
	mScale = minView / minTex;

	if (!isValid(mScale)) mScale = 1.0f;
}

void nex::gui::TextureView::setViewSize(const ImVec2& size)
{
	mViewSize = size;
}

const ImVec2& nex::gui::TextureView::getViewSize() const
{
	return mViewSize;
}

const ImVec2& nex::gui::TextureView::getTextureSize() const
{
	return mTextureSize;
}

void nex::gui::TextureView::showMipMapSelection(bool show)
{
	mShowMipMapSelection = show;
}

void nex::gui::TextureView::showScaleConfig(bool show)
{
	mShowScaleConfig = show;
}

void nex::gui::TextureView::showOpacityConfig(bool show)
{
	mShowOpacityConfig = show;
}

void nex::gui::TextureView::showShowTransparencyConfig(bool show)
{
	mShowShowTransparencyConfig = show;
}

void nex::gui::TextureView::showToneMappingConfig(bool show)
{
	mShowToneMappingConfig = show;
}

void nex::gui::TextureView::showFilteringConfig(bool show)
{
	mShowFilteringConfig = show;
}

void nex::gui::TextureView::addCheckBoardPattern(const ImVec2& size)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return;

	ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
	ImGui::ItemSize(bb);
	if (!ImGui::ItemAdd(bb, 0))
		return;

	auto* drawList = ImGui::GetWindowDrawList();
	static ImDrawCallback test = [](const ImDrawList* parent_list, const ImDrawCmd* cmd)-> void
	{
		TextureView* view = (TextureView*)cmd->UserCallbackData;
		view->getTextureDesc();
		const auto& io = ImGui::GetIO();

		static CheckerboardPattern pass;

		const glm::mat4 ortho_projection =
		{
			{ 2.0f / io.DisplaySize.x, 0.0f,                   0.0f, 0.0f },
			{ 0.0f,                  2.0f / -io.DisplaySize.y, 0.0f, 0.0f },
			{ 0.0f,                  0.0f,                  -1.0f, 0.0f },
			{ -1.0f,                  1.0f,                   0.0f, 1.0f },
		};

		pass.bind();
		pass.setProjMtx(ortho_projection);
	};
	drawList->AddDrawCmd();
	ImDrawCmd* current_cmd = &drawList->CmdBuffer.back();
	drawList->PrimReserve(6, 4);
	drawList->PrimRectUV(bb.Min, bb.Max, ImVec2(0,0), ImVec2(1, 1), ImGui::GetColorU32(ImVec4(1,1,1,1)));

	current_cmd->UserCallback = test;
	current_cmd->UserCallbackData = this;
}

ImVec2 nex::gui::TextureView::calcTextureSize(const ImGUI_TextureDesc& desc)
{
	if (desc.texture == nullptr) return { 0.0f,0.0f };
	return { (float)desc.texture->getWidth(), (float)desc.texture->getHeight() };
}

void nex::gui::TextureView::drawSelf()
{
	if (mDesc.texture == nullptr) return;

	ImGui::PushID(mId.c_str());

	//ImGui::SetNextWindowContentSize(ImVec2(128, 128));

	const auto target = mDesc.texture->getTarget();

	const auto& data = mDesc.texture->getTextureData();

	std::stringstream ss1;
	const auto mipMapCount = data.lodMaxLevel - data.lodBaseLevel + 1;


	std::vector<const char*> items(mipMapCount);
	std::vector<std::string> content(mipMapCount);
	for (unsigned i = 0; i < mipMapCount; ++i)
	{
		content[i] = std::to_string(i);
		items[i] = content[i].c_str();
	}

	if (mShowMipMapSelection)
		ImGui::Combo("Mipmap level", (int*)&mDesc.lod, (const char**)items.data(), (int)items.size());

	if (target == TextureTarget::CUBE_MAP || target == TextureTarget::CUBE_MAP_ARRAY)
	{
		const char* items[] = { "Right", "Left", "Top", "Bottom", "Front", "Back" };
		ImGui::Combo("Side", (int*)&mDesc.side, items, IM_ARRAYSIZE(items));
	}

	if (target == TextureTarget::CUBE_MAP_ARRAY || target == TextureTarget::TEXTURE2D_ARRAY) {
		std::vector<const char*> items(mDesc.texture->getDepth());
		std::vector<std::string> content(mDesc.texture->getDepth());

		for (unsigned i = 0; i < mDesc.texture->getDepth(); ++i)
		{
			content[i] = std::to_string(i);
			items[i] = content[i].c_str();
			
		}

		ImGui::Combo("Index", (int*)&mDesc.level, (const char**)items.data(), (int)items.size());
	}

	ImGui::BeginChild(mScrollPaneID.c_str(), ImVec2(mViewSize.x +20, mViewSize.y + 20), true, ImGuiWindowFlags_HorizontalScrollbar);

	ImVec2 imageSize(mScale * mTextureSize.x, mScale * mTextureSize.y);

	
	
	if (!mDesc.sampler) mDesc.sampler = &mSampler;
	
	if (mDesc.useTransparency  || !mDesc.texture)
	{
		auto position = ImGui::GetCursorPos();
		addCheckBoardPattern(imageSize);
		ImGui::SetCursorPos(position);
	}

	if (mDesc.texture) {
		ImGui::Image((void*)&mDesc, imageSize, ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, mOpacity), ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
		ImGui::EndChild();
	}
	
	if (mShowScaleConfig) {
		if (ImGui::Button("+"))
		{
			mScale += 0.1f;
		}
		ImGui::SameLine(0, 0);
		if (ImGui::Button("-"))
		{
			mScale -= 0.1f;
		}
		mScale = std::clamp(mScale, 0.0f, 1000.0f);

		ImGui::SameLine();
		std::stringstream ss;
		ss.str("");
		ss << "Scale: " << std::setprecision(2) << mScale;
		ImGui::Text(ss.str().c_str());
	}

	if (mShowOpacityConfig)
		ImGui::SliderFloat("Opacity: ", &mOpacity, 0.0f, 1.0f);
	
	if (mShowShowTransparencyConfig)
		ImGui::Checkbox("show transparency", &mDesc.useTransparency);

	if (mShowToneMappingConfig)
		ImGui::Checkbox("use tone mapping", &mDesc.useToneMapping);

	if (mShowFilteringConfig) {
		const char* filterings[] = { "Nearest", "Linear"};
		if (ImGui::Combo("Filtering", (int*)&mSelectedFiltering, filterings, IM_ARRAYSIZE(filterings)))
		{
			switch (mSelectedFiltering)
			{
			case 0:
				mSampler.setMinFilter(TexFilter::Near_Mipmap_Near);
				break;
			case 1:
				mSampler.setMinFilter(TexFilter::Linear_Mipmap_Linear);
			}
		}
	}

	ImGui::PopID();
}