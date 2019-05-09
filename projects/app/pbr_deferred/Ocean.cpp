#include <pbr_deferred/Ocean.hpp>
#include "nex/util/Math.hpp"
#include <random>

nex::Complex::Complex() : re(0.0f), im(0.0f)
{
}

nex::Complex::Complex(float real, float imaginary) : re(real), im(imaginary)
{
}

float nex::Complex::arg() const
{
	return atan2(im, re);
}

glm::vec2 nex::Complex::cartesian() const
{
	return glm::vec2(re, im);
}

nex::Complex nex::Complex::conjugate() const
{
	return Complex(re, -im);
}

nex::Complex nex::Complex::euler(float exponent)
{
	return { cos(exponent), sin(exponent) };
}

nex::Complex nex::Complex::exp() const
{
	//e^(a + ib)= e^(a) * e^(ib) = e^(a) * (cos(a)+ i*sin(b))
	return expf(re) * (Complex(cosf(re), sinf(im)));
}

float nex::Complex::magnitude() const
{
	return sqrt(re*re + im*im);
}

nex::Polar nex::Complex::polar() const
{
	return Polar(magnitude(), arg());
}

nex::Complex nex::Complex::operator+(const Complex& c) const
{
	Complex result(*this);
	result.add(c);
	return result;
}

nex::Complex& nex::Complex::operator+=(const Complex& c)
{
	add(c);
	return *this;
}

nex::Complex nex::Complex::operator-(const Complex& c) const
{
	Complex result(*this);
	result.subtract(c);
	return result;
}

nex::Complex& nex::Complex::operator-=(const Complex& c)
{
	subtract(c);
	return *this;
}

nex::Complex nex::Complex::operator*(const Complex& c) const
{
	Complex result(*this);
	result.multiply(c);
	return result;
}

nex::Complex& nex::Complex::operator*=(const Complex& c)
{
	multiply(c);
	return *this;
}

nex::Complex nex::Complex::operator*(float scalar) const
{
	Complex result(*this);
	result.multiply(scalar);
	return result;
}

nex::Complex& nex::Complex::operator*=(float scalar)
{
	multiply(scalar);
	return *this;
}

nex::Complex nex::Complex::operator/(const Complex& c) const
{
	Complex result(*this);
	result.divide(c);
	return result;
}

nex::Complex& nex::Complex::operator/=(const Complex& c)
{
	divide(c);
	return *this;
}

nex::Complex nex::Complex::operator/(float scalar) const
{
	return operator*(1.0f / scalar);
}

nex::Complex& nex::Complex::operator/=(float scalar)
{
	return operator*=(1.0f / scalar);
}


void nex::Complex::add(const Complex& c)
{
	re += c.re;
	im += c.im;
}

void nex::Complex::divide(const Complex& c)
{
	// (a + ib) / (c + id) =  ((ac + bd) + i(bc - ad)) / (c*c + d*d)

	const float denominator = c.re * c.re + c.im * c.im;
	re = (re * c.re + im * c.im) / denominator;
	im = (im * c.re - re * c.im) / denominator;
}

nex::Complex nex::operator*(float scalar, const Complex& c)
{
	return c * scalar;
}

nex::Complex& nex::operator*=(float scalar, Complex& c)
{
	return c *= scalar;
}

nex::Complex nex::operator/(float scalar, const Complex& c)
{
	return c / scalar;
}

nex::Complex& nex::operator/=(float scalar, Complex& c)
{
	return c /= scalar;
}

nex::Polar::Polar(float distance, float azimuth) : distance(distance), azimuth(azimuth)
{
}

nex::Polar::Polar() : distance(0.0f), azimuth(0.0f)
{
}

void nex::Complex::multiply(const Complex& c)
{
	// (a + ib) * (c + id) = (ac - bd) + i(ad + bc)
	re = re * c.re - im * c.im;
	im = re * c.im + im * c.re;
}

void nex::Complex::multiply(float scalar)
{
	re *= scalar;
	im *= scalar;
}

void nex::Complex::subtract(const Complex& c)
{
	re -= c.re;
	im -= c.im;
}

nex::Complex nex::Polar::complex() const
{
	return Complex(distance * cosf(azimuth), distance * sinf(azimuth));
}

glm::vec2 nex::Polar::cartesian() const
{
	return glm::vec2(distance * cosf(azimuth), distance * sinf(azimuth));
}

nex::Ocean::Ocean(unsigned pointNumberX, unsigned pointNumberZ, float dimensionX, float dimensionZ) : N(pointNumberX), M(pointNumberZ), Lx(dimensionX), Lz(dimensionZ),
mWindSpeed(10.0f), mWindDirection(1.0f, 1.0f)
{
}

float nex::Ocean::computeHeight(const glm::vec2& locationXZ, float t)
{
	const auto& twoPi = 2*util::PI;
	const auto& pi = util::PI;

	const auto& x = locationXZ.x;
	const auto& z = locationXZ.y;

	float height = 0.0f;


	for (int mDash = 0; mDash < M; ++mDash)
	{
		for (int nDash = 0; nDash < N; ++nDash)
		{
			const auto kx = (twoPi * nDash - pi * N) / Lx;
			const auto kz = (twoPi * mDash - pi * M) / Lz;
			const auto euler = Complex::euler(kx* x + kz * z);
			
			// will have form e^(0) * (cos(0) + i*sin(kx* x + kz*z)) = 1 + i*sin(kx* x + kz*z)
			const auto complexResult = heightTilde(glm::vec2(kx, kz), t) * euler;

			//TODO convert complex number to a reasonable real number
			// real component is the amplitude of the sinusoid -> sum up amplitudes to get the height
			height += complexResult.re;

		}
	}

	return height;
}

nex::Complex nex::Ocean::heightTildeZero(const glm::vec2& wave) const
{
	static const auto inverseRootTwo = 1 / std::sqrt(2.0);
	const Complex random(generateGaussianRand(), generateGaussianRand());

	return inverseRootTwo * random * std::sqrt(philipsSpectrum(wave));
}

nex::Complex nex::Ocean::heightTilde(const glm::vec2& wave, float time) const
{
	const auto dispersion = philipsSpectrum(wave);
	
	return heightTildeZero(wave) * Complex::euler(dispersion * time)
			+ heightTildeZero(-wave).conjugate() * Complex::euler(-dispersion * time);
}

float nex::Ocean::philipsSpectrum(const glm::vec2& wave) const
{
	//TODO
	static const auto A = 1.0f;
	static const auto g = 9.8f;


	const auto L = mWindSpeed * mWindSpeed / g;

	// length of the wave vector
	const auto lambda = length(wave);

	// magnitude of the wave vector
	const auto k = 2 * util::PI / lambda;

	const auto kLSquare = k*L * k*L;
	const auto kFour = k * k*k*k;
	const auto absAngleWaveWind = abs(dot(wave, mWindDirection));

	return A * exp(-1.0f / kLSquare) / kFour * (absAngleWaveWind * absAngleWaveWind);
}

float nex::Ocean::generateGaussianRand()
{
	static std::random_device device;
	const static std::mt19937 engine(device());

	// note: we need mean 0 and standard deviation 1!
	static std::normal_distribution<> distribution(0, 1);
	return distribution(engine);
}