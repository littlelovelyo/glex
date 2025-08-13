/**
 * Basic classes to implement PhysX
 * memory management and error handling, etc.
 */
#pragma once
#include "Core/Memory/mem.h"
#include "Core/Thread/task.h"
#include "Core/log.h"
#include <PxPhysicsAPI.h>

namespace glex::px
{
	class Allocator : public physx::PxAllocatorCallback
	{
	public:
		virtual void* allocate(size_t size, char const* typeName, char const* filename, int line) override
		{
			return Mem::Alloc(size, 16);
		}

		virtual void deallocate(void* ptr) override
		{
			Mem::Free(ptr);
		}
	};

	class ErrorReporter : public physx::PxErrorCallback
	{
	public:
		virtual void reportError(physx::PxErrorCode::Enum code, char const* message, char const* file, int line) override
		{
			constexpr int k_severeMask = physx::PxErrorCode::eINVALID_PARAMETER | physx::PxErrorCode::eINVALID_OPERATION | physx::PxErrorCode::eOUT_OF_MEMORY | physx::PxErrorCode::eINTERNAL_ERROR | physx::PxErrorCode::eABORT;
			if (code & k_severeMask)
				Logger::Error("PhysX error: %s\n", message);
			else
				Logger::Warn("PhysX warning: %s\n", message);
		}
	};

	class SyncDispatcher : public physx::PxCpuDispatcher
	{
	public:
		virtual void submitTask(physx::PxBaseTask& task) override
		{
			task.run();
			task.release();
		}

		virtual uint32_t getWorkerCount() const
		{
			return 0;
		}
	};

	class AsyncDispatcher : public physx::PxCpuDispatcher
	{
	public:
		virtual void submitTask(physx::PxBaseTask& task) override
		{
			Async::SubmitWork([&]()
			{
				task.run();
				task.release();
			});
		}

		virtual uint32_t getWorkerCount() const
		{
			return Async::FreeThreadCount();
		}
	};
}