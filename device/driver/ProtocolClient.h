#ifndef PROTOCOL_CLIENT_H
#define PROTOCOL_CLIENT_H


#include <functional>
#include <memory>
#include <iostream>
#include <cstdint>
#include "ProtocolEnums.h"

namespace ct
{
struct Point2D
{
	float x,y;
	float intensity;
};
struct CameraPacket2D
{
	std::array<Point2D,4> points;
	std::uint16_t latency;
};
struct BarPacket2D
{
	std::array<CameraPacket2D,4> cameras;
	uint32_t frame;
	std::uint8_t mask;
	TrackingFlags2D flags;
};

class ProtocolClient
{
public:
	class Impl;
	class CallbackSet
	{
	public:
		std::function<void (const std::string&)> onError;
		std::function<void (const std::string&)> onDebugText;
		std::function<void (const BarPacket2D&)> onPacket2D;
	};
protected:
	std::shared_ptr<Impl> impl;
public:
	CallbackSet callbacks;
	ProtocolClient(const std::string& portfile,std::ostream& tlog=std::cerr);

	void open(const std::string& port);
	void close();

	operator bool() const;

	bool blockForMessage();
};

}

std::ostream& operator<<(std::ostream&,const ct::Point2D&);
std::ostream& operator<<(std::ostream&,const ct::CameraPacket2D&);
std::ostream& operator<<(std::ostream&,const ct::BarPacket2D&);
#endif
