#include <nex/mesh/VertexLayout.hpp>

void nex::VertexLayout::read(nex::BinStream& in)
{
	in >> mElements;
	in >> mStride;
}

void nex::VertexLayout::write(nex::BinStream& out) const
{
	out << mElements;
	out << mStride;
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