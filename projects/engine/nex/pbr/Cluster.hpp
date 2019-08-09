#pragma once

#include <nex/gui/MenuWindow.hpp>
#include <nex/camera/Camera.hpp>
#include <nex/Scene.hpp>

namespace nex
{
	class SimpleColorMaterial;
	class ShaderStorageBuffer;
	class Window;

	class EnvLightCuller
	{
	public:
		struct EnvironmentLight
		{
			glm::vec4 position;
			glm::vec4 minWorld;
			glm::vec4 maxWorld;
			float sphereRange;
			glm::uint enabled;
			glm::uint usesBoundingBox; // specifies whether to use AABB or Sphere volume
		};

		struct LightGrid
		{
			glm::uint offset;
			glm::uint count;
		};

		struct AABB
		{
			glm::vec4 minView;
			glm::vec4 maxView;
			glm::vec4 minWorld;
			glm::vec4 maxWorld;
		};

		using GlobalLightIndexCount = glm::uint;

		using GlobalLightIndexListElement = glm::uint;

		/**
		 * Note: Has to be in sync with shader!
		 */
		static const unsigned MAX_VISIBLES_LIGHTS_PER_CLUSTER = 10;


		EnvLightCuller();
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
		 * Returns a buffer containing an array of LightGrid
		 * of size 'cluster size'
		 */
		ShaderStorageBuffer* getLightGrids();

		/**
		 * Performas light culling of environment lights using the GPU.
		 * @param clusters : A buffer containing an array of AABB
		 * @param envLights : A buffer containing an array of EnvironmentLight
		 */
		void cullLights(const glm::mat4& viewMatrix,
			ShaderStorageBuffer* clusters,
			const glm::vec3& clusterSize,
			ShaderStorageBuffer* envLights);

	private:

		class CullPass;

		std::unique_ptr<ShaderStorageBuffer> mGlobalLightIndexCountBuffer;
		std::unique_ptr<ShaderStorageBuffer> mGlobalLightIndexListBuffer;
		std::unique_ptr<ShaderStorageBuffer> mLightGridsBuffer;
		std::unique_ptr<CullPass> mCullPass;
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

		void generateCluster(const ClusterSize& clusterSize, unsigned width, unsigned height);

		void generateClusterCpuTest(const ClusterSize& clusterSize);

		void generateClusterGpu(const ClusterSize& clusterSize, unsigned width, unsigned height);

		void collectActiveClusterGpuTest(const ClusterSize& clusterSize, 
			float zNearDistance, 
			float zFarDistance, 
			unsigned width, 
			unsigned height);

		void cleanActiveClusterListGpuTest(const ClusterSize& clusterSize, ShaderStorageBuffer* activeClusters);

	private:

		class GenerateClusterPass;
		class CollectClustersPass;
		class CleanClusterListPass;
		class CullLightsPass;

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
		std::unique_ptr<SimpleColorMaterial> mMaterial;
		std::unique_ptr<GenerateClusterPass> mGenerateClusterShader;
		std::unique_ptr<ShaderStorageBuffer> mConstantsBuffer;
		std::unique_ptr<ShaderStorageBuffer> mClusterAABBBuffer;

		std::unique_ptr<CollectClustersPass> mCollectClustersPass;
		std::unique_ptr<CleanClusterListPass> mCleanClusterListPass;
		std::unique_ptr<CullLightsPass> mCullLightsPass;
	};

	class CullEnvironmentLightsCsCpuShader 
	{
	public:
		struct EnvironmentLight
		{
			glm::vec4 position;
			glm::vec4 minWorld;
			glm::vec4 maxWorld;
			float sphereRange;
			glm::uint enabled;
			glm::uint usesBoundingBox; // specifies whether to use AABB or Sphere volume
		};

		struct LightGrid
		{
			glm::uint offset;
			glm::uint count;
		};

		struct AABB
		{
			glm::vec4 minView;
			glm::vec4 maxView;
			glm::vec4 minWorld;
			glm::vec4 maxWorld;
		};

		std::vector<AABB>& getClusters();
		std::vector<EnvironmentLight>& getEnvironmentLights();
		glm::uint getGlobalLightIndexCount();
		std::vector<glm::uint>& getGlobalLightIndexList();
		std::vector<LightGrid>& getLightGrids();

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
		std::vector<AABB> clusters;
		std::vector<EnvironmentLight> environmentLights;
		mutable std::vector<glm::uint> globalLightIndexList;
		mutable std::vector<LightGrid> lightGrids;
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
				nex::Window* window);

			void drawSelf() override;
		
		private:
			ProbeCluster* mCluster;
			PerspectiveCamera* mActiveCamera;
			ProbeCluster::ClusterElement mClusterElement;
			ProbeCluster::ClusterSize mClusterSize;
			nex::Window* mWindow;
		};
	}
}