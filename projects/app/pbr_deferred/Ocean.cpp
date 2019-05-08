#include <pbr_deferred/Ocean.hpp>
#include "nex/util/Math.hpp"

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

nex::Ocean::Ocean(unsigned pointNumberX, unsigned pointNumberZ, float dimensionX, float dimensionZ) : N(pointNumberX), M(pointNumberZ), Lx(dimensionX), Lz(dimensionZ)
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
			const Complex exponent(0, kx* x + kz*z);
			const Complex exponential = exponent.exp();
			
			// will have form e^(0) * (cos(0) + i*sin(kx* x + kz*z)) = 1 + i*sin(kx* x + kz*z)
			const auto complexResult = heightTildeDash(nDash, mDash, t) * exponential;

			//TODO convert complex number to a reasonable real number
			// real component is the amplitude of the sinusoid -> sum up amplitudes to get the height
			height += complexResult.re;

		}
	}

	return height;
}

float nex::Ocean::heightTildeDash(int nDash, int mDash, float t)
{
	//TODO
	return 0.0f;
}

void nex::Ocean::test()
{
	
}