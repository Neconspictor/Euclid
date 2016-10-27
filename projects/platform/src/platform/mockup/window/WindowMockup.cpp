#include "platform/mockup/window/WindowMockup.hpp"
#include <iostream>

using namespace std;

WindowMockup::WindowMockup(WindowStruct const& description): Window(description)
{}

void WindowMockup::embedRenderer(Renderer* renderer)
{
	cout << "WindowMockup::embedRenderer(Renderer*): called!" << endl;
}

void WindowMockup::setVisible(bool visible)
{
	cout << "WindowMockup::setVisible(bool): called!" << endl;
}

void WindowMockup::setFullscreen()
{
	cout << "WindowMockup::setFullscreen(): called!" << endl;
}

void WindowMockup::setWindowed()
{
	cout << "WindowMockup::setWindowed(): called!" << endl;
}

void WindowMockup::resize(int newWidth, int newHeight)
{
	cout << "WindowMockup::resize(int, int): called!" << endl;
}

bool WindowMockup::isOpen()
{
	cout << "WindowMockup::isOpen(): called!" << endl;
	return m_isOpen;
}

void WindowMockup::close()
{
	cout << "WindowMockup::close(): called!" << endl;
	m_isOpen = false;
}

void WindowMockup::pollEvents()
{
	cout << "WindowMockup::pollEvents(): called!" << endl;
}