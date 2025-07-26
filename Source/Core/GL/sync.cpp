#include "sync.h"
#include "Core/GL/context.h"

using namespace glex::gl;

bool Semaphore::Create()
{
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	if (vkCreateSemaphore(Context::GetDevice(), &semaphoreInfo, Context::HostAllocator(), &m_handle) == VK_SUCCESS)
		return true;
	m_handle = VK_NULL_HANDLE;
	return false;
}

void Semaphore::Destroy()
{
	vkDestroySemaphore(Context::GetDevice(), m_handle, Context::HostAllocator());
}

bool Fence::Create(bool signaled)
{
	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	if (signaled)
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	if (vkCreateFence(Context::GetDevice(), &fenceInfo, Context::HostAllocator(), &m_handle) == VK_SUCCESS)
		return true;
	m_handle = VK_NULL_HANDLE;
	return false;
}

void Fence::Destroy()
{
	vkDestroyFence(Context::GetDevice(), m_handle, Context::HostAllocator());
}

void Fence::Wait() const
{
	!vkWaitForFences(Context::GetDevice(), 1, &m_handle, VK_TRUE, UINT64_MAX);
}

void Fence::Reset()
{
	!vkResetFences(Context::GetDevice(), 1, &m_handle);
}