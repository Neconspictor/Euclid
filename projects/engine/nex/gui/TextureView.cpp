#include <nex/gui/TextureView.hpp>
#include <nex/texture/Texture.hpp>
#include <nex/math/Math.hpp>

nex::gui::TextureView::TextureView(const ImGUI_ImageDesc& textureDesc, const ImVec2& viewSize) : mDesc(textureDesc), mViewSize(viewSize),
mScale(1.0f)
{
	updateScale();
}

nex::gui::ImGUI_ImageDesc& nex::gui::TextureView::getTexture()
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
	auto minTex = std::min(mTextureSize.x, mTextureSize.y);
	auto minView = std::min(mViewSize.x, mViewSize.y);
	mScale = minView / minTex;

	if (!isValid(mScale)) mScale = 1.0f;
}

void nex::gui::TextureView::setViewSize(const ImVec2& size)
{
	mViewSize = size;
}

ImVec2 nex::gui::TextureView::calcTextureSize(const ImGUI_ImageDesc& desc)
{
	if (desc.texture == nullptr) return { 0.0f,0.0f };
	return { (float)desc.texture->getWidth(), (float)desc.texture->getHeight() };
}

void nex::gui::TextureView::drawSelf()
{
	if (mDesc.texture == nullptr) return;

	//ImGui::SetNextWindowContentSize(ImVec2(128, 128));

	const auto target = mDesc.texture->getTarget();

	if (target == TextureTarget::CUBE_MAP)
	{
		const char* items[] = { "Right", "Left", "Top", "Bottom", "Front", "Back" };
		ImGui::Combo("Side", (int*)&mDesc.side, items, IM_ARRAYSIZE(items));
	}

	ImGui::BeginChild("scrolling", ImVec2(mViewSize.x +20, mViewSize.y + 20), true, ImGuiWindowFlags_HorizontalScrollbar);
	ImGui::Image((void*)&mDesc, { mScale * mTextureSize.x, mScale * mTextureSize.y }, ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(0.2, 0.2, 0.2, 1.0));
	ImGui::EndChild();
	if (ImGui::Button("+"))
	{
		mScale += 0.1f;
	}
	ImGui::SameLine(0,0);
	if (ImGui::Button("-"))
	{
		mScale -= 0.1f;
	}
	mScale = std::clamp(mScale, 0.0f, 1000.0f);

	ImGui::SameLine();
	std::stringstream ss;
	ss << "Scale: " << std::setprecision(2) << mScale;
	ImGui::Text(ss.str().c_str());
}
