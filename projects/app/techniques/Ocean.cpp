﻿#include "Ocean.hpp"
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
	mWireframe(true)
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


nex::OceanCpu::OceanCpu(unsigned N, unsigned maxWaveLength, float dimension, float spectrumScale,
	const glm::vec2& windDirection, float windSpeed, float periodTime) : 
Ocean(N, maxWaveLength, dimension, spectrumScale, windDirection, windSpeed, periodTime),
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

	VertexBuffer vertexBuffer;
	vertexBuffer.bind();
	vertexBuffer.resize(vertexCount * sizeof(VertexRender), mVerticesRender.data(), ShaderBuffer::UsageHint::DYNAMIC_DRAW);
	IndexBuffer indexBuffer(IndexElementType::BIT_32, static_cast<unsigned>(mIndices.size()), mIndices.data());
	indexBuffer.bind();

	VertexLayout layout;
	layout.push<glm::vec3>(1); // position
	layout.push<glm::vec3>(1); // normal

	VertexArray vertexArray;
	vertexArray.bind();
	vertexArray.useBuffer(vertexBuffer, layout);

	vertexArray.unbind();
	indexBuffer.unbind();


	//TODO
	AABB boundingBox;
	//boundingBox.min = glm::vec3(0.0f);
	//boundingBox.max = glm::vec3(0.0f);

	mMesh = std::make_unique<Mesh>();
	mMesh->init(std::move(vertexBuffer), std::move(layout), std::move(indexBuffer), std::move(boundingBox), Topology::TRIANGLES);
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

	mMesh->getVertexBuffer()->resize(sizeof(VertexRender) * mVerticesRender.size(), mVerticesRender.data(), ShaderBuffer::UsageHint::DYNAMIC_DRAW);

	// Only draw the first triangle
	RenderBackend::get()->drawWithIndices(state, Topology::TRIANGLES, mMesh->getIndexBuffer()->getCount(), mMesh->getIndexBuffer()->getType());
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


nex::OceanCpu::SimpleShadedPass::SimpleShadedPass() : Pass(Shader::create("ocean/simple_shaded_vs.glsl", "ocean/simple_shaded_fs.glsl"))
{
	transform = { mShader->getUniformLocation("transform"), UniformType::MAT4 };
	lightUniform = { mShader->getUniformLocation("lightDirViewSpace"), UniformType::VEC3 };
	normalMatrixUniform = { mShader->getUniformLocation("normalMatrix"), UniformType::MAT3 };
}

void nex::OceanCpu::SimpleShadedPass::setUniforms(const glm::mat4& projection, const glm::mat4& view, 
	const glm::mat4& trafo, const glm::vec3& lightDir)
{
	auto modelView = view * trafo;

	mShader->setMat3(normalMatrixUniform.location, createNormalMatrix(modelView));
	mShader->setMat4(transform.location, projection * view * trafo);


	glm::vec3 lightDirViewSpace = glm::vec3(view * glm::vec4(lightDir, 0.0));
	mShader->setVec3(lightUniform.location, normalize(lightDirViewSpace));
}


nex::OceanCpuDFT::OceanCpuDFT(unsigned N, unsigned maxWaveLength, float dimension, float spectrumScale,
	const glm::vec2& windDirection, float windSpeed, float periodTime) : 
OceanCpu(N, maxWaveLength, dimension, spectrumScale, windDirection, windSpeed, periodTime)
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
	float spectrumScale,
	const glm::vec2& windDirection,
	float windSpeed,
	float periodTime) : OceanCpu(N, maxWaveLength, dimension, spectrumScale, windDirection, windSpeed, periodTime),
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

nex::OceanGPU::OceanGPU(unsigned N, unsigned maxWaveLength, float dimension,
	float spectrumScale, const glm::vec2& windDirection, float windSpeed, float periodTime) :
	Ocean(N, maxWaveLength, dimension, spectrumScale, windDirection, windSpeed, periodTime),
	mHeightZeroComputePass(std::make_unique<HeightZeroComputePass>(glm::uvec2(mN), glm::vec2(mN), mWindDirection, mSpectrumScale, mWindSpeed)), // mUniquePointCount.x, mUniquePointCount.y
	mHeightComputePass(std::make_unique<HeightComputePass>(glm::uvec2(mN), glm::vec2(mN), mPeriodTime)),
	mButterflyComputePass(std::make_unique<ButterflyComputePass>(mN)),
	mIfftComputePass(std::make_unique<IfftPass>(mN)),
	mNormalizePermutatePass(std::make_unique<NormalizePermutatePass>(mN)),
	mSimpleShadedPass(std::make_unique<SimpleShadedPass>())
{
	mHeightZeroComputePass->compute();
	
	computeButterflyTexture();

	testHeightGeneration();

	generateMesh();
}

nex::OceanGPU::~OceanGPU() = default;

void nex::OceanGPU::draw(const glm::mat4& projection, const glm::mat4& view, const glm::vec3& lightDir)
{
	mSimpleShadedPass->bind();
	glm::mat4 model;
	model = translate(model, glm::vec3(0, 2, -1));
	model = scale(model, glm::vec3(1 / (float)mWaveLength));

	mSimpleShadedPass->setUniforms(projection, view, model, lightDir, 
		mHeightComputePass->getHeight(),
		mHeightComputePass->getSlopeX(),
		mHeightComputePass->getSlopeZ(),
		mHeightComputePass->getDx(),
		mHeightComputePass->getDz());

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

	mMesh->getVertexBuffer()->resize(sizeof(Vertex) * mVertices.size(), mVertices.data(), ShaderBuffer::UsageHint::DYNAMIC_DRAW);

	// Only draw the first triangle
	RenderBackend::get()->drawWithIndices(state, Topology::TRIANGLES, mMesh->getIndexBuffer()->getCount(), mMesh->getIndexBuffer()->getType());
}

void nex::OceanGPU::simulate(float t)
{
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

	VertexBuffer vertexBuffer;
	vertexBuffer.bind();
	vertexBuffer.resize(vertexCount * sizeof(Vertex), mVertices.data(), ShaderBuffer::UsageHint::DYNAMIC_DRAW);
	IndexBuffer indexBuffer(IndexElementType::BIT_32, static_cast<unsigned>(mIndices.size()), mIndices.data());
	indexBuffer.unbind();

	VertexLayout layout;
	layout.push<glm::vec3>(1); // position
	layout.push<glm::vec2>(1); // texCoords

	//TODO
	AABB boundingBox;
	boundingBox.min = glm::vec3(0.0f);
	boundingBox.max = glm::vec3(0.0f);

	mMesh = std::make_unique<Mesh>();
	mMesh->init(std::move(vertexBuffer), std::move(layout), std::move(indexBuffer), std::move(boundingBox), Topology::TRIANGLES);
	mMesh->finalize();
}


nex::OceanGPU::HeightZeroComputePass::HeightZeroComputePass(const glm::uvec2& uniquePointCount, const glm::vec2& waveLength, const glm::vec2& windDirection,
	float spectrumScale, float windSpeed) :
	ComputePass(Shader::createComputeShader("ocean/height_zero_precompute_cs.glsl")),
	mUniquePointCount(uniquePointCount), mWaveLength(waveLength), mWindDirection(windDirection), mSpectrumScale(spectrumScale), mWindSpeed(windSpeed)
{
	TextureDesc desc;
	desc.internalFormat = InternFormat::RGBA32F;
	desc.colorspace = ColorSpace::RGBA;
	desc.pixelDataType = PixelDataType::FLOAT;
	desc.generateMipMaps = false;
	desc.magFilter = desc.minFilter = TextureFilter::NearestNeighbor;
	mHeightZero = std::make_unique<Texture2D>(mUniquePointCount.x, mUniquePointCount.y, desc, nullptr);

	mResultTexture = { mShader->getUniformLocation("result"), UniformType::IMAGE2D, 0 };

	mUniquePointCountUniform = { mShader->getUniformLocation("uniquePointCount"), UniformType::UVEC2 };
	mWaveLengthUniform = { mShader->getUniformLocation("waveLength"), UniformType::VEC2 };
	mWindDirectionUniform = { mShader->getUniformLocation("windDirection"), UniformType::VEC2 };
	mSpectrumScaleUniform = { mShader->getUniformLocation("spectrumScale"), UniformType::FLOAT };
	mWindSpeedUniform = { mShader->getUniformLocation("windSpeed"), UniformType::FLOAT };

	mShader->bind();

	mShader->setUVec2(mUniquePointCountUniform.location, mUniquePointCount);
	mShader->setVec2(mWaveLengthUniform.location, mWaveLength);
	mShader->setVec2(mWindDirectionUniform.location, mWindDirection);
	mShader->setFloat(mSpectrumScaleUniform.location, mSpectrumScale);
	mShader->setFloat(mWindSpeedUniform.location, mWindSpeed);


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
	randDesc.internalFormat = InternFormat::RGBA32F;
	randDesc.colorspace = ColorSpace::RGBA;
	randDesc.pixelDataType = PixelDataType::FLOAT;
	randDesc.generateMipMaps = false;
	randDesc.magFilter = randDesc.minFilter = TextureFilter::NearestNeighbor;

	mRandNormalDistributed = std::make_unique<Texture2D>(mUniquePointCount.x, mUniquePointCount.y, randDesc, randValues.data());
	mRandTextureUniform = { mShader->getUniformLocation("randTexture"), UniformType::TEXTURE2D, 1 };
}

void nex::OceanGPU::HeightZeroComputePass::compute()
{
	mShader->bind();

	mShader->setImageLayerOfTexture(mResultTexture.location,
		mHeightZero.get(),
		mResultTexture.bindingSlot,
		TextureAccess::READ_WRITE,
		InternFormat::RGBA32F,
		0,
		false,
		0);

	mShader->setImageLayerOfTexture(mRandTextureUniform.location,
		mRandNormalDistributed.get(),
		mRandTextureUniform.bindingSlot,
		TextureAccess::READ_ONLY,
		InternFormat::RGBA32F,
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
	ComputePass(Shader::createComputeShader("ocean/height_cs.glsl")),
	mUniquePointCount(uniquePointCount)
{
	mUniquePointCountUniform = { mShader->getUniformLocation("uniquePointCount"), UniformType::UVEC2 };
	mWaveLengthUniform = { mShader->getUniformLocation("waveLength"), UniformType::VEC2 };

	mResultHeightTextureUniform = { mShader->getUniformLocation("resultHeight"), UniformType::IMAGE2D, 0 };
	mResultSlopeXTextureUniform = { mShader->getUniformLocation("resultSlopeX"), UniformType::IMAGE2D, 1 };
	mResultSlopeZTextureUniform = { mShader->getUniformLocation("resultSlopeZ"), UniformType::IMAGE2D, 2 };
	mResultDxTextureUniform = { mShader->getUniformLocation("resultDx"), UniformType::IMAGE2D, 3 };
	mResultDzTextureUniform = { mShader->getUniformLocation("resultDz"), UniformType::IMAGE2D, 4 };

	mHeightZeroTextureUniform = { mShader->getUniformLocation("heightZero"), UniformType::IMAGE2D, 5 };

	mTimeUniform = { mShader->getUniformLocation("currentTime"), UniformType::FLOAT };
	mPeriodTimeUniform = { mShader->getUniformLocation("periodTime"), UniformType::FLOAT };

	TextureDesc desc;
	desc.internalFormat = InternFormat::RG32F;
	desc.colorspace = ColorSpace::RG;
	desc.pixelDataType = PixelDataType::FLOAT;
	desc.generateMipMaps = false;
	desc.magFilter = desc.minFilter = TextureFilter::NearestNeighbor;
	mHeight = std::make_unique<Texture2D>(mUniquePointCount.x, mUniquePointCount.y, desc, nullptr);
	mHeightSlopeX = std::make_unique<Texture2D>(mUniquePointCount.x, mUniquePointCount.y, desc, nullptr);
	mHeightSlopeZ = std::make_unique<Texture2D>(mUniquePointCount.x, mUniquePointCount.y, desc, nullptr);
	mHeightDx = std::make_unique<Texture2D>(mUniquePointCount.x, mUniquePointCount.y, desc, nullptr);
	mHeightDz = std::make_unique<Texture2D>(mUniquePointCount.x, mUniquePointCount.y, desc, nullptr);

	mShader->bind();
	mShader->setUVec2(mUniquePointCountUniform.location, uniquePointCount);
	mShader->setVec2(mWaveLengthUniform.location, waveLength);
	mShader->setFloat(mPeriodTimeUniform.location, periodTime);
}

void nex::OceanGPU::HeightComputePass::compute(float time, Texture2D* heightZero)
{
	// Ensure that the updated content of heightZero is visible
	RenderBackend::get()->syncMemoryWithGPU((MemorySync)(MemorySync_ShaderImageAccess | MemorySync_TextureUpdate));
	mShader->bind();
	mShader->setFloat(mTimeUniform.location, time);

	mShader->setImageLayerOfTexture(mResultHeightTextureUniform.location,
		mHeight.get(),
		mResultHeightTextureUniform.bindingSlot,
		TextureAccess::READ_WRITE,
		InternFormat::RG32F,
		0,
		false,
		0);

	mShader->setImageLayerOfTexture(mResultSlopeXTextureUniform.location,
		mHeightSlopeX.get(),
		mResultSlopeXTextureUniform.bindingSlot,
		TextureAccess::WRITE_ONLY,
		InternFormat::RG32F,
		0,
		false,
		0);

	mShader->setImageLayerOfTexture(mResultSlopeZTextureUniform.location,
		mHeightSlopeZ.get(),
		mResultSlopeZTextureUniform.bindingSlot,
		TextureAccess::WRITE_ONLY,
		InternFormat::RG32F,
		0,
		false,
		0);

	mShader->setImageLayerOfTexture(mResultDxTextureUniform.location,
		mHeightDx.get(),
		mResultDxTextureUniform.bindingSlot,
		TextureAccess::WRITE_ONLY,
		InternFormat::RG32F,
		0,
		false,
		0);

	mShader->setImageLayerOfTexture(mResultDzTextureUniform.location,
		mHeightDz.get(),
		mResultDzTextureUniform.bindingSlot,
		TextureAccess::WRITE_ONLY,
		InternFormat::RG32F,
		0,
		false,
		0);

	mShader->setImageLayerOfTexture(mHeightZeroTextureUniform.location,
		heightZero,
		mHeightZeroTextureUniform.bindingSlot,
		TextureAccess::READ_WRITE,
		InternFormat::RGBA32F,
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


nex::OceanGPU::ButterflyComputePass::ButterflyComputePass(unsigned N) : ComputePass(Shader::createComputeShader("ocean/butterfly_cs.glsl")),
mN(N)
{
	if (!nex::isPow2(mN)) throw std::invalid_argument("nex::Ocean::ButterflyComputePass : N has to be a power of 2!");

	TextureDesc desc;
	desc.internalFormat = InternFormat::RGBA32F;
	desc.colorspace = ColorSpace::RGBA;
	desc.pixelDataType = PixelDataType::FLOAT;
	desc.generateMipMaps = false;
	desc.magFilter = desc.minFilter = TextureFilter::NearestNeighbor;
	mButterfly = std::make_unique<Texture2D>(mN, static_cast<unsigned>(std::log2(mN)), desc, nullptr);

	mButterflyUniform = { mShader->getUniformLocation("butterfly"), UniformType::IMAGE2D, 0 };
	mNUniform = { mShader->getUniformLocation("N"), UniformType::INT };

	mShader->bind();
	mShader->setInt(mNUniform.location, mN);
}

void nex::OceanGPU::ButterflyComputePass::compute()
{
	mShader->bind();

	mShader->setImageLayerOfTexture(mButterflyUniform.location,
		mButterfly.get(),
		mButterflyUniform.bindingSlot,
		TextureAccess::WRITE_ONLY,
		InternFormat::RGBA32F,
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

nex::OceanGPU::IfftPass::IfftPass(int N) : ComputePass(Shader::createComputeShader("ocean/ifft_cs.glsl")),
mBlit(std::make_unique<ComputePass>(nex::Shader::createComputeShader("ocean/blit_cs.glsl"))), mN(N), mLog2N(std::log2(mN))

{
	TextureDesc desc;
	desc.internalFormat = InternFormat::RG32F;
	desc.colorspace = ColorSpace::RG;
	desc.pixelDataType = PixelDataType::FLOAT;
	mPingPong = std::make_unique<Texture2D>(N, N, desc, nullptr);


	mNUniform = { mShader->getUniformLocation("N"), UniformType::INT };
	mStageUniform = { mShader->getUniformLocation("stage"), UniformType::INT };
	mVerticalUniform = { mShader->getUniformLocation("vertical"), UniformType::INT };
	mInputUniform = { mShader->getUniformLocation("inputImage"), UniformType::IMAGE2D, 0 };
	mButterflyUniform = { mShader->getUniformLocation("butterfly"), UniformType::IMAGE2D, 1 };
	mOutputUniform = { mShader->getUniformLocation("outputImage"), UniformType::IMAGE2D, 2 };


	mShader->bind();
	mShader->setInt(mNUniform.location, mN);

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
	mShader->setImageLayerOfTexture(mButterflyUniform.location,
		butterfly,
		mButterflyUniform.bindingSlot,
		TextureAccess::READ_ONLY,
		InternFormat::RGBA32F,
		0,
		false,
		0);
}

void nex::OceanGPU::IfftPass::setInput(Texture2D* input)
{
	mShader->setImageLayerOfTexture(mInputUniform.location,
		input,
		mInputUniform.bindingSlot,
		TextureAccess::READ_ONLY,
		InternFormat::RG32F,
		0,
		false,
		0);
}

void nex::OceanGPU::IfftPass::setOutput(Texture2D* output)
{
	mShader->setImageLayerOfTexture(mOutputUniform.location,
		output,
		mOutputUniform.bindingSlot,
		TextureAccess::WRITE_ONLY,
		InternFormat::RG32F,
		0,
		false,
		0);
}

void nex::OceanGPU::IfftPass::setStage(int stage)
{
	mShader->setInt(mStageUniform.location, stage);
}

void nex::OceanGPU::IfftPass::setVertical(bool vertical)
{
	mShader->setInt(mVerticalUniform.location, (int)vertical);
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
			InternFormat::RG32F,
			0,
			false,
			0);

		blitShader->setImageLayerOfTexture(mBlitDestUniform.location,
			input,
			mBlitDestUniform.bindingSlot,
			TextureAccess::WRITE_ONLY,
			InternFormat::RG32F,
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
ComputePass(Shader::createComputeShader("ocean/normalize_permutate_cs.glsl")),
mN(N)
{
	mNUniform = { mShader->getUniformLocation("N"), UniformType::INT };
	mHeightUniform = { mShader->getUniformLocation("height"), UniformType::IMAGE2D, 0 };
	mSlopeXUniform = { mShader->getUniformLocation("slopeX"), UniformType::IMAGE2D, 1 };
	mSlopeZUniform = { mShader->getUniformLocation("slopeZ"), UniformType::IMAGE2D, 2 };
	mdXUniform = { mShader->getUniformLocation("dX"), UniformType::IMAGE2D, 3 };
	mdZUniform = { mShader->getUniformLocation("dZ"), UniformType::IMAGE2D, 4 };

	mShader->bind();
	mShader->setInt(mNUniform.location, mN);
}

void nex::OceanGPU::NormalizePermutatePass::compute(Texture2D* height, Texture2D* slopeX, Texture2D* slopeZ, Texture2D* dX, Texture2D* dZ)
{
	bind();

	mShader->setImageLayerOfTexture(mHeightUniform.location,
		height,
		mHeightUniform.bindingSlot,
		TextureAccess::READ_WRITE,
		InternFormat::RG32F,
		0,
		false,
		0);

	mShader->setImageLayerOfTexture(mSlopeXUniform.location,
		slopeX,
		mSlopeXUniform.bindingSlot,
		TextureAccess::READ_WRITE,
		InternFormat::RG32F,
		0,
		false,
		0);

	mShader->setImageLayerOfTexture(mSlopeZUniform.location,
		slopeZ,
		mSlopeZUniform.bindingSlot,
		TextureAccess::READ_WRITE,
		InternFormat::RG32F,
		0,
		false,
		0);

	mShader->setImageLayerOfTexture(mdXUniform.location,
		dX,
		mdXUniform.bindingSlot,
		TextureAccess::READ_WRITE,
		InternFormat::RG32F,
		0,
		false,
		0);

	mShader->setImageLayerOfTexture(mdZUniform.location,
		dZ,
		mdZUniform.bindingSlot,
		TextureAccess::READ_WRITE,
		InternFormat::RG32F,
		0,
		false,
		0);

	dispatch(mN, mN, 1);
}

nex::OceanGPU::SimpleShadedPass::SimpleShadedPass() : Pass(Shader::create("ocean/simple_shaded_gpu_vs.glsl", "ocean/simple_shaded_gpu_fs.glsl"))
{
	transform = { mShader->getUniformLocation("transform"), UniformType::MAT4 };
	modelViewUniform = { mShader->getUniformLocation("modelViewMatrix"), UniformType::MAT4 };
	lightUniform = { mShader->getUniformLocation("lightDirViewSpace"), UniformType::VEC3 };
	normalMatrixUniform = { mShader->getUniformLocation("normalMatrix"), UniformType::MAT3 };

	heightUniform = { mShader->getUniformLocation("height"), UniformType::TEXTURE2D, 0 };

	slopeXUniform = { mShader->getUniformLocation("slopeX"), UniformType::TEXTURE2D, 1 };
	slopeZUniform = { mShader->getUniformLocation("slopeZ"), UniformType::TEXTURE2D, 2 };
	dXUniform = { mShader->getUniformLocation("dX"), UniformType::TEXTURE2D, 3 };
	dZUniform = { mShader->getUniformLocation("dZ"), UniformType::TEXTURE2D, 4 };

	sampler.setMinFilter(TextureFilter::NearestNeighbor);
	sampler.setMagFilter(TextureFilter::NearestNeighbor);
	sampler.setWrapR(TextureUVTechnique::Repeat);
	sampler.setWrapS(TextureUVTechnique::Repeat);
	sampler.setWrapT(TextureUVTechnique::Repeat);
}

void nex::OceanGPU::SimpleShadedPass::setUniforms(const glm::mat4& projection, const glm::mat4& view, 
	const glm::mat4& trafo, const glm::vec3& lightDir,
	Texture2D* height, Texture2D* slopeX, Texture2D* slopeZ, Texture2D* dX, Texture2D* dZ)
{
	auto modelView = view * trafo;

	mShader->setMat3(normalMatrixUniform.location, createNormalMatrix(modelView));
	mShader->setMat4(transform.location, projection * view * trafo);
	mShader->setMat4(modelViewUniform.location, modelView);


	glm::vec3 lightDirViewSpace = glm::vec3(view * glm::vec4(lightDir, 0.0));
	mShader->setVec3(lightUniform.location, normalize(lightDirViewSpace));

	mShader->setTexture(height, &sampler, heightUniform.bindingSlot);
	mShader->setTexture(slopeX, &sampler, slopeXUniform.bindingSlot);
	mShader->setTexture(slopeZ, &sampler, slopeZUniform.bindingSlot);
	mShader->setTexture(dX, &sampler, dXUniform.bindingSlot);
	mShader->setTexture(dZ, &sampler, dZUniform.bindingSlot);
}




nex::gui::OceanConfig::OceanConfig(Ocean* ocean) : mOcean(ocean)
{
}

void nex::gui::OceanConfig::drawSelf()
{
	ImGui::Checkbox("Ocean Wireframe", mOcean->getWireframeState());
}