#ifndef TRACKINGPACKET_H
#define TRACKINGPACKET_H

struct Point
{
	uint16_t x,y;
	uint16_t intensity;
	Point():x(0xFFFF),y(0xFFFF),intensity(0xFFFF)
	{}
};
struct TrackingPacket
{
	Point points[4];
};

#endif
