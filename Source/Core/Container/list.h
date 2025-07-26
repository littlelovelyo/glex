#pragma once
#include "Core/Memory/mem.h"
#include "Core/Thread/atomic.h"
#include <concepts>

namespace glex
{
	template <typename T>
	class LockFreeList
	{
	private:
		struct Node
		{
			Node* next;
			T value;
		};

		Node* m_head;

	public:
		LockFreeList() : m_head(nullptr) {}

		template <typename R>
		void Push(R&& value) requires std::is_same_v<std::remove_reference_t<R>, T>
		{
			Node* oldHead;
			Node* newNode = Mem::New<Node>(nullptr, std::forward<R>(value));
			do
			{
				oldHead = Atomic::Load(&m_head);
				newNode->next = oldHead;
			} while (Atomic::CompareAndExchange(&m_head, oldHead, newNode) != oldHead);
		}

		bool Pop(T& out)
		{
			Node* oldHead = Atomic::Load(&m_head);
			do
			{
				if (oldHead == nullptr)
					return false;
			} while (Atomic::CompareAndExchange(&m_head, oldHead, oldHead->next) != oldHead);
			out = std::move(oldHead->value);
			Mem::Delete(oldHead); // Use delete because T may want to be destructed.
			return true;
		}
	};
}