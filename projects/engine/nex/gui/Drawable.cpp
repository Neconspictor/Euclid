#pragma once

#include <nex/gui/Drawable.hpp>

#include <imgui/imgui.h>
#include <nex/gui/imgui_tabs.h>
#include <sstream>

namespace nex::gui
{
	Drawable::Drawable(): mIsVisible(true)
	{
		std::stringstream ss;
		ss << std::hex << reinterpret_cast<long long>(this);
		mId = ss.str();
	}

	std::vector<Drawable*>& Drawable::getReferencedChilds()
	{
		return mReferencedChilds;
	}

	void Drawable::drawGUI()
	{
		// Do not draw gui if this view is invisible!
		if (!isVisible()) return;

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
		mReferencedChilds.push_back(child);
	}

	void Drawable::useStyleClass(StyleClassPtr styleClass)
	{
		mStyle = std::move(styleClass);
	}

	void Drawable::setVisible(bool visible)
	{
		mIsVisible = visible;
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
			if (child->isVisible())
				child->drawGUI();
		}

		for (auto& child : mReferencedChilds)
		{
			if (child->isVisible())
				child->drawGUI();
		}
	}

	void Drawable::drawContent()
	{
		ImGui::PushID(mId.c_str());
			drawSelf();
			drawChilds();
		ImGui::PopID();
	}

	Window::Window(std::string name, bool useCloseCross): Drawable(),
	                                                      mImGuiFlags(0), mName(std::move(name)),
	                                                      mUseCloseCross(useCloseCross)
	{
		mName += "###" + mId;
	}

	Window::Window(std::string name, bool useCloseCross, int imGuiFlags): Drawable(),
	                                                                      mImGuiFlags(imGuiFlags),
	                                                                      mName(std::move(name)),
	                                                                      mUseCloseCross(useCloseCross)
	{
		mName += "###" + mId;
	}

	void Window::drawGUI()
	{
		// Do not draw gui if this view is invisible!
		if (!isVisible()) return;

		// Apply style class changes
		if (mStyle) mStyle->pushStyleChanges();

		if (mUseCloseCross) {
			bool visible = isVisible();
			ImGui::Begin(mName.c_str(), &visible, mImGuiFlags);
			setVisible(visible);
		} else 
		{
			ImGui::Begin(mName.c_str(), nullptr, mImGuiFlags);
		}

		drawContent();

		ImGui::End();

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


	Tab::Tab(std::string name): Drawable(), mName(std::move(name))
	{
		mId = mName + "###" + mId;
	}

	void Tab::drawGUI()
	{
		// Do not draw gui if this drawable is invisible!
		if (!isVisible()) return;

		// Apply style class changes
		if (mStyle) mStyle->pushStyleChanges();

		if (ImGui::TabItem(mId.c_str()))
		{
			drawContent();
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
		if (!isVisible()) return;

		// Apply style class changes
		if (mStyle) mStyle->pushStyleChanges();

		ImGui::BeginTabBar(mName.c_str());
			drawContent();
		ImGui::EndTabBar();

		// Revert style class changes
		if (mStyle) mStyle->popStyleChanges();
	}
}