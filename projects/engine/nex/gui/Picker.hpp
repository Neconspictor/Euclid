#pragma once
#include <memory>
#include <nex/util/CallbackContainer.hpp>
#include <nex/gui/Drawable.hpp>


namespace nex
{
	class Ray;
	class Scene;
	class Vob;
	class Mesh;
	class MeshGroup;
	class SimpleColorPass;
	struct AABB;
	class MeshOwningVob;
}

namespace nex::gui
{
	class Picker
	{
	public:

		using PickedChangedCallback = CallbackCollection<void(Vob*)>;

		Picker();
		virtual ~Picker();

		PickedChangedCallback::Handle addPickedChangeCallback(const PickedChangedCallback::Callback& callback);
		void removePickedChangeCallback(const PickedChangedCallback::Handle& handle);

		void deselect(Scene& scene);

		void select(Scene& scene, Vob* vob);

		bool getShowBoundingBox() const;
		void setShowBoundingBox(bool show);

		/**
		 * Traverses a scene and picks a scene node by a screen ray.
		 * If the ray intersects no node nullptr will be returned.
		 */
		Vob* pick(Scene& scene, const Ray& screenRayWorld);

		Vob* getPicked();

		/**
		 * Updates the world transformation matrix of the bounding box if a scene node is currently selected.
		 */
		void updateBoundingBoxTrafo();

	private:

		struct SelectionTest {
			Vob* vob = nullptr;
			float vobDistance = 0.0f;
			float vobRayMinDistance = 0.0f;
			float vobVolume = 0.0f;
		};

		static std::unique_ptr<Mesh> createBoundingBoxMesh();

		static std::unique_ptr<Mesh> createLineMesh();

		std::unique_ptr<MeshGroup> mBoundingBoxMesh;



		//std::unique_ptr<MeshGroup> mLineMesh;
		std::unique_ptr<SimpleColorPass> mSimpleColorPass;

		float calcVolume(const nex::AABB& box);

		int compare(const SelectionTest& a, const SelectionTest& b);
		bool checkIntersection(const Vob* vob, const nex::Ray& ray);

		std::unique_ptr<Vob> mBoundingBoxVob;

		std::unique_ptr<Vob> mProbeInfluenceBoundingBoxVob;
		std::unique_ptr<Vob> mProbeInfluenceSphereVob;

		//SceneNode* mLineNode;
		SelectionTest mSelected;

		PickedChangedCallback mCallbacks;

		bool mUseLocalBoudningBox = true;
		bool mShowBoundingBox = true;
	};


	class Picker_View : public nex::gui::Drawable {
	public:
		Picker_View(Picker* picker);
	protected:

		void drawSelf() override;

		Picker* mPicker;
	};
}