#pragma once

namespace nex {
	class ScrollListener
	{
	public:
		virtual void onScroll(float yOffset) = 0;
		virtual ~ScrollListener() {};
	};
}