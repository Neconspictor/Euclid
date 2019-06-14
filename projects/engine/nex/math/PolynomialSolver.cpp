#include "PolynomialSolver.hpp"
#include "../../extern_sources/engine/extern/quartic/quartic.hpp"
#include "../../extern_sources/engine/extern/quartic/quartic_real.hpp"

std::vector<nex::Complex> nex::PolynomialSolver::solveQuartic(const Complex& a, const Complex& b, const Complex& c,
	const Complex& d, const Complex& e)
{
	complex_t poly[5];
	poly[0] = {a.re, a.im};
	poly[1] = { b.re, b.im };
	poly[2] = { c.re, c.im };
	poly[3] = { d.re, d.im };
	poly[4] = { e.re, e.im };

	std::vector<nex::Complex> result;
	solve(4, poly, result);

	return result;
}

std::vector<double> nex::PolynomialSolver::solveQuartic(double a, double b, double c, double d, double e)
{
	double poly[5];
	poly[0] = a;
	poly[1] = b;
	poly[2] = c;
	poly[3] = d;
	poly[4] = e;

	std::vector<double> result;
	solve(4, poly, result);
	return result;
}

std::vector<nex::Complex> nex::PolynomialSolver::solveCubic(const Complex& a, const Complex& b, const Complex& c,
	const Complex& d)
{
	complex_t poly[4];
	poly[0] = { a.re, a.im };
	poly[1] = { b.re, b.im };
	poly[2] = { c.re, c.im };
	poly[3] = { d.re, d.im };

	std::vector<nex::Complex> result;
	solve(3, poly, result);

	return result;
}

std::vector<double> nex::PolynomialSolver::solveCubic(double a, double b, double c, double d)
{
	double poly[4];
	poly[0] = a;
	poly[1] = b;
	poly[2] = c;
	poly[3] = d;

	std::vector<double> result;
	solve(3, poly, result);
	return result;
}

std::vector<nex::Complex> nex::PolynomialSolver::solveQuadratic(const Complex& a, const Complex& b, const Complex& c)
{
	complex_t poly[3];
	poly[0] = { a.re, a.im };
	poly[1] = { b.re, b.im };
	poly[2] = { c.re, c.im };

	std::vector<nex::Complex> result;
	solve(2, poly, result);

	return result;
}

std::vector<double> nex::PolynomialSolver::solveQuadratic(double a, double b, double c)
{
	double poly[3];
	poly[0] = a;
	poly[1] = b;
	poly[2] = c;

	std::vector<double> result;
	solve(2, poly, result);
	return result;
}

void nex::PolynomialSolver::solve(int degree, complex_t* in, std::vector<Complex>& out)
{
	// Note: degree will be <= 4; so we can use stack allocation
	complex_t resultPoly[4];

	const auto count = solve_poly(degree, in, resultPoly);

	out.resize(count);
	for (int i = 0; i < count; ++i)
	{
		out[i] = { (float)resultPoly[i].re, (float)resultPoly[i].im };
	}
}

void nex::PolynomialSolver::solve(int degree, double* in, std::vector<double>& out)
{
	double temp[4];
	const auto count = solve_real_poly(degree, in, temp);
	for (auto i = 0; i < count; ++i)
	{
		if (!std::isnan(temp[i]))
		{
			out.push_back(temp[i]);
		}
	}
}