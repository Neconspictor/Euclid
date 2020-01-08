#include <nex/gui/ImGUI_Extension.hpp>
#include <nex/gui/ImGui.hpp>
#include <nex/texture/TextureManager.hpp>
#include <nex/texture/Texture.hpp>


void nex::gui::Separator(float thickness, bool vertical)
{
	ImGui::Separator();
}

bool nex::gui::Vector3D(glm::vec3* vec, const char* label, float speed)
{
	//std::stringstream ss;
	//ss << label << ":";
	//ImGui::TextUnformatted(ss.str().c_str());
	return ImGui::DragFloat3(label, (float*)vec);
}

void nex::gui::EulerRot(glm::vec3* vec, const char* label, float speed)
{
	std::stringstream ss;
	ss << label << ":";
	ImGui::TextUnformatted(ss.str().c_str());

	ImGui::PushID(label);
	ImGui::TextUnformatted("X: ");
	ImGui::SameLine();
	ImGui::DragFloat("###X", &vec->x, speed);

	ImGui::TextUnformatted("Y: ");
	ImGui::SameLine();
	ImGui::DragFloat("###Y", &vec->y, speed);
		
		
	ImGui::TextUnformatted("Z: ");
	ImGui::SameLine();
	ImGui::DragFloat("###Z", &vec->z, speed);
	ImGui::PopID();
}

void nex::gui::Quat(glm::quat* quat, const char* label)
{
	std::stringstream ss;
	ss << label << ":";
	ImGui::TextUnformatted(ss.str().c_str());

	ImGui::PushID(label);
	ImGui::TextUnformatted("X: ");
	ImGui::SameLine();
	ImGui::DragFloat("###X", &quat->x, 0.1f);
	ImGui::TextUnformatted("Y: ");
	ImGui::SameLine();
	ImGui::DragFloat("###Y", &quat->y, 0.1f);
	ImGui::TextUnformatted("Z: ");
	ImGui::SameLine();
	ImGui::DragFloat("###Z", &quat->z, 0.1f);
	ImGui::TextUnformatted("W: ");
	ImGui::SameLine();
	ImGui::DragFloat("###W", &quat->w, 0.1f);
	ImGui::PopID();
}

nex::gui::ID::ID(int id)
{
	ImGui::PushID(id);
}
nex::gui::ID::~ID()
{
	ImGui::PopID();
}

ImVec2 nex::gui::GetWindowContentPadding()
{
	const auto& style = GImGui->Style;
	return style.WindowPadding + style.FramePadding;
}

ImVec2 nex::gui::GetWindowContentEffectiveSize()
{
	const auto& style = GImGui->Style;
	return style.WindowPadding + style.FramePadding;
}

bool nex::gui::TreeNodeExCustomShape(const char* label,
    const CustomShapeRenderFunc& renderFunc, bool clipFrameToContent, ImVec2 offset, ImGuiTreeNodeFlags flags)
{
    using namespace ImGui;
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    return TreeNodeBehaviourCustomShape(window->GetID(label), flags, label, NULL, renderFunc, clipFrameToContent, offset);
}

bool nex::gui::TreeNodeBehaviourCustomShape(ImGuiID id, ImGuiTreeNodeFlags flags, const char* label, const char* label_end,
    const CustomShapeRenderFunc& renderFunc,  bool clipFrameToContent, ImVec2 offset)
{
    using namespace ImGui;

    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const bool display_frame = (flags & ImGuiTreeNodeFlags_Framed) != 0;
    const ImVec2 padding = (display_frame || (flags & ImGuiTreeNodeFlags_FramePadding)) ? style.FramePadding : ImVec2(style.FramePadding.x, ImMin(window->DC.CurrLineTextBaseOffset, style.FramePadding.y));

    if (!label_end)
        label_end = FindRenderedTextEnd(label);
    const ImVec2 label_size = CalcTextSize(label, label_end, false);

    // We vertically grow up to current line height up the typical widget height.
    const float frame_height = ImMax(ImMin(window->DC.CurrLineSize.y, g.FontSize + style.FramePadding.y * 2), label_size.y + padding.y * 2);
    ImRect frame_bb;
    frame_bb.Min.x = (flags & ImGuiTreeNodeFlags_SpanFullWidth) ? window->WorkRect.Min.x : window->DC.CursorPos.x;
    frame_bb.Min.y = window->DC.CursorPos.y;
    frame_bb.Max.x = window->WorkRect.Max.x;
    frame_bb.Max.y = window->DC.CursorPos.y + frame_height;
    if (display_frame)
    {
        // Framed header expand a little outside the default padding, to the edge of InnerClipRect
        // (FIXME: May remove this at some point and make InnerClipRect align with WindowPadding.x instead of WindowPadding.x*0.5f)
        frame_bb.Min.x -= IM_FLOOR(window->WindowPadding.x * 0.5f - 1.0f);
        frame_bb.Max.x += IM_FLOOR(window->WindowPadding.x * 0.5f);
    }

    const float text_offset_x = g.FontSize + (display_frame ? padding.x * 3 : padding.x * 2);               // Collapser arrow width + Spacing
    const float text_offset_y = ImMax(padding.y, window->DC.CurrLineTextBaseOffset);                    // Latch before ItemSize changes it
    const float text_width = g.FontSize + (label_size.x > 0.0f ? label_size.x + padding.x * 2 : 0.0f);    // Include collapser
    ImVec2 text_pos = ImVec2(window->DC.CursorPos.x + text_offset_x, window->DC.CursorPos.y + text_offset_y) + offset;
    ItemSize(ImVec2(text_width, frame_height), padding.y);

    // For regular tree nodes, we arbitrary allow to click past 2 worth of ItemSpacing
    ImRect interact_bb = frame_bb;
    if (!display_frame && (flags & (ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_SpanFullWidth) == 0)
        || clipFrameToContent) {
        interact_bb.Max.x = frame_bb.Min.x + text_width + style.ItemSpacing.x *2.0f;
    }
    if (clipFrameToContent) {
        frame_bb = interact_bb;
    }

    // Store a flag for the current depth to tell if we will allow closing this node when navigating one of its child.
    // For this purpose we essentially compare if g.NavIdIsAlive went from 0 to 1 between TreeNode() and TreePop().
    // This is currently only support 32 level deep and we are fine with (1 << Depth) overflowing into a zero.
    const bool is_leaf = (flags & ImGuiTreeNodeFlags_Leaf) != 0;
    bool is_open = TreeNodeBehaviorIsOpen(id, flags);
    if (is_open && !g.NavIdIsAlive && (flags & ImGuiTreeNodeFlags_NavLeftJumpsBackHere) && !(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen))
        window->DC.TreeMayJumpToParentOnPopMask |= (1 << window->DC.TreeDepth);

    bool item_add = ItemAdd(interact_bb, id);
    window->DC.LastItemStatusFlags |= ImGuiItemStatusFlags_HasDisplayRect;
    window->DC.LastItemDisplayRect = frame_bb;

    if (!item_add)
    {
        if (is_open && !(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen))
            TreePushOverrideID(id);
        IMGUI_TEST_ENGINE_ITEM_INFO(window->DC.LastItemId, label, window->DC.ItemFlags | (is_leaf ? 0 : ImGuiItemStatusFlags_Openable) | (is_open ? ImGuiItemStatusFlags_Opened : 0));
        return is_open;
    }

    // Flags that affects opening behavior:
    // - 0 (default) .................... single-click anywhere to open
    // - OpenOnDoubleClick .............. double-click anywhere to open
    // - OpenOnArrow .................... single-click on arrow to open
    // - OpenOnDoubleClick|OpenOnArrow .. single-click on arrow or double-click anywhere to open
    ImGuiButtonFlags button_flags = 0;
    if (flags & ImGuiTreeNodeFlags_AllowItemOverlap)
        button_flags |= ImGuiButtonFlags_AllowItemOverlap;
    if (flags & ImGuiTreeNodeFlags_OpenOnDoubleClick)
        button_flags |= ImGuiButtonFlags_PressedOnDoubleClick | ((flags & ImGuiTreeNodeFlags_OpenOnArrow) ? ImGuiButtonFlags_PressedOnClickRelease : 0);
    if (!is_leaf)
        button_flags |= ImGuiButtonFlags_PressedOnDragDropHold;

    // We allow clicking on the arrow section with keyboard modifiers held, in order to easily 
    // allow browsing a tree while preserving selection with code implementing multi-selection patterns.
    // When clicking on the rest of the tree node we always disallow keyboard modifiers.
    const float hit_padding_x = style.TouchExtraPadding.x;
    const float arrow_hit_x1 = (text_pos.x - text_offset_x) - hit_padding_x;
    const float arrow_hit_x2 = (text_pos.x - text_offset_x) + (g.FontSize + padding.x * 2.0f) + hit_padding_x;
    if (window != g.HoveredWindow || !(g.IO.MousePos.x >= arrow_hit_x1 && g.IO.MousePos.x < arrow_hit_x2))
        button_flags |= ImGuiButtonFlags_NoKeyModifiers;

    bool selected = (flags & ImGuiTreeNodeFlags_Selected) != 0;
    const bool was_selected = selected;

    bool hovered, held;
    bool pressed = ButtonBehavior(interact_bb, id, &hovered, &held, button_flags);
    if (!is_leaf)
    {
        bool toggled = false;
        if (pressed)
        {
            if ((flags & (ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick)) == 0 || (g.NavActivateId == id))
                toggled = true;
            if (flags & ImGuiTreeNodeFlags_OpenOnArrow)
                toggled |= (g.IO.MousePos.x >= arrow_hit_x1 && g.IO.MousePos.x < arrow_hit_x2) && (!g.NavDisableMouseHover); // Lightweight equivalent of IsMouseHoveringRect() since ButtonBehavior() already did the job
            if ((flags & ImGuiTreeNodeFlags_OpenOnDoubleClick) && g.IO.MouseDoubleClicked[0])
                toggled = true;
            if (g.DragDropActive && is_open) // When using Drag and Drop "hold to open" we keep the node highlighted after opening, but never close it again.
                toggled = false;
        }

        if (g.NavId == id && g.NavMoveRequest && g.NavMoveDir == ImGuiDir_Left && is_open)
        {
            toggled = true;
            NavMoveRequestCancel();
        }
        if (g.NavId == id && g.NavMoveRequest && g.NavMoveDir == ImGuiDir_Right && !is_open) // If there's something upcoming on the line we may want to give it the priority?
        {
            toggled = true;
            NavMoveRequestCancel();
        }

        if (toggled)
        {
            is_open = !is_open;
            window->DC.StateStorage->SetInt(id, is_open);
            window->DC.LastItemStatusFlags |= ImGuiItemStatusFlags_ToggledOpen;
        }
    }
    if (flags & ImGuiTreeNodeFlags_AllowItemOverlap)
        SetItemAllowOverlap();

    // In this branch, TreeNodeBehavior() cannot toggle the selection so this will never trigger.
    if (selected != was_selected) //-V547
        window->DC.LastItemStatusFlags |= ImGuiItemStatusFlags_ToggledSelection;

    // Render
    const ImU32 text_col = GetColorU32(ImGuiCol_Text);
    ImGuiNavHighlightFlags nav_highlight_flags = ImGuiNavHighlightFlags_TypeThin;
    if (display_frame)
    {
        // Framed type
        const ImU32 bg_col = GetColorU32((held && hovered) ? ImGuiCol_HeaderActive : hovered ? ImGuiCol_HeaderHovered : ImGuiCol_Header);
        //RenderFrame(frame_bb.Min, frame_bb.Max, bg_col, true, style.FrameRounding);
        renderFunc(id, frame_bb.Min, frame_bb.Max, bg_col, true, style.FrameRounding);
        RenderNavHighlight(frame_bb, id, nav_highlight_flags);
        if (flags & ImGuiTreeNodeFlags_Bullet)
            RenderBullet(window->DrawList, ImVec2(text_pos.x - text_offset_x * 0.60f, text_pos.y + g.FontSize * 0.5f), text_col);
        else if (!is_leaf)
            RenderArrow(window->DrawList, ImVec2(text_pos.x - text_offset_x + padding.x, text_pos.y), text_col, is_open ? ImGuiDir_Down : ImGuiDir_Right, 1.0f);
        else // Leaf without bullet, left-adjusted text
            text_pos.x -= text_offset_x;
        if (flags & ImGuiTreeNodeFlags_ClipLabelForTrailingButton)
            frame_bb.Max.x -= g.FontSize + style.FramePadding.x;
        if (g.LogEnabled)
        {
            // NB: '##' is normally used to hide text (as a library-wide feature), so we need to specify the text range to make sure the ## aren't stripped out here.
            const char log_prefix[] = "\n##";
            const char log_suffix[] = "##";
            LogRenderedText(&text_pos, log_prefix, log_prefix + 3);
            RenderTextClipped(text_pos, frame_bb.Max, label, label_end, &label_size);
            LogRenderedText(&text_pos, log_suffix, log_suffix + 2);
        }
        else
        {
            RenderTextClipped(text_pos, frame_bb.Max, label, label_end, &label_size);
        }
    }
    else
    {
        // Unframed typed for tree nodes
        if (hovered || selected)
        {
            const ImU32 bg_col = GetColorU32((held && hovered) ? ImGuiCol_HeaderActive : hovered ? ImGuiCol_HeaderHovered : ImGuiCol_Header);
            //RenderFrame(frame_bb.Min, frame_bb.Max, bg_col, false);
            renderFunc(id, frame_bb.Min, frame_bb.Max, bg_col, true, style.FrameRounding);
            RenderNavHighlight(frame_bb, id, nav_highlight_flags);
        }
        if (flags & ImGuiTreeNodeFlags_Bullet)
            RenderBullet(window->DrawList, ImVec2(text_pos.x - text_offset_x * 0.5f, text_pos.y + g.FontSize * 0.5f), text_col);
        else if (!is_leaf)
            RenderArrow(window->DrawList, ImVec2(text_pos.x - text_offset_x + padding.x, text_pos.y + g.FontSize * 0.15f), text_col, is_open ? ImGuiDir_Down : ImGuiDir_Right, 0.70f);
        if (g.LogEnabled)
            LogRenderedText(&text_pos, ">");
        RenderText(text_pos, label, label_end, false);
    }

    if (is_open && !(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen))
        TreePushOverrideID(id);
    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.ItemFlags | (is_leaf ? 0 : ImGuiItemStatusFlags_Openable) | (is_open ? ImGuiItemStatusFlags_Opened : 0));
    return is_open;
}

bool nex::gui::BeginImageMenuCustom(const char* labelID, const ImGUI_TextureDesc& textureDesc, ImVec2 size, bool tightSpanning, bool enabled)
{
    using namespace ImGui;

    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    ImGuiStyle& style = g.Style;

    std::string selectableID = "##" + std::string(labelID);
    const ImGuiID id = window->GetID(labelID);

    const auto labelSize = ImGui::CalcTextSize(labelID, nullptr, true);

    bool pressed;
    bool menu_is_open = IsPopupOpen(id);
    bool menuset_is_open = !(window->Flags & ImGuiWindowFlags_Popup) && (g.OpenPopupStack.Size > g.BeginPopupStack.Size&& g.OpenPopupStack[g.BeginPopupStack.Size].OpenParentId == window->IDStack.back());
    ImGuiWindow* backed_nav_window = g.NavWindow;
    if (menuset_is_open)
        g.NavWindow = window;  // Odd hack to allow hovering across menus of a same menu-set (otherwise we wouldn't be able to hover parent)

    // The reference position stored in popup_pos will be used by Begin() to find a suitable position for the child menu,
    // However the final position is going to be different! It is choosen by FindBestWindowPosForPopup().
    // e.g. Menus tend to overlap each other horizontally to amplify relative Z-ordering.
    ImVec2 popup_pos, pos = window->DC.CursorPos;
    if (window->DC.LayoutType == ImGuiLayoutType_Horizontal)
    {
        //auto backupItemSpacing = style.ItemSpacing.x;
        //style.ItemSpacing.x = 0;

        // Menu inside an horizontal menu bar
        // Selectable extend their highlight by half ItemSpacing in each direction.
        // For ChildMenu, the popup position will be overwritten by the call to FindBestWindowPosForPopup() in Begin()
        popup_pos = ImVec2(pos.x - 1.0f - IM_FLOOR(style.ItemSpacing.x * 0.5f), pos.y - style.FramePadding.y + window->MenuBarHeight());
        
        if (!tightSpanning) {
            window->DC.CursorPos.x += IM_FLOOR(style.ItemSpacing.x * 0.5f);
            PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x * 2.0f, style.ItemSpacing.y));
        }
        
        //float w = label_size.x;
        float width = size.x;

        if (width == 0) {
            width = window->DC.CurrLineSize.y - window->DC.CurrLineTextBaseOffset;
        }
        
        if (labelSize.x > 0.0f) {
            width += labelSize.x + style.ItemSpacing.x;
        }
       
        pressed = Selectable(selectableID.c_str(), menu_is_open, ImGuiSelectableFlags_NoHoldingActiveID | ImGuiSelectableFlags_PressedOnClick | ImGuiSelectableFlags_DontClosePopups | (!enabled ? ImGuiSelectableFlags_Disabled : 0), ImVec2(width + style.ItemSpacing.y, 0));
        const auto& bbLast = window->DC.LastItemRect;
        auto rectSize = bbLast.Max - bbLast.Min;

        auto maxPaddingX = std::max<float>(style.WindowPadding.x, style.ItemSpacing.x);

        auto minSize = std::min<float>(rectSize.x - maxPaddingX, rectSize.y - style.FramePadding.y * 2);

        if (size.x == 0) size.x = minSize;
        if (size.y == 0) size.y = minSize;



        if (!tightSpanning) {
            PopStyleVar();
            window->DC.CursorPos.x += IM_FLOOR(style.ItemSpacing.x * (-1.0f + 0.5f)); // -1 spacing to compensate the spacing added when Selectable() did a SameLine(). It would also work to call SameLine() ourselves after the PopStyleVar().
        }
        
        ImRect bb;

        if (tightSpanning) {
            auto expandedSize = ImVec2(size.x + style.ItemSpacing.x + labelSize.x, size.y);
            auto middlePoint = bbLast.Min + rectSize / 2;
            middlePoint.x -= (style.ItemSpacing.x + labelSize.x) / 2;
            bb.Min = middlePoint - size / 2;
            bb.Max = bb.Min + size;

        }
        else {
            bb.Min = bbLast.Min + ImVec2(maxPaddingX, style.FramePadding.y);
            bb.Max = bbLast.Min + ImVec2(maxPaddingX, style.FramePadding.y) + size;
        }

        window->DrawList->AddImage((void*)&textureDesc, bb.Min, bb.Max);


        ImVec2 pos = bbLast.Min + ImVec2(maxPaddingX + bb.GetSize().x + style.ItemSpacing.x, 4); //style.FramePadding.y
        pos.x = bb.Min.x + bb.GetSize().x + style.ItemSpacing.x;
        pos.y = window->DC.CursorPos.y + window->DC.CurrLineTextBaseOffset;
        RenderText(pos, labelID);
  
    }
    else
    {
        // Menu inside a menu
        popup_pos = ImVec2(pos.x, pos.y - style.WindowPadding.y);
        
        //auto sizeX = 16;// window->DC.CurrLineSize.y - window->DC.CurrLineTextBaseOffset;
        //auto width = window->DC.CurrLineSize.y - window->DC.CurrLineTextBaseOffset;
        auto contentWidth = size.x + style.ItemSpacing.x + labelSize.x + style.ItemSpacing.x;
        float w = window->MenuColumns.DeclColumns(contentWidth, 0.0f, IM_FLOOR(g.FontSize * 1.20f)); // Feedback to next frame
        
        float extra_w = ImMax(0.0f, GetContentRegionAvail().x - w);
        
        pressed = Selectable(selectableID.c_str(), menu_is_open, ImGuiSelectableFlags_NoHoldingActiveID | ImGuiSelectableFlags_PressedOnClick | ImGuiSelectableFlags_DontClosePopups | ImGuiSelectableFlags_DrawFillAvailWidth | (!enabled ? ImGuiSelectableFlags_Disabled : 0), ImVec2(w, 0.0f));
        const auto& bbLast = window->DC.LastItemRect;
        auto rectSize = bbLast.Max - bbLast.Min;

        auto minSize = std::min<float>(rectSize.x - style.FramePadding.x * 2, rectSize.y - style.FramePadding.y * 2);

        if (size.x == 0) size.x = minSize;
        if (size.y == 0) size.y = minSize;
        
        auto offset = (ImVec2(style.ItemSpacing.x, rectSize.y - size.y)) * 0.5f;

        ImRect bb(bbLast.Min + offset,
            bbLast.Min + offset + ImVec2(size.x, size.y));
        window->DrawList->AddImage((void*)&textureDesc, bb.Min, bb.Max);

        //ImVec2 pos = bbLast.Min + ImVec2(style.FramePadding.x + bb.GetSize().x + style.ItemSpacing.x, 0); //style.FramePadding.y
        //pos.x = bb.Min.x + bb.GetSize().x + style.ItemSpacing.x;
        //pos.y = window->DC.CursorPos.y + window->DC.CurrLineTextBaseOffset - 10;
        RenderText(pos + ImVec2(window->MenuColumns.Pos[0] + style.ItemSpacing.x + size.x, 0.0f), labelID);

        ImU32 text_col = GetColorU32(enabled ? ImGuiCol_Text : ImGuiCol_TextDisabled);
        RenderArrow(window->DrawList, pos + ImVec2(window->MenuColumns.Pos[2] + extra_w + g.FontSize * 0.30f, 0.0f), text_col, ImGuiDir_Right);
    }

    const bool hovered = enabled && ItemHoverable(window->DC.LastItemRect, id);
    if (menuset_is_open)
        g.NavWindow = backed_nav_window;

    bool want_open = false;
    bool want_close = false;
    if (window->DC.LayoutType == ImGuiLayoutType_Vertical) // (window->Flags & (ImGuiWindowFlags_Popup|ImGuiWindowFlags_ChildMenu))
    {
        // Close menu when not hovering it anymore unless we are moving roughly in the direction of the menu
        // Implement http://bjk5.com/post/44698559168/breaking-down-amazons-mega-dropdown to avoid using timers, so menus feels more reactive.
        bool moving_toward_other_child_menu = false;

        ImGuiWindow* child_menu_window = (g.BeginPopupStack.Size < g.OpenPopupStack.Size && g.OpenPopupStack[g.BeginPopupStack.Size].SourceWindow == window) ? g.OpenPopupStack[g.BeginPopupStack.Size].Window : NULL;
        if (g.HoveredWindow == window && child_menu_window != NULL && !(window->Flags & ImGuiWindowFlags_MenuBar))
        {
            // FIXME-DPI: Values should be derived from a master "scale" factor.
            ImRect next_window_rect = child_menu_window->Rect();
            ImVec2 ta = g.IO.MousePos - g.IO.MouseDelta;
            ImVec2 tb = (window->Pos.x < child_menu_window->Pos.x) ? next_window_rect.GetTL() : next_window_rect.GetTR();
            ImVec2 tc = (window->Pos.x < child_menu_window->Pos.x) ? next_window_rect.GetBL() : next_window_rect.GetBR();
            float extra = ImClamp(ImFabs(ta.x - tb.x) * 0.30f, 5.0f, 30.0f);    // add a bit of extra slack.
            ta.x += (window->Pos.x < child_menu_window->Pos.x) ? -0.5f : +0.5f; // to avoid numerical issues
            tb.y = ta.y + ImMax((tb.y - extra) - ta.y, -100.0f);                // triangle is maximum 200 high to limit the slope and the bias toward large sub-menus // FIXME: Multiply by fb_scale?
            tc.y = ta.y + ImMin((tc.y + extra) - ta.y, +100.0f);
            moving_toward_other_child_menu = ImTriangleContainsPoint(ta, tb, tc, g.IO.MousePos);
            //GetForegroundDrawList()->AddTriangleFilled(ta, tb, tc, moving_within_opened_triangle ? IM_COL32(0,128,0,128) : IM_COL32(128,0,0,128)); // [DEBUG]
        }
        if (menu_is_open && !hovered && g.HoveredWindow == window && g.HoveredIdPreviousFrame != 0 && g.HoveredIdPreviousFrame != id && !moving_toward_other_child_menu)
            want_close = true;

        if (!menu_is_open && hovered && pressed) // Click to open
            want_open = true;
        else if (!menu_is_open && hovered && !moving_toward_other_child_menu) // Hover to open
            want_open = true;

        if (g.NavActivateId == id)
        {
            want_close = menu_is_open;
            want_open = !menu_is_open;
        }
        if (g.NavId == id && g.NavMoveRequest && g.NavMoveDir == ImGuiDir_Right) // Nav-Right to open
        {
            want_open = true;
            NavMoveRequestCancel();
        }
    }
    else
    {
        // Menu bar
        if (menu_is_open && pressed && menuset_is_open) // Click an open menu again to close it
        {
            want_close = true;
            want_open = menu_is_open = false;
        }
        else if (pressed || (hovered && menuset_is_open && !menu_is_open)) // First click to open, then hover to open others
        {
            want_open = true;
        }
        else if (g.NavId == id && g.NavMoveRequest && g.NavMoveDir == ImGuiDir_Down) // Nav-Down to open
        {
            want_open = true;
            NavMoveRequestCancel();
        }
    }

    if (!enabled) // explicitly close if an open menu becomes disabled, facilitate users code a lot in pattern such as 'if (BeginMenu("options", has_object)) { ..use object.. }'
        want_close = true;
    if (want_close && IsPopupOpen(id))
        ClosePopupToLevel(g.BeginPopupStack.Size, true);

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.ItemFlags | ImGuiItemStatusFlags_Openable | (menu_is_open ? ImGuiItemStatusFlags_Opened : 0));

    if (!menu_is_open && want_open && g.OpenPopupStack.Size > g.BeginPopupStack.Size)
    {
        // Don't recycle same menu level in the same frame, first close the other menu and yield for a frame.
        OpenPopup(labelID);
        return false;
    }

    menu_is_open |= want_open;
    if (want_open)
        OpenPopup(labelID);

    if (menu_is_open)
    {
        // Sub-menus are ChildWindow so that mouse can be hovering across them (otherwise top-most popup menu would steal focus and not allow hovering on parent menu)
        SetNextWindowPos(popup_pos, ImGuiCond_Always);
        ImGuiWindowFlags flags = ImGuiWindowFlags_ChildMenu | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNavFocus;
        if (window->Flags & (ImGuiWindowFlags_Popup | ImGuiWindowFlags_ChildMenu))
            flags |= ImGuiWindowFlags_ChildWindow;
        menu_is_open = BeginPopupEx(id, flags); // menu_is_open can be 'false' when the popup is completely clipped (e.g. zero size display)
    }

    return menu_is_open;
}

bool nex::gui::Splitter(const char* label, bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size)
{
    using namespace ImGui;
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImGuiID id = window->GetID(label);
    ImRect bb;
    bb.Min = window->DC.CursorPos + (split_vertically ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size1));
    bb.Max = bb.Min + CalcItemSize(split_vertically ? ImVec2(thickness, splitter_long_axis_size) : ImVec2(splitter_long_axis_size, thickness), 0.0f, 0.0f);
    return SplitterBehavior(bb, id, split_vertically ? ImGuiAxis_X : ImGuiAxis_Y, size1, size2, min_size1, min_size2, 0.0f);
}
nex::gui::StyleColorPush::StyleColorPush(ImGuiCol idx, const ImVec4& col)
{
	ImGui::PushStyleColor(idx, col);
}

nex::gui::StyleColorPush::~StyleColorPush()
{
	ImGui::PopStyleColor();
}

nex::gui::DragDropTarget::DragDropTarget()
{
    mIsActive = ImGui::BeginDragDropTarget();
}

nex::gui::DragDropTarget::~DragDropTarget()
{
    if (mIsActive) ImGui::EndDragDropTarget();
}

bool nex::gui::DragDropTarget::isActive() const
{
    return mIsActive;
}