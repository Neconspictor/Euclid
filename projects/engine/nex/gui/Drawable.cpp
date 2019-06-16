#pragma once

#include <nex/gui/Drawable.hpp>

#include <imgui/imgui.h>
#include <nex/gui/imgui_tabs.h>
#include <sstream>

namespace nex::gui
{
	Drawable::Drawable(): m_isVisible(true)
	{
		std::stringstream ss;
		ss << std::hex << reinterpret_cast<long long>(this);
		m_id = ss.str();
	}

	std::vector<Drawable*>& Drawable::getReferencedChilds()
	{
		return mReferencedChilds;
	}

	void Drawable::drawGUI()
	{
		// Do not draw gui if this view is invisible!
		if (!m_isVisible) return;

		// Apply style class changes
		if (m_style) m_style->pushStyleChanges();

		drawSelf();
		drawChilds();

		// Revert style class changes
		if (m_style) m_style->popStyleChanges();
	}

	void Drawable::addChild(std::unique_ptr<Drawable> child)
	{
		m_childs.emplace_back(std::move(child));
	}

	void Drawable::addChild(Drawable* child)
	{
		mReferencedChilds.push_back(child);
	}

	void Drawable::useStyleClass(StyleClassPtr styleClass)
	{
		m_style = std::move(styleClass);
	}

	void Drawable::setVisible(bool visible)
	{
		m_isVisible = visible;
	}

	bool Drawable::isVisible() const
	{
		return m_isVisible;
	}

	const char* Drawable::getID() const
	{
		return m_id.c_str();
	}

	void Drawable::drawChilds()
	{
		for (auto& child : m_childs)
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

	Window::Window(std::string name, bool useCloseCross): Drawable(),
	                                                      m_imGuiFlags(0), m_name(std::move(name)),
	                                                      m_useCloseCross(useCloseCross)
	{
		m_name += "###" + m_id;
	}

	Window::Window(std::string name, bool useCloseCross, int imGuiFlags): Drawable(),
	                                                                      m_imGuiFlags(imGuiFlags),
	                                                                      m_name(std::move(name)),
	                                                                      m_useCloseCross(useCloseCross)
	{
		m_name += "###" + m_id;
	}

	void Window::drawGUI()
	{
		// Do not draw gui if this view is invisible!
		if (!m_isVisible) return;

		// Apply style class changes
		if (m_style) m_style->pushStyleChanges();

		drawSelf();
		drawChilds();

		ImGui::End();

		// Revert style class changes
		if (m_style) m_style->popStyleChanges();
	}

	void Window::drawSelf()
	{
		if (m_useCloseCross)
			ImGui::Begin(m_name.c_str(), &m_isVisible, m_imGuiFlags);
		else
			ImGui::Begin(m_name.c_str(), nullptr, m_imGuiFlags);
	}

	Tab::Tab(std::string name): Drawable(), m_name(std::move(name))
	{
		m_id = m_name + "###" + m_id;
	}

	void Tab::drawGUI()
	{
		// Do not draw gui if this drawable is invisible!
		if (!m_isVisible) return;

		// Apply style class changes
		if (m_style) m_style->pushStyleChanges();

		if (ImGui::TabItem(m_id.c_str()))
		{
			drawChilds();
		}

		// Revert style class changes
		if (m_style) m_style->popStyleChanges();
	}

	void Tab::drawSelf()
	{
	}

	TabBar::TabBar(std::string name): Drawable(), m_name(std::move(name))
	{
		m_name += "###" + m_id;
	}

	Tab* TabBar::newTab(std::string tabName)
	{
		m_childs.emplace_back(std::make_unique<Tab>(std::move(tabName)));
		return dynamic_cast<Tab*>((m_childs.back()).get());
	}

	Tab* TabBar::getTab(const char* tabName)
	{
		for (auto& child : m_childs)
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
		if (!m_isVisible) return;

		// Apply style class changes
		if (m_style) m_style->pushStyleChanges();

		drawSelf();

		// Revert style class changes
		if (m_style) m_style->popStyleChanges();
	}

	void TabBar::drawSelf()
	{
		ImGui::BeginTabBar(m_name.c_str());
		drawChilds();
		ImGui::EndTabBar();
	}

	void Container::drawSelf()
	{
	}
}