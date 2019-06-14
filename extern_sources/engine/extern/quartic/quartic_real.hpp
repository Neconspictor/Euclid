/* Solve Polynomials of up to the fourth degree. (over real numbers)
 * Algorithms by Ferrari, Tartaglia, Cardano, et al. (16th century Italy)
 */

#pragma once

namespace nex {
	int solve_real_poly(int degree, const double* poly, double* results);
}