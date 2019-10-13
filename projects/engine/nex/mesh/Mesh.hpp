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


		/**
		 * The mesh class can be used for conveniently storing buffers containing actual vertex data.
		 */
		void addVertexDataBuffer(std::unique_ptr<GpuBuffer> buffer);

		void finalize() override;


		const AABB& getAABB() const;
		IndexBuffer& getIndexBuffer();
		const IndexBuffer& getIndexBuffer() const;

		const VertexLayout& getLayout() const;
		VertexLayout& getLayout();
		
		Topology getTopology() const;
		VertexArray& getVertexArray();
		const VertexArray& getVertexArray() const;
		
		std::vector<std::unique_ptr<GpuBuffer>>& getVertexBuffers();
		const std::vector<std::unique_ptr<GpuBuffer>>& getVertexBuffers() const;

		void setIndexBuffer(IndexBuffer buffer);
		void setTopology(Topology topology);
		void setVertexArray(VertexArray vertexArray);
		void setBoundingBox(AABB box);
		void setLayout(VertexLayout layout);
		
		std::string mDebugName;

	protected:
		std::unique_ptr<VertexArray> mVertexArray;
		VertexLayout mLayout;
		IndexBuffer mIndexBuffer;
		std::vector<std::unique_ptr<GpuBuffer>> mBuffers;
		AABB mBoundingBox;

		Topology mTopology;
	};
}
