#pragma once

#include "VertexArray.hpp"
#include "VertexBuffer.hpp"
#include "IndexBuffer.hpp"
#include <nex/mesh/MeshTypes.hpp>
#include <nex/math/BoundingBox.hpp>

namespace nex
{
	class MeshFactory;

	/**
	 * Represents a 3d mesh consisting of vertices and a list of indices describing
	 * a stream of three sided polygons. A vertex describes the position of a 3d point
	 * and can have additional information like a normal and texture uv coordinates.
	 * The so-called vertex slice describes of how many (Real) data elements a vertex
	 * consists of. Theoretically, a vertex isn't bound to Realing point units, but this
	 * implementation narrows it to Reals for ease of use.
	 */
	class Mesh
	{
	public:
		using Vertex = VertexPositionNormalTexTangent;

		Mesh(VertexArray vertexArray, VertexBuffer vertexBuffer, IndexBuffer indexBuffer, AABB boundingBox, Topology topology = Topology::TRIANGLES);
		Mesh();

		Mesh(Mesh&& other) noexcept = default;
		Mesh& operator=(Mesh&& o) noexcept = default;

		Mesh(const Mesh& o) = delete;
		Mesh& operator=(const Mesh& o) = delete;

		virtual ~Mesh() = default;

		const AABB& getAABB() const;
		IndexBuffer* getIndexBuffer();
		Topology getTopology() const;
		VertexArray* getVertexArray();
		VertexBuffer* getVertexBuffer();

		void setIndexBuffer(IndexBuffer buffer);
		void setTopology(Topology topology);
		void setVertexArray(VertexArray vertexArray);
		void setVertexBuffer(VertexBuffer buffer);
		void setBoundingBox(const AABB& box);
		
		std::string mDebugName;

	protected:
		VertexArray mVertexArray;
		IndexBuffer mIndexBuffer;
		VertexBuffer mVertexBuffer;
		AABB mBoundingBox;

		Topology mTopology;
	};
}
