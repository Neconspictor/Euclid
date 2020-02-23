#include <nex/scene/VobStore.hpp>

std::ostream& nex::operator<<(nex::BinStream& out, const nex::VobBaseStore& vob)
{
	out << vob.localToParentTrafo;
	//out << vob.meshToLocalTrafo;
	out << vob.meshes;
	out << vob.children;
	out << vob.meshes;

	return out;
}

std::istream& nex::operator>>(nex::BinStream& in, nex::VobBaseStore& vob)
{
	in >> vob.localToParentTrafo;
	//in >> vob.meshToLocalTrafo;
	in >> vob.children;
	in >> vob.meshes;

	return in;
}