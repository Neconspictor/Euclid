#pragma once
#include <unordered_set>
#include <nex/renderer/Drawer.hpp>
#include <vector>
#include <memory>
#include <nex/util/Iterator.hpp>
#include <nex/math/BoundingBox.hpp>
#include <nex/renderer/RenderCommandFactory.hpp>
#include <nex/common/FrameUpdateable.hpp>


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
	class Scene : public RenderCommandFactory, public FrameUpdateable
	{
	public:

		using VobRange = std::vector<Vob*>;
		using VobStore = std::unordered_set<std::unique_ptr<Vob>>;
		using FrameUpdateableRange = std::unordered_set<FrameUpdateable*>;
		using ProbeRange = std::vector<ProbeVob*>;

		/**
		 * Creates a new scene object.
		 */
		Scene();
		~Scene();

		UniqueLock acquireLock() const;

		void addActiveVobUnsafe(Vob* vob, bool recursive = true);
		void removeActiveVobUnsafe(Vob* vob, bool recursive = true);
		bool deleteVobUnsafe(Vob* vob, bool recursive = true);


		Vob* addVobUnsafe(std::unique_ptr<Vob> vob, bool setActive = true);

		void calcSceneBoundingBoxUnsafe();

		/**
		 * Deletes all nodes except the root node.
		 */
		void clearUnsafe();

		void collectRenderCommands(RenderCommandQueue& queue, bool doCulling, ShaderStorageBuffer* boneTrafoBuffer) override;

		Vob* createVobUnsafe(std::vector<MeshBatch>* batches, bool setActive = true);

		void frameUpdate(const RenderContext& constants) override;

		/**
		 * Provides all vobs that are currently active.
		 */
		const VobRange& getActiveVobsUnsafe() const;

		/**
		 * Provides all root vobs (vobs having no parent).
		 */
		VobRange& getActiveRootsUnsafe();
		const VobRange& getActiveRootsUnsafe() const;
		void removeActiveRoot(Vob* vob);

		/**
		 * Provides all vobs that are currently active.
		 */
		const FrameUpdateableRange& getActiveFrameUpdateables() const;

		/**
		 * Provides all probe vobs that are currently active.
		 */
		const ProbeRange& getActiveProbeVobsUnsafe() const;

		const AABB& getSceneBoundingBox() const;

		/**
		 * Provides all vobs of this scene.
		 */
		const VobStore& getVobsUnsafe() const;
		VobStore& getVobsUnsafe();

		bool hasChangedUnsafe() const;

		void setHasChangedUnsafe(bool changed);

		
		void updateWorldTrafoHierarchyUnsafe(bool resetPrevWorldTrafo);

	private:
		VobRange mActiveRoots;
		VobRange mActiveVobsFlat;
		FrameUpdateableRange mActiveUpdateables;
		ProbeRange mActiveProbeVobs;
		std::unordered_set<std::unique_ptr<Vob>> mVobStore;
		mutable std::recursive_mutex mMutex;
		AABB mBoundingBox;
		bool mHasChanged;
	};
}