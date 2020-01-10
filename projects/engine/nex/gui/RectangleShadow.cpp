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


#include <nex/gui/RectangleShadow.hpp>


ImVec4 operator*(float val, ImVec4 p2) { return ImVec4(val * p2.x, val * p2.y, val * p2.z, val * p2.w); }
ImVec4 operator*(ImVec4 p2, float val) { return ImVec4(val * p2.x, val * p2.y, val * p2.z, val * p2.w); }
ImVec4 operator/(ImVec4 p1, ImVec4 p2) { return ImVec4(p1.x / p2.x, p1.y / p2.y, p1.z / p2.z, p1.w / p2.w); }
ImVec4 operator+(float val, ImVec4 p2) { return ImVec4(val + p2.x, val + p2.y, val + p2.z, val + p2.w); }

void nex::gui::RectangleShadow::drawRectangleShadowVerticesAdaptive() const
{
    const int    samplesSpan = samplesPerCornerSide * spacingBetweenSamples;
    const int    halfWidth = static_cast<int>(rectSize.x / 2);
    const int    numSamplesInHalfWidth = (halfWidth / spacingBetweenSamples) == 0 ? 1 : halfWidth / spacingBetweenSamples;
    const int    numSamplesWidth = samplesSpan > halfWidth ? numSamplesInHalfWidth : samplesPerCornerSide;
    const int    halfHeight = static_cast<int>(rectSize.y / 2);
    const int    numSamplesInHalfHeight = (halfHeight / spacingBetweenSamples) == 0 ? 1 : halfHeight / spacingBetweenSamples;
    const int    numSamplesHeight = samplesSpan > halfHeight ? numSamplesInHalfHeight : samplesPerCornerSide;
    const int    numVerticesInARing = numSamplesWidth * 4 + numSamplesHeight * 4 + 4;
    const ImVec2 whiteTexelUV = ImGui::GetIO().Fonts->TexUvWhitePixel;
    const ImVec2 origin = ImGui::GetCursorScreenPos();
    const ImVec2 rectangleTopLeft = origin + rectPos; // + rectPos
    const ImVec2 rectangleBottomRight = rectangleTopLeft + rectSize;
    const ImVec2 rectangleTopRight = rectangleTopLeft + ImVec2(rectSize.x, 0);
    const ImVec2 rectangleBottomLeft = rectangleTopLeft + ImVec2(0, rectSize.y);

    ImColor color = shadowColor;
    totalVertices = numVerticesInARing * rings;
    totalIndices = 6 * (numVerticesInARing) * (rings - 1);

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->PrimReserve(totalIndices, totalVertices);
    const ImDrawVert* shadowVertices = drawList->_VtxWritePtr;
    ImDrawVert*& vertexPointer = drawList->_VtxWritePtr;

    for (int r = 0; r < rings; ++r)
    {
        const float  adaptiveScale = (r / 2.5f) + 1;
        const ImVec2 ringOffset = ImVec2(adaptiveScale * r * spacingBetweenRings, adaptiveScale * r * spacingBetweenRings);
        for (int j = 0; j < 4; ++j)
        {
            ImVec2      corner;
            ImVec2      direction[2];
            const float spacingBetweenSamplesOnARing = static_cast<float>(spacingBetweenSamples);
            switch (j)
            {
            case 0:
                corner = rectangleTopLeft + ImVec2(-ringOffset.x, -ringOffset.y);
                direction[0] = ImVec2(1, 0) * spacingBetweenSamplesOnARing;
                direction[1] = ImVec2(0, 1) * spacingBetweenSamplesOnARing;
                for (int i = 0; i < numSamplesWidth; ++i)
                {
                    const ImVec2 point = corner + direction[0] * (numSamplesWidth - i);
                    color.Value.w = boxShadow(rectangleTopLeft, rectangleBottomRight, point - shadowOffset, sigma, linear);
                    vertexPointer->pos = point;
                    vertexPointer->uv = whiteTexelUV;
                    vertexPointer->col = color;
                    vertexPointer++;
                }

                color.Value.w = boxShadow(rectangleTopLeft, rectangleBottomRight, corner - shadowOffset, sigma, linear);
                vertexPointer->pos = corner;
                vertexPointer->uv = whiteTexelUV;
                vertexPointer->col = color;
                vertexPointer++;

                for (int i = 0; i < numSamplesHeight; ++i)
                {
                    const ImVec2 point = corner + direction[1] * (i + 1);
                    color.Value.w = boxShadow(rectangleTopLeft, rectangleBottomRight, point - shadowOffset, sigma, linear);
                    vertexPointer->pos = point;
                    vertexPointer->uv = whiteTexelUV;
                    vertexPointer->col = color;
                    vertexPointer++;
                }
                break;
            case 1:
                corner = rectangleBottomLeft + ImVec2(-ringOffset.x, +ringOffset.y);
                direction[0] = ImVec2(1, 0) * spacingBetweenSamplesOnARing;
                direction[1] = ImVec2(0, -1) * spacingBetweenSamplesOnARing;
                for (int i = 0; i < numSamplesHeight; ++i)
                {
                    const ImVec2 point = corner + direction[1] * (numSamplesHeight - i);
                    color.Value.w = boxShadow(rectangleTopLeft, rectangleBottomRight, point - shadowOffset, sigma, linear);
                    vertexPointer->pos = point;
                    vertexPointer->uv = whiteTexelUV;
                    vertexPointer->col = color;
                    vertexPointer++;
                }

                color.Value.w = boxShadow(rectangleTopLeft, rectangleBottomRight, corner - shadowOffset, sigma, linear);
                vertexPointer->pos = corner;
                vertexPointer->uv = whiteTexelUV;
                vertexPointer->col = color;
                vertexPointer++;

                for (int i = 0; i < numSamplesWidth; ++i)
                {
                    const ImVec2 point = corner + direction[0] * (i + 1);
                    color.Value.w = boxShadow(rectangleTopLeft, rectangleBottomRight, point - shadowOffset, sigma, linear);
                    vertexPointer->pos = point;
                    vertexPointer->uv = whiteTexelUV;
                    vertexPointer->col = color;
                    vertexPointer++;
                }
                break;
            case 2:
                corner = rectangleBottomRight + ImVec2(+ringOffset.x, +ringOffset.y);
                direction[0] = ImVec2(-1, 0) * spacingBetweenSamplesOnARing;
                direction[1] = ImVec2(0, -1) * spacingBetweenSamplesOnARing;
                for (int i = 0; i < numSamplesWidth; ++i)
                {
                    const ImVec2 point = corner + direction[0] * (numSamplesWidth - i);
                    color.Value.w = boxShadow(rectangleTopLeft, rectangleBottomRight, point - shadowOffset, sigma, linear);
                    vertexPointer->pos = point;
                    vertexPointer->uv = whiteTexelUV;
                    vertexPointer->col = color;
                    vertexPointer++;
                }

                color.Value.w = boxShadow(rectangleTopLeft, rectangleBottomRight, corner - shadowOffset, sigma, linear);
                vertexPointer->pos = corner;
                vertexPointer->uv = whiteTexelUV;
                vertexPointer->col = color;
                vertexPointer++;

                for (int i = 0; i < numSamplesHeight; ++i)
                {
                    const ImVec2 point = corner + direction[1] * (i + 1);
                    color.Value.w = boxShadow(rectangleTopLeft, rectangleBottomRight, point - shadowOffset, sigma, linear);
                    vertexPointer->pos = point;
                    vertexPointer->uv = whiteTexelUV;
                    vertexPointer->col = color;
                    vertexPointer++;
                }
                break;
            case 3:
                corner = rectangleTopRight + ImVec2(+ringOffset.x, -ringOffset.y);
                direction[0] = ImVec2(-1, 0) * spacingBetweenSamplesOnARing;
                direction[1] = ImVec2(0, 1) * spacingBetweenSamplesOnARing;
                for (int i = 0; i < numSamplesHeight; ++i)
                {
                    const ImVec2 point = corner + direction[1] * (numSamplesHeight - i);
                    color.Value.w = boxShadow(rectangleTopLeft, rectangleBottomRight, point - shadowOffset, sigma, linear);
                    vertexPointer->pos = point;
                    vertexPointer->uv = whiteTexelUV;
                    vertexPointer->col = color;
                    vertexPointer++;
                }

                color.Value.w = boxShadow(rectangleTopLeft, rectangleBottomRight, corner - shadowOffset, sigma, linear);
                vertexPointer->pos = corner;
                vertexPointer->uv = whiteTexelUV;
                vertexPointer->col = color;
                vertexPointer++;

                for (int i = 0; i < numSamplesWidth; ++i)
                {
                    const ImVec2 point = corner + direction[0] * (i + 1);
                    color.Value.w = boxShadow(rectangleTopLeft, rectangleBottomRight, point - shadowOffset, sigma, linear);
                    vertexPointer->pos = point;
                    vertexPointer->uv = whiteTexelUV;
                    vertexPointer->col = color;
                    vertexPointer++;
                }
                break;
            }
        }
    }

    ImDrawIdx idx = (ImDrawIdx)drawList->_VtxCurrentIdx;

    for (int r = 0; r < rings - 1; ++r)
    {
        const ImDrawIdx startOfRingIndex = idx;
        for (int i = 0; i < numVerticesInARing - 1; ++i)
        {
            drawList->_IdxWritePtr[0] = idx + 0;
            drawList->_IdxWritePtr[1] = idx + 1;
            drawList->_IdxWritePtr[2] = idx + numVerticesInARing;
            drawList->_IdxWritePtr[3] = idx + 1;
            drawList->_IdxWritePtr[4] = idx + numVerticesInARing + 1;
            drawList->_IdxWritePtr[5] = idx + numVerticesInARing;

            idx += 1;
            drawList->_IdxWritePtr += 6;
        }

        drawList->_IdxWritePtr[0] = idx + 0;
        drawList->_IdxWritePtr[1] = startOfRingIndex + 0;
        drawList->_IdxWritePtr[2] = startOfRingIndex + numVerticesInARing;
        drawList->_IdxWritePtr[3] = idx + 0;
        drawList->_IdxWritePtr[4] = startOfRingIndex + numVerticesInARing;
        drawList->_IdxWritePtr[5] = idx + numVerticesInARing;

        drawList->_IdxWritePtr += 6;
        idx += 1;
    }
    drawList->_VtxCurrentIdx += totalVertices;

    if (enableDebugVisualization)
    {
        const ImColor lineColor(0, 0, 255, 50);
        for (int r = 0; r < rings - 1; ++r)
        {
            const ImDrawIdx startOfRingIndex = r * numVerticesInARing;

            ImVec2 a;
            ImVec2 b;
            for (int i = 0; i < numVerticesInARing - 1; ++i)
            {
                a = shadowVertices[startOfRingIndex + i + 1].pos;
                b = shadowVertices[startOfRingIndex + i + numVerticesInARing].pos;
                drawList->AddLine(a, b, lineColor);
                a = shadowVertices[startOfRingIndex + i + 0].pos;
                b = shadowVertices[startOfRingIndex + i + numVerticesInARing].pos;
                drawList->AddLine(a, b, lineColor);
                a = shadowVertices[startOfRingIndex + i + numVerticesInARing + 1].pos;
                b = shadowVertices[startOfRingIndex + i + numVerticesInARing].pos;
                drawList->AddLine(a, b, lineColor);
            }

            a = shadowVertices[startOfRingIndex + numVerticesInARing - 1].pos;
            b = shadowVertices[startOfRingIndex + numVerticesInARing].pos;
            drawList->AddLine(a, b, lineColor);
            a = shadowVertices[startOfRingIndex + numVerticesInARing + numVerticesInARing - 1].pos;
            b = shadowVertices[startOfRingIndex + numVerticesInARing].pos;
            drawList->AddLine(a, b, lineColor);
        }

        for (int i = 0; i < totalVertices; ++i)
        {
            const ImVec2 bmin = shadowVertices[i].pos - ImVec2(2, 2);
            const ImVec2 bmax = shadowVertices[i].pos + ImVec2(2, 2);
            drawList->AddRectFilled(bmin, bmax, ImColor(255, 0, 0, 50));
        }
    }
}

float nex::gui::RectangleShadow::boxShadow(const ImVec2& lower, const ImVec2& upper, const ImVec2& point, float sigma, bool linearInterpolation)
{
    const ImVec2 pointLower = point - lower;
    const ImVec2 pointUpper = point - upper;
    const ImVec4 query = ImVec4(pointLower.x, pointLower.y, pointUpper.x, pointUpper.y);
    const ImVec4 pointToSample = query * (sqrtf(0.5f) / sigma);
    const ImVec4 integral = linearInterpolation ? 0.5f + 0.5f * boxLinearInterpolation(pointToSample) : 0.5f + 0.5f * boxGaussianIntegral(pointToSample);
    return (integral.z - integral.x) * (integral.w - integral.y);
}

ImVec4 nex::gui::RectangleShadow::boxLinearInterpolation(const ImVec4& x)
{
    const float maxClamp = 1.0f;
    const float minClamp = -1.0f;
    return ImVec4(x.x > maxClamp ? maxClamp : x.x < minClamp ? minClamp : x.x,
        x.y > maxClamp ? maxClamp : x.y < minClamp ? minClamp : x.y,
        x.z > maxClamp ? maxClamp : x.z < minClamp ? minClamp : x.z,
        x.w > maxClamp ? maxClamp : x.w < minClamp ? minClamp : x.w);
}

ImVec4 nex::gui::RectangleShadow::boxGaussianIntegral(const ImVec4& x)
{
    const ImVec4 s = ImVec4(x.x > 0 ? 1.0f : -1.0f, x.y > 0 ? 1.0f : -1.0f, x.z > 0 ? 1.0f : -1.0f, x.w > 0 ? 1.0f : -1.0f);
    const ImVec4 a = ImVec4(fabsf(x.x), fabsf(x.y), fabsf(x.z), fabsf(x.w));
    const ImVec4 res = 1.0f + (0.278393f + (0.230389f + 0.078108f * (a * a)) * a) * a;
    const ImVec4 resSquared = res * res;
    return s - s / (resSquared * resSquared);
}