#pragma once

#include <glm/glm.hpp>
#include <nex/math/Constant.hpp>


namespace nex
{
	/**
	 * A struct for complex numbers
	 */
	struct Complex
	{
		Real re;
		Real im;

		Complex();
		Complex(Real real, Real imaginary);
		Complex(const Complex& c) = default;
		Complex(Complex&& c) = default;
		~Complex() = default;
		Complex& operator=(const Complex&) = default;
		Complex& operator=(Complex&&) = default;


		/**
		 * Computes the complex argument from this number.
		 */
		Real arg() const;

		/**
		 * Transforms this complex number to the cartesian plane.
		 */
		glm::vec2 cartesian() const;

		/**
		 * Computes the complex conjugate of this number.
		 */
		Complex conjugate() const;

		/**
		 * Computes the exponential of this number.
		 */
		Complex exp() const;

		/**
		 * Provides a complex number by a given exponent.
		 * The result will match euler's formula e^(i*exponent) = cos(exponent) + i*sin(exponent)
		 */
		static Complex euler(Real exponent);

		/**
		 * Computes the magnitude of this number.
		 */
		Real magnitude() const;

		Complex operator+(const Complex& c) const;
		Complex& operator+=(const Complex& c);
		Complex operator-(const Complex& c) const;
		Complex& operator-=(const Complex& c);
		Complex operator*(const Complex& c) const;
		Complex& operator*=(const Complex& c);
		Complex operator*(Real scalar) const;
		Complex& operator*=(Real scalar);
		Complex operator/(const Complex& c) const;
		Complex& operator/=(const Complex& c);
		Complex operator/(Real scalar) const;
		Complex& operator/=(Real scalar);

	private:
		void add(const Complex& c);
		void subtract(const Complex& c);
		void multiply(const Complex& c);
		void multiply(Real scalar);
		void divide(const Complex& c);
	};
}

nex::Complex operator*(nex::Real scalar, const nex::Complex& c);
nex::Complex& operator*=(nex::Real scalar, nex::Complex& c);
nex::Complex operator/(nex::Real scalar, const nex::Complex& c);
nex::Complex& operator/=(nex::Real scalar, nex::Complex& c);