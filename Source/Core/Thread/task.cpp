#include "Core/Thread/task.h"
#include "Core/log.h"

using namespace glex;

void Async::Startup(uint32_t numThreads)
{
	if (numThreads == 0)
		Logger::Warn("Thread pool size is 0. You can't use async functions.");
	s_threadPool.Emplace(numThreads);
}

void Async::Shutdown()
{
	s_threadPool.Destroy();
}