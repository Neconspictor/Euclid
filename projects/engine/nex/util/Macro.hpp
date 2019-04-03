#pragma once

/**
 * A macro for executing code when not all optimizations should be applied 
 */
#ifndef EUCLID_ALL_OPTIMIZATIONS
	#define EUCLID_DEBUG(x) x;
#else
#define EUCLID_DEBUG(x)
#endif