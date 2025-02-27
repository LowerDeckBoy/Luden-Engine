#pragma once

// TODO:

#include "Types.hpp"
#include <thread>
#include <vector>

namespace Luden::Core
{
	class ThreadPool
	{
	public:
		ThreadPool(uint32 NumThreads = std::thread::hardware_concurrency());
		~ThreadPool();

	private:


	};
} // namespace Luden::Core
