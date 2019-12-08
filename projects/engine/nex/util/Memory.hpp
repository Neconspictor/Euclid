#pragma once

#include <memory>

namespace nex {

	template<class T>
	void setUnique(std::unique_ptr<T>& smart, T&& t) {
		if (!smart) smart = std::make_unique<T>(std::move(t));
		else *smart = std::move(t);
	}
}