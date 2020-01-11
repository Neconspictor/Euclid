#include <nex/gui/RectangleShadow.hpp>

nex::gui::RectangleShadow shadowSettings;

void drawShadowTestExampleWindow()
{
    
    static ImColor transparentColor(0, 0, 0, 0);
    static ImColor backgroundColor(255, 255, 255, 255);
    static ImColor rectangleColor(240, 240, 240, 255);
    
   // ImGui::SetNextWindowSize(ImVec2(400,800), ImGuiCond_Once);
    
    ImGui::Begin("Test Shadows");
    static bool showRectangles          = true;
    static bool calculateMinimumPadding = false;
    
    ImGui::Checkbox("Wireframe", &shadowSettings.enableDebugVisualization);
    ImGui::Checkbox("Draw Rectangle", &showRectangles);
    ImGui::Checkbox("Recalculate Padding", &calculateMinimumPadding);
    ImGui::Checkbox("Linear Falloff", &shadowSettings.linear);
    ImGui::SliderFloat2("Rectangle Size", &shadowSettings.rectSize.x, 10, 256);
    ImGui::SliderFloat("Shadow Sigma", &shadowSettings.sigma, 0, 50);
    ImGui::SliderFloat2("Shadow Offset", &shadowSettings.shadowOffset.x, -10, 10);
    ImGui::SliderInt("Rings number", &shadowSettings.rings, 1, 10);
    ImGui::SliderInt("Rings spacing", &shadowSettings.spacingBetweenRings, 1, 20);
    ImGui::SliderInt("Corner samples", &shadowSettings.samplesPerCornerSide, 1, 20);
    ImGui::SliderInt("Corner spacing", &shadowSettings.spacingBetweenSamples, 1, 20);
    ImGui::ColorPicker3("Shadow Color", &shadowSettings.shadowColor.Value.x, ImGuiColorEditFlags_PickerHueWheel);
    ImGui::ColorPicker3("Rectangle Color", &rectangleColor.Value.x, ImGuiColorEditFlags_PickerHueWheel);
    ImGui::ColorPicker3("Background Color", &backgroundColor.Value.x, ImGuiColorEditFlags_PickerHueWheel);
    ImGui::End();
    
    if (calculateMinimumPadding)
    {
        shadowSettings.padding = ImVec2(3 * shadowSettings.sigma, 3 * shadowSettings.sigma);
    }
    shadowSettings.rectPos    = shadowSettings.padding;
    shadowSettings.shadowSize = shadowSettings.rectSize + shadowSettings.padding * 2;
    
    //ImGui::SetNextWindowPos(ImVec2(700, 0), ImGuiCond_Once);
    //ImGui::PushStyleColor(ImGuiCol_WindowBg, (ImU32)transparentColor);
    //ImGuiWindowFlags_NoBackground


    
    //ImGui::SetNextWindowSize(shadowSettings.rectSize);
    //ImGui::SetNextWindowPos(origin + shadowSettings.rectPos);
    //ImGui::SetNextWindowContentSize(shadowSettings.shadowSize);



    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

    
    if (ImGui::Begin("Shadow Outputs", 0, ImGuiWindowFlags_NoScrollbar)) {
        //ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, (ImU32)backgroundColor);
    //ImGui::Text("rings:%d, spacingH:%d, spacingR:%d\ntotalVertices:%d totalIndices:%d", shadowSettings.rings, shadowSettings.spacingBetweenRings, shadowSettings.spacingBetweenSamples, shadowSettings.totalVertices, shadowSettings.totalIndices);

        const ImVec2 origin(ImGui::GetCursorScreenPos());
        

        //window->DC.CursorPos = origin;

        //const ImVec2 origin(ImGui::GetCursorScreenPos());
        
        
        
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        auto* window = ImGui::GetCurrentWindow();
        

        //drawList->AddRectFilled(origin, origin + ImGui::GetContentRegionAvail(), rectangleColor);

        ImRect bb;
        bb.Min = origin;
        bb.Max = origin + ImGui::GetContentRegionAvail();
        ImGui::ItemSize(bb);
        

    }
    //ImGui::PopStyleColor();
    ImGui::End();
    ImGui::PopStyleVar();

    


    auto* windowI = ImGui::FindWindowByName("Shadow Outputs");

    
    auto* window = ImGui::GetCurrentWindow();
    
    
    


    shadowSettings.rectSize = windowI->Size;
    shadowSettings.shadowSize = windowI->Size + shadowSettings.padding * 2;
    
    GImGui->CurrentWindow = windowI;
    const ImVec2 backupPos(ImGui::GetCursorScreenPos());
    windowI->DC.CursorPos = windowI->Pos - shadowSettings.rectPos; //GImGui->Style.WindowPadding

    
    const auto& rect = windowI->ClipRect;
    ImGui::PushClipRect(rect.Min - shadowSettings.rectPos, rect.Max + shadowSettings.rectPos, false);
    shadowSettings.drawRectangleShadowVerticesAdaptive();
    ImGui::PopClipRect();
    
    windowI->DC.CursorPos = backupPos;
    GImGui->CurrentWindow = window;
    

   // ImGui::PopStyleColor();
}
