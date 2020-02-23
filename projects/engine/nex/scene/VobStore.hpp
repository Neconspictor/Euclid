#pragma once

#include <nex/mesh/MeshStore.hpp>

namespace nex {

	/**
	* Simple class containing basis data for creating vobs in a general way (contains as little data as possible)
	*/
	struct VobBaseStore {
		// Used to create a mesh group
		// Note: MeshStore is a base class; we need pointers for polymorphy
		using MeshVec = std::vector<nex::MeshStore>;


		// Transforms to parent space
		glm::mat4 localToParentTrafo;
		// Transforms meshes to local space
		//glm::mat4 meshToLocalTrafo;
		MeshVec meshes;
		std::vector<VobBaseStore> children;
	};

	std::ostream& operator<<(nex::BinStream& out, const nex::VobBaseStore& vob);
	std::istream& operator>>(nex::BinStream& in, nex::VobBaseStore& vob);
}