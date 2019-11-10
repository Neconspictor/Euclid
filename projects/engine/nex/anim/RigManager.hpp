#pragma once
#include <nex/anim/Rig.hpp>


namespace nex {

	class ImportScene;
	
	class RigManager {
	public:

		/**
		 * Adds a rig.
		 * @throws std::invalid_argument : if the manager contains a rig having the same id as the rig to be added.
		 */
		void add(std::unique_ptr<Rig> rig);

		/**
		 * Provides a rig by its id.
		 * If no suitable rig is found, null is returned.
		 */
		const Rig* getByID(unsigned id) const;

		/**
		 * Provides the rig manager.
		 */
		static RigManager* get();

		/**
		 * Loads a Rig from a scene.
		 */
		const Rig* load(const ImportScene& importScene);

	private:
		std::unordered_map<unsigned, std::unique_ptr<Rig>> mRigs;
	};
}