#pragma once

#include <nex/gui/Drawable.hpp>

#include <nex/gui/ImGUI.hpp>
#include <sstream>
#include <functional>

namespace nex::gui
{
	Drawable::Drawable(bool isVisibleDefaultState): mIsVisible(isVisibleDefaultState), mDrawChildren(isVisibleDefaultState)
	{
		std::stringstream ss;
		ss << std::hex << reinterpret_cast<long long>(this);
		mId = ss.str();
	}

	void Drawable::drawGUI()
	{
		// Do not draw gui if this view is invisible!
		if (!mIsVisible && !mDrawChildren) return;

		// Apply style class changes
		if (mStyle) mStyle->pushStyleChanges();

		drawContent();

		// Revert style class changes
		if (mStyle) mStyle->popStyleChanges();
	}

	void Drawable::addChild(std::unique_ptr<Drawable> child)
	{
		mChilds.emplace_back(std::move(child));
	}

	void Drawable::addChild(Drawable* child)
	{
		mChilds.push_back(nex::flexible_ptr(child, false));
	}

	std::vector<nex::flexible_ptr<Drawable>>& Drawable::getChilds()
	{
		return mChilds;
	}

	void Drawable::useStyleClass(StyleClassPtr styleClass)
	{
		mStyle = std::move(styleClass);
	}

	void Drawable::setVisible(bool visible, bool recursive)
	{
		mIsVisible = visible;
		mDrawChildren = recursive ? mIsVisible : true;

		if (!recursive) return;
		for (auto& child : mChilds) {
			child->setVisible(visible);
		}
	}

	bool Drawable::isVisible() const
	{
		return mIsVisible;
	}

	const char* Drawable::getID() const
	{
		return mId.c_str();
	}


	void Drawable::drawSelf() {}

	void Drawable::drawChilds()
	{
		for (auto& child : mChilds)
		{
			child->drawGUI();
		}
	}

	void Drawable::drawContent()
	{
		ImGui::PushID(mId.c_str());

			if (mIsVisible)
				drawSelf();

			if (mDrawChildren)
				drawChilds();
		ImGui::PopID();
	}

	void Drawable::onCanvasResize(unsigned width, unsigned height)
	{
		onCanvasResizeSelf(width, height);
		for (auto& child : mChilds) {
			child->onCanvasResize(width, height);
		}
	}

	void Drawable::onCanvasResizeSelf(unsigned width, unsigned height)
	{
	}

	Window::Window(std::string name, bool useCloseCross): Drawable(),
	                                                      mImGuiFlags(0), mName(std::move(name)),
	                                                      mUseCloseCross(useCloseCross)
	{
		mName += "###" + mId;
	}

	Window::Window(std::string name, bool useCloseCross, int imGuiFlags):	Drawable(),
																			mImGuiFlags(imGuiFlags),
																			mName(std::move(name)),
																			mUseCloseCross(useCloseCross),
																			mUseDropShadow(false)
	{
		mName += "###" + mId;
	}

	void Window::drawGUI()
	{
		// Do not draw gui if this view is invisible!
		if (mIsVisible && !mDrawChildren) return;

		// Apply style class changes
		if (mStyle) mStyle->pushStyleChanges();


		//ImGui::SetNextWindowContentSize(mExplicitContentSize);

		auto* gui = nex::gui::ImGUI_Impl::get();
		auto* fontHeader = gui->getDefaultFont(gui->getHeadingFontSize());
		auto* fontContent = gui->getDefaultFont(gui->getContentFontSize());

		ImGui::PushFont(fontHeader);

		if (mUseCloseCross) {
			bool visible = isVisible();
			ImGui::Begin(mName.c_str(), &visible, mImGuiFlags);
			
			if (visible != mIsVisible)
			setVisible(visible);
			
			//setVisible(visible);
		} else 
		{
			
			
			ImGui::Begin(mName.c_str(), nullptr, mImGuiFlags);
			
		}

		if (mUseDropShadow) {
			auto size = ImGui::GetWindowSize();
			mShadow.rectSize;
		}
		

		//if (mExplicitContentSize.x != 0 && mExplicitContentSize.y != 0)
		//	ImGui::SetWindowSize(mExplicitContentSize);

		

		ImGui::PushFont(fontContent);
		drawContent();
		ImGui::PopFont();

		ImGui::End();
		ImGui::PopFont();


		// Revert style class changes
		if (mStyle) mStyle->popStyleChanges();
	}

	int Window::getImGuiFlags() const
	{
		return mImGuiFlags;
	}

	void Window::setImGuiFlags(int flags)
	{
		mImGuiFlags = flags;
	}

	void Window::setExplicitContentSize(const ImVec2& size)
	{
		mExplicitContentSize = size;
	}

	void Window::useDropShadow(bool use)
	{
		mUseDropShadow = use;
	}


	Tab::Tab(std::string name): Drawable(), mName(std::move(name))
	{
		mId = mName + "###" + mId;
	}

	void Tab::drawGUI()
	{
		// Do not draw gui if this drawable is invisible!
		if (!mIsVisible && !mDrawChildren) return;

		// Apply style class changes
		if (mStyle) mStyle->pushStyleChanges();

		if (ImGui::BeginTabItem(mId.c_str()))
		{
			drawContent();
			ImGui::EndTabItem();
		}

		// Revert style class changes
		if (mStyle) mStyle->popStyleChanges();
	}

	TabBar::TabBar(std::string name): Drawable(), mName(std::move(name))
	{
		mName += "###" + mId;
	}

	Tab* TabBar::newTab(std::string tabName)
	{
		mChilds.emplace_back(std::make_unique<Tab>(std::move(tabName)));
		return dynamic_cast<Tab*>((mChilds.back()).get());
	}

	Tab* TabBar::getTab(const char* tabName)
	{
		for (auto& child : mChilds)
		{
			Tab* tab = dynamic_cast<Tab*>(child.get());
			if (tab != nullptr && (tab->getName() == tabName))
			{
				return tab;
			}
		}

		// no tab with the specified name was found
		return nullptr;
	}

	void TabBar::drawGUI()
	{
		// Do not draw gui if this drawable is invisible!
		if (!mIsVisible && !mDrawChildren) return;

		// Apply style class changes
		if (mStyle) mStyle->pushStyleChanges();

		if (ImGui::BeginTabBar(mName.c_str())) {
			drawContent();
			ImGui::EndTabBar();
		}

		// Revert style class changes
		if (mStyle) mStyle->popStyleChanges();
	}
	Button::Button(std::string title) : mTitle(std::move(title))
	{
		mStyle = std::make_shared<ButtonStyle>();
	}
	void Button::setAction(std::function<void()> action)
	{
		mAction = std::move(action);
	}
	void Button::enable(bool enabled)
	{
		mEnabled = enabled;

		if (!mEnabled) {
			mStyle = std::make_shared<ButtonStyle>();
		}
		else {
			mStyle = nullptr;
		}
	}
	void Button::drawSelf()
	{
		if (mEnabled && ImGui::Button(mTitle.c_str())) {
			mAction();
		}
	}
	bool Button::drawImmediate()
	{
		// Do not draw gui if this drawable is invisible!
		if (!mIsVisible && !mDrawChildren) return false;

		// Apply style class changes
		if (mStyle) mStyle->pushStyleChanges();


		bool clicked = ImGui::Button(mTitle.c_str());

		// Revert style class changes
		if (mStyle) mStyle->popStyleChanges();

		return clicked && mEnabled;
	}
	void Button::ButtonStyle::pushStyleChangesSelf()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		const auto& buttonColor = style.Colors[ImGuiCol_Button];
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, buttonColor);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, buttonColor);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, style.Alpha * 0.5f);
	}
	void Button::ButtonStyle::popStyleChangesSelf()
	{
		ImGui::PopStyleVar(1);
		ImGui::PopStyleColor(2);
	}

	ApplyButton::ApplyButton(std::function<void()> apply, std::function<void()> revert, std::function<bool()> condition) :
		mApply(std::move(apply)), 
		mRevert(std::move(revert)),
		mCondition(std::move(condition))
	{
		
	}

	void ApplyButton::drawSelf()
	{
		if (!mCondition()) return;

		auto* context = ImGui::GetCurrentContext();
		if (ImGui::ButtonEx("Apply", { 0, 0 }, 0))
		{
			mApply();
		}

		ImGui::SameLine(0, context->Style.ItemInnerSpacing.x);

		if (ImGui::ButtonEx("Revert", { 0, 0 }, 0))
		{
			mRevert();
		}
	}
}