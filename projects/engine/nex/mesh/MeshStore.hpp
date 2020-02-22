#pragma once

#include <nex/mesh/MeshTypes.hpp>
#include <nex/math/BoundingBox.hpp>
#include <vector>
#include "VertexLayout.hpp"
#include "nex/material/Material.hpp"
#include <string>

namespace nex
{
	struct MeshStore
	{
		IndexElementType indexType;
		VertexLayout layout;
		AABB boundingBox;
		Topology topology;
		MaterialStore material;

		std::vector<char> indices;
		std::map<const nex::GpuBuffer*, std::vector<char>> verticesMap;

		bool useIndexBuffer;
		size_t arrayOffset;
		size_t vertexCount;

		bool isSkinned;
		std::string rigID; // only used by skinned meshes

		void read(nex::BinStream& in);
		void write(nex::BinStream& out) const;

		static void test();
	};

	std::ostream& operator<<(nex::BinStream& out, const nex::MeshStore& mesh);
	std::istream& operator>>(nex::BinStream& in, nex::MeshStore& mesh);
}
