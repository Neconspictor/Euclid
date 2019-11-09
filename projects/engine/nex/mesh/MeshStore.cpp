#include <nex/mesh/MeshStore.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include "nex/common/File.hpp"
#include <nex/mesh/VertexLayout.hpp>


void nex::MeshStore::read(nex::BinStream& in)
{
	in >> indexType;
	in >> layout;
	in >> boundingBox;
	in >> topology;
	in >> material;
	in >> indices;
	in >> vertices;
}

void nex::MeshStore::write(nex::BinStream& out) const
{
	out << indexType;
	out << layout;
	out << boundingBox;
	out << topology;
	out << material;
	out << indices;
	out << vertices;
}

void nex::MeshStore::test()
{
	MeshStore store;
	store.indexType = IndexElementType::BIT_32;
	store.indices.resize(24 * 4);
	store.boundingBox.min = glm::vec3(1);
	store.boundingBox.max = glm::vec3(10);
	store.layout.push<glm::vec3>(2, nullptr, false, false);
	store.topology = Topology::TRIANGLES;
	store.vertices.resize(8*store.layout.getStride());

	{
		BinStream file(0);
		file.open("mesh.mesh", std::ios::out | std::ios::trunc);
		file << store;
	}
	{
		MeshStore store2;
		BinStream file(0);
		file.open("mesh.mesh", std::ios::in); 
		file >> store2;
	}

}

std::ostream& nex::operator<<(nex::BinStream& out, const nex::MeshStore& mesh)
{
	mesh.write(out);
	return out;
}

std::istream& nex::operator>>(nex::BinStream& in, nex::MeshStore& mesh)
{
	mesh.read(in);
	return in;
}