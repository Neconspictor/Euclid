#pragma once

#include "VertexArray.hpp"
#include <nex/buffer/VertexBuffer.hpp>
#include <nex/buffer/IndexBuffer.hpp>
#include <nex/mesh/MeshTypes.hpp>
#include <nex/math/BoundingBox.hpp>
#include "VertexLayout.hpp"
#include <nex/resource/Resource.hpp>

namespace nex
{
	class MeshFactory;

	/**
	 * Represents a 3d mesh consisting of vertices and a list of indices describing
	 * a stream of three sided polygons. A vertex describes the position of a 3d point
	 * and can have additional information like a normal and texture uv coordinates.
	 * The so-called vertex slice describes of how many (float) data elements a vertex
	 * consists of. Theoretically, a vertex isn't bound to floating point units, but this
	 * implementation narrows it to floats for ease of use.
	 */
	class Mesh : public Resource
	{
	public:
		using Vertex = VertexPositionNormalTexTangent;

		Mesh();

		virtual ~Mesh() = default;

		void finalize() override;


		const AABB& getAABB() const;
		IndexBuffer* getIndexBuffer();
		const VertexLayout& getLayout() const;
		Topology getTopology() const;
		VertexArray* getVertexArray();
		VertexBuffer* getVertexBuffer();

		void init(VertexBuffer vertexBuffer, VertexLayout layout, IndexBuffer indexBuffer, AABB boundingBox, Topology topology = Topology::TRIANGLES);

		void setIndexBuffer(IndexBuffer buffer);
		void setTopology(Topology topology);
		void setVertexArray(VertexArray vertexArray);
		void setVertexBuffer(VertexBuffer buffer);
		void setBoundingBox(const AABB& box);
		
		std::string mDebugName;

	protected:
		std::unique_ptr<VertexArray> mVertexArray;
		VertexLayout mLayout;
		IndexBuffer mIndexBuffer;
		VertexBuffer mVertexBuffer;
		AABB mBoundingBox;

		Topology mTopology;
	};
}
