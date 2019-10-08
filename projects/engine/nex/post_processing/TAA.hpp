#pragma once

#include <memory>

namespace nex
{
	class Texture;
	class Camera;

	class TAA
	{
	public:
		TAA();
		~TAA();

		/**
		 * Renders an antialiased version of the specified source texture into the currently bound rendertarget (color attachment 0).
		 */
		void antialias(Texture* source, Texture* sourceHistory, Texture* depth, const Camera& camera);

	private:

		class TaaPass;

		std::unique_ptr<TaaPass> mTaaPass;
	};
}