#pragma once

#include <nex/gui/Dockspace.hpp>

namespace nex::gui
{

    class DockSpaceStyle : public StyleClass {
    public:
    protected:
        void pushStyleChangesSelf() override {

            ImGuiStyle& style = ImGui::GetStyle();
            ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4(0,0,0,0));
        }
        void popStyleChangesSelf() override {
            ImGui::PopStyleColor(1);
        }
    };

	Dockspace::Dockspace() : Drawable(true)
	{
        mParentWindowID = "##ParentWindow" + mId;
        useStyleClass(std::make_shared<DockSpaceStyle>());
	}
	void Dockspace::drawGUI()
	{
        ImGuiIO& io = ImGui::GetIO();
        if (!(io.ConfigFlags & ImGuiConfigFlags_DockingEnable)) return;

        // Apply style class changes
        if (mStyle) mStyle->pushStyleChanges();

        static bool opt_fullscreen_persistant = true;
        bool opt_fullscreen = opt_fullscreen_persistant;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None | ImGuiDockNodeFlags_PassthruCentralNode;
        //ImGuiDockNodeFlags_NoDockingInCentralNode
        //ImGuiDockNodeFlags_AutoHideTabBar
        //ImGuiDockNodeFlags_NoSplit
        //ImGuiDockNodeFlags_NoResize

        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking; //ImGuiWindowFlags_MenuBar |
        if (opt_fullscreen)
        {
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->Pos);
            ImGui::SetNextWindowSize(viewport->Size);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        }

        // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background 
        // and handle the pass-thru hole, so we ask Begin() to not render a background.
        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
        // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
        // all active windows docked into it will lose their parent and become undocked.
        // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
        // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        auto visible = isVisible();

        ImGui::Begin(mParentWindowID.c_str(), &visible, window_flags);
        ImGui::PopStyleVar();

        if (opt_fullscreen)
            ImGui::PopStyleVar(2);

        // DockSpace        
        ImGuiID dockspace_id = ImGui::GetID(mId.c_str());
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

        ImGui::End();

        // Revert style class changes
        if (mStyle) mStyle->popStyleChanges();

        if (mIsVisible != visible) setVisible(visible);
	}
	void Dockspace::drawSelf()
	{
	}
}