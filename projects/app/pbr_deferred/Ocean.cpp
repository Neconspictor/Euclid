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

void nex::OceanFFT::fft(nex::Complex* input, nex::Complex* output, int stride, int offset)
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

	for (int i = 0; i < N; i++) output[i * stride + offset] = c[which][i];
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
	mTildePointCount(pointCount),
	mWaveLength(maxWaveLength),
	mDimension(dimension),
	mSpectrumScale(spectrumScale),
	mWindDirection(glm::normalize(windDirection)),
	mWindSpeed(windSpeed),
	mPeriodTime(periodTime),
	mSimpleShadedPass(std::make_unique<SimpleShadedPass>()),
	mWireframe(true),
	N(mUniquePointCount.x),
	fft(N)
{
	assert(pointCount.x >= 2);
	assert(pointCount.y >= 2);
	assert(dimension.x > 0.0f);
	assert(dimension.y > 0.0f);
	assert(spectrumScale > 0.0f);
	assert(length(windDirection) > 0.0f);
	assert(periodTime > 0.0f);


	h_tilde.resize(N * N);
	h_tilde_slopex.resize(N * N);
	h_tilde_slopez.resize(N * N);
	h_tilde_dx.resize(N * N);
	h_tilde_dz.resize(N * N);


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
	//simulateFFT(10.0f);

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
	const float displacementDirectionScale = -1.0;

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

void nex::Ocean::simulateFFT(float t)
{
	float lambda = -1.0;

	for (int z = 0; z < N; z++) {
		//kz = M_PI * (2.0f * m_prime - N) / length;
		const float kz = (2 * util::PI * z - util::PI * mUniquePointCount.y) / mWaveLength.y; //(twoPi * mDash - pi * M) / Lz;  nex::util::PI * (2.0f * mDash - M) / Lz;
		
		for (int x = 0; x < N; x++) {
			//kx = M_PI * (2 * n_prime - N) / length;
			const float kx = (2 * util::PI * x - util::PI * mUniquePointCount.x) / mWaveLength.x; //(twoPi * nDash - pi * N) / Lx;   (nex::util::PI * ((2.0f * nDash) - N)) / Lx;


			float len = sqrt(kx * kx + kz * kz);
			const unsigned index  = z * N + x;

			h_tilde[index] = height(x, z, t);
			// (a + ib) * (c + id) = (ac - bd) + i(ad + bc)
			// (h.re + i*h.im) * (0 + i*kx) = (h.re*0 - h.im * kx) + i(h.re*kx + h.im*0) = (-h.im * kx) + i(h.re*kx)
			//h_tilde_slopex[index] = h_tilde[index] * Complex(0, kx);
			h_tilde_slopex[index] = Complex( - h_tilde[index].im *kx, 0);
			h_tilde_slopez[index] = Complex(-h_tilde[index].im *kz, 0);
			//h_tilde_slopez[index] = h_tilde[index] * Complex(0, kz);

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
		fft.fft(h_tilde.data(), h_tilde.data(), N, n_prime);
		fft.fft(h_tilde_slopex.data(), h_tilde_slopex.data(), N, n_prime);
		fft.fft(h_tilde_slopez.data(), h_tilde_slopez.data(), N, n_prime);
		fft.fft(h_tilde_dx.data(), h_tilde_dx.data(), N, n_prime);
		fft.fft(h_tilde_dz.data(), h_tilde_dz.data(), N, n_prime);
	}
	
	
	for (int m_prime = 0; m_prime < N; m_prime++) {
		fft.fft(h_tilde.data(), h_tilde.data(), 1, m_prime * N);
		fft.fft(h_tilde_slopex.data(), h_tilde_slopex.data(), 1, m_prime * N);
		fft.fft(h_tilde_slopez.data(), h_tilde_slopez.data(), 1, m_prime * N);
		fft.fft(h_tilde_dx.data(), h_tilde_dx.data(), 1, m_prime * N);
		fft.fft(h_tilde_dz.data(), h_tilde_dz.data(), 1, m_prime * N);
	}
	

	int sign;
	float signs[] = { 1.0f, -1.0f };
	glm::vec3 n;

	for (int z = 0; z < mUniquePointCount.y; z++) {
		for (int x = 0; x < mUniquePointCount.x; x++) {
			const unsigned heightIndex = z * mUniquePointCount.x + x;     // index into h_tilde..
			const unsigned vertexIndex = z * mTildePointCount.x + x;    // index into vertices

			sign = signs[(x + z) & 1];

			h_tilde[heightIndex] = h_tilde[heightIndex] * sign;

			auto& vertex = mVerticesRender[vertexIndex];
			const auto& computeData = mVerticesCompute[vertexIndex];


			// height
			vertex.position.y = h_tilde[heightIndex].re;

			// displacement
			h_tilde_dx[heightIndex] = h_tilde_dx[heightIndex] * sign;
			h_tilde_dz[heightIndex] = h_tilde_dz[heightIndex] * sign;
			vertex.position.x = computeData.originalPosition.x + h_tilde_dx[heightIndex].re * lambda;
			vertex.position.z = computeData.originalPosition.z + h_tilde_dz[heightIndex].re * lambda;

			// normal
			h_tilde_slopex[heightIndex] = h_tilde_slopex[heightIndex] * sign;
			h_tilde_slopez[heightIndex] = h_tilde_slopez[heightIndex] * sign;
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

nex::gui::OceanConfig::OceanConfig(Ocean* ocean) : mOcean(ocean)
{
}

void nex::gui::OceanConfig::drawSelf()
{
	ImGui::Checkbox("Ocean Wireframe", mOcean->getWireframeState());
}