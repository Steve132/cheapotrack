#ifndef PROTOCOL_SERVER_H
#define PROTOCOL_SERVER_H

#include "ProtocolEnums.h"
#include "Arduino.h"
#include "Point.h"

namespace ct
{

class IntrinsicParameters
{
	float su,sv,tu,tv;
};

class ProtocolServer
{
public:
	bool tracking_active;
	uint16_t framerate_delay_us;
	uint8_t cam_mask;
	uint32_t current_frame;

protected:
	IntrinsicParameters K[4];

	void cmdStopTracking();
	void cmdStartTracking();
	void cmdRead();
	void cmdWrite();
public:
	ProtocolServer():
		tracking_active(false),
		framerate_delay_us(8333),
		cam_mask(0xFF),
		current_frame(0)
	{}

	void onCommandReady();

	void onFrameCapture(const PointGroup* pointgroups,const uint16_t* delays_us,uint8_t num_cams);

	void beginSerialDebug();
	void endSerialDebug();
	void beginSerialError();
	void endSerialError();
};

};

#endif
