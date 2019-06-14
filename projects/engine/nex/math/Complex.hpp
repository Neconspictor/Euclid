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
		float re;
		float im;

		Complex();
		Complex(float real, float imaginary);
		Complex(const Complex& c) = default;
		Complex(Complex&& c) = default;
		~Complex() = default;
		Complex& operator=(const Complex&) = default;
		Complex& operator=(Complex&&) = default;


		/**
		 * Computes the complex argument from this number.
		 */
		float arg() const;

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
		static Complex euler(float exponent);

		/**
		 * Computes the magnitude of this number.
		 */
		float magnitude() const;

		Complex operator+(const Complex& c) const;
		Complex& operator+=(const Complex& c);
		Complex operator-(const Complex& c) const;
		Complex& operator-=(const Complex& c);
		Complex operator*(const Complex& c) const;
		Complex& operator*=(const Complex& c);
		Complex operator*(float scalar) const;
		Complex& operator*=(float scalar);
		Complex operator/(const Complex& c) const;
		Complex& operator/=(const Complex& c);
		Complex operator/(float scalar) const;
		Complex& operator/=(float scalar);

	private:
		void add(const Complex& c);
		void subtract(const Complex& c);
		void multiply(const Complex& c);
		void multiply(float scalar);
		void divide(const Complex& c);
	};
}

nex::Complex operator*(float scalar, const nex::Complex& c);
nex::Complex& operator*=(float scalar, nex::Complex& c);
nex::Complex operator/(float scalar, const nex::Complex& c);
nex::Complex& operator/=(float scalar, nex::Complex& c);