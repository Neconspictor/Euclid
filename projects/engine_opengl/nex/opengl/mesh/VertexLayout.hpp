#pragma once
#include <vector>
#include <string>

struct LayoutElement
{
	unsigned int type;
	unsigned int count;
	unsigned char normalized;

	static inline unsigned int getSizeOfType(unsigned int type);
};

class VertexLayout
{
private:
	std::vector<LayoutElement> mElements;
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

#include "VertexLayout.inl"