#pragma once

namespace nex
{
	struct Polar;

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
		 * Computes the magnitude of this number.
		 */
		float magnitude() const;

		/**
		 * Transforms this complex number to the polar coordination system.
		 */
		Polar polar() const;

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

	Complex operator*(float scalar, const Complex& c);
	Complex& operator*=(float scalar, Complex& c);
	Complex operator/(float scalar, const Complex& c);
	Complex& operator/=(float scalar, Complex& c);

	struct Polar
	{
		float distance; // distance/radius to the origin
		float azimuth; // azimuth angle in radians

		Polar(float distance, float azimuth);
		Polar();
		Polar(const Polar&) = default;
		Polar(Polar&&) = default;
		~Polar() = default;
		Polar& operator=(const Polar&) = default;
		Polar& operator=(Polar&&) = default;

		/**
		 * Transforms this polar coordinate into the complex number plane.
		 */
		Complex complex() const;

		/**
		 * Transforms this polar coordinate into the cartesian plane.
		 */
		glm::vec2 cartesian() const;
	};

	class Ocean
	{
	public:

		/**
		 * Creates a new Ocean object.
		 * @param pointNumberX : Amount of points in x direction
		 * @param pointNumberZ : Amount of points in z direction
		 * @param dimensionX : (object space) extension of the ocean in x direction.
		 * @param dimensionZ : (object space) extension of the ocean in z direction.
		 */
		Ocean(unsigned pointNumberX, 
			unsigned pointNumberZ, 
			float dimensionX, 
			float dimensionZ);

		/**
		 * Computes the height of a location on the (x,z) plane at a specific time. 
		 */
		float computeHeight(const glm::vec2& locationXZ, float time);

		float heightTildeDash(int nDash, int mDash, float time);

		float exp(const Complex& exponent);

		void test();

	private:
		/**
		 * Amount of points in z direction
		 */
		unsigned M;

		/**
		 * Amount of points in x direction
		 */
		unsigned N;

		/**
		 * (object space) extension of the ocean in x direction.
		 */
		float Lx;

		/**
		 * (object space) extension of the ocean in z direction.
		 */
		float Lz;
	};
}