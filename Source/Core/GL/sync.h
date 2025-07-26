#pragma once
#include <vulkan/vulkan.h>

namespace glex::gl
{
	class Semaphore
	{
	private:
		VkSemaphore m_handle;

	public:
		Semaphore() : m_handle(VK_NULL_HANDLE) {}
		Semaphore(VkSemaphore handle) : m_handle(handle) {}
		bool Create();
		void Destroy();
		VkSemaphore GetHandle() const { return m_handle; }
	};

	class Fence
	{
	private:
		VkFence m_handle;

	public:
		Fence() : m_handle(VK_NULL_HANDLE) {}
		Fence(VkFence handle) : m_handle(handle) {}
		bool Create(bool signaled);
		void Destroy();
		VkFence GetHandle() const { return m_handle; }
		void Wait() const;
		void Reset();
	};
}