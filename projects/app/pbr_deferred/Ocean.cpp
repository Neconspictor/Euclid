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


nex::OceanFFT::OceanFFT(unsigned N) : N(N)
{
	log_2_N = log2(N);

	// bit reversal precomputation
	reversed.resize(N);
	for (int i = 0; i < reversed.size(); ++i) reversed[i] = reverse(i);

	// prepare twiddle factors
	int pow2 = 1;
	T.resize(log_2_N);
	for (int i = 0; i < T.size(); ++i)
	{
		T[i].resize(pow2);
		for (int j = 0; j < pow2; ++j) T[i][j] = twiddle(j, pow2*2);
		pow2 *= 2;
	}

	c[0].resize(N);
	c[1].resize(N);
	which = 0;
}

unsigned nex::OceanFFT::reverse(unsigned i) const
{
	// Find the max number for bit reversing. All bits higher than this number will be zeroed.
	// the max number is determined by 2^bitCount - 1.
	const unsigned maxNumber = (1 << log_2_N) - 1;
	// Initialize the reversed number 
	unsigned reversedN = i;

	// Shift n to the right, reversed n to the left, 
	// and give least-significant bit of n to reversed n.
	// Do this process as long as n is greater zero.
	// Technically we have to do this process bitCount times
	// But we can save some loops by ignoring shifts if n is zero and 
	// just left shift reversed n by the remaining bits.
	// Therefore we need the remainingBits variable.
	unsigned remainingBits = log_2_N - 1;
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

nex::Complex nex::OceanFFT::twiddle(unsigned x, unsigned N)
{
	return Complex(cos(pi2 * x / N), sin(pi2 * x / N));
}

void nex::OceanFFT::fft(nex::Complex* input, nex::Complex* output, int stride, int offset, bool vertical)
{
	for (int i = 0; i < N; i++) c[which][i] = input[reversed[i] * stride + offset];

	int loops = N >> 1;
	int size = 1 << 1;
	int size_over_2 = 1;
	int w_ = 0;
	for (int i = 1; i <= log_2_N; i++) {
		which ^= 1;
		for (int j = 0; j < loops; j++) {
			for (int k = 0; k < size_over_2; k++) {
				c[which][size * j + k] = c[which ^ 1][size * j + k] +
					c[which ^ 1][size * j + size_over_2 + k] * T[w_][k];
			}

			for (int k = size_over_2; k < size; k++) {
				c[which][size * j + k] = c[which ^ 1][size * j - size_over_2 + k] -
					c[which ^ 1][size * j + k] * T[w_][k - size_over_2];
			}
		}
		loops >>= 1;
		size <<= 1;
		size_over_2 <<= 1;
		w_++;
	}

	for (unsigned k = 0; k < N / 2; ++k)
	{
		//std::swap(x[reverse(k)], x[reverse(k + N / 2)]);
		std::swap(c[which][k], c[which][k + N / 2]);
	}

	if (vertical)
	{
		for (unsigned k = 1; k < N / 2; ++k)
		{
			//std::swap(x[reverse(k)], x[reverse(N - k)]);
			std::swap(c[which][k], c[which][N - k]);
		}
	}

	for (int i = 0; i < N; i++) output[i * stride + offset] = c[which][i];
}

void nex::OceanFFT::fft(const nex::Iterator2D& input, nex::Iterator2D& output, bool vertical)
{
	for (int i = 0; i < N; i++) c[which][i] = input[reversed[i]];

	int loops = N >> 1;
	int size = 1 << 1;
	int size_over_2 = 1;
	int w_ = 0;
	for (int i = 1; i <= log_2_N; i++) {
		which ^= 1;
		for (int j = 0; j < loops; j++) {
			for (int k = 0; k < size_over_2; k++) {
				c[which][size * j + k] = c[which ^ 1][size * j + k] +
					c[which ^ 1][size * j + size_over_2 + k] * T[w_][k];
			}

			for (int k = size_over_2; k < size; k++) {
				c[which][size * j + k] = c[which ^ 1][size * j - size_over_2 + k] -
					c[which ^ 1][size * j + k] * T[w_][k - size_over_2];
			}
		}
		loops >>= 1;
		size <<= 1;
		size_over_2 <<= 1;
		w_++;
	}

	/*for (unsigned k = 0; k < N / 2; ++k)
	{
		//std::swap(x[reverse(k)], x[reverse(k + N / 2)]);
		std::swap(c[which][k], c[which][k + N / 2]);
	}

	if (vertical)
	{
		for (unsigned k = 1; k < N / 2; ++k)
		{
			//std::swap(x[reverse(k)], x[reverse(N - k)]);
			std::swap(c[which][k], c[which][N - k]);
		}
	}*/


	for (int i = 0; i < N; i++) output[i] = c[which][i];
}

void nex::OceanFFT::fftInPlace(std::vector<nex::Complex>& x, bool inverse)
{
	/* Do the bit reversal */
	{
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
	}


	/**
	 * Do FFT
	 */
	Complex c(-1.0, 0.0);
	unsigned l2 = 1;
	for (unsigned l = 0; l < log_2_N; l++)
	{
		unsigned l1 = l2;
		l2 <<= 1;
		nex::Complex u(1.0, 0.0);

		for (unsigned j = 0; j < l1; j++)
		{
			for (unsigned i = j; i < N; i += l2)
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
		for (unsigned i = 0; i < N; i++)
			x[i] /= N;
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


nex::Ocean::Ocean(const glm::uvec2& pointCount,
	const glm::vec2& maxWaveLength,
	const glm::vec2& dimension,
	float spectrumScale,
	const glm::vec2& windDirection,
	float windSpeed,
	float periodTime) :
	mUniquePointCount(pointCount - glm::uvec2(1, 1)),
	N(mUniquePointCount.x),
	mTildePointCount(pointCount),
	mWaveLength(maxWaveLength),
	mDimension(dimension),
	mSpectrumScale(spectrumScale),
	mWindDirection(glm::normalize(windDirection)),
	mWindSpeed(windSpeed),
	mPeriodTime(periodTime),
	mSimpleShadedPass(std::make_unique<SimpleShadedPass>()),
	mWireframe(true),
	mHeightZeroComputePass(std::make_unique<HeightZeroComputePass>(glm::uvec2(N), glm::vec2(N), mWindDirection, mSpectrumScale, mWindSpeed)), // mUniquePointCount.x, mUniquePointCount.y
	mHeightComputePass(std::make_unique<HeightComputePass>(glm::uvec2(N), glm::vec2(N), mPeriodTime)),
	mButterflyComputePass(std::make_unique<ButterflyComputePass>(N)),
	mIfftComputePass(std::make_unique<IfftPass>(N)),
	fft(N)
{
	assert(pointCount.x >= 2);
	assert(pointCount.y >= 2);
	assert(dimension.x > 0.0f);
	assert(dimension.y > 0.0f);
	assert(spectrumScale > 0.0f);
	assert(length(windDirection) > 0.0f);
	assert(periodTime > 0.0f);

	const float twoPi = 2.0f * util::PI;
	const float pi = util::PI;

	

	h_tilde.resize(N * N);
	h_tilde_slopex.resize(N * N);
	h_tilde_slopez.resize(N * N);
	h_tilde_dx.resize(N * N);
	h_tilde_dz.resize(N * N);


	const auto vertexCount = mTildePointCount.x * mTildePointCount.y;
	const auto quadCount = (mTildePointCount.x - 1) * (mTildePointCount.y - 1);
	
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
				(x - mUniquePointCount.x / 2.0f) * mWaveLength.x / (float)mUniquePointCount.x,
				0.0f,
				getZValue((z - mUniquePointCount.y / 2.0f) * mWaveLength.y / (float)mUniquePointCount.y)
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


	//simulate(10.0f);
	simulateFFT(10.0f, true);

	VertexBuffer vertexBuffer;
	vertexBuffer.bind();
	vertexBuffer.fill(mVerticesRender.data(), vertexCount * sizeof(VertexRender), ShaderBuffer::UsageHint::DYNAMIC_DRAW);
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






	std::vector<glm::vec2> test(N*N);

	for (int z = 0; z < N; ++z)
	{
		for (int x = 0; x < N; ++x)
		{
			unsigned index = z * N + x;

			const float kx = (twoPi * x - pi * N) / (float)N;
			const float kz = (twoPi * z - pi * N) / (float)N;
			auto height = heightZero(glm::vec2(kx, kz));
			test[index] = glm::vec2(height.re, height.im);
		}
	}

	std::vector<glm::vec4> heightZeros(N*N);
	//mHeightZeroComputePass->getResult()->readback(0, ColorSpace::RGBA, PixelDataType::FLOAT, heightZeros.data(), heightZeros.size() * sizeof(glm::vec4));

	mHeightZeroComputePass->compute();
	mHeightComputePass->compute(10.0f, mHeightZeroComputePass->getResult());
	//RenderBackend::get()->sync
	std::vector<glm::vec2> height(N*N);
	std::vector<glm::vec2> slopeX(N*N);
	std::vector<glm::vec2> slopeZ(N*N);
	std::vector<glm::vec2> dx(N*N);
	std::vector<glm::vec2> dz(N*N);
	mHeightZeroComputePass->getResult()->readback(0, ColorSpace::RGBA, PixelDataType::FLOAT, heightZeros.data(), heightZeros.size() * sizeof(glm::vec4));
	mHeightComputePass->getHeight()->readback(0, ColorSpace::RG, PixelDataType::FLOAT, height.data(), height.size() * sizeof(glm::vec2));
	mHeightComputePass->getSlopeX()->readback(0, ColorSpace::RG, PixelDataType::FLOAT, slopeX.data(), slopeX.size() * sizeof(glm::vec2));
	mHeightComputePass->getSlopeZ()->readback(0, ColorSpace::RG, PixelDataType::FLOAT, slopeZ.data(), slopeZ.size() * sizeof(glm::vec2));
	mHeightComputePass->getDx()->readback(0, ColorSpace::RG, PixelDataType::FLOAT, dx.data(), dx.size() * sizeof(glm::vec2));
	mHeightComputePass->getDz()->readback(0, ColorSpace::RG, PixelDataType::FLOAT, dz.data(), dz.size() * sizeof(glm::vec2));


	mButterflyComputePass->compute();
	std::vector<glm::vec4> butterfly(N * std::log2(N));

	GenericImage genericImage;
	genericImage.width = N;
	genericImage.height = std::log2(N);
	genericImage.components = 3;
	genericImage.format = (unsigned)InternFormat::RGB32F;
	genericImage.pixelSize = sizeof(glm::vec3);
	genericImage.pixels.resize(N * std::log2(N) * sizeof(glm::vec3));

	std::vector<glm::vec4> butterflyImage(N * std::log2(N));
	std::vector<byte> butterflyOut(N * std::log2(N) * 3);


	mButterflyComputePass->getButterfly()->
		readback(0, ColorSpace::RGBA, PixelDataType::FLOAT, butterflyImage.data(), butterflyImage.size() * sizeof(glm::vec4));


	for (unsigned i = 0; i < butterflyImage.size(); ++i)
	{
		const auto& pixel = butterflyImage[i];
		char* memory = &genericImage.pixels[i * sizeof(glm::vec3)];
		const auto source = 127.5f + (255.0f * glm::vec3(pixel)) / 2.0f;

		const auto source2 = 0.5f + glm::vec3(pixel) * 0.5f;
		memcpy_s(memory, sizeof(glm::vec3), (char*)&source2, sizeof(glm::vec3));
		byte r = static_cast<byte>(source.x);
		byte g = static_cast<byte>(source.y);
		byte b = static_cast<byte>(pixel.z);
		butterflyOut[i * 3] = r;
		butterflyOut[i * 3 + 1] = g;
		butterflyOut[i * 3 + 2] = b;
	}

	ImageFactory::writeToPNG("./oceanTest/butterfly.png", (const char*)butterflyOut.data(), N, std::log2(N), 3, N * 3, false);
	ImageFactory::writeHDR(genericImage, "./oceanTest/butterfly.hdr", false);

	auto* heightFFT = mHeightComputePass->getHeight();
	auto* dxFFT = mHeightComputePass->getDx();
	auto* dzFFT = mHeightComputePass->getDz();
	auto* slopeXFFT = mHeightComputePass->getSlopeX();
	auto* slopeZFFT = mHeightComputePass->getSlopeZ();

	mIfftComputePass->bind();
	mIfftComputePass->setButterfly(mButterflyComputePass->getButterfly());

	// horizontal 1D iFFT
	mIfftComputePass->setVertical(false);
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
	mIfftComputePass->setVertical(true);
	mIfftComputePass->computeAllStages(heightFFT);
	//mIfftComputePass->computeAllStages(slopeXFFT);
	//mIfftComputePass->computeAllStages(slopeZFFT);
	//mIfftComputePass->computeAllStages(dxFFT);
	//mIfftComputePass->computeAllStages(dzFFT);


	heightFFT->readback(0, ColorSpace::RG, PixelDataType::FLOAT, height.data(), height.size() * sizeof(glm::vec2));

	bool t = true;
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
			
			
			/*auto test = -twoPi / (float)(mUniquePointCount.x * mUniquePointCount.y);
			const auto index = (z - mUniquePointCount.x / 2.0f)* mUniquePointCount.x + (x - mUniquePointCount.x / 2.0f);

			//
			// * mVerticesCompute[index].originalPosition = glm::vec3(
			//	(x - mUniquePointCount.x / 2.0f) * mWaveLength.x / mUniquePointCount.x,
			//	0.0f,
			//	getZValue((z - mUniquePointCount.y / 2.0f) * mWaveLength.y / mUniquePointCount.y)
			//);
			 
			const auto index2X = (locationXZ.x * mUniquePointCount.x) / (float)mWaveLength.x; //+ (mUniquePointCount.x / 2.0f);
			const auto index2Z = ((-locationXZ.y) * mUniquePointCount.y) / (float)mWaveLength.y;// +(mUniquePointCount.y / 2.0f);

			test *= index * (index2Z * mUniquePointCount.x + index2X);*/
			
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
	glm::mat4 model;
	model = translate(model, glm::vec3(0,2, -1));
	model = scale(model, glm::vec3(1 / mWaveLength.x));

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

	mMesh->getVertexBuffer()->fill(mVerticesRender.data(), sizeof(VertexRender) * mVerticesRender.size(), ShaderBuffer::UsageHint::DYNAMIC_DRAW);

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
	return nex::Complex(1.0, 1.0f);
	//return nex::Complex(1.0 - abs(x1 * w)/10000.0f, 1.0 - abs(x2 * w)/10000.0);
	//return nex::Complex(x1 * w, x2 * w);

	static std::random_device device;
	static std::mt19937 engine(device());

	// note: we need mean 0 and standard deviation 1!
	static std::normal_distribution<float> distribution(0, 1);

	return { distribution(engine), distribution(engine) };
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
	Complex c1(-cos_, -sin_);


	const auto index = z * mTildePointCount.y + x;
	const auto& vertex = mVerticesCompute[index];

	return vertex.height0 * c0//* Complex::euler(w * time)
		+ vertex.height0NegativeWaveConjugate * c1; //* Complex::euler(-w * time);
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

bool* nex::Ocean::getWireframeState()
{
	return &mWireframe;
}

void nex::Ocean::simulate(float t)
{
	const float displacementDirectionScale = 0.0;

	for (int z = 0; z < mUniquePointCount.y; z++) {
		for (int x = 0; x < mUniquePointCount.x; x++) {
			auto index = z * mTildePointCount.x + x;


			auto& vertex = mVerticesRender[index];
			const auto& computeData = mVerticesCompute[index];

			glm::vec2 locationXZ (vertex.position.x, vertex.position.z);

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
				const auto replicateIndex = index + mUniquePointCount.x;
				auto& sample = mVerticesRender[replicateIndex];
				const auto& sampleComputeData = mVerticesCompute[replicateIndex];
				sample.position.x = sampleComputeData.originalPosition.x + displacementDirectionScale * displacement.x;
				sample.position.y = vertex.position.y;
				sample.position.z = sampleComputeData.originalPosition.z + displacementDirectionScale * displacement.y;
				sample.normal = vertex.normal;
			}
			if (z == 0) {
				const auto replicateIndex = index + mUniquePointCount.y * mTildePointCount.x;
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

void nex::Ocean::simulateFFT(float t, bool skip)
{
	float lambda = -0.8;
	const float twoPi = 2.0f * util::PI;
	const float pi = util::PI;

	for (int z = 0; z < N; z++) {
		//kz = M_PI * (2.0f * m_prime - N) / length;
		const float kz = (twoPi * z - pi * mUniquePointCount.y) / mWaveLength.y; //(twoPi * mDash - pi * M) / Lz;  nex::util::PI * (2.0f * mDash - M) / Lz;
		
		for (int x = 0; x < N; x++) {
			//kx = M_PI * (2 * n_prime - N) / length;
			const float kx = (twoPi * x - pi * mUniquePointCount.x) / mWaveLength.x; //(twoPi * nDash - pi * N) / Lx;   (nex::util::PI * ((2.0f * nDash) - N)) / Lx;


			float len = sqrt(kx * kx + kz * kz);
			const unsigned index  = z * N + x;

			h_tilde[index] = height(x, z, t) * 1.0f;
			// (a + ib) * (c + id) = (ac - bd) + i(ad + bc)
			// (h.re + i*h.im) * (0 + i*kx) = (h.re*0 - h.im * kx) + i(h.re*kx + h.im*0) = (-h.im * kx) + i(h.re*kx)
			h_tilde_slopex[index] = h_tilde[index] * Complex(0, kx);
			//h_tilde_slopex[index] = Complex( - h_tilde[index].im *kx, 0);
			//h_tilde_slopez[index] = Complex(-h_tilde[index].im *kz, 0);
			h_tilde_slopez[index] = h_tilde[index] * Complex(0, kz);

			//h_tilde_slopex[index] = Complex(0.0f, 0);
			//h_tilde_slopez[index] = Complex(0.0f, 0);
			if (len < 0.000001f) {
				h_tilde_dx[index] = Complex(0.0f, 0.0f);
				h_tilde_dz[index] = Complex(0.0f, 0.0f);
			}
			else {
				h_tilde_dx[index] = h_tilde[index] * Complex(0, -kx / len);
				h_tilde_dz[index] = h_tilde[index] * Complex(0, -kz / len);
				//h_tilde_dx[index] = Complex(0.0f, 0.0f);
				//h_tilde_dz[index] = Complex(0.0f, 0.0f);;
			}
		}
	}


	//fft.fft(h_tilde.data(), h_tilde.data(), 1, 0);
	bool inverse = false;
	/*fft.fftInPlace(h_tilde, inverse);
	fft.fftInPlace(h_tilde_slopex, inverse);
	fft.fftInPlace(h_tilde_slopez, inverse);
	fft.fftInPlace(h_tilde_dx, inverse);
	fft.fftInPlace(h_tilde_dz, inverse);*/

	for (int n_prime = 0; n_prime < N; n_prime++) {

		Iterator2D h_tildeIt(h_tilde, Iterator2D::PrimitiveMode::ROWS, n_prime, N);
		Iterator2D h_tilde_slopexIt(h_tilde_slopex, Iterator2D::PrimitiveMode::ROWS, n_prime, N);
		Iterator2D h_tilde_slopezIt(h_tilde_slopez, Iterator2D::PrimitiveMode::ROWS, n_prime, N);
		Iterator2D h_tilde_dxIt(h_tilde_dx, Iterator2D::PrimitiveMode::ROWS, n_prime, N);
		Iterator2D h_tilde_dzIt(h_tilde_dz, Iterator2D::PrimitiveMode::ROWS, n_prime, N);

		fft.fft(h_tildeIt, h_tildeIt, false);
		fft.fft(h_tilde_slopexIt, h_tilde_slopexIt, false);
		fft.fft(h_tilde_slopezIt, h_tilde_slopezIt, false);
		fft.fft(h_tilde_dxIt, h_tilde_dxIt, false);
		fft.fft(h_tilde_dzIt, h_tilde_dzIt, false);


		/*fft.fft(h_tilde.data(), h_tilde.data(), N, n_prime, false);

		

		fft.fft(h_tilde_slopex.data(), h_tilde_slopex.data(), N, n_prime, false);
		fft.fft(h_tilde_slopez.data(), h_tilde_slopez.data(), N, n_prime, false);
		fft.fft(h_tilde_dx.data(), h_tilde_dx.data(), N, n_prime, false);
		fft.fft(h_tilde_dz.data(), h_tilde_dz.data(), N, n_prime, false);*/
	}
	
	for (int m_prime = 0; m_prime < N; m_prime++) {

		Iterator2D h_tildeIt(h_tilde, Iterator2D::PrimitiveMode::COLUMNS, m_prime, N);
		Iterator2D h_tilde_slopexIt(h_tilde_slopex, Iterator2D::PrimitiveMode::COLUMNS, m_prime, N);
		Iterator2D h_tilde_slopezIt(h_tilde_slopez, Iterator2D::PrimitiveMode::COLUMNS, m_prime, N);
		Iterator2D h_tilde_dxIt(h_tilde_dx, Iterator2D::PrimitiveMode::COLUMNS, m_prime, N);
		Iterator2D h_tilde_dzIt(h_tilde_dz, Iterator2D::PrimitiveMode::COLUMNS, m_prime, N);

		fft.fft(h_tildeIt, h_tildeIt, true);
		fft.fft(h_tilde_slopexIt, h_tilde_slopexIt, true);
		fft.fft(h_tilde_slopezIt, h_tilde_slopezIt, true);
		fft.fft(h_tilde_dxIt, h_tilde_dxIt, true);
		fft.fft(h_tilde_dzIt, h_tilde_dzIt, true);


		/*fft.fft(h_tilde.data(), h_tilde.data(), 1, m_prime * N, true);
		fft.fft(h_tilde_slopex.data(), h_tilde_slopex.data(), 1, m_prime * N, true);
		fft.fft(h_tilde_slopez.data(), h_tilde_slopez.data(), 1, m_prime * N, true);
		fft.fft(h_tilde_dx.data(), h_tilde_dx.data(), 1, m_prime * N, true);
		fft.fft(h_tilde_dz.data(), h_tilde_dz.data(), 1, m_prime * N, true);*/
	}

	if (skip)return;

	int sign;
	float signs[] = { 1.0f, -1.0f };
	glm::vec3 n;

	for (int z = 0; z < mUniquePointCount.y; z++) {
		for (int x = 0; x < mUniquePointCount.x; x++) {
			const unsigned heightIndex = z * mUniquePointCount.x + x;     // index into h_tilde..
			const unsigned vertexIndex = z * mTildePointCount.x + x;    // index into vertices

			sign = signs[(x + z) & 1];


			const double normalization = (mWaveLength.x ); // (float) 128.0f;

			h_tilde[heightIndex] = h_tilde[heightIndex] * sign / normalization;

			auto& vertex = mVerticesRender[vertexIndex];
			const auto& computeData = mVerticesCompute[vertexIndex];


			// height
			vertex.position.y = h_tilde[heightIndex].re;

			// displacement
			h_tilde_dx[heightIndex] = h_tilde_dx[heightIndex] * sign / normalization;
			h_tilde_dz[heightIndex] = h_tilde_dz[heightIndex] * sign / normalization;
			vertex.position.x = computeData.originalPosition.x + h_tilde_dx[heightIndex].re * lambda;
			vertex.position.z = computeData.originalPosition.z + h_tilde_dz[heightIndex].re * lambda;

			// normal
			h_tilde_slopex[heightIndex] = h_tilde_slopex[heightIndex] * sign / normalization;
			h_tilde_slopez[heightIndex] = h_tilde_slopez[heightIndex] * sign / normalization;
			n = normalize(glm::vec3(-h_tilde_slopex[heightIndex].re, 1.0f, -h_tilde_slopez[heightIndex].re));
			vertex.normal.x = n.x;
			vertex.normal.y = n.y;
			vertex.normal.z = n.z;

			// for tiling
			// first point has to be replicated three times
			if (x == 0 && z == 0) {
				const auto replicateIndex = mVerticesRender.size() - 1;
				auto& sample = mVerticesRender[replicateIndex];
				const auto& sampleComputeData = mVerticesCompute[replicateIndex];
				sample.position.x = sampleComputeData.originalPosition.x + lambda * h_tilde_dx[heightIndex].re;
				sample.position.y = vertex.position.y;
				sample.position.z = sampleComputeData.originalPosition.z + lambda * h_tilde_dz[heightIndex].re;
				sample.normal = vertex.normal;
			}
			if (x == 0) {
				const auto replicateIndex = vertexIndex + mUniquePointCount.x;
				auto& sample = mVerticesRender[replicateIndex];
				const auto& sampleComputeData = mVerticesCompute[replicateIndex];
				sample.position.x = sampleComputeData.originalPosition.x + lambda * h_tilde_dx[heightIndex].re;
				sample.position.y = vertex.position.y;
				sample.position.z = sampleComputeData.originalPosition.z +lambda * h_tilde_dz[heightIndex].re;
				sample.normal = vertex.normal;
			}
			if (z == 0) {
				const auto replicateIndex = vertexIndex + mUniquePointCount.y * mTildePointCount.x;
				auto& sample = mVerticesRender[replicateIndex];
				const auto& sampleComputeData = mVerticesCompute[replicateIndex];
				sample.position.x = sampleComputeData.originalPosition.x + lambda * h_tilde_dx[heightIndex].re;
				sample.position.y = vertex.position.y;
				sample.position.z = sampleComputeData.originalPosition.z + lambda * h_tilde_dz[heightIndex].re;
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

void nex::Ocean::simulateFFT(std::vector<ResultData>& out, float t, unsigned startIndex, unsigned N, int stride)
{
	if (N == 1)
	{
		// index = z * mTildePointCount.x + x;
		glm::uvec2 index(startIndex % mUniquePointCount.x, startIndex / mUniquePointCount.x);

		
	}
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

nex::Ocean::HeightZeroComputePass::HeightZeroComputePass(const glm::uvec2& uniquePointCount, const glm::vec2& waveLength, const glm::vec2& windDirection,
	float spectrumScale, float windSpeed) : 
ComputePass(Shader::createComputeShader("ocean/height_zero_precompute_cs.glsl")),
mUniquePointCount(uniquePointCount), mWaveLength(waveLength), mWindDirection(windDirection), mSpectrumScale(spectrumScale), mWindSpeed(windSpeed)
{
	TextureData desc;
	desc.internalFormat = InternFormat::RGBA32F;
	desc.colorspace = ColorSpace::RGBA;
	desc.pixelDataType = PixelDataType::FLOAT;
	desc.generateMipMaps = false;
	desc.magFilter = desc.minFilter = TextureFilter::NearestNeighbor;
	mHeightZero = std::make_unique<Texture2D>(mUniquePointCount.x, mUniquePointCount.y, desc, nullptr);

	mResultTexture = { mShader->getUniformLocation("result"), UniformType::IMAGE2D, 0 };

	mUniquePointCountUniform = { mShader->getUniformLocation("uniquePointCount"), UniformType::UVEC2};
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



	std::random_device device;
	std::mt19937 engine(device());

	// note: we need mean 0 and standard deviation 1!
	std::normal_distribution<float> distribution(0, 1);
	
	std::vector<glm::vec4> randValues(mUniquePointCount.x * mUniquePointCount.y);

	for (unsigned z = 0; z < mUniquePointCount.y; ++z)
	{
		for (unsigned x = 0; x < mUniquePointCount.x ;++x)
		{
			randValues[z * mUniquePointCount.x + x].x  = distribution(engine);
			randValues[z * mUniquePointCount.x + x].y  = distribution(engine);
			randValues[z * mUniquePointCount.x + x].z = distribution(engine);
			randValues[z * mUniquePointCount.x + x].w = distribution(engine);
			randValues[z * mUniquePointCount.x + x].x = 1;
			randValues[z * mUniquePointCount.x + x].y = 1;
			randValues[z * mUniquePointCount.x + x].z = 1;
			randValues[z * mUniquePointCount.x + x].w = 1;
		}
	}

	TextureData randDesc;
	randDesc.internalFormat = InternFormat::RGBA32F;
	randDesc.colorspace = ColorSpace::RGBA;
	randDesc.pixelDataType = PixelDataType::FLOAT;
	randDesc.generateMipMaps = false;
	randDesc.magFilter = randDesc.minFilter = TextureFilter::NearestNeighbor;

	mRandNormalDistributed = std::make_unique<Texture2D>(mUniquePointCount.x, mUniquePointCount.y, randDesc, randValues.data());
	mRandTextureUniform = { mShader->getUniformLocation("randTexture"), UniformType::TEXTURE2D, 1 };
}

void nex::Ocean::HeightZeroComputePass::compute()
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

nex::Texture2D* nex::Ocean::HeightZeroComputePass::getResult()
{
	return mHeightZero.get();
}

nex::Ocean::HeightComputePass::HeightComputePass(const glm::uvec2& uniquePointCount, const glm::vec2& waveLength,
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

	TextureData desc;
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

void nex::Ocean::HeightComputePass::compute(float time, Texture2D* heightZero)
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

nex::Texture2D* nex::Ocean::HeightComputePass::getHeight()
{
	return mHeight.get();
}

nex::Texture2D* nex::Ocean::HeightComputePass::getSlopeX()
{
	return mHeightSlopeX.get();
}

nex::Texture2D* nex::Ocean::HeightComputePass::getSlopeZ()
{
	return mHeightSlopeZ.get();
}

nex::Texture2D* nex::Ocean::HeightComputePass::getDx()
{
	return mHeightDx.get();
}

nex::Texture2D* nex::Ocean::HeightComputePass::getDz()
{
	return mHeightDz.get();
}


nex::Ocean::ButterflyComputePass::ButterflyComputePass(unsigned N) : ComputePass(Shader::createComputeShader("ocean/butterfly_cs.glsl")),
mN(N)
{
	if (!nex::isPow2(mN)) throw std::invalid_argument("nex::Ocean::ButterflyComputePass : N has to be a power of 2!");

	TextureData desc;
	desc.internalFormat = InternFormat::RGBA32F;
	desc.colorspace = ColorSpace::RGBA;
	desc.pixelDataType = PixelDataType::FLOAT;
	desc.generateMipMaps = false;
	desc.magFilter = desc.minFilter = TextureFilter::NearestNeighbor;
	mButterfly = std::make_unique<Texture2D>(mN, std::log2(mN), desc, nullptr);

	mButterflyUniform = { mShader->getUniformLocation("butterfly"), UniformType::IMAGE2D, 0 };
	mNUniform = { mShader->getUniformLocation("N"), UniformType::INT};

	mShader->bind();
	mShader->setInt(mNUniform.location, mN);
}

void nex::Ocean::ButterflyComputePass::compute()
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
	RenderBackend::get()->syncMemoryWithGPU(MemorySync_ShaderImageAccess);
}

nex::Texture2D* nex::Ocean::ButterflyComputePass::getButterfly()
{
	return mButterfly.get();
}

nex::Ocean::IfftPass::IfftPass(int N) : ComputePass(Shader::createComputeShader("ocean/ifft_cs.glsl")),
mBlit(std::make_unique<ComputePass>(nex::Shader::createComputeShader("ocean/blit_cs.glsl"))), mN(N), mLog2N(std::log2(mN))

{
	TextureData desc;
	desc.internalFormat = InternFormat::RG32F;
	desc.colorspace = ColorSpace::RG;
	desc.pixelDataType = PixelDataType::FLOAT;
	mPingPong = std::make_unique<Texture2D>(N, N, desc, nullptr);


	mNUniform = { mShader->getUniformLocation("N"), UniformType::INT };
	mStageUniform = { mShader->getUniformLocation("stage"), UniformType::INT };
	mVerticalUniform = { mShader->getUniformLocation("vertical"), UniformType::INT };
	mInputUniform = {mShader->getUniformLocation("inputImage"), UniformType::IMAGE2D, 0};
	mButterflyUniform = { mShader->getUniformLocation("butterfly"), UniformType::IMAGE2D, 1 };
	mOutputUniform = { mShader->getUniformLocation("outputImage"), UniformType::IMAGE2D, 2};


	mBlitSourceUniform = { mBlit->getShader()->getUniformLocation("source"), UniformType::IMAGE2D, 0 };
	mBlitDestUniform = { mBlit->getShader()->getUniformLocation("dest"), UniformType::IMAGE2D, 1 };


	mShader->bind();
	mShader->setInt(mNUniform.location, mN);
}

void nex::Ocean::IfftPass::setButterfly(Texture2D* butterfly)
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

void nex::Ocean::IfftPass::setInput(Texture2D* input)
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

void nex::Ocean::IfftPass::setOutput(Texture2D* output)
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

void nex::Ocean::IfftPass::setStage(int stage)
{
	mShader->setInt(mStageUniform.location, stage);
}

void nex::Ocean::IfftPass::setVertical(bool vertical)
{
	mShader->setInt(mVerticalUniform.location, (int)vertical);
}

void nex::Ocean::IfftPass::computeAllStages(Texture2D* input)
{
	Texture2D* textures[2] = {input, mPingPong.get()};
	int index = 0;

	for (unsigned n = 0; n < mLog2N; ++n)
	{
		const int nextIndex = (index + 1) % 2;

		setStage(n);
		setInput(textures[index]);
		setOutput(textures[nextIndex]);
		RenderBackend::get()->syncMemoryWithGPU(MemorySync_ShaderImageAccess);
		dispatch(mN, mN, 1);
		RenderBackend::get()->syncMemoryWithGPU(MemorySync_ShaderImageAccess);
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


		mBlit->dispatch(mN, mN, 1);
		RenderBackend::get()->syncMemoryWithGPU(MemorySync_ShaderImageAccess);
		// rebind this compute shader
		bind();
	}
}

nex::gui::OceanConfig::OceanConfig(Ocean* ocean) : mOcean(ocean)
{
}

void nex::gui::OceanConfig::drawSelf()
{
	ImGui::Checkbox("Ocean Wireframe", mOcean->getWireframeState());
}