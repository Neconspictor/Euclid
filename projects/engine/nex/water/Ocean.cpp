#include "Ocean.hpp"
#include <random>
#include <complex>
#include "nex/buffer/VertexBuffer.hpp"
#include "nex/buffer/IndexBuffer.hpp"
#include "nex/mesh/VertexLayout.hpp"
#include "nex/mesh/VertexArray.hpp"
#include "nex/mesh/Mesh.hpp"
#include "nex/camera/Camera.hpp"
#include "nex/renderer/RenderBackend.hpp"
#include "nex/gui/Controller.hpp"
#include <glm/gtc/matrix_transform.inl>
#include "nex/texture/Image.hpp"
#include "nex/texture/TextureManager.hpp"
#include <nex/shadow/CascadedShadow.hpp>
#include <nex/drawing/MeshDrawer.hpp>
#include <nex/pbr/GlobalIllumination.hpp>
#include <nex/water/PSSR.hpp>

nex::Iterator2D::Iterator2D(std::vector<nex::Complex>& vec,
	const PrimitiveMode mode,
	const size_t primitiveIndex,
	const size_t elementNumber) : mVec(&vec), mMode(mode), mPrimitiveIndex(primitiveIndex), mCount(elementNumber)
{
	const auto startIndex = getVectorIndex(0);
	const auto endIndex = getVectorIndex(mCount - 1);
	if (!isInRange(startIndex) || !isInRange(endIndex))
	{
		throw std::out_of_range("nex::RandomAccessIterator: Primitive is out of the range of the specified vector!");
	}
}

nex::Complex& nex::Iterator2D::operator[](const size_t index)
{
	return (*mVec)[getVectorIndex(index)];
}

nex::Complex& nex::Iterator2D::operator[](const size_t index) const
{
	return (*mVec)[getVectorIndex(index)];
}

size_t nex::Iterator2D::getVectorIndex(const size_t primitiveIndex) const
{
	if (mMode == PrimitiveMode::ROWS)
	{

		return mPrimitiveIndex * mCount + primitiveIndex;

	}

	// columns
	return primitiveIndex * mCount + mPrimitiveIndex;
}

bool nex::Iterator2D::isInRange(const size_t vectorIndex) const
{
	return vectorIndex < mVec->size();
}

nex::Ocean::Ocean(unsigned N,
	unsigned maxWaveLength,
	float dimension,
	float waterHeight,
	float spectrumScale,
	const glm::vec2& windDirection,
	float windSpeed,
	float periodTime) :
	mN(N),
	mPointCount(N+1),
	mWaveLength(maxWaveLength),
	mDimension(dimension),
	mSpectrumScale(spectrumScale),
	mWindDirection(glm::normalize(windDirection)),
	mWindSpeed(windSpeed),
	mPeriodTime(periodTime),
	mWireframe(false),
	mWaterHeight(waterHeight),
	mPosition(glm::vec3(0.0f))
{
	if (N <= 0) throw std::invalid_argument("N has to be greater than 0");
	if (!nex::isPow2(N)) throw std::invalid_argument("N has to be a power of 2");
	if (dimension <= 0) throw std::invalid_argument("dimension has to be greater than 0");
	if (spectrumScale <= 0) throw std::invalid_argument("spectrumScale has to be greater than 0");
	if (length(windDirection) <= 0) throw std::invalid_argument("length of windDirection has to be greater than 0");
	if (periodTime <= 0) throw std::invalid_argument("periodTime has to be greater than 0");
}

nex::Ocean::~Ocean() = default;

float nex::Ocean::generateGaussianRand()
{
	static std::random_device device;
	static std::mt19937 engine(device());

	// note: we need mean 0 and standard deviation 1!
	static std::normal_distribution<float> distribution(0, 1);
	return distribution(engine);
}

bool* nex::Ocean::getWireframeState()
{
	return &mWireframe;
}

nex::Complex nex::Ocean::heightZero(const glm::vec2& wave) const
{
	//static const auto inverseRootTwo = 1 / std::sqrt(2.0);
	const Complex random(generateGaussianRand(), generateGaussianRand());

	return random * std::sqrt(philipsSpectrum(wave) / 2.0f);
}

float nex::Ocean::philipsSpectrum(const glm::vec2& wave) const
{
	const auto L = mWindSpeed * mWindSpeed / GRAVITY;
	const auto L2 = L * L;
	const float smallestAllowedWaveLength = L * 0.001f;
	const float smallestAllowedWaveLength2 = smallestAllowedWaveLength * smallestAllowedWaveLength;

	// length of the wave vector
	const auto k = length(wave);//2 * util::PI / lambda;

	if (k < 0.0001) return 0.0f;

	const auto kLSquare = k * L * k*L;
	const auto k2 = k * k;
	const auto kFour = k2 * k2;
	const auto absAngleWaveWind = abs(dot(normalize(wave), normalize(mWindDirection)));
	auto windAlignmentFactor = absAngleWaveWind * absAngleWaveWind;
	auto windAlignmentFactor2 = windAlignmentFactor * windAlignmentFactor;
	auto windAlignmentFactor4 = windAlignmentFactor2 * windAlignmentFactor2;
	auto windAlignmentFactor8 = windAlignmentFactor4 * windAlignmentFactor4;

	const float smallWaveSuppression = std::exp(-k2 * smallestAllowedWaveLength2);

	return mSpectrumScale * std::exp(-1.0f / kLSquare) / kFour * windAlignmentFactor * smallWaveSuppression;
}

void nex::Ocean::updateAnimationTime(float t)
{
	mAnimationTime = t;
}

float nex::Ocean::getTileSize() const
{
	return mWaveLength;
}

glm::mat4 nex::Ocean::getModelMatrix() const
{
	glm::mat4 model;
	//model = translate(model, glm::vec3(-1, 0, -1) * mDimension * 4.0f);
	
	model = translate(model, mPosition);
	model = translate(model, glm::vec3(0, mWaterHeight, 0));
	model = scale(model, glm::vec3(1 / (float)mWaveLength) * mDimension);
	return 	model;
}

float nex::Ocean::getWaterHeight() const
{
	return mWaterHeight;
}

void nex::Ocean::setWaterHeight(float height)
{
	mWaterHeight = height;
}

const glm::vec3& nex::Ocean::getPosition() const
{
	return mPosition;
}

void nex::Ocean::setPosition(const glm::vec3& position)
{
	mPosition = position;
}


nex::OceanCpu::OceanCpu(unsigned N, unsigned maxWaveLength, float dimension, float waterHeight, float spectrumScale,
	const glm::vec2& windDirection, float windSpeed, float periodTime) : 
Ocean(N, maxWaveLength, dimension, waterHeight, spectrumScale, windDirection, windSpeed, periodTime),
mSimpleShadedPass(std::make_unique<SimpleShadedPass>())
{
	generateMesh();
}

nex::OceanCpu::~OceanCpu() = default;

void nex::OceanCpu::generateMesh()
{
	const auto vertexCount = mPointCount * mPointCount;
	const auto quadCount = (mPointCount - 1) * (mPointCount - 1);

	const unsigned indicesPerQuad = 6; // two triangles per quad

	mVerticesCompute.resize(vertexCount);
	mVerticesRender.resize(vertexCount);
	mIndices.resize(quadCount * indicesPerQuad); // two triangles per quad

	for (int z = 0; z < mPointCount; ++z)
	{
		for (int x = 0; x < mPointCount; ++x)
		{
			unsigned index = z * mPointCount + x;

			const float kx = (TWO_PI * x - PI * mN) / (float)mWaveLength;
			const float kz = (TWO_PI * z - PI * mN) / (float)mWaveLength;
			const auto wave = glm::vec2(kx, kz);

			mVerticesCompute[index].height0 = heightZero(wave);
			mVerticesCompute[index].height0NegativeWaveConjugate = heightZero(-wave).conjugate();
			mVerticesCompute[index].originalPosition = glm::vec3(
				(x - mPointCount / 2.0f) * mWaveLength / (float)mN,
				0.0f,
				getZValue((z - mPointCount / 2.0f) * mWaveLength / (float)mN)
			);

			mVerticesRender[index].position = mVerticesCompute[index].originalPosition;
			mVerticesRender[index].normal = glm::vec3(0.0f, 1.0f, 0.0f);
		}
	}


	const auto quadNumber = mN;

	for (int x = 0; x < quadNumber; ++x)
	{
		for (int z = 0; z < quadNumber; ++z)
		{
			const unsigned indexStart = indicesPerQuad * (x * quadNumber + z);

			const unsigned vertexBottomLeft = x * mPointCount + z;

			// first triangle
			mIndices[indexStart] = vertexBottomLeft;
			mIndices[indexStart + 1] = vertexBottomLeft + 1; // one column to the right
			mIndices[indexStart + 2] = vertexBottomLeft + mPointCount + 1; // one row up and one column to the right

			// second triangle
			mIndices[indexStart + 3] = vertexBottomLeft;
			mIndices[indexStart + 4] = vertexBottomLeft + mPointCount + 1;
			mIndices[indexStart + 5] = vertexBottomLeft + mPointCount; // one row up
		}
	}

	std::unique_ptr<VertexBuffer> vertexBuffer = std::make_unique<VertexBuffer>();
	vertexBuffer->bind();
	vertexBuffer->resize(vertexCount * sizeof(VertexRender), mVerticesRender.data(), ShaderBuffer::UsageHint::DYNAMIC_DRAW);

	IndexBuffer indexBuffer(IndexElementType::BIT_32, static_cast<unsigned>(mIndices.size()), mIndices.data());
	indexBuffer.bind();

	VertexLayout layout;
	layout.push<glm::vec3>(1, vertexBuffer.get(), false, false, true); // position
	layout.push<glm::vec3>(1, vertexBuffer.get(), false, false, true); // normal

	VertexArray vertexArray;
	vertexArray.bind();
	vertexArray.init(layout);

	vertexArray.unbind();
	indexBuffer.unbind();


	//TODO
	AABB boundingBox;
	//boundingBox.min = glm::vec3(0.0f);
	//boundingBox.max = glm::vec3(0.0f);

	mMesh = std::make_unique<Mesh>();
	mMesh->addVertexDataBuffer(std::move(vertexBuffer));
	mMesh->setIndexBuffer(std::move(indexBuffer));
	mMesh->setBoundingBox(std::move(boundingBox));
	mMesh->setLayout(std::move(layout));
	mMesh->setTopology(Topology::TRIANGLES);
	mMesh->setVertexArray(std::move(vertexArray));
	mMesh->finalize();
}

float nex::OceanCpu::dispersion(const glm::vec2& wave) const
{
	static const float w0 = TWO_PI / mPeriodTime;
	return std::floor(std::sqrt(GRAVITY * length(wave)) / w0) * w0;
}

void nex::OceanCpu::draw(const glm::mat4& projection, const glm::mat4& view, const glm::vec3& lightDir)
{
	mSimpleShadedPass->bind();
	glm::mat4 model;
	model = translate(model, glm::vec3(0, 2, -1));
	model = scale(model, glm::vec3(1 / (float)mWaveLength));

	mSimpleShadedPass->setUniforms(projection, view, model, lightDir);

	//mMesh->bind();
	mMesh->getVertexArray().bind();
	mMesh->getIndexBuffer().bind();
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


	state.depthCompare = CompFunc::LESS;

	auto& buffer = *mMesh->getVertexBuffers()[0];

	buffer.resize(sizeof(VertexRender) * mVerticesRender.size(), mVerticesRender.data(), ShaderBuffer::UsageHint::DYNAMIC_DRAW);

	// Only draw the first triangle
	RenderBackend::get()->drawWithIndices(state, Topology::TRIANGLES, mMesh->getIndexBuffer().getCount(), mMesh->getIndexBuffer().getType());
}

nex::Complex nex::OceanCpu::height(int x, int z, float time) const
{
	const float kx = (TWO_PI * x - PI * mN) / (float)mWaveLength;
	const float kz = (TWO_PI * z - PI * mN) / (float)mWaveLength;

	const auto wave = glm::vec2(kx, kz);
	const auto w = dispersion(wave);

	const auto omegat = w * time;
	auto cos_ = cos(omegat);
	auto sin_ = sin(omegat);
	Complex c0(cos_, sin_);
	Complex c1(-cos_, -sin_);


	const auto index = z * mPointCount + x;
	const auto& vertex = mVerticesCompute[index];

	return vertex.height0 * c0//* Complex::euler(w * time)
		+ vertex.height0NegativeWaveConjugate * c1; //* Complex::euler(-w * time);
}


nex::OceanCpu::SimpleShadedPass::SimpleShadedPass() : Shader(ShaderProgram::create("ocean/simple_shaded_vs.glsl", "ocean/simple_shaded_fs.glsl"))
{
	transform = { mProgram->getUniformLocation("transform"), UniformType::MAT4 };
	lightUniform = { mProgram->getUniformLocation("lightDirViewSpace"), UniformType::VEC3 };
	normalMatrixUniform = { mProgram->getUniformLocation("normalMatrix"), UniformType::MAT3 };
}

void nex::OceanCpu::SimpleShadedPass::setUniforms(const glm::mat4& projection, const glm::mat4& view, 
	const glm::mat4& trafo, const glm::vec3& lightDir)
{
	auto modelView = view * trafo;

	mProgram->setMat3(normalMatrixUniform.location, createNormalMatrix(modelView));
	mProgram->setMat4(transform.location, projection * view * trafo);


	glm::vec3 lightDirViewSpace = glm::vec3(view * glm::vec4(lightDir, 0.0));
	mProgram->setVec3(lightUniform.location, normalize(lightDirViewSpace));
}


nex::OceanCpuDFT::OceanCpuDFT(unsigned N, unsigned maxWaveLength, float dimension, float waterHeight, float spectrumScale,
	const glm::vec2& windDirection, float windSpeed, float periodTime) : 
OceanCpu(N, maxWaveLength, dimension, waterHeight, spectrumScale, windDirection, windSpeed, periodTime)
{
}

nex::OceanCpuDFT::~OceanCpuDFT() = default;

nex::OceanCpu::ResultData nex::OceanCpuDFT::simulatePoint(const glm::vec2& locationXZ, float t) const
{
	Complex height(0.0, 0.0);
	glm::vec2 gradient(0.0f);
	glm::vec2 displacement(0.0f);

	const auto normalization = (double)mWaveLength;


	for (unsigned z = 0; z < mN; ++z)
	{
		for (unsigned x = 0; x < mN; ++x)
		{
			const float kx = (TWO_PI * x - PI * mN) / (float)mWaveLength; //(twoPi * nDash - pi * N) / Lx;   (nex::util::PI * ((2.0f * nDash) - N)) / Lx;
			//const float kx2 = (twoPi * nDash - pi * N) / Lx;
			const float kz = (TWO_PI * z - PI * mN) / (float)mWaveLength; //(twoPi * mDash - pi * M) / Lz;  nex::util::PI * (2.0f * mDash - M) / Lz;

			const auto wave = glm::vec2(kx, kz);
			const auto angle = dot(wave, locationXZ);

			const auto euler = Complex::euler(angle);
			const auto sample = this->height(x, z, t) * euler / normalization;

			// real component is the amplitude of the sinusoid -> sum up amplitudes to get the height
			height += sample;


			// Note: We only consider the real part of the gradient, as the imaginary part isn't needed
			// The gradient is a two dimensional complex vector: i*(kx, kz) * h = <(-h.im * kx) +  i*(h.re * kx), (-h.im * kz) + i*(h.re * kz)>
			gradient += glm::vec2(-sample.im * kx, -sample.im * kz);

			//if (k_length < 0.000001) continue;
			//D = D + glm::vec2(kx / k_length * htilde_c.imag(), kz / k_length * htilde_c.imag());
			const auto length = glm::length(wave);
			if (length >= 0.00001)
			{
				displacement += glm::vec2(kx / length * sample.im, kz / length * sample.im);
			}
		}
	}

	// The normal vector can be calculated from the real part of the gradient
	glm::vec3 normal = normalize(glm::vec3(-gradient.x, 1.0, -gradient.y));

	return { height, displacement, normal };
}

void nex::OceanCpuDFT::simulate(float t)
{
	const float displacementDirectionScale = -1.0;

	for (int z = 0; z < mN; z++) {
		for (int x = 0; x < mN; x++) {

			// Note: we need to use mPointCount and not mN!
			const auto index = z * mPointCount + x;

			auto& vertex = mVerticesRender[index];
			const auto& computeData = mVerticesCompute[index];

			glm::vec2 locationXZ(vertex.position.x, vertex.position.z);

			//auto height = computeHeight(x, t);

			const auto data = simulatePoint(locationXZ, t);
			const auto height = data.height.re;
			const auto displacement = data.displacement;

			vertex.position.x = computeData.originalPosition.x + displacementDirectionScale * displacement.x;
			vertex.position.y = height;
			vertex.position.z = computeData.originalPosition.z + displacementDirectionScale * displacement.y;

			vertex.normal = data.normal;

			// first point has to be replicated three times
			if (x == 0 && z == 0) {
				const auto replicateIndex = mVerticesRender.size() - 1;
				auto& sample = mVerticesRender[replicateIndex];
				const auto& sampleComputeData = mVerticesCompute[replicateIndex];
				sample.position.x = sampleComputeData.originalPosition.x + displacementDirectionScale * displacement.x;
				sample.position.y = vertex.position.y;
				sample.position.z = sampleComputeData.originalPosition.z + displacementDirectionScale * displacement.y;
				sample.normal = vertex.normal;
			}
			if (x == 0) {
				const auto replicateIndex = index + mN;
				auto& sample = mVerticesRender[replicateIndex];
				const auto& sampleComputeData = mVerticesCompute[replicateIndex];
				sample.position.x = sampleComputeData.originalPosition.x + displacementDirectionScale * displacement.x;
				sample.position.y = vertex.position.y;
				sample.position.z = sampleComputeData.originalPosition.z + displacementDirectionScale * displacement.y;
				sample.normal = vertex.normal;
			}
			if (z == 0) {
				const auto replicateIndex = index + mN * mPointCount;
				auto& sample = mVerticesRender[replicateIndex];
				const auto& sampleComputeData = mVerticesCompute[replicateIndex];
				sample.position.x = sampleComputeData.originalPosition.x + displacementDirectionScale * displacement.x;
				sample.position.y = vertex.position.y;
				sample.position.z = sampleComputeData.originalPosition.z + displacementDirectionScale * displacement.y;
				sample.normal = vertex.normal;
			}
		}
	}
}

nex::OceanCpuFFT::OceanCpuFFT(unsigned N,
	unsigned maxWaveLength,
	float dimension,
	float waterHeight,
	float spectrumScale,
	const glm::vec2& windDirection,
	float windSpeed,
	float periodTime) : OceanCpu(N, maxWaveLength, dimension, waterHeight, spectrumScale, windDirection, windSpeed, periodTime),
	mLogN(log2(mN))
{
	// bit reversal precomputation
	mReversed.resize(N);
	for (int i = 0; i < mReversed.size(); ++i) mReversed[i] = reverse(i);

	// prepare twiddle factors
	int pow2 = 1;
	mTwiddle.resize(mLogN);
	for (int i = 0; i < mTwiddle.size(); ++i)
	{
		mTwiddle[i].resize(pow2);
		for (int j = 0; j < pow2; ++j) mTwiddle[i][j] = twiddle(j, pow2 * 2);
		pow2 *= 2;
	}

	mTemp[0].resize(N);
	mTemp[1].resize(N);
	mCurrent = 0;


	mHeights.resize(N * N);
	mSlopeX.resize(N * N);
	mSlopeZ.resize(N * N);
	mHeightDx.resize(N * N);
	mHeightDz.resize(N * N);
}

nex::OceanCpuFFT::~OceanCpuFFT() = default;

void nex::OceanCpuFFT::simulate(float t)
{
	float lambda = -0.8f;

	for (int z = 0; z < mN; z++) {
		//kz = M_PI * (2.0f * m_prime - N) / length;
		const float kz = (TWO_PI * z - PI * mN) / (float)mWaveLength; //(twoPi * mDash - pi * M) / Lz;  nex::util::PI * (2.0f * mDash - M) / Lz;

		for (int x = 0; x < mN; x++) {
			//kx = M_PI * (2 * n_prime - N) / length;
			const float kx = (TWO_PI * x - PI * mN) / (float)mWaveLength; //(twoPi * nDash - pi * N) / Lx;   (nex::util::PI * ((2.0f * nDash) - N)) / Lx;


			float len = sqrt(kx * kx + kz * kz);
			const unsigned index = z * mN + x;

			mHeights[index] = height(x, z, t) * 1.0f;
			// (a + ib) * (c + id) = (ac - bd) + i(ad + bc)
			// (h.re + i*h.im) * (0 + i*kx) = (h.re*0 - h.im * kx) + i(h.re*kx + h.im*0) = (-h.im * kx) + i(h.re*kx)
			mSlopeX[index] = mHeights[index] * Complex(0, kx);
			//h_tilde_slopex[index] = Complex( - h_tilde[index].im *kx, 0);
			//h_tilde_slopez[index] = Complex(-h_tilde[index].im *kz, 0);
			mSlopeZ[index] = mHeights[index] * Complex(0, kz);

			if (len < 0.000001f) {
				mHeightDx[index] = Complex(0.0f, 0.0f);
				mHeightDz[index] = Complex(0.0f, 0.0f);
			}
			else {
				mHeightDx[index] = mHeights[index] * Complex(0, -kx / len);
				mHeightDz[index] = mHeights[index] * Complex(0, -kz / len);
			}
		}
	}


	for (int m_prime = 0; m_prime < mN; m_prime++) {

		Iterator2D h_tildeIt(mHeights, Iterator2D::PrimitiveMode::COLUMNS, m_prime, mN);
		Iterator2D h_tilde_slopexIt(mSlopeX, Iterator2D::PrimitiveMode::COLUMNS, m_prime, mN);
		Iterator2D h_tilde_slopezIt(mSlopeZ, Iterator2D::PrimitiveMode::COLUMNS, m_prime, mN);
		Iterator2D h_tilde_dxIt(mHeightDx, Iterator2D::PrimitiveMode::COLUMNS, m_prime, mN);
		Iterator2D h_tilde_dzIt(mHeightDz, Iterator2D::PrimitiveMode::COLUMNS, m_prime, mN);

		fft(h_tildeIt, h_tildeIt, true);
		fft(h_tilde_slopexIt, h_tilde_slopexIt, true);
		fft(h_tilde_slopezIt, h_tilde_slopezIt, true);
		fft(h_tilde_dxIt, h_tilde_dxIt, true);
		fft(h_tilde_dzIt, h_tilde_dzIt, true);
	}

	//if (skip)return;


	for (int n_prime = 0; n_prime < mN; n_prime++) {

		Iterator2D h_tildeIt(mHeights, Iterator2D::PrimitiveMode::ROWS, n_prime, mN);
		Iterator2D h_tilde_slopexIt(mSlopeX, Iterator2D::PrimitiveMode::ROWS, n_prime, mN);
		Iterator2D h_tilde_slopezIt(mSlopeZ, Iterator2D::PrimitiveMode::ROWS, n_prime, mN);
		Iterator2D h_tilde_dxIt(mHeightDx, Iterator2D::PrimitiveMode::ROWS, n_prime, mN);
		Iterator2D h_tilde_dzIt(mHeightDz, Iterator2D::PrimitiveMode::ROWS, n_prime, mN);

		fft(h_tildeIt, h_tildeIt, false);
		fft(h_tilde_slopexIt, h_tilde_slopexIt, false);
		fft(h_tilde_slopezIt, h_tilde_slopezIt, false);
		fft(h_tilde_dxIt, h_tilde_dxIt, false);
		fft(h_tilde_dzIt, h_tilde_dzIt, false);
	}
	//if (skip)return;

	int sign;
	float signs[] = { 1.0f, -1.0f };
	glm::vec3 n;

	for (int z = 0; z < mN; z++) {
		for (int x = 0; x < mN; x++) {
			const unsigned heightIndex = z * mN + x;     // index into h_tilde..
			const unsigned vertexIndex = z * mPointCount + x;    // index into vertices

			sign = signs[(x + z) & 1];


			const auto normalization = (double)mWaveLength; // (float) 128.0f;

			mHeights[heightIndex] = mHeights[heightIndex] * sign / normalization;

			auto& vertex = mVerticesRender[vertexIndex];
			const auto& computeData = mVerticesCompute[vertexIndex];


			// height
			vertex.position.y = mHeights[heightIndex].re;

			// displacement
			mHeightDx[heightIndex] = mHeightDx[heightIndex] * sign / normalization;
			mHeightDz[heightIndex] = mHeightDz[heightIndex] * sign / normalization;
			vertex.position.x = computeData.originalPosition.x + mHeightDx[heightIndex].re * lambda;
			vertex.position.z = computeData.originalPosition.z + mHeightDz[heightIndex].re * lambda;

			// normal
			mSlopeX[heightIndex] = mSlopeX[heightIndex] * sign / normalization;
			mSlopeZ[heightIndex] = mSlopeZ[heightIndex] * sign / normalization;
			n = normalize(glm::vec3(-mSlopeX[heightIndex].re, 1.0f, -mSlopeZ[heightIndex].re));
			vertex.normal.x = n.x;
			vertex.normal.y = n.y;
			vertex.normal.z = n.z;

			// for tiling
			// first point has to be replicated three times
			if (x == 0 && z == 0) {
				const auto replicateIndex = mVerticesRender.size() - 1;
				auto& sample = mVerticesRender[replicateIndex];
				const auto& sampleComputeData = mVerticesCompute[replicateIndex];
				sample.position.x = sampleComputeData.originalPosition.x + lambda * mHeightDx[heightIndex].re;
				sample.position.y = vertex.position.y;
				sample.position.z = sampleComputeData.originalPosition.z + lambda * mHeightDz[heightIndex].re;
				sample.normal = vertex.normal;
			}
			if (x == 0) {
				const auto replicateIndex = vertexIndex + mN;
				auto& sample = mVerticesRender[replicateIndex];
				const auto& sampleComputeData = mVerticesCompute[replicateIndex];
				sample.position.x = sampleComputeData.originalPosition.x + lambda * mHeightDx[heightIndex].re;
				sample.position.y = vertex.position.y;
				sample.position.z = sampleComputeData.originalPosition.z + lambda * mHeightDz[heightIndex].re;
				sample.normal = vertex.normal;
			}
			if (z == 0) {
				const auto replicateIndex = vertexIndex + mN * mPointCount;
				auto& sample = mVerticesRender[replicateIndex];
				const auto& sampleComputeData = mVerticesCompute[replicateIndex];
				sample.position.x = sampleComputeData.originalPosition.x + lambda * mHeightDx[heightIndex].re;
				sample.position.y = vertex.position.y;
				sample.position.z = sampleComputeData.originalPosition.z + lambda * mHeightDz[heightIndex].re;
				sample.normal = vertex.normal;
			}

		}
	}
}

unsigned nex::OceanCpuFFT::reverse(unsigned i) const
{
	// Find the max number for bit reversing. All bits higher than this number will be zeroed.
	// the max number is determined by 2^bitCount - 1.
	const unsigned maxNumber = (1 << mLogN) - 1;
	// Initialize the reversed number 
	unsigned reversedN = i;

	// Shift n to the right, reversed n to the left, 
	// and give least-significant bit of n to reversed n.
	// Do this process as long as n is greater zero.
	// Technically we have to do this process bitCount times
	// But we can save some loops by ignoring shifts if n is zero and 
	// just left shift reversed n by the remaining bits.
	// Therefore we need the remainingBits variable.
	unsigned remainingBits = mLogN - 1;
	for (i >>= 1; i > 0; i >>= 1)
	{
		reversedN <<= 1;
		reversedN |= i & 1;
		remainingBits--;
	}

	// left shift reversed n by the remaining bits.
	reversedN <<= remainingBits;

	// Clear all bits more significant than the max number 
	reversedN &= maxNumber;

	return reversedN;
}

nex::Complex nex::OceanCpuFFT::twiddle(unsigned x, unsigned N)
{
	return Complex(cos(pi2 * x / N), sin(pi2 * x / N));
}

void nex::OceanCpuFFT::fft(nex::Complex* input, nex::Complex* output, int stride, int offset, bool vertical)
{
	for (int i = 0; i < mN; i++) mTemp[mCurrent][i] = input[mReversed[i] * stride + offset];

	int loops = mN >> 1;
	int size = 1 << 1;
	int size_over_2 = 1;
	int w_ = 0;
	for (int i = 1; i <= mLogN; i++) {
		mCurrent ^= 1;
		for (int j = 0; j < loops; j++) {
			for (int k = 0; k < size_over_2; k++) {
				mTemp[mCurrent][size * j + k] = mTemp[mCurrent ^ 1][size * j + k] +
					mTemp[mCurrent ^ 1][size * j + size_over_2 + k] * mTwiddle[w_][k];
			}

			for (int k = size_over_2; k < size; k++) {
				mTemp[mCurrent][size * j + k] = mTemp[mCurrent ^ 1][size * j - size_over_2 + k] -
					mTemp[mCurrent ^ 1][size * j + k] * mTwiddle[w_][k - size_over_2];
			}
		}
		loops >>= 1;
		size <<= 1;
		size_over_2 <<= 1;
		w_++;
	}

	for (unsigned k = 0; k < mN / 2; ++k)
	{
		//std::swap(x[reverse(k)], x[reverse(k + N / 2)]);
		std::swap(mTemp[mCurrent][k], mTemp[mCurrent][k + mN / 2]);
	}

	if (vertical)
	{
		for (unsigned k = 1; k < mN / 2; ++k)
		{
			//std::swap(x[reverse(k)], x[reverse(N - k)]);
			std::swap(mTemp[mCurrent][k], mTemp[mCurrent][mN - k]);
		}
	}

	for (int i = 0; i < mN; i++) output[i * stride + offset] = mTemp[mCurrent][i];
}

void nex::OceanCpuFFT::fft(const nex::Iterator2D& input, nex::Iterator2D& output, bool vertical)
{
	for (int i = 0; i < mN; i++) mTemp[mCurrent][i] = input[mReversed[i]];

	int loops = mN >> 1;
	int size = 1 << 1;
	int size_over_2 = 1;
	int w_ = 0;
	for (int i = 1; i <= mLogN; i++) {
		mCurrent ^= 1;
		for (int j = 0; j < loops; j++) {
			for (int k = 0; k < size_over_2; k++) {
				mTemp[mCurrent][size * j + k] = mTemp[mCurrent ^ 1][size * j + k] +
					mTemp[mCurrent ^ 1][size * j + size_over_2 + k] * mTwiddle[w_][k];
			}

			for (int k = size_over_2; k < size; k++) {
				mTemp[mCurrent][size * j + k] = mTemp[mCurrent ^ 1][size * j - size_over_2 + k] -
					mTemp[mCurrent ^ 1][size * j + k] * mTwiddle[w_][k - size_over_2];
			}
		}
		loops >>= 1;
		size <<= 1;
		size_over_2 <<= 1;
		w_++;
	}

	for (unsigned k = 0; k < mN / 2; ++k)
	{
		//std::swap(x[reverse(k)], x[reverse(k + N / 2)]);
		std::swap(mTemp[mCurrent][k], mTemp[mCurrent][k + mN / 2]);
	}

	if (vertical)
	{
		for (unsigned k = 1; k < mN / 2; ++k)
		{
			//std::swap(x[reverse(k)], x[reverse(N - k)]);
			std::swap(mTemp[mCurrent][k], mTemp[mCurrent][mN - k]);
		}
	}


	for (int i = 0; i < mN; i++) output[i] = mTemp[mCurrent][i];
}

void nex::OceanCpuFFT::fftInPlace(std::vector<nex::Complex>& x, bool inverse)
{
	/* Do the bit reversal */
	{
		unsigned i2 = mN >> 1;
		unsigned j = 0;

		for (unsigned i = 0; i < mN - 1; i++)
		{
			if (i < j)
				std::swap(x[i], x[j]);

			unsigned k = i2;

			while (k <= j)
			{
				j -= k;
				k >>= 1;
			}

			j += k;
		}
	}


	/**
	 * Do FFT
	 */
	Complex c(-1.0, 0.0);
	unsigned l2 = 1;
	for (unsigned l = 0; l < mLogN; l++)
	{
		unsigned l1 = l2;
		l2 <<= 1;
		nex::Complex u(1.0, 0.0);

		for (unsigned j = 0; j < l1; j++)
		{
			for (unsigned i = j; i < mN; i += l2)
			{
				unsigned i1 = i + l1;
				auto t1 = u * x[i1];
				x[i1] = x[i] - t1;
				x[i] += t1;
			}

			u = u * c;
		}

		c.im = sqrt((1.0 - c.re) / 2.0);
		if (!inverse)
		{
			c.im = -c.im;
		}
		c.re = sqrt((1.0 + c.re) / 2.0);
	}

	/* Scaling for forward transform */
	if (inverse)
	{
		for (unsigned i = 0; i < mN; i++)
			x[i] /= (float)mN;
	}

	/* Do the bit reversal */
	/*{
		unsigned i2 = N >> 1;
		unsigned j = 0;

		for (unsigned i = 0; i < N - 1; i++)
		{
			if (i < j)
				std::swap(x[i], x[j]);

			unsigned k = i2;

			while (k <= j)
			{
				j -= k;
				k >>= 1;
			}

			j += k;
		}
	}*/
}

void nex::OceanGPU::testHeightGeneration()
{
	std::vector<glm::vec4> heightZeros(mN*mN);

	mHeightComputePass->compute(10.0f, mHeightZeroComputePass->getResult());
	//RenderBackend::get()->sync
	std::vector<glm::vec2> height(mN*mN);
	std::vector<glm::vec2> slopeX(mN*mN);
	std::vector<glm::vec2> slopeZ(mN*mN);
	std::vector<glm::vec2> dx(mN*mN);
	std::vector<glm::vec2> dz(mN*mN);
	mHeightZeroComputePass->getResult()->readback(0, ColorSpace::RGBA, PixelDataType::FLOAT, heightZeros.data(), heightZeros.size() * sizeof(glm::vec4));
	mHeightComputePass->getHeight()->readback(0, ColorSpace::RG, PixelDataType::FLOAT, height.data(), height.size() * sizeof(glm::vec2));
	mHeightComputePass->getSlopeX()->readback(0, ColorSpace::RG, PixelDataType::FLOAT, slopeX.data(), slopeX.size() * sizeof(glm::vec2));
	mHeightComputePass->getSlopeZ()->readback(0, ColorSpace::RG, PixelDataType::FLOAT, slopeZ.data(), slopeZ.size() * sizeof(glm::vec2));
	mHeightComputePass->getDx()->readback(0, ColorSpace::RG, PixelDataType::FLOAT, dx.data(), dx.size() * sizeof(glm::vec2));
	mHeightComputePass->getDz()->readback(0, ColorSpace::RG, PixelDataType::FLOAT, dz.data(), dz.size() * sizeof(glm::vec2));



	auto* heightFFT = mHeightComputePass->getHeight();
	auto* dxFFT = mHeightComputePass->getDx();
	auto* dzFFT = mHeightComputePass->getDz();
	auto* slopeXFFT = mHeightComputePass->getSlopeX();
	auto* slopeZFFT = mHeightComputePass->getSlopeZ();

	mIfftComputePass->bind();

	mIfftComputePass->useButterfly(mButterflyComputePass->getButterfly());

	// horizontal 1D iFFT
	mIfftComputePass->setVertical(true);
	mIfftComputePass->computeAllStages(heightFFT);
	//mIfftComputePass->computeAllStages(slopeXFFT);
	//mIfftComputePass->computeAllStages(slopeZFFT);
	//mIfftComputePass->computeAllStages(dxFFT);
	//mIfftComputePass->computeAllStages(dzFFT);

	heightFFT->readback(0, ColorSpace::RG, PixelDataType::FLOAT, height.data(), height.size() * sizeof(glm::vec2));
	//dxFFT->readback(0, ColorSpace::RG, PixelDataType::FLOAT, slopeX.data(), slopeX.size() * sizeof(glm::vec2));
	//dzFFT->readback(0, ColorSpace::RG, PixelDataType::FLOAT, slopeZ.data(), slopeZ.size() * sizeof(glm::vec2));
	//slopeXFFT->readback(0, ColorSpace::RG, PixelDataType::FLOAT, dx.data(), dx.size() * sizeof(glm::vec2));
	//slopeZFFT->readback(0, ColorSpace::RG, PixelDataType::FLOAT, dz.data(), dz.size() * sizeof(glm::vec2));

	// vertical 1D iFFT
	mIfftComputePass->setVertical(false);
	mIfftComputePass->computeAllStages(heightFFT);
	//mIfftComputePass->computeAllStages(slopeXFFT);
	//mIfftComputePass->computeAllStages(slopeZFFT);
	//mIfftComputePass->computeAllStages(dxFFT);
	//mIfftComputePass->computeAllStages(dzFFT);


	heightFFT->readback(0, ColorSpace::RG, PixelDataType::FLOAT, height.data(), height.size() * sizeof(glm::vec2));

}

nex::OceanGPU::OceanGPU(unsigned N, unsigned maxWaveLength, float dimension, float waterHeight,
	float spectrumScale, const glm::vec2& windDirection, float windSpeed, float periodTime) :
	Ocean(N, maxWaveLength, dimension, waterHeight, spectrumScale, windDirection, windSpeed, periodTime),
	mHeightZeroComputePass(std::make_unique<HeightZeroComputePass>(glm::uvec2(mN), glm::vec2(mN), mWindDirection, mSpectrumScale, mWindSpeed)), // mUniquePointCount.x, mUniquePointCount.y
	mHeightComputePass(std::make_unique<HeightComputePass>(glm::uvec2(mN), glm::vec2(maxWaveLength), mPeriodTime)),
	mButterflyComputePass(std::make_unique<ButterflyComputePass>(mN)),
	mIfftComputePass(std::make_unique<IfftPass>(mN)),
	mNormalizePermutatePass(std::make_unique<NormalizePermutatePass>(mN)),
	mSimpleShadedPass(std::make_unique<WaterShading>()),
	mWaterDepthClearPass(std::make_unique<WaterDepthClearPass>()),
	mWaterDepthPass(std::make_unique<WaterDepthPass>()),
	mUnderWaterView(std::make_unique<UnderWaterView>()),
	mPSSR(std::make_unique<PSSR>())
{
	mHeightZeroComputePass->compute();
	
	computeButterflyTexture();

	testHeightGeneration();

	generateMesh();

	TextureDesc desc;
	desc.generateMipMaps = false;
	desc.wrapR = desc.wrapS = desc.wrapT = UVTechnique::Repeat;
	desc.colorspace = ColorSpace::SRGB;
	mFoamTexture = TextureManager::get()->getImage("_intern/ocean/foam.png", desc);
}

nex::OceanGPU::~OceanGPU() = default;

void nex::OceanGPU::computeWaterDepths(Texture * waterMinDepth, 
	Texture* waterMaxDepth,
	Texture * depth, Texture * stencil,
	const glm::mat4& inverseViewProjMatrix)
{
	mWaterDepthClearPass->bind();
	mWaterDepthClearPass->setWaterMinDepthOut(waterMinDepth);
	mWaterDepthClearPass->setWaterMaxDepthOut(waterMaxDepth);
	mWaterDepthClearPass->dispatch(depth->getWidth(), 1, 1);
	RenderBackend::get()->syncMemoryWithGPU(MemorySync_ShaderImageAccess | MemorySync_TextureFetch);

	std::vector<float> dest(depth->getWidth());
	waterMinDepth->readback(0, ColorSpace::RED_INTEGER, PixelDataType::INT, dest.data(), sizeof(float) * dest.size());
	auto test = dest[0];

	mWaterDepthPass->bind();
	mWaterDepthPass->setDepth(depth);
	mWaterDepthPass->setStencil(stencil);
	mWaterDepthPass->setWaterMinDepthOut(waterMinDepth);
	mWaterDepthPass->setWaterMaxDepthOut(waterMaxDepth);
	mWaterDepthPass->setInverseViewProjMatrix(inverseViewProjMatrix);

	constexpr unsigned TILE_WIDTH = 64;
	constexpr unsigned TILE_HEIGHT = 128;

	unsigned wgX = (depth->getWidth() + TILE_WIDTH - 1) / TILE_WIDTH;
	unsigned wgY = (depth->getHeight() + TILE_HEIGHT - 1) / TILE_HEIGHT;

	mWaterDepthPass->dispatch(depth->getWidth(), depth->getHeight(), 1);
	RenderBackend::get()->syncMemoryWithGPU(MemorySync_ShaderImageAccess | MemorySync_TextureFetch);
	
	//std::vector<float> dest2(depth->getWidth());
	//waterDepth->readback(0, ColorSpace::RED_INTEGER, PixelDataType::INT, dest2.data(), sizeof(float) * dest2.size());
	//auto test2 = dest2[0];
}

void nex::OceanGPU::draw(const glm::mat4& projection, 
	const glm::mat4& view, 
	const glm::mat4& inverseViewProjMatrix,
	const glm::vec3& lightDir, 
	nex::CascadedShadow* cascadedShadow,
	nex::Texture* color, 
	nex::Texture* luminance, 
	nex::Texture* depth,
	nex::Texture* irradiance,
	GlobalIllumination* gi,
	const glm::vec3& cameraPosition,
	const glm::vec3& cameraDir)
{

	mPSSR->renderProjectionHash(depth, projection * view, inverseViewProjMatrix, mWaterHeight, cameraDir);


	mSimpleShadedPass->bind();
	
	auto model = getModelMatrix();

	mSimpleShadedPass->setUniforms(projection, 
		view, 
		model,
		inverseViewProjMatrix,
		lightDir, 
		cascadedShadow,
		mHeightComputePass->getHeight(),
		mHeightComputePass->getSlopeX(),
		mHeightComputePass->getSlopeZ(),
		mHeightComputePass->getDx(),
		mHeightComputePass->getDz(),
		color,
		luminance, 
		depth,
		irradiance,
		mFoamTexture,
		mPSSR->getProjHashTexture(),
		gi,
		cameraPosition,
		mWindDirection,
		mAnimationTime,
		mWaveLength,
		mWaterHeight);

	//mMesh->bind();
	mMesh->getVertexArray().bind();
	mMesh->getIndexBuffer().bind();
	RenderState state;
	state.doBlend = false;
	state.blendDesc = BlendDesc::createAlphaTransparency();

	state.doDepthTest = true;
	state.doDepthWrite = true;
	state.doCullFaces = false;

	if (mWireframe)
	{
		state.fillMode = FillMode::LINE;
	}
	else
	{
		state.fillMode = FillMode::FILL;
	}

	state.depthCompare = CompFunc::LESS;

	RenderBackend::get()->drawWithIndicesInstanced(64, state, Topology::TRIANGLES, mMesh->getIndexBuffer().getCount(), mMesh->getIndexBuffer().getType());
}

void nex::OceanGPU::drawUnderWaterView(Texture* color, 
	Texture* waterMinDepth, 
	Texture* waterMaxDepth, 
	Texture* depth, 
	Texture* waterStencil, 
	const glm::mat4& inverseViewProjMatrix, 
	const glm::vec3& cameraPos)
{
	mUnderWaterView->bind();
	mUnderWaterView->setColorMap(color);
	mUnderWaterView->setOceanMinHeightMap(waterMinDepth);
	mUnderWaterView->setOceanMaxHeightMap(waterMaxDepth);
	mUnderWaterView->setDepthMap(depth);
	mUnderWaterView->setStencilMap(waterStencil);
	mUnderWaterView->setCameraPosition(cameraPos);
	mUnderWaterView->setInverseViewProjMatrix_GPass(inverseViewProjMatrix);

	mUnderWaterView->setOceanDX(mHeightComputePass->getDx());
	mUnderWaterView->setOceanDZ(mHeightComputePass->getDz());
	mUnderWaterView->setOceanHeightMap(mHeightComputePass->getHeight());
	mUnderWaterView->setInverseModelMatrix_Ocean(inverse(getModelMatrix()));
	mUnderWaterView->setOceanTileSize(getTileSize());

	MeshDrawer::drawFullscreenTriangle(RenderState::getNoDepthTest(), mUnderWaterView.get());
}

void nex::OceanGPU::simulate(float t)
{
	//unsigned mult = mAnimationTime / mPeriodTime;
	//mAnimationTime = mAnimationTime - mult * mPeriodTime;

	mHeightComputePass->compute(t, mHeightZeroComputePass->getResult());


	auto* heightFFT = mHeightComputePass->getHeight();
	auto* dxFFT = mHeightComputePass->getDx();
	auto* dzFFT = mHeightComputePass->getDz();
	auto* slopeXFFT = mHeightComputePass->getSlopeX();
	auto* slopeZFFT = mHeightComputePass->getSlopeZ();

	mIfftComputePass->bind();

	mIfftComputePass->useButterfly(mButterflyComputePass->getButterfly());


	// horizontal 1D iFFT
	mIfftComputePass->setVertical(true);
	
	mIfftComputePass->computeAllStages(heightFFT);
	mIfftComputePass->computeAllStages(slopeXFFT);
	mIfftComputePass->computeAllStages(slopeZFFT);
	mIfftComputePass->computeAllStages(dxFFT);
	mIfftComputePass->computeAllStages(dzFFT);

	// vertical 1D iFFT
	mIfftComputePass->setVertical(false);
	mIfftComputePass->computeAllStages(heightFFT);
	mIfftComputePass->computeAllStages(slopeXFFT);
	mIfftComputePass->computeAllStages(slopeZFFT);
	mIfftComputePass->computeAllStages(dxFFT);
	mIfftComputePass->computeAllStages(dzFFT);

	RenderBackend::get()->syncMemoryWithGPU(MemorySync_ShaderImageAccess);
	mNormalizePermutatePass->compute(heightFFT, slopeXFFT, slopeZFFT, dxFFT, dzFFT);
}

nex::Texture* nex::OceanGPU::getHeightMap()
{
	return mHeightComputePass->getHeight();
}

nex::Texture* nex::OceanGPU::getDX()
{
	return mHeightComputePass->getDx();
}

nex::Texture* nex::OceanGPU::getDZ()
{
	return mHeightComputePass->getDz();
}

void nex::OceanGPU::resize(unsigned width, unsigned height)
{
	mPSSR->resize(width, height);
}

void nex::OceanGPU::computeButterflyTexture(bool debug)
{
	mButterflyComputePass->compute();
	std::vector<glm::vec4> butterfly(mN * std::log2(mN));

	if (!debug) return;

	std::vector<glm::vec4> butterflyImage(mN * std::log2(mN));
	std::vector<byte> butterflyOut(mN * std::log2(mN) * 3);


	mButterflyComputePass->getButterfly()->
		readback(0, ColorSpace::RGBA, PixelDataType::FLOAT, butterflyImage.data(), butterflyImage.size() * sizeof(glm::vec4));


	for (unsigned i = 0; i < butterflyImage.size(); ++i)
	{
		const auto& pixel = butterflyImage[i];
		const auto source = 127.5f + (255.0f * glm::vec3(pixel)) / 2.0f;
		byte r = static_cast<byte>(source.x);
		byte g = static_cast<byte>(source.y);
		byte b = static_cast<byte>(pixel.z);
		butterflyOut[i * 3] = r;
		butterflyOut[i * 3 + 1] = g;
		butterflyOut[i * 3 + 2] = b;
	}

	ImageFactory::writeToPNG("./oceanTest/butterfly.png", (const char*)butterflyOut.data(), mN, std::log2(mN), 3, mN * 3, false);
}

void nex::OceanGPU::generateMesh()
{
	const auto vertexCount = mPointCount * mPointCount;
	const auto quadCount = (mPointCount - 1) * (mPointCount - 1);

	const unsigned indicesPerQuad = 6; // two triangles per quad

	mVertices.resize(vertexCount);
	mIndices.resize(quadCount * indicesPerQuad); // two triangles per quad

	for (unsigned z = 0; z < mPointCount; ++z)
	{
		for (unsigned x = 0; x < mPointCount; ++x)
		{
			const auto index = z * mPointCount + x;

			mVertices[index].position = glm::vec3(
				(x - mPointCount / 2.0f) * mWaveLength / (float)mN,
				0.0f,
				getZValue((z - mPointCount / 2.0f) * mWaveLength / (float)mN)
			);

			/**
			 * By modulating the texture coordinate with N, the ocean is tilable
			 */
			mVertices[index].texCoords = glm::vec2(x % mN, z % mN) / (float)mPointCount;
		}
	}


	const auto quadNumber = mN;

	for (unsigned x = 0; x < quadNumber; ++x)
	{
		for (unsigned z = 0; z < quadNumber; ++z)
		{
			const auto indexStart = indicesPerQuad * (x * quadNumber + z);

			const auto vertexBottomLeft = x * mPointCount + z;

			// first triangle
			mIndices[indexStart] = vertexBottomLeft;
			mIndices[indexStart + 1] = vertexBottomLeft + 1; // one column to the right
			mIndices[indexStart + 2] = vertexBottomLeft + mPointCount + 1; // one row up and one column to the right

			// second triangle
			mIndices[indexStart + 3] = vertexBottomLeft;
			mIndices[indexStart + 4] = vertexBottomLeft + mPointCount + 1;
			mIndices[indexStart + 5] = vertexBottomLeft + mPointCount; // one row up
		}
	}

	auto vertexBuffer = std::make_unique<VertexBuffer>();
	vertexBuffer->resize(vertexCount * sizeof(Vertex), mVertices.data(), ShaderBuffer::UsageHint::DYNAMIC_DRAW);
	IndexBuffer indexBuffer(IndexElementType::BIT_32, static_cast<unsigned>(mIndices.size()), mIndices.data());
	indexBuffer.unbind();

	VertexLayout layout;
	layout.push<glm::vec3>(1, vertexBuffer.get(), false, false, true); // position
	layout.push<glm::vec2>(1, vertexBuffer.get(), false, false, true); // texCoords

	//TODO
	AABB boundingBox;
	boundingBox.min = glm::vec3(0.0f);
	boundingBox.max = glm::vec3(0.0f);

	mMesh = std::make_unique<Mesh>();
	mMesh->addVertexDataBuffer(std::move(vertexBuffer));
	mMesh->setBoundingBox(std::move(boundingBox));
	mMesh->setIndexBuffer(std::move(indexBuffer));
	mMesh->setLayout(std::move(layout));
	mMesh->setTopology(Topology::TRIANGLES);
	mMesh->finalize();
}


nex::OceanGPU::UnderWaterView::UnderWaterView()
{
	mProgram = nex::ShaderProgram::create("screen_space_vs.glsl", "ocean/under_water_view_fs.glsl");

	mColorMap = mProgram->createTextureUniform("colorMap", UniformType::TEXTURE2D, 0);
	mOceanHeightMap = mProgram->createTextureUniform("oceanHeightMap", UniformType::TEXTURE2D, 1);
	mDepthMap = mProgram->createTextureUniform("depthMap", UniformType::TEXTURE2D, 2);
	mStencilMap = mProgram->createTextureUniform("stencilMap", UniformType::TEXTURE2D, 3);
	//mOceanDX = mProgram->createTextureUniform("oceanDX", UniformType::TEXTURE2D, 4);
	//mOceanDZ = mProgram->createTextureUniform("oceanDZ", UniformType::TEXTURE2D, 5);
	mOceanMinHeightMap = mProgram->createTextureUniform("oceanMinHeightMap", UniformType::TEXTURE1D, 4);
	mOceanMaxHeightMap = mProgram->createTextureUniform("oceanMaxHeightMap", UniformType::TEXTURE1D, 5);
	mInverseViewProjMatrix_GPass = { mProgram->getUniformLocation("inverseViewProjMatrix_GPass"), UniformType::MAT4 };
	mInverseModelMatrix_Ocean = { mProgram->getUniformLocation("inverseModelMatrix_Ocean"), UniformType::MAT4 };
	mOceanTileSize = { mProgram->getUniformLocation("oceanTileSize"), UniformType::FLOAT };
	mCameraPosition = { mProgram->getUniformLocation("cameraPosition"), UniformType::VEC3 };
}

void nex::OceanGPU::UnderWaterView::setColorMap(Texture* texture)
{
	mProgram->setTexture(texture, Sampler::getPoint(), mColorMap.bindingSlot);
}

void nex::OceanGPU::UnderWaterView::setInverseViewProjMatrix_GPass(const glm::mat4& mat) {
	mProgram->setMat4(mInverseViewProjMatrix_GPass.location, mat);
}

void nex::OceanGPU::UnderWaterView::setInverseModelMatrix_Ocean(const glm::mat4& mat) {
	mProgram->setMat4(mInverseModelMatrix_Ocean.location, mat);
}

void nex::OceanGPU::UnderWaterView::setOceanTileSize(float tileSize) {
	mProgram->setFloat(mOceanTileSize.location, tileSize);
}

void nex::OceanGPU::UnderWaterView::setDepthMap(Texture* texture) {
	mProgram->setTexture(texture, Sampler::getPoint(), mDepthMap.bindingSlot);
}

void nex::OceanGPU::UnderWaterView::setOceanHeightMap(Texture* texture) {
	mProgram->setTexture(texture, Sampler::getLinearRepeat(), mOceanHeightMap.bindingSlot);
}

void nex::OceanGPU::UnderWaterView::setOceanDX(Texture* texture) {
	//mProgram->setTexture(texture, Sampler::getLinearRepeat(), mOceanDX.bindingSlot);
}

void nex::OceanGPU::UnderWaterView::setOceanDZ(Texture* texture) {
	//mProgram->setTexture(texture, Sampler::getLinearRepeat(), mOceanDZ.bindingSlot);
}

void nex::OceanGPU::UnderWaterView::setOceanMinHeightMap(Texture* texture) {
	mProgram->setTexture(texture, Sampler::getLinearRepeat(), mOceanMinHeightMap.bindingSlot);
}

void nex::OceanGPU::UnderWaterView::setOceanMaxHeightMap(Texture* texture) {
	mProgram->setTexture(texture, Sampler::getLinearRepeat(), mOceanMaxHeightMap.bindingSlot);
}

void nex::OceanGPU::UnderWaterView::setStencilMap(Texture* texture) {
	mProgram->setTexture(texture, Sampler::getPoint(), mStencilMap.bindingSlot);
}

void nex::OceanGPU::UnderWaterView::setCameraPosition(const glm::vec3& pos) {
	mProgram->setVec3(mCameraPosition.location, pos);
}



nex::OceanGPU::WaterDepthClearPass::WaterDepthClearPass() :
	ComputeShader(ShaderProgram::createComputeShader("ocean/water_surface_depth_clear_cs.glsl"))
{
	mWaterMinDepth = mProgram->createTextureUniform("waterMinDepths", UniformType::IMAGE1D, 0);
	mWaterMaxDepth = mProgram->createTextureUniform("waterMaxDepths", UniformType::IMAGE1D, 1);
}

void nex::OceanGPU::WaterDepthClearPass::setWaterMinDepthOut(Texture* waterMinDepth)
{
	mProgram->setImageLayerOfTexture(mWaterMinDepth.location,
		waterMinDepth,
		mWaterMinDepth.bindingSlot,
		TextureAccess::WRITE_ONLY,
		InternalFormat::R32I,
		0,
		false,
		0);
}

void nex::OceanGPU::WaterDepthClearPass::setWaterMaxDepthOut(Texture* waterMaxDepth)
{
	mProgram->setImageLayerOfTexture(mWaterMaxDepth.location,
		waterMaxDepth,
		mWaterMaxDepth.bindingSlot,
		TextureAccess::WRITE_ONLY,
		InternalFormat::R32I,
		0,
		false,
		0);
}

nex::OceanGPU::WaterDepthPass::WaterDepthPass() : 
	ComputeShader(ShaderProgram::createComputeShader("ocean/water_surface_depth_cs.glsl"))
{
	mWaterMinDepth = mProgram->createTextureUniform("waterMinDepths", UniformType::IMAGE1D, 0);
	mWaterMaxDepth = mProgram->createTextureUniform("waterMaxDepths", UniformType::IMAGE1D, 1);

	mDepth = mProgram->createTextureUniform("depthMap", UniformType::TEXTURE2D, 0);
	mStencil = mProgram->createTextureUniform("stencilMap", UniformType::TEXTURE2D, 1);
	mInverseViewProjMatrix = {mProgram->getUniformLocation("inverseViewProjMatrix"), UniformType::MAT4};
}

void nex::OceanGPU::WaterDepthPass::setDepth(Texture* depth)
{
	mProgram->setTexture(depth, Sampler::getPoint(), mDepth.bindingSlot);
}

void nex::OceanGPU::WaterDepthPass::setStencil(Texture* stencil)
{
	mProgram->setTexture(stencil, Sampler::getPoint(), mStencil.bindingSlot);
}

void nex::OceanGPU::WaterDepthPass::setWaterMinDepthOut(Texture* waterMinDepth)
{
	mProgram->setImageLayerOfTexture(mWaterMinDepth.location,
		waterMinDepth,
		mWaterMinDepth.bindingSlot,
		TextureAccess::WRITE_ONLY,
		InternalFormat::R32I,
		0,
		false,
		0);
}

void nex::OceanGPU::WaterDepthPass::setWaterMaxDepthOut(Texture* waterMaxDepth)
{
	mProgram->setImageLayerOfTexture(mWaterMaxDepth.location,
		waterMaxDepth,
		mWaterMaxDepth.bindingSlot,
		TextureAccess::WRITE_ONLY,
		InternalFormat::R32I,
		0,
		false,
		0);
}

void nex::OceanGPU::WaterDepthPass::setInverseViewProjMatrix(const glm::mat4& mat)
{
	mProgram->setMat4(mInverseViewProjMatrix.location, mat);
}


nex::OceanGPU::HeightZeroComputePass::HeightZeroComputePass(const glm::uvec2& uniquePointCount, const glm::vec2& waveLength, const glm::vec2& windDirection,
	float spectrumScale, float windSpeed) :
	ComputeShader(ShaderProgram::createComputeShader("ocean/height_zero_precompute_cs.glsl")),
	mUniquePointCount(uniquePointCount), mWaveLength(waveLength), mWindDirection(windDirection), mSpectrumScale(spectrumScale), mWindSpeed(windSpeed)
{
	TextureDesc desc;
	desc.internalFormat = InternalFormat::RGBA32F;
	desc.colorspace = ColorSpace::RGBA;
	desc.pixelDataType = PixelDataType::FLOAT;
	desc.generateMipMaps = false;
	desc.magFilter = desc.minFilter = TexFilter::Nearest;
	mHeightZero = std::make_unique<Texture2D>(mUniquePointCount.x, mUniquePointCount.y, desc, nullptr);

	mResultTexture = { mProgram->getUniformLocation("result"), UniformType::IMAGE2D, 0 };

	mUniquePointCountUniform = { mProgram->getUniformLocation("uniquePointCount"), UniformType::UVEC2 };
	mWaveLengthUniform = { mProgram->getUniformLocation("waveLength"), UniformType::VEC2 };
	mWindDirectionUniform = { mProgram->getUniformLocation("windDirection"), UniformType::VEC2 };
	mSpectrumScaleUniform = { mProgram->getUniformLocation("spectrumScale"), UniformType::FLOAT };
	mWindSpeedUniform = { mProgram->getUniformLocation("windSpeed"), UniformType::FLOAT };

	mProgram->bind();

	mProgram->setUVec2(mUniquePointCountUniform.location, mUniquePointCount);
	mProgram->setVec2(mWaveLengthUniform.location, mWaveLength);
	mProgram->setVec2(mWindDirectionUniform.location, mWindDirection);
	mProgram->setFloat(mSpectrumScaleUniform.location, mSpectrumScale);
	mProgram->setFloat(mWindSpeedUniform.location, mWindSpeed);


	std::vector<glm::vec4> randValues(mUniquePointCount.x * mUniquePointCount.y);

	for (unsigned z = 0; z < mUniquePointCount.y; ++z)
	{
		for (unsigned x = 0; x < mUniquePointCount.x; ++x)
		{
			const auto index = z * mUniquePointCount.x + x;
			randValues[index].x = generateGaussianRand();
			randValues[index].y = generateGaussianRand();
			randValues[index].z = generateGaussianRand();
			randValues[index].w = generateGaussianRand();
		}
	}

	TextureDesc randDesc;
	randDesc.internalFormat = InternalFormat::RGBA32F;
	randDesc.colorspace = ColorSpace::RGBA;
	randDesc.pixelDataType = PixelDataType::FLOAT;
	randDesc.generateMipMaps = false;
	randDesc.magFilter = randDesc.minFilter = TexFilter::Nearest;

	mRandNormalDistributed = std::make_unique<Texture2D>(mUniquePointCount.x, mUniquePointCount.y, randDesc, randValues.data());
	mRandTextureUniform = { mProgram->getUniformLocation("randTexture"), UniformType::TEXTURE2D, 1 };
}

void nex::OceanGPU::HeightZeroComputePass::compute()
{
	mProgram->bind();

	mProgram->setImageLayerOfTexture(mResultTexture.location,
		mHeightZero.get(),
		mResultTexture.bindingSlot,
		TextureAccess::READ_WRITE,
		InternalFormat::RGBA32F,
		0,
		false,
		0);

	mProgram->setImageLayerOfTexture(mRandTextureUniform.location,
		mRandNormalDistributed.get(),
		mRandTextureUniform.bindingSlot,
		TextureAccess::READ_ONLY,
		InternalFormat::RGBA32F,
		0,
		false,
		0);

	dispatch(mUniquePointCount.x, mUniquePointCount.y, 1);
	RenderBackend::get()->syncMemoryWithGPU(MemorySync_ShaderImageAccess);
}

nex::Texture2D* nex::OceanGPU::HeightZeroComputePass::getResult()
{
	return mHeightZero.get();
}

nex::OceanGPU::HeightComputePass::HeightComputePass(const glm::uvec2& uniquePointCount, const glm::vec2& waveLength,
	float periodTime) :
	ComputeShader(ShaderProgram::createComputeShader("ocean/height_cs.glsl")),
	mUniquePointCount(uniquePointCount)
{
	mUniquePointCountUniform = { mProgram->getUniformLocation("uniquePointCount"), UniformType::UVEC2 };
	mWaveLengthUniform = { mProgram->getUniformLocation("waveLength"), UniformType::VEC2 };

	mResultHeightTextureUniform = { mProgram->getUniformLocation("resultHeight"), UniformType::IMAGE2D, 0 };
	mResultSlopeXTextureUniform = { mProgram->getUniformLocation("resultSlopeX"), UniformType::IMAGE2D, 1 };
	mResultSlopeZTextureUniform = { mProgram->getUniformLocation("resultSlopeZ"), UniformType::IMAGE2D, 2 };
	mResultDxTextureUniform = { mProgram->getUniformLocation("resultDx"), UniformType::IMAGE2D, 3 };
	mResultDzTextureUniform = { mProgram->getUniformLocation("resultDz"), UniformType::IMAGE2D, 4 };

	mHeightZeroTextureUniform = { mProgram->getUniformLocation("heightZero"), UniformType::IMAGE2D, 5 };

	mTimeUniform = { mProgram->getUniformLocation("currentTime"), UniformType::FLOAT };
	mPeriodTimeUniform = { mProgram->getUniformLocation("periodTime"), UniformType::FLOAT };

	TextureDesc desc;
	desc.internalFormat = InternalFormat::RG32F;
	desc.colorspace = ColorSpace::RG;
	desc.pixelDataType = PixelDataType::FLOAT;
	desc.generateMipMaps = false;
	desc.magFilter = desc.minFilter = TexFilter::Nearest;
	mHeight = std::make_unique<Texture2D>(mUniquePointCount.x, mUniquePointCount.y, desc, nullptr);
	mHeightSlopeX = std::make_unique<Texture2D>(mUniquePointCount.x, mUniquePointCount.y, desc, nullptr);
	mHeightSlopeZ = std::make_unique<Texture2D>(mUniquePointCount.x, mUniquePointCount.y, desc, nullptr);
	mHeightDx = std::make_unique<Texture2D>(mUniquePointCount.x, mUniquePointCount.y, desc, nullptr);
	mHeightDz = std::make_unique<Texture2D>(mUniquePointCount.x, mUniquePointCount.y, desc, nullptr);

	mProgram->bind();
	mProgram->setUVec2(mUniquePointCountUniform.location, uniquePointCount);
	mProgram->setVec2(mWaveLengthUniform.location, waveLength);
	mProgram->setFloat(mPeriodTimeUniform.location, periodTime);
}

void nex::OceanGPU::HeightComputePass::compute(float time, Texture2D* heightZero)
{
	// Ensure that the updated content of heightZero is visible
	RenderBackend::get()->syncMemoryWithGPU((MemorySync)(MemorySync_ShaderImageAccess | MemorySync_TextureUpdate));
	mProgram->bind();
	mProgram->setFloat(mTimeUniform.location, time);

	mProgram->setImageLayerOfTexture(mResultHeightTextureUniform.location,
		mHeight.get(),
		mResultHeightTextureUniform.bindingSlot,
		TextureAccess::READ_WRITE,
		InternalFormat::RG32F,
		0,
		false,
		0);

	mProgram->setImageLayerOfTexture(mResultSlopeXTextureUniform.location,
		mHeightSlopeX.get(),
		mResultSlopeXTextureUniform.bindingSlot,
		TextureAccess::WRITE_ONLY,
		InternalFormat::RG32F,
		0,
		false,
		0);

	mProgram->setImageLayerOfTexture(mResultSlopeZTextureUniform.location,
		mHeightSlopeZ.get(),
		mResultSlopeZTextureUniform.bindingSlot,
		TextureAccess::WRITE_ONLY,
		InternalFormat::RG32F,
		0,
		false,
		0);

	mProgram->setImageLayerOfTexture(mResultDxTextureUniform.location,
		mHeightDx.get(),
		mResultDxTextureUniform.bindingSlot,
		TextureAccess::WRITE_ONLY,
		InternalFormat::RG32F,
		0,
		false,
		0);

	mProgram->setImageLayerOfTexture(mResultDzTextureUniform.location,
		mHeightDz.get(),
		mResultDzTextureUniform.bindingSlot,
		TextureAccess::WRITE_ONLY,
		InternalFormat::RG32F,
		0,
		false,
		0);

	mProgram->setImageLayerOfTexture(mHeightZeroTextureUniform.location,
		heightZero,
		mHeightZeroTextureUniform.bindingSlot,
		TextureAccess::READ_WRITE,
		InternalFormat::RGBA32F,
		0,
		false,
		0);

	dispatch(mUniquePointCount.x, mUniquePointCount.y, 1);
	RenderBackend::get()->syncMemoryWithGPU(MemorySync_ShaderImageAccess);
}

nex::Texture2D* nex::OceanGPU::HeightComputePass::getHeight()
{
	return mHeight.get();
}

nex::Texture2D* nex::OceanGPU::HeightComputePass::getSlopeX()
{
	return mHeightSlopeX.get();
}

nex::Texture2D* nex::OceanGPU::HeightComputePass::getSlopeZ()
{
	return mHeightSlopeZ.get();
}

nex::Texture2D* nex::OceanGPU::HeightComputePass::getDx()
{
	return mHeightDx.get();
}

nex::Texture2D* nex::OceanGPU::HeightComputePass::getDz()
{
	return mHeightDz.get();
}


nex::OceanGPU::ButterflyComputePass::ButterflyComputePass(unsigned N) : ComputeShader(ShaderProgram::createComputeShader("ocean/butterfly_cs.glsl")),
mN(N)
{
	if (!nex::isPow2(mN)) throw std::invalid_argument("nex::Ocean::ButterflyComputePass : N has to be a power of 2!");

	TextureDesc desc;
	desc.internalFormat = InternalFormat::RGBA32F;
	desc.colorspace = ColorSpace::RGBA;
	desc.pixelDataType = PixelDataType::FLOAT;
	desc.generateMipMaps = false;
	desc.magFilter = desc.minFilter = TexFilter::Nearest;
	mButterfly = std::make_unique<Texture2D>(mN, static_cast<unsigned>(std::log2(mN)), desc, nullptr);

	mButterflyUniform = { mProgram->getUniformLocation("butterfly"), UniformType::IMAGE2D, 0 };
	mNUniform = { mProgram->getUniformLocation("N"), UniformType::INT };

	mProgram->bind();
	mProgram->setInt(mNUniform.location, mN);
}

void nex::OceanGPU::ButterflyComputePass::compute()
{
	mProgram->bind();

	mProgram->setImageLayerOfTexture(mButterflyUniform.location,
		mButterfly.get(),
		mButterflyUniform.bindingSlot,
		TextureAccess::WRITE_ONLY,
		InternalFormat::RGBA32F,
		0,
		false,
		0);

	dispatch(mButterfly->getWidth(), mButterfly->getHeight(), 1);
	RenderBackend::get()->syncMemoryWithGPU(MemorySync_TextureUpdate | MemorySync_ShaderImageAccess);
}

nex::Texture2D* nex::OceanGPU::ButterflyComputePass::getButterfly()
{
	return mButterfly.get();
}

nex::OceanGPU::IfftPass::IfftPass(int N) : ComputeShader(ShaderProgram::createComputeShader("ocean/ifft_cs.glsl")),
mBlit(std::make_unique<ComputeShader>(nex::ShaderProgram::createComputeShader("ocean/blit_cs.glsl"))), mN(N), mLog2N(std::log2(mN))

{
	TextureDesc desc;
	desc.internalFormat = InternalFormat::RG32F;
	desc.colorspace = ColorSpace::RG;
	desc.pixelDataType = PixelDataType::FLOAT;
	mPingPong = std::make_unique<Texture2D>(N, N, desc, nullptr);


	mNUniform = { mProgram->getUniformLocation("N"), UniformType::INT };
	mStageUniform = { mProgram->getUniformLocation("stage"), UniformType::INT };
	mVerticalUniform = { mProgram->getUniformLocation("vertical"), UniformType::INT };
	mInputUniform = { mProgram->getUniformLocation("inputImage"), UniformType::IMAGE2D, 0 };
	mButterflyUniform = { mProgram->getUniformLocation("butterfly"), UniformType::IMAGE2D, 1 };
	mOutputUniform = { mProgram->getUniformLocation("outputImage"), UniformType::IMAGE2D, 2 };


	mProgram->bind();
	mProgram->setInt(mNUniform.location, mN);

	mBlit->bind();
	mBlitSourceUniform = { mBlit->getShader()->getUniformLocation("source"), UniformType::IMAGE2D, 0 };
	mBlitDestUniform = { mBlit->getShader()->getUniformLocation("dest"), UniformType::IMAGE2D, 1 };
}

void nex::OceanGPU::IfftPass::useButterfly(Texture2D* butterfly)
{
	mButterfly = butterfly;
}

void nex::OceanGPU::IfftPass::setButterfly(Texture2D* butterfly)
{
	mProgram->setImageLayerOfTexture(mButterflyUniform.location,
		butterfly,
		mButterflyUniform.bindingSlot,
		TextureAccess::READ_ONLY,
		InternalFormat::RGBA32F,
		0,
		false,
		0);
}

void nex::OceanGPU::IfftPass::setInput(Texture2D* input)
{
	mProgram->setImageLayerOfTexture(mInputUniform.location,
		input,
		mInputUniform.bindingSlot,
		TextureAccess::READ_ONLY,
		InternalFormat::RG32F,
		0,
		false,
		0);
}

void nex::OceanGPU::IfftPass::setOutput(Texture2D* output)
{
	mProgram->setImageLayerOfTexture(mOutputUniform.location,
		output,
		mOutputUniform.bindingSlot,
		TextureAccess::WRITE_ONLY,
		InternalFormat::RG32F,
		0,
		false,
		0);
}

void nex::OceanGPU::IfftPass::setStage(int stage)
{
	mProgram->setInt(mStageUniform.location, stage);
}

void nex::OceanGPU::IfftPass::setVertical(bool vertical)
{
	mProgram->setInt(mVerticalUniform.location, (int)vertical);
}

void nex::OceanGPU::IfftPass::computeAllStages(Texture2D* input)
{
	Texture2D* textures[2] = { input, mPingPong.get() };
	int index = 0;

	setButterfly(mButterfly);

	for (unsigned n = 0; n < mLog2N; ++n)
	{
		const int nextIndex = (index + 1) % 2;

		setStage(n);
		setInput(textures[index]);
		setOutput(textures[nextIndex]);
		RenderBackend::get()->syncMemoryWithGPU(MemorySync_ShaderImageAccess);
		dispatch(mN, mN, 1);
		//RenderBackend::get()->syncMemoryWithGPU(MemorySync_ShaderImageAccess);
		index = nextIndex;
	}

	// Conditionally copy to input texture
	if (index == 1)
	{
		mBlit->bind();
		auto* blitShader = mBlit->getShader();
		blitShader->setImageLayerOfTexture(mBlitSourceUniform.location,
			mPingPong.get(),
			mBlitSourceUniform.bindingSlot,
			TextureAccess::READ_ONLY,
			InternalFormat::RG32F,
			0,
			false,
			0);

		blitShader->setImageLayerOfTexture(mBlitDestUniform.location,
			input,
			mBlitDestUniform.bindingSlot,
			TextureAccess::WRITE_ONLY,
			InternalFormat::RG32F,
			0,
			false,
			0);


		RenderBackend::get()->syncMemoryWithGPU(MemorySync_ShaderImageAccess);
		mBlit->dispatch(mN, mN, 1);
		//RenderBackend::get()->syncMemoryWithGPU(MemorySync_ShaderImageAccess);
		// rebind this compute shader
		bind();
	}
}


nex::OceanGPU::NormalizePermutatePass::NormalizePermutatePass(int N) :
ComputeShader(ShaderProgram::createComputeShader("ocean/normalize_permutate_cs.glsl")),
mN(N)
{
	mNUniform = { mProgram->getUniformLocation("N"), UniformType::INT };
	mHeightUniform = { mProgram->getUniformLocation("height"), UniformType::IMAGE2D, 0 };
	mSlopeXUniform = { mProgram->getUniformLocation("slopeX"), UniformType::IMAGE2D, 1 };
	mSlopeZUniform = { mProgram->getUniformLocation("slopeZ"), UniformType::IMAGE2D, 2 };
	mdXUniform = { mProgram->getUniformLocation("dX"), UniformType::IMAGE2D, 3 };
	mdZUniform = { mProgram->getUniformLocation("dZ"), UniformType::IMAGE2D, 4 };

	mProgram->bind();
	mProgram->setInt(mNUniform.location, mN);
}

void nex::OceanGPU::NormalizePermutatePass::compute(Texture2D* height, Texture2D* slopeX, Texture2D* slopeZ, Texture2D* dX, Texture2D* dZ)
{
	bind();

	mProgram->setImageLayerOfTexture(mHeightUniform.location,
		height,
		mHeightUniform.bindingSlot,
		TextureAccess::READ_WRITE,
		InternalFormat::RG32F,
		0,
		false,
		0);

	mProgram->setImageLayerOfTexture(mSlopeXUniform.location,
		slopeX,
		mSlopeXUniform.bindingSlot,
		TextureAccess::READ_WRITE,
		InternalFormat::RG32F,
		0,
		false,
		0);

	mProgram->setImageLayerOfTexture(mSlopeZUniform.location,
		slopeZ,
		mSlopeZUniform.bindingSlot,
		TextureAccess::READ_WRITE,
		InternalFormat::RG32F,
		0,
		false,
		0);

	mProgram->setImageLayerOfTexture(mdXUniform.location,
		dX,
		mdXUniform.bindingSlot,
		TextureAccess::READ_WRITE,
		InternalFormat::RG32F,
		0,
		false,
		0);

	mProgram->setImageLayerOfTexture(mdZUniform.location,
		dZ,
		mdZUniform.bindingSlot,
		TextureAccess::READ_WRITE,
		InternalFormat::RG32F,
		0,
		false,
		0);

	dispatch(mN, mN, 1);
}

nex::OceanGPU::WaterShading::WaterShading() : Shader(ShaderProgram::create("ocean/water_vs.glsl", "ocean/water_fs.glsl"))
{
	mInverseViewProjMatrix = { mProgram->getUniformLocation("inverseViewProjMatrix"), UniformType::MAT4 };
	transform = { mProgram->getUniformLocation("transform"), UniformType::MAT4 };
	modelViewUniform = { mProgram->getUniformLocation("modelViewMatrix"), UniformType::MAT4 };
	modelMatrixUniform = { mProgram->getUniformLocation("modelMatrix"), UniformType::MAT4 };
	lightUniform = { mProgram->getUniformLocation("lightDirViewSpace"), UniformType::VEC3 };
	normalMatrixUniform = { mProgram->getUniformLocation("normalMatrix"), UniformType::MAT3 };
	windDirection = { mProgram->getUniformLocation("windDirection"), UniformType::VEC2 };
	animationTime = { mProgram->getUniformLocation("animationTime"), UniformType::FLOAT };
	mTileSize = { mProgram->getUniformLocation("tileSize"), UniformType::FLOAT };

	heightUniform = { mProgram->getUniformLocation("height"), UniformType::TEXTURE2D, 0 };

	slopeXUniform = { mProgram->getUniformLocation("slopeX"), UniformType::TEXTURE2D, 1 };
	slopeZUniform = { mProgram->getUniformLocation("slopeZ"), UniformType::TEXTURE2D, 2 };
	dXUniform = { mProgram->getUniformLocation("dX"), UniformType::TEXTURE2D, 3 };
	dZUniform = { mProgram->getUniformLocation("dZ"), UniformType::TEXTURE2D, 4 };
	colorUniform = mProgram->createTextureUniform("colorMap", UniformType::TEXTURE2D, 5);
	luminanceUniform = mProgram->createTextureUniform("luminanceMap", UniformType::TEXTURE2D, 6);
	depthUniform = mProgram->createTextureUniform("depthMap", UniformType::TEXTURE2D, 7);

	cascadedDepthMap = mProgram->createTextureUniform("cascadedDepthMap", UniformType::TEXTURE2D_ARRAY, 8);

	mIrradiance = mProgram->createTextureUniform("irradianceMap", UniformType::TEXTURE2D, 9);
	mVoxelTexture = mProgram->createTextureUniform("voxelTexture", UniformType::TEXTURE3D, 10);
	mFoamTexture = mProgram->createTextureUniform("foamMap", UniformType::TEXTURE2D, 11);
	mProjHash = mProgram->createTextureUniform("projHashMap", UniformType::TEXTURE2D, 12);

	mCameraPosition = { mProgram->getUniformLocation("cameraPosition"), UniformType::VEC3 };
	mWaterLevel = { mProgram->getUniformLocation("waterLevel"), UniformType::FLOAT };

	sampler.setMinFilter(TexFilter::Linear);
	sampler.setMagFilter(TexFilter::Linear);
	sampler.setWrapR(UVTechnique::Repeat);
	sampler.setWrapS(UVTechnique::Repeat);
	sampler.setWrapT(UVTechnique::Repeat);
}

void nex::OceanGPU::WaterShading::setUniforms(const glm::mat4& projection, 
	const glm::mat4& view, 
	const glm::mat4& trafo, 
	const glm::mat4& inverseViewProjMatrix,
	const glm::vec3& lightDir,
	nex::CascadedShadow* cascadedShadow,
	Texture2D* height, Texture2D* slopeX, Texture2D* slopeZ, Texture2D* dX, Texture2D* dZ,
	Texture* color, 
	Texture* luminance,
	Texture* depth,
	Texture* irradiance,
	Texture* foam,
	Texture* projHash,
	GlobalIllumination* gi,
	const glm::vec3& cameraPosition,
	const glm::vec2& windDir,
	float time,
	float tileSize,
	float waterLevel)
{
	auto modelView = view * trafo;

	mProgram->setMat3(normalMatrixUniform.location, createNormalMatrix(modelView));
	mProgram->setMat4(transform.location, projection * view * trafo);
	mProgram->setMat4(modelViewUniform.location, modelView);
	mProgram->setMat4(modelMatrixUniform.location, trafo);
	mProgram->setMat4(mInverseViewProjMatrix.location, inverseViewProjMatrix);
	mProgram->setVec2(windDirection.location, windDir);
	mProgram->setFloat(animationTime.location, time);
	mProgram->setFloat(mTileSize.location, tileSize);
	mProgram->setVec3(mCameraPosition.location, cameraPosition);
	mProgram->setFloat(mWaterLevel.location, waterLevel);


	glm::vec3 lightDirViewSpace = glm::vec3(view * glm::vec4(lightDir, 0.0));
	mProgram->setVec3(lightUniform.location, normalize(lightDirViewSpace));

	mProgram->setTexture(height, &sampler, heightUniform.bindingSlot);
	mProgram->setTexture(slopeX, &sampler, slopeXUniform.bindingSlot);
	mProgram->setTexture(slopeZ, &sampler, slopeZUniform.bindingSlot);
	mProgram->setTexture(dX, &sampler, dXUniform.bindingSlot);
	mProgram->setTexture(dZ, &sampler, dZUniform.bindingSlot);

	mProgram->setTexture(color, Sampler::getLinearRepeat(), colorUniform.bindingSlot);
	mProgram->setTexture(luminance, &sampler, luminanceUniform.bindingSlot);
	mProgram->setTexture(depth, &sampler, depthUniform.bindingSlot);
	mProgram->setTexture(cascadedShadow->getDepthTextureArray(), Sampler::getPoint(), cascadedDepthMap.bindingSlot);
	mProgram->setTexture(irradiance, Sampler::getLinear(), mIrradiance.bindingSlot);
	mProgram->setTexture(gi->getVoxelTexture(), Sampler::getLinearMipMap(), mVoxelTexture.bindingSlot);
	mProgram->setTexture(foam, Sampler::getLinearRepeat(), mFoamTexture.bindingSlot);
	mProgram->setTexture(projHash, Sampler::getLinearRepeat(), mProjHash.bindingSlot);

	

	cascadedShadow->getCascadeBuffer()->bindToTarget(0);
	gi->getVoxelConstants()->bindToTarget(0);
}




nex::gui::OceanConfig::OceanConfig(Ocean* ocean) : mOcean(ocean)
{
}

void nex::gui::OceanConfig::drawSelf()
{
	ImGui::Checkbox("Ocean Wireframe", mOcean->getWireframeState());
}