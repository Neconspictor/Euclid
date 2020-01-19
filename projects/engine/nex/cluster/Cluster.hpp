#pragma once

#include <nex/gui/MenuWindow.hpp>
#include <nex/camera/Camera.hpp>
#include <nex/scene/Scene.hpp>
#include <nex/light/Light.hpp>
#include <interface/cluster_interface.h>

namespace nex
{
	class SimpleColorMaterial;
	class ShaderStorageBuffer;
	class Window;
	class Renderer;

	class ClusterGenerator {
	public:

		ClusterGenerator();
		virtual ~ClusterGenerator();

		void generateClusters(ShaderStorageBuffer* output,
			const glm::uvec3& clusterSize,
			const cluster::Constants& constants);

		/**
		 * Generates a list of active clusters (with duplicates). To get a duplicate free version call cleanActiveClusterList after this function call.
		 * @param clusterSize : The size of the cluster in all three dimensions.
		 * @param depth : Used for determining active clusters
		 * @param zNearDistance : The distance of the near plane. Note: Must be positive!
		 * @param zFarDistance : The distance to the far plane. Note: Must be positive!
		 * @param clusterListOutput :	a buffer containing an array of unsigned int, size will be a flat version of clusterSize. The buffer might be resized 
		 *								if it is not big enough.
		 */
		void collectActiveClusterList(const glm::uvec3& clusterSize, Texture* depth, float zNearDistance,
			float zFarDistance, const glm::mat4& projMatrix, ShaderStorageBuffer* clusterListOutput);

		/**
		 * @param clusterLisInput: An array of unsigned int, size will be a flat version of clusterSize.
		 * @param cleanedClusterListOutput: First element: size of cluster list (unsigned int); Remainder: An array of unsigned int,  
		 */
		void cleanActiveClusterList(const glm::uvec3& clusterSize, ShaderStorageBuffer* clusterListInput, ShaderStorageBuffer* cleanedClusterListOutput);

	private:
		class GenerateClusterPass;
		class CollectActiveClustersPass;
		class CleanActiveClusterListPass;

		std::unique_ptr<GenerateClusterPass> mGenerateClusterPass;
		std::unique_ptr<CollectActiveClustersPass> mCollectActiveClustersPass;
		std::unique_ptr<CleanActiveClusterListPass> mCleanActiveClusterListPass;

	};

	class EnvLightCuller
	{
	public:

		using GlobalLightIndexCount = glm::uint;

		using GlobalLightIndexListElement = glm::uint;

		/**
		 * Note: Has to be in sync with shader!
		 */
		static const unsigned MAX_VISIBLES_LIGHTS_PER_CLUSTER = 8;


		/**
		 * @param xSize : Cluster size in x direction
		 * @param ySize : Cluster size in y direction
		 * @param zLocalSize: cluster size in z direction for one batch
		 * @param zBatchSize: amount of batches in z direction.
		 * @param maxVisibleLights: The maximum of environment lights that can influence pixels of a certain cluster.
		 *
		 * @throws invalid_argument : if xSize * ySize * zLocalSize > getMaxLocalWorkgroupSize()
		 * Note: cluster size inf z direction is zLocalSize * zBatchSize
		 */
		EnvLightCuller(unsigned xSize, unsigned ySize, unsigned zLocalSize, unsigned zBatchSize, unsigned maxVisibleLights = 8);

		EnvLightCuller (EnvLightCuller&& o) = default;
		EnvLightCuller& operator=(EnvLightCuller&& o) = default;

		virtual ~EnvLightCuller();

		/**
		 *  Returns a buffer containing an array of GlobalLightIndexCount
		 */
		ShaderStorageBuffer* getGlobalLightIndexCount();

		/**
		 * Returns a buffer containing an array of GlobalLightIndexListElement
		 * of size MAX_VISIBLES_LIGHTS * cluster size.
		 */
		ShaderStorageBuffer* getGlobalLightIndexList();

		/**
		 * Returns a buffer containing an array of ClusterLightGrid
		 * of size 'cluster size'
		 */
		ShaderStorageBuffer* getLightGrids();

		unsigned getMaxVisibleLightsSize() const;

		/**
		 * Provides the multiplication of the maximum local workgroup size supported.
		 * If cluster is a variable of type glm::vec3, than the flattened size is 
		 * cluster.x * cluster.y * cluster.z .
		 */
		static constexpr unsigned getMaxLocalWorkgroupSize();

		/**
		 * Performas light culling of environment lights using the GPU.
		 * @param clusters : A buffer containing an array of AABB
		 * @param envLights : A buffer containing an array of EnvironmentLightGPU
		 */
		void cullLights(const glm::mat4& viewMatrix,
			ShaderStorageBuffer* clusters,
			ShaderStorageBuffer* envLights);

		bool isOutOfDate(unsigned xSize, unsigned ySize, unsigned zLocalSize, unsigned zBatchSize, unsigned maxVisibleLights) const;

	private:

		class CullPass;

		std::unique_ptr<ShaderStorageBuffer> mGlobalLightIndexCountBuffer;
		std::unique_ptr<ShaderStorageBuffer> mGlobalLightIndexListBuffer;
		std::unique_ptr<ShaderStorageBuffer> mLightGridsBuffer;
		std::unique_ptr<CullPass> mCullPass;

		unsigned mXSize; 
		unsigned mYSize; 
		unsigned mZLocalSize; 
		unsigned mZBatchSize; 
		unsigned mMaxVisibleLights;
	};



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

		ProbeCluster();
		virtual ~ProbeCluster();

		nex::PerspectiveCamera& getCamera();

		void generate(const Frustum& frustum, Scene* scene);

		void generateClusterElement(const ClusterElement& elem, Scene* scene);

		void generateCluster(const glm::uvec4& clusterSize, Texture* depth, const Camera* camera, Scene* scene);

		void generateClusterCpuTest(const glm::uvec4& clusterSize, Scene* scene);

		void generateClusterGpu(const glm::uvec4& clusterSize, Texture* depth, const Camera* camera, Scene* scene);

		EnvLightCuller* getEnvLightCuller();

		ShaderStorageBuffer* getClusterAABBBuffer();
		ShaderStorageBuffer* getActiveClusterList();

	private:
		class CullLightsPass;

		nex::AABB main(const glm::uvec3& gl_WorkGroupID,
			const glm::uvec3& gl_NumWorkGroups,
			const glm::vec2& zNearFar, 
			const glm::uvec4& tileSizes,
			const glm::mat4& invProj, 
			const glm::vec2& screenDimension);

		glm::vec3 lineIntersectionToZPlane(const glm::vec3& firstPoint, glm::vec3& secondPoint, float zValueViewSpace);
		glm::vec4 screen2View(const glm::vec4& screen, const glm::mat4& invProj, const glm::vec2& screenDimension);
		glm::vec4 clipToView(const glm::vec4& clip, const glm::mat4& invProj);

		nex::PerspectiveCamera mCamera;
		std::unique_ptr<SimpleColorMaterial> mMaterial;
		std::unique_ptr<ShaderStorageBuffer> mClusterAABBBuffer;
		std::unique_ptr<ShaderStorageBuffer> mActiveClusterList;
		std::unique_ptr<ShaderStorageBuffer> mCleanActiveClusterList;

		std::unique_ptr<CullLightsPass> mCullLightsPass;
		EnvLightCuller mEnvLightCuller;
		ClusterGenerator mClusterGenerator;
	};

	class CullEnvironmentLightsCsCpuShader 
	{
	public:

		std::vector<cluster::AABB>& getClusters();
		std::vector<EnvironmentLight>& getEnvironmentLights();
		glm::uint getGlobalLightIndexCount();
		std::vector<glm::uint>& getGlobalLightIndexList();
		std::vector<cluster::LightGrid>& getLightGrids();

		void initInstance(const glm::uvec3& gl_NumWorkGroups, const glm::uvec3& gl_GlobalInvocationID, const glm::uvec3& gl_LocalInvocationID);


		static void test0();

		void main() const;

		void setGlobalIndexCount(glm::uint count);
		void setViewMatrix(const glm::mat4& mat);


	private:


		float sqDistPointAABB(glm::vec3 point, glm::uint clusterID)  const;
		bool testAABBWorld(glm::uint light, glm::uint clusterID) const;
		bool testSphereAABB(glm::uint light, glm::uint clusterID) const;

		static const unsigned MAX_VISIBLES_LIGHTS = 100;
		static const unsigned LOCAL_SIZE_X = 16;
		static const unsigned LOCAL_SIZE_Y = 9;
		static const unsigned LOCAL_SIZE_Z = 4;


		glm::mat4 viewMatrix;
		std::vector<cluster::AABB> clusters;
		std::vector<EnvironmentLight> environmentLights;
		mutable std::vector<glm::uint> globalLightIndexList;
		mutable std::vector<cluster::LightGrid> lightGrids;
		mutable glm::uint globalIndexCount;
		mutable std::vector<EnvironmentLight> sharedLights;


		static constexpr glm::uvec3 gl_WorkGroupSize = glm::uvec3(LOCAL_SIZE_X, LOCAL_SIZE_Y, LOCAL_SIZE_Z);
		glm::uvec3 gl_NumWorkGroups;
		glm::uvec3 gl_GlobalInvocationID;
		glm::uvec3 gl_LocalInvocationID;
		glm::uint gl_LocalInvocationIndex;
	};

	namespace gui {

		class ProbeClusterView : public nex::gui::MenuWindow {
		public:
			ProbeClusterView(std::string title,
				MainMenuBar* menuBar,
				Menu* menu, ProbeCluster* cluster,
				PerspectiveCamera* activeCamera,
				nex::Window* window,
				nex::Renderer* renderer,
				Scene* scene);

			void setDepth(Texture* depth);

			void drawSelf() override;
		
		private:
			ProbeCluster* mCluster;
			Scene* mScene;
			PerspectiveCamera* mActiveCamera;
			ProbeCluster::ClusterElement mClusterElement;
			glm::uvec4 mClusterSize;
			nex::Window* mWindow;
			bool mShowErrorMessage;
			nex::gui::Button mGenerateButton;
			Texture* mDepth;
			nex::Renderer* mRenderer;
		};
	}
}