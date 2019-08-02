#pragma once

#include <nex/gui/MenuWindow.hpp>
#include <nex/camera/Camera.hpp>
#include <nex/Scene.hpp>

namespace nex
{

	class Technique;
	class SimpleColorPass;
	class Material;

	class ProbeCluster
	{
	public:

		/**
		 * Defines an element of frustum cluster.
		 */
		struct ClusterElement {
			float xOffset = 0.0f; // Relative: range[0,1]
			float yOffset = 0.0f; // Relative: range[0,1]
			float zOffset = 0.0f; // Relative: range[0,1]
			float width = 1.0f;  // Relative:  range[0,1]; xOffset +  width  <= 1
			float height = 1.0f; // Relative:  range[0,1]; yOffset +  height <= 1
			float depth = 1.0f; // Relative:   range[0,1]; zOffset +  depth  <= 1
		};

		struct ClusterSize {
			size_t xSize = 1;
			size_t ySize = 1;
			size_t zSize = 1;
		};

		ProbeCluster(Scene* scene);
		virtual ~ProbeCluster();

		nex::PerspectiveCamera& getCamera();

		void generate(const Frustum& frustum);

		void generateClusterElement(const ClusterElement& elem);

		void generateCluster(const ClusterSize& clusterSize);

	private:

		nex::PerspectiveCamera mCamera;
		Scene* mScene;
		std::unique_ptr<SimpleColorPass> mPass;
		std::unique_ptr<Technique> mTechnique;
		std::unique_ptr<Material> mMaterial;
	};

	namespace gui {

		class ProbeClusterView : public nex::gui::MenuWindow {
		public:
			ProbeClusterView(std::string title,
				MainMenuBar* menuBar,
				Menu* menu, ProbeCluster* cluster,
				PerspectiveCamera* activeCamera);

			void drawSelf() override;
		
		private:
			ProbeCluster* mCluster;
			PerspectiveCamera* mActiveCamera;
			ProbeCluster::ClusterElement mClusterElement;
			ProbeCluster::ClusterSize mClusterSize;
		};
	}
}