#include <nex/mesh/VertexLayout.hpp>

void nex::VertexLayout::read(nex::BinStream& in)
{
	in >> mMap;
	in >> mLocationCounter;
}

void nex::VertexLayout::write(nex::BinStream& out) const
{
	out << mMap;
	out << mLocationCounter;
}

nex::BinStream& nex::operator>>(nex::BinStream& in, nex::BufferLayout& layout)
{
	in >> layout.attributes;
	in >> layout.stride;
	in >> layout.offset;
	return in;
}

nex::BinStream& nex::operator<<(nex::BinStream& out, const nex::BufferLayout& layout)
{
	out << layout.attributes;
	out << layout.stride;
	out << layout.offset;
	return out;
}

nex::BinStream& nex::operator>>(nex::BinStream& in, nex::VertexLayout& layout)
{
	layout.read(in);
	return in;
}

nex::BinStream& nex::operator<<(nex::BinStream& out, const nex::VertexLayout& layout)
{
	layout.write(out);
	return out;
}