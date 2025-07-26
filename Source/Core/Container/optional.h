#pragma once
#include <utility>
#include <string.h>

namespace glex
{
	template <typename T>
	union Optional
	{
	private:
		T object;
		uint8_t dummy[sizeof(T)];

	public:
		Optional() {}
		~Optional() {}
		T* operator&() { return &object; }
		T const* operator&() const { return &object; }
		T* operator->() { return &object; }
		T const* operator->() const { return &object; }
		T& operator*() { return object; }
		T const& operator*() const { return object; }
		void Destroy() { object.~T(); }
		void Zero() { memset(dummy, 0, sizeof(T)); }
		void Invalidate() { object.Invalidate(); }

		template <typename... Args>
		T& Emplace(Args&&... args)
		{
			new(&object) T(std::forward<Args>(args)...);
			return object;
		}
	};
}