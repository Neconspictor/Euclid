#pragma once
#include <vector>
#include "Complex.hpp"

namespace nex
{
	struct complex_t;

	class PolynomialSolver
	{
	public:
		static std::vector<Complex> solveQuartic(const Complex& a, const Complex& b, const Complex& c, const Complex& d, const Complex& e);
		static std::vector<double> solveQuartic(double a, double b, double c, double d, double e);

		static std::vector<Complex> solveCubic(const Complex& a, const Complex& b, const Complex& c, const Complex& d);
		static std::vector<double> solveCubic(double a, double b, double c, double d);

		static std::vector<Complex> solveQuadratic(const Complex& a, const Complex& b, const Complex& c);
		static std::vector<double> solveQuadratic(double a, double b, double c);

	private:
		static void solve(int degree, complex_t* in, std::vector<Complex>& out);
		static void solve(int degree, double* in, std::vector<double>& out);
	};
}
