#pragma once

#include <memory>
#include <nex/GI/PbrProbe.hpp>
#include <glm/glm.hpp>
#include <nex/renderer/RenderCommandQueue.hpp>

namespace nex
{
	class Scene;
	class Vob;
	class RenderCommandQueue;
	class CubeMap;
	class TransformShader;
	class PbrDeferred;
	class PbrForward;
	class Renderer;
	class ProbeVob;
	struct DirLight;
	class MeshGroup;


	class ProbeBaker {
	public:

		ProbeBaker();

		~ProbeBaker();

		void bakeProbes(Scene& scene, const DirLight& light, PbrProbeFactory& factory, Renderer* renderer);
		void bakeProbe(ProbeVob*, const Scene& scene, const DirLight& light, PbrProbeFactory& factory, Renderer* renderer);

	private:

		class ProbeBakePass;

		void collectBakeCommands(nex::RenderCommandQueue& queue, const Scene& scene, bool doCulling);

		std::shared_ptr<nex::CubeMap> renderToCubeMap(const nex::RenderCommandQueue& queue,
			Renderer* renderer,
			CubeRenderTarget& renderTarget,
			nex::Camera& camera,
			const glm::vec3& worldPosition,
			const DirLight& light);

		std::shared_ptr<nex::CubeMap> renderToDepthCubeMap(const nex::RenderCommandQueue& queue,
			Renderer* renderer,
			CubeRenderTarget& renderTarget,
			nex::Camera& camera,
			const glm::vec3& worldPosition,
			const DirLight& light);

		std::unique_ptr<ProbeBakePass> mProbeBakePass;
		std::unique_ptr<PbrDeferred> mDeferred;
		std::unique_ptr<PbrForward> mForward;
		std::unique_ptr<TransformShader> mIrradianceDepthPass;
		std::unique_ptr<MeshGroup> mSphere;
	};
}