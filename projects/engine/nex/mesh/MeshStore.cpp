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
	in >> useIndexBuffer;
	in >> arrayOffset;
	in >> vertexCount;
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
	out << useIndexBuffer;
	out << arrayOffset;
	out << vertexCount;
}

void nex::MeshStore::test()
{

	struct NotTrivial {
		std::string myStr;
		int myInt;
	};

	NotTrivial notTrivial;

	std::unordered_map<int, char> map;
	map.insert(std::pair<int, char>(2,5));
	map.insert(std::pair<int, char>(3,4));

	MeshStore store;
	store.indexType = IndexElementType::BIT_32;
	store.indices.resize(24 * 4);
	store.boundingBox.min = glm::vec3(1);
	store.boundingBox.max = glm::vec3(10);
	store.layout.push<glm::vec3>(2, nullptr, false, false, true);
	store.topology = Topology::TRIANGLES;
	store.vertices.resize(8*store.layout.getLayout(nullptr).stride);
	store.useIndexBuffer = true;
	store.arrayOffset = 0;
	store.vertexCount = 8;

	{
		BinStream file(0);
		file.open("mesh.mesh", std::ios::out | std::ios::trunc);
		file << store;
		file << map;
		//file << notTrivial;
	}
	{
		MeshStore store2;
		BinStream file(0);
		file.open("mesh.mesh", std::ios::in); 
		file >> store2;
		file >> map;

		//std::unordered_map<int, char>::iterator it;
		std::vector<int>::iterator it;
		using type = std::remove_const_t<std::remove_reference_t<decltype(*it)>>;
		type t;// = {};
		t;
		//file >> notTrivial;
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

void nex::SkinnedMeshStore::read(nex::BinStream& in)
{
	MeshStore::read(in);
	in >> rigID;
}

void nex::SkinnedMeshStore::write(nex::BinStream& out) const
{
	MeshStore::write(out);
	out << rigID;
}

std::ostream& nex::operator<<(nex::BinStream& out, const nex::SkinnedMeshStore& mesh)
{
	mesh.write(out);
	return out;
}

std::istream& nex::operator>>(nex::BinStream& in, nex::SkinnedMeshStore& mesh)
{
	mesh.read(in);
	return in;
}