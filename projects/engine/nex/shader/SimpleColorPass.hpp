#include <nex/shader/Pass.hpp>

namespace nex
{
	class SimpleColorPass : public TransformPass
	{
	public:
		SimpleColorPass();

		void setColor(const glm::vec4 color);

	private:
		glm::vec4 mColor;
		Uniform mColorUniform;
	};
}