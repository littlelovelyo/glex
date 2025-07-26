/**
 * A heap allocated buffer returned by some function
 * that will be automatically freed.
 */
#pragma once
#include "Core/Memory/mem.h"

namespace glex
{
	template <typename T>
	class TemporaryBuffer : private Uncopyable
	{
	private:
		T* m_buffer;

	public:
		TemporaryBuffer(T* buffer) : m_buffer(buffer) {}
		~TemporaryBuffer() { Mem::Free(const_cast<std::remove_const_t<T>*>(m_buffer)); }
		TemporaryBuffer(TemporaryBuffer<T>&& rhs) : m_buffer(rhs.m_buffer) { rhs.m_buffer = nullptr; };
		TemporaryBuffer& operator=(TemporaryBuffer<T>&& rhs) { std::swap(m_buffer, rhs.m_buffer); return *this; }
		TemporaryBuffer& operator=(T* newBuffer) { Mem::Free(const_cast<std::remove_const_t<T>*>(m_buffer)); m_buffer = newBuffer; return *this; }
		operator T*() const { return m_buffer; }
		T* Get() const { return m_buffer; }
		T* Extract() { T* result = m_buffer; m_buffer = nullptr; return result; }
	};
}