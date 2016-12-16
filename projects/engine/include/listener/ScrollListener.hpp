#pragma once

class ScrollListener
{
public:
	virtual void onScroll(float yOffset) = 0;
	virtual ~ScrollListener(){};
};