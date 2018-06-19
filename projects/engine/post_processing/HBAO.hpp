#ifndef HBAO_HPP
#define HBAO_HPP

#include <vector>
#include <sprite/Sprite.hpp>

class RenderTarget;
class PBR_GBuffer;
class SceneNode;
class Texture;

/*-----------------------------------------------------------------------
Copyright (c) 2014, NVIDIA. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Neither the name of its contributors may be used to endorse
or promote products derived from this software without specific
prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
-----------------------------------------------------------------------*/

namespace hbao {

#define UBO_SCENE     0
#define AO_RANDOMTEX_SIZE 4

	struct Projection {
		float nearplane;
		float farplane;
		float fov;
		float orthoheight;
		bool  perspective;
		glm::mat4  matrix;
	};

	struct SceneData {
		glm::mat4  viewProjMatrix;
		glm::mat4  viewMatrix;
		glm::mat4  viewMatrixIT;

		glm::uvec2 viewport;
		glm::uvec2 _pad;
	};

	struct HBAOData {
		float   RadiusToScreen;        // radius
		float   R2;     // radius * radius
		float   NegInvR2;     // -1/radius
		float   NDotVBias;

		glm::vec2    InvFullResolution;
		glm::vec2    InvQuarterResolution;

		float   AOMultiplier;
		float   PowExponent;
		glm::vec2    _pad0;

		glm::vec4    projInfo;
		glm::vec2    projScale;
		int     projOrtho;
		int     _pad1;

		glm::vec4    float2Offsets[AO_RANDOMTEX_SIZE*AO_RANDOMTEX_SIZE];
		glm::vec4    jitters[AO_RANDOMTEX_SIZE*AO_RANDOMTEX_SIZE];
	};

	class HBAO_Deferred {
	public:

		HBAO_Deferred(unsigned int windowWidth,
			unsigned int windowHeight);

		virtual ~HBAO_Deferred() = default;

		virtual Texture* getAO_Result() = 0;
		virtual Texture* getBlurredResult() = 0;
		virtual void onSizeChange(unsigned int newWidth, unsigned int newHeight) = 0;

		virtual void renderAO(Texture* depth, const Projection& projection, bool blur) = 0;
		virtual void displayAOTexture() = 0;
		virtual void displayTexture(Texture* texture) = 0;


	protected:
		float randomFloat(float a, float b);
		float lerp(float a, float b, float f);

	protected:
		unsigned int windowWidth;
		unsigned int windowHeight;

		Sprite screenSprite;

		static const int  HBAO_RANDOM_SIZE = AO_RANDOMTEX_SIZE;
		static const int  HBAO_RANDOM_ELEMENTS = HBAO_RANDOM_SIZE*HBAO_RANDOM_SIZE;
		static const int  MAX_SAMPLES = 8;
	};
}

#endif //HBAO_HPP