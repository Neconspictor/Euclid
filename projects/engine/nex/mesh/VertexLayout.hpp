#pragma once
#include <vector>

namespace nex
{

	enum class LayoutType
	{
		UNSIGNED_INT,
		FLOAT,
		UNSIGNED_BYTE
	};


	struct LayoutElement
	{
		LayoutType type;
		unsigned int count;
		bool normalized;

		static inline unsigned int getSizeOfType(LayoutType type);
	};

	class VertexLayout
	{
	private:
		std::vector<nex::LayoutElement> mElements;
		unsigned int mStride;

	public:
		VertexLayout() : mStride(0) {}

		template<typename T>
		inline void push(unsigned int count);

		template<>
		inline void push<float>(unsigned int count);

		template<>
		inline void push<unsigned int>(unsigned int count);

		template<>
		inline void push<unsigned char>(unsigned int count);

		template<>
		inline void push<glm::vec3>(unsigned int count);

		template<>
		inline void push<glm::vec2>(unsigned int count);

		inline unsigned int getStride() const;
		inline const std::vector<LayoutElement>& getElements() const;
	};

}

#include "VertexLayout.inl"