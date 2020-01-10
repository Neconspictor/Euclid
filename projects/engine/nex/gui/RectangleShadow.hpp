//NOTE: Modified original file from Stefano Cristiano for integration and refactoring purposes
//
//The MIT License (MIT)
//
//Copyright (c) 2017 Stefano Cristiano
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

// ---------------------------------------------------------------------------
// Box Shadow for imgui using vertex colors
// Current Limitations:
// - Only works for axis aligned non rounded rectangle
// - Not optimized at all
// - User needs to figure out how many slices / rings are needed and what 
//   spacing would look good for a box of given size and shadow sigma.
//	 Ideally the parameters should be automatically calculated based on some
//   high, medium, low quality settings
// ---------------------------------------------------------------------------

#pragma once

#include <nex/gui/ImGUI.hpp>

namespace nex::gui
{
	struct RectangleShadow {

        //NOTE: public access since it is a POD

        // Inputs
        bool    linear = false;
        float   sigma = 3;
        ImVec2  padding = ImVec2(50, 50);
        ImVec2  rectPos = ImVec2(50, 50);
        ImVec2  rectSize = ImVec2(120, 120);
        ImVec2  shadowOffset = ImVec2(0, 0);
        ImVec2  shadowSize = ImVec2(120, 50);
        ImColor shadowColor = ImColor(0.6f, 0.6f, 0.6f, 1.0f);

        int  rings = 3;
        int  spacingBetweenRings = 6;
        int  samplesPerCornerSide = 1;
        int  spacingBetweenSamples = 15;

        // Outputs
        mutable int totalVertices = 0;
        mutable int totalIndices = 0;

        // Visualization
        bool enableDebugVisualization = false;


        void drawRectangleShadowVerticesAdaptive() const;
        
    private:
        static float boxShadow(const ImVec2& lower, const ImVec2& upper, const ImVec2& point, float sigma, bool linearInterpolation);
        static ImVec4 boxLinearInterpolation(const ImVec4& x);
        static ImVec4 boxGaussianIntegral(const ImVec4& x);
	};
}