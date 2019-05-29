#pragma once

#include <nex/mesh/MeshTypes.hpp>
#include <nex/util/Math.hpp>
#include <vector>
#include "VertexLayout.hpp"

namespace nex
{
	struct MeshStore
	{
		IndexElementType indexType;
		VertexLayout layout;
		AABB boundingBox;
		Topology topology;
		std::vector<char> indices;
		std::vector<char> vertices;

		void read(std::istream& in);
		void write(std::ostream& out) const;

		static void test();
	};

	std::ostream& operator<<(std::ostream& out, const nex::MeshStore& mesh);
	std::istream& operator>>(std::istream& in, nex::MeshStore& mesh);
}