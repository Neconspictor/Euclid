#include "Complex.hpp"
#include <valarray>

nex::Complex::Complex() : re(0.0), im(0.0)
{
}

nex::Complex::Complex(Real real, Real imaginary) : re(real), im(imaginary)
{
}

nex::Complex::Real nex::Complex::arg() const
{
	return std::atan2<Real>(im, re);
}

glm::vec2 nex::Complex::cartesian() const
{
	return { re, im };
}

nex::Complex nex::Complex::conjugate() const
{
	return Complex(re, -im);
}

nex::Complex nex::Complex::exp() const
{
	//e^(a + ib)= e^(a) * e^(ib) = e^(a) * (cos(a)+ i*sin(b))
	return std::exp(re) * (Complex(cos(re), sin(im)));
}

nex::Complex nex::Complex::euler(Real exponent)
{
	return { cos(exponent), sin(exponent) };
}

nex::Complex::Real nex::Complex::magnitude() const
{
	return std::sqrt(re*re + im * im);
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

nex::Complex nex::Complex::operator*(Real scalar) const
{
	Complex result(*this);
	result.multiply(scalar);
	return result;
}

nex::Complex& nex::Complex::operator*=(Real scalar)
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

nex::Complex nex::Complex::operator/(Real scalar) const
{
	return operator*(1.0 / scalar);
}

nex::Complex& nex::Complex::operator/=(Real scalar)
{
	return operator*=(1.0 / scalar);
}


void nex::Complex::add(const Complex& c)
{
	re += c.re;
	im += c.im;
}

void nex::Complex::divide(const Complex& c)
{
	// (a + ib) / (c + id) =  ((ac + bd) + i(bc - ad)) / (c*c + d*d)

	auto re_ = re;
	auto im_ = im;

	const auto denominator = c.re * c.re + c.im * c.im;
	re = (re_ * c.re + im_ * c.im) / denominator;
	im = (im_ * c.re - re_ * c.im) / denominator;
}

nex::Complex operator*(nex::Complex::Real scalar, const nex::Complex& c)
{
	return c * scalar;
}

nex::Complex& operator*=(nex::Complex::Real scalar, nex::Complex& c)
{
	return c *= scalar;
}

nex::Complex operator/(nex::Complex::Real scalar, const nex::Complex& c)
{
	return c / scalar;
}

nex::Complex& operator/=(nex::Complex::Real scalar, nex::Complex& c)
{
	return c /= scalar;
}

void nex::Complex::multiply(const Complex& c)
{
	// (a + ib) * (c + id) = (ac - bd) + i(ad + bc)
	auto re_ = re;
	auto im_ = im;
	re = re_ * c.re - im_ * c.im;
	im = re_ * c.im + im_ * c.re;
}

void nex::Complex::multiply(Real scalar)
{
	re *= scalar;
	im *= scalar;
}

void nex::Complex::subtract(const Complex& c)
{
	re -= c.re;
	im -= c.im;
}