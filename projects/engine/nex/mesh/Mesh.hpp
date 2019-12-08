#pragma once

#include "VertexArray.hpp"
#include <nex/buffer/VertexBuffer.hpp>
#include <nex/buffer/IndexBuffer.hpp>
#include <nex/mesh/MeshTypes.hpp>
#include <nex/math/BoundingBox.hpp>
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
		size_t getArrayOffset() const;

		/**
		 * Provides access to the mesh's index buffer.
		 * Nullptr is returned, if the mesh doesn't use an index buffer.
		 */
		IndexBuffer* getIndexBuffer();

		/**
		 * Provides access to the mesh's index buffer.
		 * Nullptr is returned, if the mesh doesn't use an index buffer.
		 */
		const IndexBuffer* getIndexBuffer() const;
		
		Topology getTopology() const;
		bool getUseIndexBuffer() const;
		VertexArray& getVertexArray();
		const VertexArray& getVertexArray() const;
		size_t getVertexCount() const;
		
		
		std::vector<std::unique_ptr<GpuBuffer>>& getVertexBuffers();
		const std::vector<std::unique_ptr<GpuBuffer>>& getVertexBuffers() const;

		void setArrayOffset(size_t offset);
		void setIndexBuffer(IndexBuffer&& buffer);
		void setTopology(Topology topology);
		void setVertexArray(VertexArray&& vertexArray);
		void setBoundingBox(const AABB& box);
		void setUseIndexBuffer(bool use);
		void setVertexCount(size_t count);

		std::string mDebugName;

	protected:
		VertexArray mVertexArray;
		std::unique_ptr<IndexBuffer> mIndexBuffer;
		std::vector<std::unique_ptr<GpuBuffer>> mBuffers;
		AABB mBoundingBox;
		Topology mTopology;
		bool mUseIndexBuffer;
		size_t mVertexCount;
		size_t mArrayOffset;

	};

	class SkinnedMesh : public Mesh
	{
	public:
		SkinnedMesh() = default;
		virtual ~SkinnedMesh() = default;

		const std::string& getRigID() const;
		void setRigID(const std::string& rigID);

	private:
		std::string mRigSID;
	};
}
