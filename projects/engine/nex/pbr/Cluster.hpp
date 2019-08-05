#pragma once

#include <nex/gui/MenuWindow.hpp>
#include <nex/camera/Camera.hpp>
#include <nex/Scene.hpp>

namespace nex
{

	class Technique;
	class SimpleColorPass;
	class Material;
	class ShaderStorageBuffer;

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

		void generateClusterCpuTest(const ClusterSize& clusterSize);

		void generateClusterGpu(const ClusterSize& clusterSize);

		void collectActiveClusterGpuTest(const ClusterSize& clusterSize, float zNearDistance, float zFarDistance);

	private:

		class GenerateClusterPass;
		class CollectClustersPass;

		nex::AABB main(const glm::vec3& gl_WorkGroupID,
			const glm::vec3& gl_NumWorkGroups, 
			const glm::vec2& zNearFar, 
			const glm::uvec4& tileSizes,
			const glm::mat4& invProj, 
			const glm::vec2& screenDimension);

		glm::vec3 lineIntersectionToZPlane(const glm::vec3& firstPoint, glm::vec3& secondPoint, float zValueViewSpace);
		glm::vec4 screen2View(const glm::vec4& screen, const glm::mat4& invProj, const glm::vec2& screenDimension);
		glm::vec4 clipToView(const glm::vec4& clip, const glm::mat4& invProj);

		nex::PerspectiveCamera mCamera;
		Scene* mScene;
		std::unique_ptr<SimpleColorPass> mPass;
		std::unique_ptr<Technique> mTechnique;
		std::unique_ptr<Material> mMaterial;
		std::unique_ptr<GenerateClusterPass> mGenerateClusterShader;
		std::unique_ptr<ShaderStorageBuffer> mConstantsBuffer;
		std::unique_ptr<ShaderStorageBuffer> mClusterAABBBuffer;

		std::unique_ptr<CollectClustersPass> mCollectClustersPass;
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