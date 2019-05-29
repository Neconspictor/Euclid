#include <nex/mesh/MeshStore.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include "nex/common/File.hpp"
#include <nex/mesh/VertexLayout.hpp>


void nex::MeshStore::read(std::istream& in)
{
	in >> indexType;
	in >> layout;
	in >> boundingBox;
	in >> topology;
	in >> indices;
	in >> vertices;
}

void nex::MeshStore::write(std::ostream& out) const
{
	out << indexType;
	out << layout;
	out << boundingBox;
	out << topology;
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
	store.layout.push<glm::vec3>(2);
	store.topology = Topology::TRIANGLES;
	store.vertices.resize(8*store.layout.getStride());

	{
		File file(0);
		file.open("mesh.mesh", std::ios::binary | std::ios::out | std::ios::trunc) << store;
	}
	{
		MeshStore store2;
		File file(0);
		file.open("mesh.mesh", std::ios::binary | std::ios::in) >> store2;
		std::cout << "Read store mesh:\n" << store2 << std::endl;
	}

}

std::ostream& nex::operator<<(std::ostream& out, const nex::MeshStore& mesh)
{
	mesh.write(out);
	return out;
}

std::istream& nex::operator>>(std::istream& in, nex::MeshStore& mesh)
{
	mesh.read(in);
	return in;
}