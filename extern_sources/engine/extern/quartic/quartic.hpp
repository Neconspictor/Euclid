/* Solve Polynomials of up to the fourth degree. (over complex numbers)
 * Algorithms by Ferrari, Tartaglia, Cardano, et al. (16th century Italy)
 */

#pragma once
namespace nex
{
	struct complex_t
	{
		double re;
		double im;
	};

	int solve_poly(int degree, const complex_t* poly, complex_t* results);
}