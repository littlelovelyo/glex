#pragma once

namespace glex
{
	template <typename Fn>
	class AutoCleaner
	{
	private:
		Fn m_fn;

	public:
		AutoCleaner(Fn&& fn) : m_fn(std::move(fn)) {}
		~AutoCleaner() { m_fn(); }
	};

	template <typename Fn>
	class CancellableAutoCleaner
	{
	private:
		bool m_clean = true;
		Fn m_fn;

	public:
		CancellableAutoCleaner(Fn&& fn) : m_fn(std::move(fn)) {}
		void Cancel() { m_clean = false; }
		~CancellableAutoCleaner() { if (m_clean) m_fn(); }
	};

	template <typename Fn>
	class AutoRegister
	{
	public:
		AutoRegister(Fn&& fn) { fn(); }
	};
}