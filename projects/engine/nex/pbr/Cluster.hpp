#pragma once

#include <nex/gui/MenuWindow.hpp>
#include <nex/camera/Camera.hpp>
#include <nex/Scene.hpp>
#include <nex/light/Light.hpp>

namespace nex
{
	class SimpleColorMaterial;
	class ShaderStorageBuffer;
	class Window;


	struct ClusterAABB
	{
		glm::vec4 minView;
		glm::vec4 maxView;
		glm::vec4 minWorld;
		glm::vec4 maxWorld;
	};

	struct ClusterLightGrid
	{
		glm::uint offset;
		glm::uint count;
	};

	class ClusterGenerator {
	public:

		struct Constants {
			glm::mat4 invProj;
			glm::mat4 invView;
			glm::vec4 zNearFar; // near and far plane in view space; z and w component unused
		};

		ClusterGenerator();
		virtual ~ClusterGenerator();

		void generateClusters(ShaderStorageBuffer* output,
			const glm::uvec3& clusterSize,
			const Constants& constants);

	private:
		class GenerateClusterPass;

		std::unique_ptr<GenerateClusterPass> mGenerateClusterPass;

	};

	class EnvLightCuller
	{
	public:

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
		 * Returns a buffer containing an array of ClusterLightGrid
		 * of size 'cluster size'
		 */
		ShaderStorageBuffer* getLightGrids();

		/**
		 * Performas light culling of environment lights using the GPU.
		 * @param clusters : A buffer containing an array of AABB
		 * @param envLights : A buffer containing an array of EnvironmentLightGPU
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

		ProbeCluster(Scene* scene);
		virtual ~ProbeCluster();

		nex::PerspectiveCamera& getCamera();

		void generate(const Frustum& frustum);

		void generateClusterElement(const ClusterElement& elem);

		void generateCluster(const glm::uvec3& clusterSize, unsigned width, unsigned height);

		void generateClusterCpuTest(const glm::uvec3& clusterSize);

		void generateClusterGpu(const glm::uvec3& clusterSize, unsigned width, unsigned height);

		void collectActiveClusterGpuTest(const glm::uvec3& clusterSize,
			float zNearDistance, 
			float zFarDistance, 
			unsigned width, 
			unsigned height);

		void cleanActiveClusterListGpuTest(const glm::uvec3& clusterSize, ShaderStorageBuffer* activeClusters);

	private:
		class CollectClustersPass;
		class CleanClusterListPass;
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
		Scene* mScene;
		std::unique_ptr<SimpleColorMaterial> mMaterial;
		std::unique_ptr<ShaderStorageBuffer> mClusterAABBBuffer;

		std::unique_ptr<CollectClustersPass> mCollectClustersPass;
		std::unique_ptr<CleanClusterListPass> mCleanClusterListPass;
		std::unique_ptr<CullLightsPass> mCullLightsPass;
		ClusterGenerator mClusterGenerator;
	};

	class CullEnvironmentLightsCsCpuShader 
	{
	public:

		std::vector<ClusterAABB>& getClusters();
		std::vector<EnvironmentLight>& getEnvironmentLights();
		glm::uint getGlobalLightIndexCount();
		std::vector<glm::uint>& getGlobalLightIndexList();
		std::vector<ClusterLightGrid>& getLightGrids();

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
		std::vector<ClusterAABB> clusters;
		std::vector<EnvironmentLight> environmentLights;
		mutable std::vector<glm::uint> globalLightIndexList;
		mutable std::vector<ClusterLightGrid> lightGrids;
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
			glm::uvec3 mClusterSize;
			nex::Window* mWindow;
		};
	}
}