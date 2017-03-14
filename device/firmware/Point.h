#ifndef POINT_H
#define POINT_H

struct Point
{
	uint16_t x,y;
	uint16_t intensity;
	Point():x(0xFFFF),y(0xFFFF),intensity(0xFFFF)
	{}
};

#endif
