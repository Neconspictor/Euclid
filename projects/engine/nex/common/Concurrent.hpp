#pragma once
#include <mutex>

namespace nex
{
	using UniqueLock = std::unique_lock<std::recursive_mutex>;
}