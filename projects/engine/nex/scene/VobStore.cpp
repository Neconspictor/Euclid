#include <nex/scene/VobStore.hpp>

std::ostream& nex::operator<<(nex::BinStream& out, const nex::VobBaseStore& vob)
{
	out << vob.localToParentTrafo;
	out << vob.meshes;
	out << vob.nodeName;
	//out << vob.meshToLocalTrafo;	
	out << vob.children;

	return out;
}

std::istream& nex::operator>>(nex::BinStream& in, nex::VobBaseStore& vob)
{
	in >> vob.localToParentTrafo;
	in >> vob.meshes;
	in >> vob.nodeName;
	//in >> vob.meshToLocalTrafo;
	in >> vob.children;
	
	

	return in;
}