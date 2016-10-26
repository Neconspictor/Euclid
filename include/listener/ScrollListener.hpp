#ifndef SCROLLLISTENER_HPP
#define SCROLLLISTENER_HPP

class ScrollListener
{
public:
	virtual void onScroll(float yOffset) = 0;
	virtual ~ScrollListener(){};
};

#endif