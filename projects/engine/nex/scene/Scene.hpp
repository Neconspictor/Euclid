#pragma once
#include <unordered_set>
#include <nex/drawing/MeshDrawer.hpp>
#include <vector>
#include <memory>
#include <nex/util/Iterator.hpp>
#include <nex/math/BoundingBox.hpp>


#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif
#include <glm/gtx/quaternion.hpp>
#include <nex/common/Concurrent.hpp>

namespace nex
{
	class MeshBatch;
	class ProbeVob;
	class FrameUpdateable;
	class MeshGroup;
	class RenderCommandQueue;

	/**
	 * A scene manages the creation and lifetime of scene nodes.
	 * A scene is a list of trees;
	 */
	class Scene
	{
	public:

		using VobRange = std::unordered_set<Vob*>;
		using FrameUpdateableRange = std::unordered_set<FrameUpdateable*>;
		using ProbeRange = std::unordered_set<ProbeVob*>;

		/**
		 * Creates a new scene object.
		 */
		Scene();
		~Scene();

		UniqueLock acquireLock() const;

		void addActiveVobUnsafe(Vob* vob);
		void removeActiveVobUnsafe(Vob* vob);
		bool deleteVobUnsafe(Vob* vob);


		Vob* addVobUnsafe(std::unique_ptr<Vob> vob, bool setActive = true);
		Vob* createVobUnsafe(std::list<MeshBatch>* batches, bool setActive = true);

		/**
		 * Provides all vobs that are currently active.
		 */
		const VobRange& getActiveVobsUnsafe() const;

		/**
		 * Provides all vobs that are currently active.
		 */
		const FrameUpdateableRange& getActiveFrameUpdateables() const;

		/**
		 * Provides all probe vobs that are currently active.
		 */
		const ProbeRange& getActiveProbeVobsUnsafe() const;

		/**
		 * Provides all vobs of this scene.
		 */
		const std::vector<std::unique_ptr<Vob>>& getVobsUnsafe() const;
		std::vector<std::unique_ptr<Vob>>& getVobsUnsafe();

		bool hasChangedUnsafe() const;


		/**
		 * Checks if a vob is active.
		 */
		bool isActive(Vob* vob) const;

		/**
		 * Deletes all nodes except the root node.
		 */
		void clearUnsafe();
		void updateWorldTrafoHierarchyUnsafe(bool resetPrevWorldTrafo);

		void calcSceneBoundingBoxUnsafe();
		const AABB& getSceneBoundingBox() const;

		void collectRenderCommands(RenderCommandQueue& commandQueue, bool doCulling, ShaderStorageBuffer* boneTrafoBuffer) const;

		void setHasChangedUnsafe(bool changed);

	private:
		VobRange mActiveVobs;
		FrameUpdateableRange mActiveUpdateables;
		ProbeRange mActiveProbeVobs;
		std::vector<std::unique_ptr<Vob>> mVobStore;
		mutable std::mutex mMutex;
		AABB mBoundingBox;
		bool mHasChanged;
	};
}