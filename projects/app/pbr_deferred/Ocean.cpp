#include "Ocean.hpp"
#include <random>
#include <complex>
#include <nex/util/Math.hpp>
#include "nex/mesh/VertexBuffer.hpp"
#include "nex/mesh/IndexBuffer.hpp"
#include "nex/mesh/VertexLayout.hpp"
#include "nex/mesh/VertexArray.hpp"
#include "nex/mesh/SubMesh.hpp"
#include "nex/camera/Camera.hpp"
#include "nex/renderer/RenderBackend.hpp"
#include "nex/gui/Controller.hpp"
#include <glm/gtc/matrix_transform.inl>

nex::Ocean::Ocean(const glm::uvec2& pointCount,
	const glm::vec2& maxWaveLength,
	const glm::vec2& dimension,
	float spectrumScale,
	const glm::vec2& windDirection,
	float windSpeed,
	float periodTime) :
	mUniquePointCount(pointCount - glm::uvec2(1, 1)),
	mTildePointCount(pointCount),
	mWaveLength(maxWaveLength),
	mDimension(dimension),
	mSpectrumScale(spectrumScale),
	mWindDirection(glm::normalize(windDirection)),
	mWindSpeed(windSpeed),
	mPeriodTime(periodTime),
	mSimpleShadedPass(std::make_unique<SimpleShadedPass>()),
	mWireframe(true)
{
	assert(pointCount.x >= 2);
	assert(pointCount.y >= 2);
	assert(dimension.x > 0.0f);
	assert(dimension.y > 0.0f);
	assert(spectrumScale > 0.0f);
	assert(length(windDirection) > 0.0f);
	assert(periodTime > 0.0f);

	const auto vertexCount = mTildePointCount.x * mTildePointCount.y;
	const auto quadCount = (mTildePointCount.x - 1) * (mTildePointCount.y - 1);
	const float twoPi = 2.0f * util::PI;
	const float pi = util::PI;
	const unsigned indicesPerQuad = 6; // two triangles per quad

	mVerticesCompute.resize(vertexCount);
	mVerticesRender.resize(vertexCount);
	mIndices.resize(quadCount * indicesPerQuad); // two triangles per quad

	for (int z = 0; z < mTildePointCount.y; ++z)
	{
		for (int x = 0; x < mTildePointCount.x; ++x)
		{
			unsigned index = z * mTildePointCount.x + x;

			const float kx = (twoPi * x - pi * mUniquePointCount.x) / mWaveLength.x;
			const float kz = (twoPi * z - pi * mUniquePointCount.y) / mWaveLength.y;
			const auto wave = glm::vec2(kx, kz);

			mVerticesCompute[index].height0 = heightZero(wave);
			mVerticesCompute[index].height0NegativeWaveConjugate = heightZero(-wave).conjugate();
			mVerticesCompute[index].originalPosition = glm::vec3(
				(x - mUniquePointCount.x / 2.0f) * mWaveLength.x / mUniquePointCount.x,
				0.0f,
				getZValue((z - mUniquePointCount.y / 2.0f) * mWaveLength.y / mUniquePointCount.y)
			);

			mVerticesRender[index].position = mVerticesCompute[index].originalPosition;
			mVerticesRender[index].normal = glm::vec3(0.0f, 1.0f, 0.0f);
		}
	}


	const glm::uvec2 quadNumber = mUniquePointCount;

	for (int x = 0; x < quadNumber.x; ++x)
	{
		for (int z = 0; z < quadNumber.y; ++z)
		{
			const unsigned indexStart = indicesPerQuad * (x * quadNumber.y + z);

			const unsigned vertexBottomLeft = x * mTildePointCount.y + z;

			// first triangle
			mIndices[indexStart] = vertexBottomLeft;
			mIndices[indexStart + 1] = vertexBottomLeft + 1; // one column to the right
			mIndices[indexStart + 2] = vertexBottomLeft + mTildePointCount.y + 1; // one row up and one column to the right

			// second triangle
			mIndices[indexStart + 3] = vertexBottomLeft;
			mIndices[indexStart + 4] = vertexBottomLeft + mTildePointCount.y + 1;
			mIndices[indexStart + 5] = vertexBottomLeft + mTildePointCount.y; // one row up
		}
	}


	simulate(10.0f);

	VertexBuffer vertexBuffer;
	vertexBuffer.bind();
	vertexBuffer.fill(mVerticesRender.data(), vertexCount * sizeof(VertexRender));
	IndexBuffer indexBuffer(mIndices.data(), mIndices.size(), IndexElementType::BIT_32);
	indexBuffer.bind();

	VertexLayout layout;
	layout.push<glm::vec3>(1); // position
	layout.push<glm::vec3>(1); // normal

	VertexArray vertexArray;
	vertexArray.bind();
	vertexArray.useBuffer(vertexBuffer, layout);

	vertexArray.unbind();
	indexBuffer.unbind();

	mMesh = std::make_unique<Mesh>(std::move(vertexArray), std::move(vertexBuffer), std::move(indexBuffer));
}

nex::Ocean::~Ocean() = default;

nex::Ocean::ResultData nex::Ocean::simulatePoint(const glm::vec2& locationXZ, float t) const
{
	const float twoPi = 2.0f * util::PI;
	const float pi = util::PI;

	Complex height( 0.0, 0.0);
	glm::vec2 gradient(0.0f);
	glm::vec2 displacement(0.0f);


	for (int z = 0; z < mUniquePointCount.y; ++z)
	{
		for (int x = 0; x < mUniquePointCount.x; ++x)
		{
			const float kx = (twoPi * x - pi * mUniquePointCount.x) / mWaveLength.x; //(twoPi * nDash - pi * N) / Lx;   (nex::util::PI * ((2.0f * nDash) - N)) / Lx;
			//const float kx2 = (twoPi * nDash - pi * N) / Lx;
			const float kz = (twoPi * z - pi * mUniquePointCount.y) / mWaveLength.y; //(twoPi * mDash - pi * M) / Lz;  nex::util::PI * (2.0f * mDash - M) / Lz;

			const auto wave = glm::vec2(kx, kz);
			const auto angle = dot(wave, locationXZ);
			const auto euler = Complex::euler(angle);

			const auto h = this->height(x, z, t) * euler;

			// real component is the amplitude of the sinusoid -> sum up amplitudes to get the height
			height += h;


			// Note: We only consider the real part of the gradient, as the imaginary part isn't needed
			// The gradient is a two dimensional complex vector: i*(kx, kz) * h = <(-h.im * kx) +  i*(h.re * kx), (-h.im * kz) + i*(h.re * kz)>
			gradient += glm::vec2(-h.im * kx, -h.im * kz);

			//if (k_length < 0.000001) continue;
			//D = D + glm::vec2(kx / k_length * htilde_c.imag(), kz / k_length * htilde_c.imag());
			const auto length = glm::length(wave);
			if (length >= 0.00001)
			{
				displacement += glm::vec2(kx / length * h.im, kz / length * h.im);
			}
		}
	}

	// The normal vector can be calculated from the real part of the gradient
	glm::vec3 normal = normalize(glm::vec3(-gradient.x, 1.0, -gradient.y));

	return {height, displacement, normal};
}

float nex::Ocean::dispersion(const glm::vec2& wave) const
{
	static const float w0 = 2.0f * util::PI / mPeriodTime;
	return std::floor(std::sqrt(GRAVITY * length(wave)) / w0) * w0;
}

void nex::Ocean::draw(Camera* camera, const glm::vec3& lightDir)
{
	mSimpleShadedPass->bind();

	glm::mat4 model = translate(glm::mat4(), glm::vec3(0,2, -1));

	mSimpleShadedPass->setUniforms(camera, model, lightDir);

	//mMesh->bind();
	mMesh->getVertexArray()->bind();
	mMesh->getIndexBuffer()->bind();
	RenderState state;
	state.doBlend = false;
	state.doDepthTest = true;
	state.doDepthWrite = true;
	state.doCullFaces = true;

	if (mWireframe)
	{
		state.fillMode = FillMode::LINE;
	}
	else
	{
		state.fillMode = FillMode::FILL;
	}


	state.depthCompare = CompareFunction::LESS;

	// Only draw the first triangle
	RenderBackend::get()->drawWithIndices(state, Topology::TRIANGLES, mMesh->getIndexBuffer()->getCount(), mMesh->getIndexBuffer()->getType());
}

float uniformRandomVariable() {
	return (float)rand() / RAND_MAX;
}

nex::Complex gaussianRandomVariable() {
	float x1, x2, w;
	do {
		x1 = 2.f * uniformRandomVariable() - 1.f;
		x2 = 2.f * uniformRandomVariable() - 1.f;
		w = x1 * x1 + x2 * x2;
	} while (w >= 1.f);
	w = sqrt((-2.f * log(w)) / w);
	//return nex::Complex(1.0, 1.0f);
	//return nex::Complex(1.0 - abs(x1 * w)/10000.0f, 1.0 - abs(x2 * w)/10000.0);
	return nex::Complex(x1 * w, x2 * w);
}

nex::Complex nex::Ocean::heightZero(const glm::vec2& wave) const
{
	//static const auto inverseRootTwo = 1 / std::sqrt(2.0);
	const Complex random = gaussianRandomVariable();//(gaussianRandomVariable(), gaussianRandomVariable());

	return random * std::sqrt(philipsSpectrum(wave) / 2.0f);
}

nex::Complex nex::Ocean::height(int x, int z, float time) const
{
	const float twoPi = 2.0f * util::PI;
	const float pi = util::PI;
	const float kx = (twoPi * x - pi * mUniquePointCount.x) / mWaveLength.x;
	const float kz = (twoPi * z - pi * mUniquePointCount.y) / mWaveLength.y;

	const auto wave = glm::vec2(kx, kz);
	const auto w = dispersion(wave);

	const auto omegat = w * time;
	auto cos_ = cos(omegat);
	auto sin_ = sin(omegat);
	Complex c0(cos_, sin_);
	Complex c1(cos_, -sin_);


	const auto index = x * mTildePointCount.y + z;
	const auto& vertex = mVerticesCompute[index];

	return vertex.height0 * c0//* Complex::euler(w * time)
		+ vertex.height0NegativeWaveConjugate * c1; //* Complex::euler(-w * time);
}

float nex::Ocean::philipsSpectrum(const glm::vec2& wave) const
{
	const auto L = mWindSpeed * mWindSpeed / GRAVITY;
	const auto L2 = L * L;
	float damping = 0.001;
	float l2 = L2 * damping * damping;

	// length of the wave vector
	const auto k = length(wave);//2 * util::PI / lambda;

	if (k < 0.0001) return 0.0f;

	const auto kLSquare = k * L * k*L;
	const auto kFour = k * k*k*k;
	const auto absAngleWaveWind = dot(normalize(wave), normalize(mWindDirection));

	return mSpectrumScale * std::exp(-1.0f / kLSquare) / kFour * (absAngleWaveWind * absAngleWaveWind) * std::exp(-k * k*l2);
}

bool* nex::Ocean::getWireframeState()
{
	return &mWireframe;
}

void nex::Ocean::simulate(float t)
{
	const float displacementDirectionScale = -0.9;

	for (int z = 0; z < mUniquePointCount.y; z++) {
		for (int x = 0; x < mUniquePointCount.x; x++) {
			auto index = z * mTildePointCount.x + x;


			auto& vertex = mVerticesRender[index];
			const auto& computeData = mVerticesCompute[index];

			glm::vec2 locationXZ (vertex.position.x, vertex.position.z);

			//auto height = computeHeight(x, t);

			const auto data = simulatePoint(locationXZ, t);
			const auto height = data.height.re;

			vertex.position.x = computeData.originalPosition.x + displacementDirectionScale * data.displacement.x;
			vertex.position.y = height;
			vertex.position.z = computeData.originalPosition.z + displacementDirectionScale * data.displacement.y;

			vertex.normal = data.normal;

			// first point has to be replicated three times
			if (x == 0 && z == 0) {
				const auto replicateIndex = mVerticesRender.size() - 1;
				auto& sample = mVerticesRender[replicateIndex];
				const auto& sampleComputeData = mVerticesCompute[replicateIndex];
				sample.position.x = sampleComputeData.originalPosition.x + displacementDirectionScale * data.displacement.x;
				sample.position.y = vertex.position.y;
				sample.position.z = sampleComputeData.originalPosition.z + displacementDirectionScale * data.displacement.y;
				sample.normal = vertex.normal;
			}
			if (x == 0) {
				const auto replicateIndex = index + mUniquePointCount.x;
				auto& sample = mVerticesRender[replicateIndex];
				const auto& sampleComputeData = mVerticesCompute[replicateIndex];
				sample.position.x = sampleComputeData.originalPosition.x + displacementDirectionScale * data.displacement.x;
				sample.position.y = vertex.position.y;
				sample.position.z = sampleComputeData.originalPosition.z + displacementDirectionScale * data.displacement.y;
				sample.normal = vertex.normal;
			}
			if (z == 0) {
				const auto replicateIndex = index + mUniquePointCount.y * mTildePointCount.x;
				auto& sample = mVerticesRender[replicateIndex];
				const auto& sampleComputeData = mVerticesCompute[replicateIndex];
				sample.position.x = sampleComputeData.originalPosition.x + displacementDirectionScale * data.displacement.x;
				sample.position.y = vertex.position.y;
				sample.position.z = sampleComputeData.originalPosition.z + displacementDirectionScale * data.displacement.y;
				sample.normal = vertex.normal;
			}
		}
	}
}

float nex::Ocean::generateGaussianRand()
{
	static std::random_device device;
	static std::mt19937 engine(device());

	// note: we need mean 0 and standard deviation 1!
	static std::normal_distribution<float> distribution(0, 1);
	return distribution(engine);
}

nex::Ocean::SimpleShadedPass::SimpleShadedPass() : Pass(Shader::create("ocean/simple_shaded_vs.glsl", "ocean/simple_shaded_fs.glsl"))
{
	transform = { mShader->getUniformLocation("transform"), UniformType::MAT4 };
	lightUniform = { mShader->getUniformLocation("lightDirViewSpace"), UniformType::VEC3 };
	normalMatrixUniform = { mShader->getUniformLocation("normalMatrix"), UniformType::MAT3 };
}

void nex::Ocean::SimpleShadedPass::setUniforms(Camera* camera, const glm::mat4& trafo, const glm::vec3& lightDir)
{
	auto projection = camera->getProjectionMatrix();
	auto view = camera->getView();

	auto modelView = view * trafo;

	mShader->setMat3(normalMatrixUniform.location, createNormalMatrix(modelView));
	mShader->setMat4(transform.location, projection * view * trafo);


	glm::vec3 lightDirViewSpace = glm::vec3(view * glm::vec4(lightDir, 0.0));
	mShader->setVec3(lightUniform.location, normalize(lightDirViewSpace));
}

nex::gui::OceanConfig::OceanConfig(Ocean* ocean) : mOcean(ocean)
{
}

void nex::gui::OceanConfig::drawSelf()
{
	ImGui::Checkbox("Ocean Wireframe", mOcean->getWireframeState());
}