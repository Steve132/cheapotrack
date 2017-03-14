#ifndef PROTOCOL_SERVER_H
#define PROTOCOL_SERVER_H

#include "ProtocolEnums.h"

namespace ct
{

class ProtocolServer
{
private:
	bool tracking_active;
public:
	ProtocolServer():
		tracking_active(false)
	{}

	void onCommandReady();

	void beginSerialDebug();
	void endSerialDebug();
	void beginSerialError();
	void endSerialError();
};

};

#endif
