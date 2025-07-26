#pragma once

namespace glex
{
	template <typename Fn>
	void DoOnce(Fn&& fn)
	{
		static bool s_done = false;
		if (!s_done)
		{
			fn();
			s_done = true;
		}
	}
}